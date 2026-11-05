// snd_openal.cpp -- OpenAL Soft audio backend, replaces sdl_snd.cpp.
//
// The engine keeps mixing into its power-of-two ring exactly like it did
// with DirectSound and SDL: dma.samples interleaved samples with masked
// reads of the play position.  Instead of a device callback the painted
// region is copied into a fixed pool of OpenAL buffers queued on one
// stereo source, and the play position is derived from what OpenAL
// actually consumed, so it never runs ahead of the data.

#include "../../sound/snd_local.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>

// the pool covers the whole 64KB mixing ring in case the mixer ever
// fills it in one go: 16 * 2048 interleaved samples = 32768
#define NUM_AL_BUFFERS      16
#define AL_BUFFER_SAMPLES   2048                // interleaved samples per buffer

static ALCdevice *s_device = NULL;
static ALCcontext *s_context = NULL;
static ALuint s_source = 0;
static ALuint s_buffers[NUM_AL_BUFFERS];
static byte *s_buffer = NULL;               // the mixing ring

static int s_freeBuffers[NUM_AL_BUFFERS];   // indices of buffers ready to fill
static int s_numFree = 0;
static int s_bufSamples[NUM_AL_BUFFERS];    // interleaved samples of each queued buffer

static unsigned long long s_submittedSamples = 0;   // interleaved samples ever queued
static unsigned long long s_playedSamples = 0;      // interleaved samples OpenAL finished

// OpenAL hands buffer names back as ALuints, keep our own index for them
static int BufferIndex( ALuint buf ) {
	int i;

	for ( i = 0; i < NUM_AL_BUFFERS; i++ ) {
		if ( s_buffers[i] == buf ) {
			return i;
		}
	}

	return -1;
}

// move finished buffers back into the free pool
static void DrainProcessed( void ) {
	ALint processed;
	ALuint buf;

	alGetSourcei( s_source, AL_BUFFERS_PROCESSED, &processed );
	while ( processed-- > 0 ) {
		int idx;

		alSourceUnqueueBuffers( s_source, 1, &buf );
		idx = BufferIndex( buf );
		if ( idx >= 0 ) {
			s_playedSamples += s_bufSamples[idx];
			s_freeBuffers[s_numFree++] = idx;
		}
	}
}

/*
==================
SNDDMA_Init

Open the default OpenAL device
Returns false if failed
==================
*/
qboolean SNDDMA_Init( void ) {
	int i;

	Com_Printf( "Initializing OpenAL\n" );

	memset( (void *)&dma, 0, sizeof( dma ) );
	dma.channels = 2;
	dma.samplebits = 16;

	if ( s_khz->integer == 44 ) {
		dma.speed = 44100;
	} else if ( s_khz->integer == 22 ) {
		dma.speed = 22050;
	} else {
		dma.speed = 11025;
	}

	s_device = alcOpenDevice( NULL );
	if ( s_device == NULL ) {
		Com_Printf( "alcOpenDevice failed\n" );
		return qfalse;
	}

	s_context = alcCreateContext( s_device, NULL );
	if ( s_context == NULL || alcMakeContextCurrent( s_context ) == ALC_FALSE ) {
		Com_Printf( "alcCreateContext failed\n" );
		if ( s_context != NULL ) {
			alcDestroyContext( s_context );
			s_context = NULL;
		}
		alcCloseDevice( s_device );
		s_device = NULL;
		return qfalse;
	}

	alGetError();     // clear any error state left from startup

	alGenSources( 1, &s_source );
	alGenBuffers( NUM_AL_BUFFERS, s_buffers );
	if ( alGetError() != AL_NO_ERROR || s_source == 0 ) {
		Com_Printf( "OpenAL: failed to create the playback source\n" );
		SNDDMA_Shutdown();
		return qfalse;
	}

	// keep the ring layout the DirectSound backend used: a 64KB stereo
	// 16-bit buffer = 32768 interleaved samples, which the mixer masks
	// against dma.samples
	dma.samples = 32768;
	dma.submission_chunk = 1;

	s_buffer = (byte *) malloc( dma.samples * dma.samplebits / 8 );
	if ( s_buffer == NULL ) {
		SNDDMA_Shutdown();
		return qfalse;
	}
	memset( s_buffer, 0, dma.samples * dma.samplebits / 8 );
	dma.buffer = s_buffer;

	for ( i = 0; i < NUM_AL_BUFFERS; i++ ) {
		s_freeBuffers[i] = i;
	}
	s_numFree = NUM_AL_BUFFERS;

	s_submittedSamples = 0;
	s_playedSamples = 0;

	Com_DPrintf( "Completed successfully\n" );

	return qtrue;
}

/*
==================
SNDDMA_Shutdown
==================
*/
void SNDDMA_Shutdown( void ) {
	Com_DPrintf( "Shutting down sound system\n" );

	if ( s_source != 0 ) {
		alSourceStop( s_source );
		alDeleteSources( 1, &s_source );
		s_source = 0;
	}

	if ( s_buffers[0] != 0 ) {
		alDeleteBuffers( NUM_AL_BUFFERS, s_buffers );
	}
	memset( s_buffers, 0, sizeof( s_buffers ) );
	s_numFree = 0;

	if ( s_context != NULL ) {
		alcMakeContextCurrent( NULL );
		alcDestroyContext( s_context );
		s_context = NULL;
	}

	if ( s_device != NULL ) {
		alcCloseDevice( s_device );
		s_device = NULL;
	}

	if ( s_buffer != NULL ) {
		free( s_buffer );
		s_buffer = NULL;
	}

	dma.buffer = NULL;
	memset( (void *)&dma, 0, sizeof( dma ) );
	s_submittedSamples = 0;
	s_playedSamples = 0;
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in interleaved samples)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up
==============
*/
int SNDDMA_GetDMAPos( void ) {
	ALint offset;
	unsigned long long played;

	if ( s_source == 0 ) {
		return 0;
	}

	DrainProcessed();

	// sample frames into the currently playing buffer
	alGetSourcei( s_source, AL_SAMPLE_OFFSET, &offset );
	if ( offset < 0 ) {
		offset = 0;
	}

	played = s_playedSamples + (unsigned long long)offset * dma.channels;
	if ( played > s_submittedSamples ) {
		played = s_submittedSamples;
	}

	return (int)( played & ( dma.samples - 1 ) );
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
==============
*/
void SNDDMA_BeginPainting( void ) {
	dma.buffer = s_buffer;
}

/*
==============
SNDDMA_Submit

Queue everything the mixer painted since the last submit into the
OpenAL buffer pool
==============
*/
void SNDDMA_Submit( void ) {
	unsigned long long end;
	unsigned long long start;
	ALint state;
	int bytes;

	if ( s_source == 0 ) {
		return;
	}

	DrainProcessed();

	end = (unsigned long long)s_paintedtime * dma.channels;
	start = s_submittedSamples;
	bytes = dma.samplebits / 8;

	while ( start < end && s_numFree > 0 ) {
		int idx = s_freeBuffers[--s_numFree];
		unsigned long long chunk = end - start;
		int offset = (int)( start & ( dma.samples - 1 ) );
		int samples;

		if ( chunk > AL_BUFFER_SAMPLES ) {
			chunk = AL_BUFFER_SAMPLES;
		}
		// split wraps so a buffer never reads past the end of the ring
		if ( chunk > (unsigned long long)( dma.samples - offset ) ) {
			chunk = dma.samples - offset;
		}

		samples = (int)chunk;
		alBufferData( s_buffers[idx], AL_FORMAT_STEREO16, s_buffer + offset * bytes, samples * bytes, dma.speed );
		alSourceQueueBuffers( s_source, 1, &s_buffers[idx] );
		s_bufSamples[idx] = samples;
		start += chunk;
	}

	// if the pool ever ran dry the data is skipped rather than queued late
	s_submittedSamples = end;

	// keep the source running across underruns and after the first submit
	alGetSourcei( s_source, AL_SOURCE_STATE, &state );
	if ( s_numFree < NUM_AL_BUFFERS && state != AL_PLAYING ) {
		alSourcePlay( s_source );
	}
}

/*
=================
SNDDMA_Activate

window focus changes need no special handling with OpenAL
=================
*/
void SNDDMA_Activate( void ) {
}
