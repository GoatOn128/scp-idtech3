// sdl_glimp.cpp -- SDL3 OpenGL backend, replaces win_glimp.cpp and
// win_gamma.cpp.  This file contains ALL platform specific stuff having
// to do with the OpenGL refresh:
//
// GLimp_EndFrame
// GLimp_Init
// GLimp_LogComment
// GLimp_Shutdown
// GLimp_SetGamma

#include <assert.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#include "../../renderer/tr_local.h"
#include "../../qcommon/qcommon.h"

// the one and only SDL window, also used by the input and main modules
SDL_Window *sdl_window = NULL;

// glad loader context, the qgl.h macros call through this
GladGLContext glimpGL;

static SDL_GLContext s_glContext = NULL;
static FILE *s_logFP = NULL;
static cvar_t *r_allowSoftwareGL = NULL;

// SDL3 removed the window gamma ramp API: no ramp is ever saved or restored

typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_UNKNOWN
} rserr_t;

static rserr_t GLW_SetMode( int mode, int colorbits, qboolean cdsFullscreen );
static qboolean GLW_StartDriverAndSetMode( int mode, int colorbits, qboolean cdsFullscreen );
static void GLW_InitExtensions( void );

/*
** GLW_CheckHardwareGamma
*/
static void GLW_CheckHardwareGamma( void ) {
	// SDL3 removed the window gamma ramp API: changing gamma now needs a
	// shader pass over the finished frame. Until one is added the renderer
	// runs without hardware gamma support
	glConfig.deviceSupportsGamma = qfalse;
}

static void GLW_RestoreGamma( void ) {
	// nothing to restore: GLW_CheckHardwareGamma never saves a ramp
}

/*
** GLimp_SetGamma
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
	// SDL3 removed the window gamma ramp API and the SDL backend reports
	// no hardware gamma support, so there is nothing to set here
	(void)red;
	(void)green;
	(void)blue;
}

/*
** GLW_SetMode
*/
static rserr_t GLW_SetMode( int mode, int colorbits, qboolean cdsFullscreen ) {
	const char *win_fs[] = { "w", "fullscreen" };
	int depthbits;
	int stencilbits;
	int stereo;
	int r, g, b, a, d, s, st;
	SDL_GLContext glctx;
	SDL_DisplayID display;
	const SDL_DisplayMode *displayMode;

	ri.Printf( PRINT_ALL, "...setting mode %d:", mode );

	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode ) ) {
		ri.Printf( PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	ri.Printf( PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, win_fs[cdsFullscreen ? 1 : 0] );

	// create or reuse the window
	if ( sdl_window == NULL ) {
		SDL_WindowFlags flags = SDL_WINDOW_OPENGL;

		if ( cdsFullscreen ) {
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		sdl_window = SDL_CreateWindow( "WolfSP", glConfig.vidWidth, glConfig.vidHeight, flags );
		if ( sdl_window == NULL ) {
			ri.Printf( PRINT_ALL, "SDL_CreateWindow failed: %s\n", SDL_GetError() );
			return RSERR_INVALID_MODE;
		}

		// SDL3 dropped the window position from SDL_CreateWindow: place the
		// windowed game window where the vid_xpos/vid_ypos cvars say
		if ( !cdsFullscreen ) {
			cvar_t *vid_xpos = ri.Cvar_Get( "vid_xpos", "3", CVAR_ARCHIVE );
			cvar_t *vid_ypos = ri.Cvar_Get( "vid_ypos", "22", CVAR_ARCHIVE );

			SDL_SetWindowPosition( sdl_window, vid_xpos->integer, vid_ypos->integer );
		}

		// we want text input events for the console
		SDL_StartTextInput( sdl_window );
	} else {
		SDL_SetWindowFullscreen( sdl_window, cdsFullscreen ? true : false );
		SDL_SetWindowSize( sdl_window, glConfig.vidWidth, glConfig.vidHeight );
	}

	// pick the framebuffer layout
	if ( colorbits == 0 ) {
		colorbits = 24;
	}

	depthbits = r_depthbits->integer;
	if ( depthbits == 0 ) {
		depthbits = ( colorbits > 16 ) ? 24 : 16;
	}

	stencilbits = r_stencilbits->integer;
	if ( depthbits < 24 ) {
		stencilbits = 0;
	}

	stereo = r_stereo->integer;

	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, ( colorbits > 16 ) ? 8 : 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, ( colorbits > 16 ) ? 8 : 6 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, ( colorbits > 16 ) ? 8 : 5 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, depthbits );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, stencilbits );
	SDL_GL_SetAttribute( SDL_GL_STEREO, stereo );

	glctx = SDL_GL_CreateContext( sdl_window );
	if ( glctx == NULL ) {
		ri.Printf( PRINT_ALL, "SDL_GL_CreateContext failed: %s\n", SDL_GetError() );
		return RSERR_INVALID_MODE;
	}
	s_glContext = glctx;

	// load the entry points through glad
	if ( !gladLoaderLoadGLContext( &glimpGL ) ) {
		ri.Printf( PRINT_ALL, "gladLoaderLoadGLContext failed\n" );
		return RSERR_INVALID_MODE;
	}

	// read back what we actually got
	SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &r );
	SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &g );
	SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &b );
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &a );
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &d );
	SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &s );
	SDL_GL_GetAttribute( SDL_GL_STEREO, &st );

	glConfig.colorBits = r + g + b + a;
	glConfig.depthBits = d;
	glConfig.stencilBits = s;
	glConfig.stereoEnabled = ( st && stereo ) ? qtrue : qfalse;
	glConfig.isFullscreen = cdsFullscreen;

	// report the current display refresh rate
	glConfig.displayFrequency = 0;
	display = SDL_GetDisplayForWindow( sdl_window );
	if ( display != 0 ) {
		displayMode = SDL_GetCurrentDisplayMode( display );
		if ( displayMode != NULL ) {
			glConfig.displayFrequency = (int)displayMode->refresh_rate;
		}
	}

	return RSERR_OK;
}

/*
** GLW_StartDriverAndSetMode
*/
static qboolean GLW_StartDriverAndSetMode( int mode, int colorbits, qboolean cdsFullscreen ) {
	rserr_t err;

	err = GLW_SetMode( mode, colorbits, cdsFullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}

	glConfig.driverType = GLDRV_ICD;

	return qtrue;
}

/**
** GLW_LoadOpenGL
**
** Creates the window and GL context through the system driver.
*/
static qboolean GLW_LoadOpenGL( void ) {
	qboolean cdsFullscreen;

	glConfig.driverType = GLDRV_ICD;

	cdsFullscreen = r_fullscreen->integer;
	if ( !GLW_StartDriverAndSetMode( r_mode->integer, r_colorbits->integer, cdsFullscreen ) ) {
		// if we're on a 24/32-bit desktop and going fullscreen, try again with a 16-bit desktop
		if ( r_colorbits->integer != 16 ||
			 cdsFullscreen != qtrue ||
			 r_mode->integer != 3 ) {
			if ( !GLW_StartDriverAndSetMode( 3, 16, qtrue ) ) {
				return qfalse;
			}
		} else
		{
			return qfalse;
		}
	}

	return qtrue;
}

/*
** GLimp_EndFrame
*/
void GLimp_EndFrame( void ) {
	//
	// swapinterval stuff
	//
	if ( r_swapInterval->modified ) {
		r_swapInterval->modified = qfalse;

		if ( !glConfig.stereoEnabled ) {    // why?
			SDL_GL_SetSwapInterval( r_swapInterval->integer );
		}
	}

	if ( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 ) {
		SDL_GL_SwapWindow( sdl_window );
	}
}

/*
** GLW_InitExtensions
*/
static void GLW_InitExtensions( void ) {
	//----(SA)	moved these up
	glConfig.textureCompression = TC_NONE;
	glConfig.textureEnvAddAvailable = qfalse;
	glimpGL.MultiTexCoord2f = NULL;
	glimpGL.ActiveTexture = NULL;
	glimpGL.ClientActiveTexture = NULL;
	glimpGL.LockArraysEXT = NULL;
	glimpGL.UnlockArraysEXT = NULL;
	glimpGL.PNTrianglesiATI = NULL;
	glimpGL.PNTrianglesfATI = NULL;
	glConfig.anisotropicAvailable = qfalse;
	glConfig.NVFogAvailable = qfalse;
	glConfig.NVFogMode = 0;
	//----(SA)	end

	if ( !r_allowExtensions->integer ) {
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	// GL_S3_s3tc
	// RF, check for GL_EXT_texture_compression_s3tc
	if ( strstr( glConfig.extensions_string, "GL_EXT_texture_compression_s3tc" ) ) {
		if ( r_ext_compressed_textures->integer ) {
			glConfig.textureCompression = TC_EXT_COMP_S3TC;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n" );
		} else
		{
			glConfig.textureCompression = TC_NONE;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_compression_s3tc\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_compression_s3tc not found\n" );
	}

	// GL_EXT_texture_env_add
	if ( strstr( glConfig.extensions_string, "EXT_texture_env_add" ) ) {
		if ( r_ext_texture_env_add->integer ) {
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		} else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	// swap control is handled through SDL on every platform now
	ri.Printf( PRINT_ALL, "...using SDL swap control\n" );
	r_swapInterval->modified = qtrue;   // force a set next frame

	// GL_ARB_multitexture
	if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" ) ) {
		if ( r_ext_multitexture->integer ) {
			if ( qglActiveTextureARB ) {
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );

				if ( glConfig.maxActiveTextures > 1 ) {
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				} else
				{
					glimpGL.MultiTexCoord2f = NULL;
					glimpGL.ActiveTexture = NULL;
					glimpGL.ClientActiveTexture = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) ) {
		if ( r_ext_compiled_vertex_array->integer ) {
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			if ( !qglLockArraysEXT || !qglUnlockArraysEXT ) {
				ri.Error( ERR_FATAL, "GL_EXT_compiled_vertex_array entry points missing\n" );
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

//----(SA)	added

	// GL_ATI_pn_triangles - ATI PN-Triangles
	if ( strstr( glConfig.extensions_string, "GL_ATI_pn_triangles" ) ) {
		if ( r_ext_ATI_pntriangles->integer ) {
			ri.Printf( PRINT_ALL, "...using GL_ATI_pn_triangles\n" );

			if ( !qglPNTrianglesiATI || !qglPNTrianglesfATI ) {
				ri.Error( ERR_FATAL, "GL_ATI_pn_triangles entry points missing\n" );
			}
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_ATI_pn_triangles\n" );
			ri.Cvar_Set( "r_ext_ATI_pntriangles", "0" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_ATI_pn_triangles not found\n" );
		ri.Cvar_Set( "r_ext_ATI_pntriangles", "0" );
	}

	// GL_EXT_texture_filter_anisotropic
	if ( strstr( glConfig.extensions_string, "GL_EXT_texture_filter_anisotropic" ) ) {
		if ( r_ext_texture_filter_anisotropic->integer ) {
			// always ignored.  unsupported.
			glConfig.anisotropicAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n" );
			ri.Cvar_Set( "r_ext_texture_filter_anisotropic", "0" );
		} else {
			glConfig.anisotropicAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n" );
			ri.Cvar_Set( "r_ext_texture_filter_anisotropic", "0" );
		}
	} else {
		ri.Cvar_Set( "r_ext_texture_filter_anisotropic", "0" );
	}

	// GL_NV_fog_distance
	if ( strstr( glConfig.extensions_string, "GL_NV_fog_distance" ) ) {
		if ( r_ext_NV_fog_dist->integer ) {
			glConfig.NVFogAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_NV_fog_distance\n" );
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_NV_fog_distance\n" );
			ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
		}
	} else {
		ri.Printf( PRINT_ALL, "...GL_NV_fog_distance not found\n" );
		ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
	}

//----(SA)	end
}

/*
** GLimp_Init
**
** This is the platform specific OpenGL initialization function.  It
** is responsible for loading OpenGL, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional OpenGL subsystem is operating
** when it returns to the ref.
*/
void GLimp_Init( void ) {
	char buf[1024];
	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );

	ri.Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );

	r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

	// load appropriate library and initialize subsystem
	if ( !GLW_LoadOpenGL() ) {
		ri.Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem\n" );
	}

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, (const char *)qglGetString( GL_VENDOR ), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, (const char *)qglGetString( GL_RENDERER ), sizeof( glConfig.renderer_string ) );
	Q_strncpyz( glConfig.version_string, (const char *)qglGetString( GL_VERSION ), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, (const char *)qglGetString( GL_EXTENSIONS ), sizeof( glConfig.extensions_string ) );

	//
	// chipset specific configuration
	//
	Q_strncpyz( buf, glConfig.renderer_string, sizeof( buf ) );
	Q_strlwr( buf );

	// all of the legacy per-chipset workarounds (Voodoo, Rage Pro, Riva,
	// Permedia, ...) are gone, modern drivers only need generic handling
	glConfig.hardwareType = GLHW_GENERIC;

	// reset the default texture filtering when the renderer changes so new
	// hardware starts from a known state
	if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) ) {
		ri.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
	}

	// Mesa-based drivers behave differently from the native vendor drivers,
	// note them in the log
	if ( strstr( buf, "mesa" ) || strstr( buf, "gallium" ) || strstr( buf, "llvmpipe" ) ) {
		ri.Printf( PRINT_ALL, "...Mesa-based OpenGL driver detected\n" );
	}

	if ( strstr( buf, "geforce" ) || strstr( buf, "ge-force" ) || strstr( buf, "radeon" ) || strstr( buf, "nv20" ) || strstr( buf, "nv30" )
		 || strstr( buf, "quadro" ) ) {
		ri.Cvar_Set( "r_highQualityVideo", "1" );
	} else {
		ri.Cvar_Set( "r_highQualityVideo", "0" );
	}

	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	GLW_InitExtensions();
	GLW_CheckHardwareGamma();
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.
*/
void GLimp_Shutdown( void ) {
	ri.Printf( PRINT_ALL, "Shutting down OpenGL subsystem\n" );

	// restore gamma first, while the window still exists
	GLW_RestoreGamma();

	if ( s_glContext != NULL ) {
		ri.Printf( PRINT_ALL, "...destroying GL context\n" );
		SDL_GL_DestroyContext( s_glContext );
		s_glContext = NULL;
	}

	if ( sdl_window != NULL ) {
		ri.Printf( PRINT_ALL, "...destroying window\n" );
		SDL_StopTextInput( sdl_window );
		SDL_DestroyWindow( sdl_window );
		sdl_window = NULL;
	}

	// close the r_logFile
	if ( s_logFP != NULL ) {
		fclose( s_logFP );
		s_logFP = NULL;
	}

	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment( char *comment ) {
	if ( s_logFP != NULL ) {
		fprintf( s_logFP, "%s", comment );
	}
}

/*
===========================================================

SMP acceleration

===========================================================
*/

// The SDL port does not spawn a render thread.  R_InitCommandBuffers
// handles a false return by reporting SMP as unavailable and the
// renderer runs single threaded.
qboolean GLimp_SpawnRenderThread( void ( *function )( void ) ) {
	(void)function;
	ri.Printf( PRINT_ALL, "...SMP not supported with the SDL backend\n" );
	return qfalse;
}

void *GLimp_RendererSleep( void ) {
	return NULL;
}

void GLimp_FrontEndSleep( void ) {
}

void GLimp_WakeRenderer( void *data ) {
	(void)data;
}
