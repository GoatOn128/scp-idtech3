// sdl_snd.cpp -- SDL3 audio backend, replaces win_snd.cpp (DirectSound).
//
// The engine keeps mixing into a power-of-two ring exactly like it did
// with DirectSound: dma.samples interleaved samples with masked reads of
// the play position.  Instead of a hardware secondary buffer the painted
// region is queued into an SDL_AudioStream and the play position is
// derived from the wall clock, never claiming more than was queued.

#include "../../sound/snd_local.h"

#include <stdlib.h>

static SDL_AudioStream *s_stream = NULL;
static byte *s_buffer = NULL;
static Uint64 s_startTime = 0;          // SDL_GetTicks() when playback started
static Uint64 s_submittedSamples = 0;   // interleaved samples ever queued

/*
==================
SNDDMA_Init

Initialize the SDL audio device
Returns false if failed
==================
*/
qboolean SNDDMA_Init( void ) {
	SDL_AudioSpec desired;

	Com_Printf( "Initializing SDL audio\n" );

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

	memset( &desired, 0, sizeof( desired ) );
	desired.format = SDL_AUDIO_S16;
	desired.channels = 2;
	desired.freq = dma.speed;

	s_stream = SDL_OpenAudioDeviceStream( SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired, NULL, NULL );
	if ( s_stream == NULL ) {
		Com_Printf( "SDL_OpenAudioDeviceStream failed: %s\n", SDL_GetError() );
		return qfalse;
	}

	// keep the ring layout the DirectSound backend used: a 64KB stereo
	// 16-bit buffer = 32768 interleaved samples, which the mixer masks
	// against dma.samples
	dma.samples = 32768;
	dma.submission_chunk = 1;

	s_buffer = (byte *) malloc( dma.samples * dma.samplebits / 8 );
	if ( s_buffer == NULL ) {
		SDL_DestroyAudioStream( s_stream );
		s_stream = NULL;
		memset( (void *)&dma, 0, sizeof( dma ) );
		return qfalse;
	}
	memset( s_buffer, 0, dma.samples * dma.samplebits / 8 );
	dma.buffer = s_buffer;

	s_submittedSamples = 0;
	s_startTime = SDL_GetTicks();

	SDL_ResumeAudioStreamDevice( s_stream );

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

	if ( s_stream != NULL ) {
		SDL_DestroyAudioStream( s_stream );
		s_stream = NULL;
	}

	if ( s_buffer != NULL ) {
		free( s_buffer );
		s_buffer = NULL;
	}

	dma.buffer = NULL;
	memset( (void *)&dma, 0, sizeof( dma ) );
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in interleaved samples)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up
===============
*/
int SNDDMA_GetDMAPos( void ) {
	Uint64 played;

	if ( s_stream == NULL ) {
		return 0;
	}

	// wall-clock estimate of how many interleaved samples have been
	// consumed, never claiming more than we actually queued
	played = ( SDL_GetTicks() - s_startTime ) * dma.speed / 1000 * dma.channels;
	if ( played > s_submittedSamples ) {
		played = s_submittedSamples;
	}

	return (int)( played & ( dma.samples - 1 ) );
}

/*
==============
SNDDMA_BeginPainting

Makes sure dma.buffer is valid
===============
*/
void SNDDMA_BeginPainting( void ) {
	dma.buffer = s_buffer;
}

/*
==============
SNDDMA_Submit

Queue everything the mixer painted since the last submit into the
audio stream, in chunks that never wrap the ring
===============
*/
void SNDDMA_Submit( void ) {
	Uint64 start;
	Uint64 end;
	int bytes;

	if ( s_stream == NULL ) {
		return;
	}

	end = (Uint64)s_paintedtime * dma.channels;
	start = s_submittedSamples;
	bytes = dma.samplebits / 8;

	while ( start < end ) {
		Uint64 chunk = end - start;
		int offset = (int)( start & ( dma.samples - 1 ) );

		if ( chunk > (Uint64)( dma.samples - offset ) ) {
			chunk = dma.samples - offset;
		}

		SDL_PutAudioStreamData( s_stream, s_buffer + offset * bytes, (int)chunk * bytes );
		start += chunk;
	}

	s_submittedSamples = end;
}

/*
=================
SNDDMA_Activate

When we change windows we need to do this
=================
*/
void SNDDMA_Activate( void ) {
}
