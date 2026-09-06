/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/


#include "tr_local.h"
#include "tr_renderprog.h"
#include <cmath>

// OpenGL 2.0 entry points are private to this module; the existing renderer
// continues to use its fixed-function dispatch table.
#define RP_PROC( result, name, args ) static result ( APIENTRY * rp##name ) args
RP_PROC( GLuint, CreateShader, ( GLenum ) );
RP_PROC( void, ShaderSource, ( GLuint, GLsizei, const char *const *, const GLint * ) );
RP_PROC( void, CompileShader, ( GLuint ) );
RP_PROC( void, GetShaderiv, ( GLuint, GLenum, GLint * ) );
RP_PROC( void, GetShaderInfoLog, ( GLuint, GLsizei, GLsizei *, char * ) );
RP_PROC( void, DeleteShader, ( GLuint ) );
RP_PROC( GLuint, CreateProgram, ( void ) );
RP_PROC( void, AttachShader, ( GLuint, GLuint ) );
RP_PROC( void, LinkProgram, ( GLuint ) );
RP_PROC( void, GetProgramiv, ( GLuint, GLenum, GLint * ) );
RP_PROC( void, GetProgramInfoLog, ( GLuint, GLsizei, GLsizei *, char * ) );
RP_PROC( void, DeleteProgram, ( GLuint ) );
RP_PROC( void, UseProgram, ( GLuint ) );
RP_PROC( GLint, GetUniformLocation, ( GLuint, const char * ) );
RP_PROC( void, Uniform1i, ( GLint, GLint ) );
RP_PROC( void, Uniform1f, ( GLint, GLfloat ) );
RP_PROC( void, Uniform2f, ( GLint, GLfloat, GLfloat ) );
RP_PROC( void, Uniform3fv, ( GLint, GLsizei, const GLfloat * ) );
RP_PROC( void, Uniform4f, ( GLint, GLfloat, GLfloat, GLfloat, GLfloat ) );
RP_PROC( void, UniformMatrix4fv, ( GLint, GLsizei, GLboolean, const GLfloat * ) );
#undef RP_PROC

static const GLenum RP_VERTEX_SHADER = 0x8B31;
static const GLenum RP_FRAGMENT_SHADER = 0x8B30;
static const GLenum RP_COMPILE_STATUS = 0x8B81;
static const GLenum RP_LINK_STATUS = 0x8B82;
static const GLenum RP_CURRENT_PROGRAM = 0x8B8D;
static const GLenum RP_CLAMP_TO_EDGE = 0x812F;
static const GLenum RP_DEPTH_COMPONENT24 = 0x81A6;

struct screenProgramData_t {
	GLuint handle;
	GLint image, depth, viewport, depthSize, inverseViewProjection;
	GLint origin, right, up, normal, softness, projection, viewOrigin, blendMode;
};
static screenProgramData_t screenPrograms[SCREEN_PROGRAM_COUNT];
static GLuint screenDepthTexture;
static int screenDepthWidth, screenDepthHeight;
static qboolean screenProgramsTried, screenProgramsReady, screenDepthReady;
static screenProgramData_t *screenProgram;
static glstate_t savedScreenState;
static GLint savedScreenProgram, savedMatrixMode;

static void *R_RenderProcAddress( const char *name ) {
#ifdef _WIN32
	void *address = reinterpret_cast<void *>( qwglGetProcAddress( name ) );
	if ( address == reinterpret_cast<void *>( 1 ) || address == reinterpret_cast<void *>( 2 ) ||
		 address == reinterpret_cast<void *>( 3 ) || address == reinterpret_cast<void *>( -1 ) ) {
		return NULL;
	}
	return address;
#else
	return NULL;
#endif
}

// Optional driver diagnostics identify the offending GL call without consuming
// errors or weakening r_ignoreGLErrors. Install only on contexts advertising it.
static void APIENTRY R_RenderProgramDebugMessage( GLenum, GLenum type, GLuint id,
		GLenum, GLsizei, const char *message, const void * ) {
	if ( type == 0x824C ) { // GL_DEBUG_TYPE_ERROR
		ri.Printf( PRINT_WARNING, "OpenGL driver error %u: %s\n", id, message );
	}
}

void R_InitRenderProgramDiagnostics( void ) {
	cvar_t *debug = ri.Cvar_Get( "r_glDebug", "0", 0 );
	if ( !debug->integer ) { return; }
	const char *extensions = reinterpret_cast<const char *>( qglGetString( GL_EXTENSIONS ) );
	if ( !extensions || !strstr( extensions, "GL_ARB_debug_output" ) ) { return; }
	typedef void ( APIENTRY *debugCallback_t )( decltype( &R_RenderProgramDebugMessage ), const void * );
	debugCallback_t callback = reinterpret_cast<debugCallback_t>( R_RenderProcAddress( "glDebugMessageCallbackARB" ) );
	if ( !callback ) { return; }
	callback( R_RenderProgramDebugMessage, NULL );
	qglEnable( 0x8242 ); // GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB
}

/*
====================
R_CompileScreenShader
====================
*/
static GLuint R_CompileScreenShader( GLenum type, const char *path ) {
	void *source = NULL;
	int length = ri.FS_ReadFile( path, &source );
	if ( length <= 0 || !source ) {
		if ( source ) { ri.FS_FreeFile( source ); }
		ri.Printf( PRINT_WARNING, "Screen FX: could not read %s\n", path );
		return 0;
	}
	GLuint shader = rpCreateShader( type );
	const char *text = static_cast<const char *>( source );
	rpShaderSource( shader, 1, &text, &length );
	rpCompileShader( shader );
	ri.FS_FreeFile( source );
	GLint compiled = 0;
	rpGetShaderiv( shader, RP_COMPILE_STATUS, &compiled );
	if ( !compiled ) {
		char log[4096] = { 0 };
		rpGetShaderInfoLog( shader, sizeof( log ) - 1, NULL, log );
		ri.Printf( PRINT_WARNING, "Screen FX: %s: %s\n", path, log );
		rpDeleteShader( shader );
		return 0;
	}
	return shader;
}

static qboolean R_LoadScreenPrograms( void ) {
	if ( atoi( reinterpret_cast<const char *>( qglGetString( GL_VERSION ) ) ) < 2 ) { return qfalse; }
	if ( !qglActiveTextureARB || glConfig.maxActiveTextures < 2 ) { return qfalse; }
#define RP_LOAD( name ) rp##name = reinterpret_cast<decltype( rp##name )>( R_RenderProcAddress( "gl" #name ) ); if ( !rp##name ) { return qfalse; }
	RP_LOAD( CreateShader ); RP_LOAD( ShaderSource ); RP_LOAD( CompileShader );
	RP_LOAD( GetShaderiv ); RP_LOAD( GetShaderInfoLog ); RP_LOAD( DeleteShader );
	RP_LOAD( CreateProgram ); RP_LOAD( AttachShader ); RP_LOAD( LinkProgram );
	RP_LOAD( GetProgramiv ); RP_LOAD( GetProgramInfoLog ); RP_LOAD( DeleteProgram );
	RP_LOAD( UseProgram ); RP_LOAD( GetUniformLocation ); RP_LOAD( Uniform1i );
	RP_LOAD( Uniform1f ); RP_LOAD( Uniform2f ); RP_LOAD( Uniform3fv );
	RP_LOAD( Uniform4f ); RP_LOAD( UniformMatrix4fv );
#undef RP_LOAD
	const char *names[SCREEN_PROGRAM_COUNT] = { "particles", "decals" };
	for ( int i = 0; i < SCREEN_PROGRAM_COUNT; i++ ) {
		GLuint vertex = R_CompileScreenShader( RP_VERTEX_SHADER, va( "renderprogs/%s.vert", names[i] ) );
		GLuint fragment = R_CompileScreenShader( RP_FRAGMENT_SHADER, va( "renderprogs/%s.frag", names[i] ) );
		if ( !vertex || !fragment ) {
			if ( vertex ) { rpDeleteShader( vertex ); }
			if ( fragment ) { rpDeleteShader( fragment ); }
			return qfalse;
		}
		screenProgramData_t *program = &screenPrograms[i];
		program->handle = rpCreateProgram();
		rpAttachShader( program->handle, vertex );
		rpAttachShader( program->handle, fragment );
		rpLinkProgram( program->handle );
		rpDeleteShader( vertex );
		rpDeleteShader( fragment );
		GLint linked = 0;
		rpGetProgramiv( program->handle, RP_LINK_STATUS, &linked );
		if ( !linked ) {
			char log[4096] = { 0 };
			rpGetProgramInfoLog( program->handle, sizeof( log ) - 1, NULL, log );
			ri.Printf( PRINT_WARNING, "Screen FX: %s link: %s\n", names[i], log );
			return qfalse;
		}
#define RP_UNIFORM( field, name ) program->field = rpGetUniformLocation( program->handle, name )
		RP_UNIFORM( blendMode, "u_BlendMode" );
		RP_UNIFORM( image, "u_Image" ); RP_UNIFORM( depth, "u_Depth" );
		RP_UNIFORM( viewport, "u_Viewport" ); RP_UNIFORM( depthSize, "u_DepthSize" );
		RP_UNIFORM( inverseViewProjection, "u_InverseViewProjection" );
		RP_UNIFORM( viewOrigin, "u_ViewOrigin" );
		RP_UNIFORM( origin, "u_Origin" ); RP_UNIFORM( right, "u_Right" );
		RP_UNIFORM( up, "u_Up" ); RP_UNIFORM( normal, "u_Normal" );
		RP_UNIFORM( softness, "u_Softness" );
		RP_UNIFORM( projection, "u_Projection" );
#undef RP_UNIFORM
	}
	ri.Printf( PRINT_ALL, "Screen FX: particle and decal GLSL programs loaded\n" );
	return qtrue;
}

static qboolean R_InitScreenPrograms( void ) {
	if ( screenProgramsTried ) { return screenProgramsReady; }
	screenProgramsTried = qtrue;
	screenProgramsReady = R_LoadScreenPrograms();
	if ( !screenProgramsReady ) {
		for ( int i = 0; i < SCREEN_PROGRAM_COUNT; i++ ) {
			if ( screenPrograms[i].handle ) { rpDeleteProgram( screenPrograms[i].handle ); screenPrograms[i].handle = 0; }
		}
		ri.Printf( PRINT_WARNING, "Screen FX disabled: OpenGL 2.0, multitexture and valid renderprogs are required.\n" );
	}
	return screenProgramsReady;
}

void R_ShutdownRenderPrograms( void ) {
	for ( int i = 0; i < SCREEN_PROGRAM_COUNT; i++ ) {
		if ( screenPrograms[i].handle ) { rpDeleteProgram( screenPrograms[i].handle ); }
	}
	memset( screenPrograms, 0, sizeof( screenPrograms ) );
	if ( screenDepthTexture ) { qglDeleteTextures( 1, &screenDepthTexture ); }
	screenDepthTexture = 0;
	screenDepthWidth = screenDepthHeight = 0;
	screenProgramsTried = screenProgramsReady = screenDepthReady = qfalse;
}

qboolean RB_ScreenEntityVisible( const refEntity_t *entity ) {
	if ( !r_drawentities->integer || ( backEnd.refdef.rdflags & RDF_NOWORLDMODEL ) ) { return qfalse; }
	if ( ( entity->renderfx & RF_FIRST_PERSON ) && backEnd.viewParms.isPortal ) { return qfalse; }
	if ( ( entity->renderfx & RF_THIRD_PERSON ) && !backEnd.viewParms.isPortal ) { return qfalse; }
	if ( !std::isfinite( entity->radius ) || entity->radius <= 0 || !entity->shaderRGBA[3] ) { return qfalse; }
	for ( int i = 0; i < 3; i++ ) {
		if ( !std::isfinite( entity->origin[i] ) ) { return qfalse; }
	}
	return qtrue;
}

// Invert projection * world view with pivoting, including mirror transforms.
static qboolean R_InverseScreenMatrix( float *inverse ) {
	double rows[4][8] = {};
	const float *projection = backEnd.viewParms.projectionMatrix;
	const float *view = backEnd.viewParms.world.modelMatrix;
	for ( int r = 0; r < 4; r++ ) {
		for ( int c = 0; c < 4; c++ ) {
			for ( int k = 0; k < 4; k++ ) { rows[r][c] += projection[k * 4 + r] * view[c * 4 + k]; }
		}
		rows[r][r + 4] = 1;
	}
	for ( int c = 0; c < 4; c++ ) {
		int pivot = c;
		for ( int r = c + 1; r < 4; r++ ) { if ( fabs( rows[r][c] ) > fabs( rows[pivot][c] ) ) { pivot = r; } }
		if ( fabs( rows[pivot][c] ) < 1e-12 ) { return qfalse; }
		for ( int k = 0; k < 8; k++ ) { double swap = rows[c][k]; rows[c][k] = rows[pivot][k]; rows[pivot][k] = swap; }
		double scale = rows[c][c];
		for ( int k = 0; k < 8; k++ ) { rows[c][k] /= scale; }
		for ( int r = 0; r < 4; r++ ) {
			if ( r == c ) { continue; }
			double factor = rows[r][c];
			for ( int k = 0; k < 8; k++ ) { rows[r][k] -= factor * rows[c][k]; }
		}
	}
	for ( int r = 0; r < 4; r++ ) {
		for ( int c = 0; c < 4; c++ ) { inverse[c * 4 + r] = static_cast<float>( rows[r][c + 4] ); }
	}
	return qtrue;
}

void RB_CaptureScreenDepth( void ) {
	screenDepthReady = qfalse;
	qboolean needed = qfalse;
	for ( int i = 0; i < backEnd.refdef.num_entities; i++ ) {
		const refEntity_t *entity = &backEnd.refdef.entities[i].e;
		if ( ( entity->reType == RT_SCREEN_PARTICLE || entity->reType == RT_SCREEN_DECAL ) && RB_ScreenEntityVisible( entity ) ) { needed = qtrue; break; }
	}
	if ( !needed || !R_InitScreenPrograms() ) { return; }
	const viewParms_t *view = &backEnd.viewParms;
	if ( view->viewportWidth <= 0 || view->viewportHeight <= 0 ) { return; }
	int width = 1, height = 1;
	while ( width < view->viewportWidth ) { width *= 2; }
	while ( height < view->viewportHeight ) { height *= 2; }
	if ( width > glConfig.maxTextureSize || height > glConfig.maxTextureSize ) { return; }
	glstate_t saved = glState;
	qglPushAttrib( GL_TEXTURE_BIT );
	GL_SelectTexture( 1 );
	if ( !screenDepthTexture ) { qglGenTextures( 1, &screenDepthTexture ); }
	qglBindTexture( GL_TEXTURE_2D, screenDepthTexture );
	if ( width != screenDepthWidth || height != screenDepthHeight ) {
		qglTexImage2D( GL_TEXTURE_2D, 0, RP_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, RP_CLAMP_TO_EDGE );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, RP_CLAMP_TO_EDGE );
		screenDepthWidth = width; screenDepthHeight = height;
	}
	qglCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, view->viewportX, view->viewportY, view->viewportWidth, view->viewportHeight );
	qglPopAttrib();
	GL_SelectTexture( saved.currenttmu );
	glState = saved;
	screenDepthReady = qtrue;
}

qboolean RB_BeginScreenPass( screenProgram_t kind ) {
	if ( !screenDepthReady ) { return qfalse; }
	float inverse[16];
	if ( !R_InverseScreenMatrix( inverse ) ) { return qfalse; }
	savedScreenState = glState;
	qglGetIntegerv( RP_CURRENT_PROGRAM, &savedScreenProgram );
	qglGetIntegerv( GL_MATRIX_MODE, &savedMatrixMode );
	qglPushAttrib( GL_ALL_ATTRIB_BITS );
	qglMatrixMode( GL_PROJECTION ); qglPushMatrix(); qglLoadMatrixf( backEnd.viewParms.projectionMatrix );
	qglMatrixMode( GL_MODELVIEW ); qglPushMatrix(); qglLoadMatrixf( backEnd.viewParms.world.modelMatrix );
	qglDepthRange( 0, 1 );
	qglDisable( GL_FOG ); qglDisable( GL_ALPHA_TEST ); qglDisable( GL_CULL_FACE );
	qglDisable( GL_STENCIL_TEST );
	if ( kind == SCREEN_DECAL_PROGRAM ) { qglDisable( GL_CLIP_PLANE0 ); }
	qglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	qglDepthMask( GL_FALSE );
	qglDisable( GL_DEPTH_TEST );
	qglEnable( GL_BLEND );
	GL_SelectTexture( 1 );
	qglBindTexture( GL_TEXTURE_2D, screenDepthTexture );
	GL_SelectTexture( 0 );
	screenProgram = &screenPrograms[kind];
	rpUseProgram( screenProgram->handle );
	rpUniform1i( screenProgram->image, 0 ); rpUniform1i( screenProgram->depth, 1 );
	const viewParms_t *view = &backEnd.viewParms;
	rpUniform4f( screenProgram->viewport, view->viewportX, view->viewportY, view->viewportWidth, view->viewportHeight );
	rpUniform2f( screenProgram->depthSize, screenDepthWidth, screenDepthHeight );
	rpUniformMatrix4fv( screenProgram->inverseViewProjection, 1, GL_FALSE, inverse );
	rpUniform3fv( screenProgram->viewOrigin, 1, view->or.origin );
	rpUniform2f( screenProgram->projection, view->projectionMatrix[10], view->projectionMatrix[14] );
	return qtrue;
}

void RB_EndScreenPass( void ) {
	rpUseProgram( savedScreenProgram );
	qglMatrixMode( GL_MODELVIEW ); qglPopMatrix();
	qglMatrixMode( GL_PROJECTION ); qglPopMatrix();
	qglMatrixMode( savedMatrixMode );
	qglPopAttrib();
	GL_SelectTexture( savedScreenState.currenttmu );
	glState = savedScreenState;
	screenProgram = NULL;
}

void RB_ScreenSoftness( float value ) {
	rpUniform1f( screenProgram->softness, value );
}

void RB_ScreenDecalProjection( const vec3_t origin, const vec3_t right, const vec3_t up, const vec3_t normal ) {
	rpUniform3fv( screenProgram->origin, 1, origin );
	rpUniform3fv( screenProgram->right, 1, right );
	rpUniform3fv( screenProgram->up, 1, up );
	rpUniform3fv( screenProgram->normal, 1, normal );
}

void RB_BindScreenMaterial( const refEntity_t *entity ) {
	shader_t *shader = R_GetShaderByHandle( entity->customShader );
	if ( shader->remappedShader ) { shader = shader->remappedShader; }
	shaderStage_t *stage = shader->stages[0];
	image_t *image = tr.defaultImage;
	if ( stage && stage->bundle[0].image[0] ) {
		textureBundle_t *bundle = &stage->bundle[0];
		int frame = 0;
		if ( bundle->numImageAnimations > 1 ) {
			float time = backEnd.refdef.time * 0.001f - entity->shaderTime;
			frame = static_cast<int>( floorf( time * bundle->imageAnimationSpeed ) );
			frame = frame < 0 ? 0 : frame % bundle->numImageAnimations;
		}
		image = bundle->image[frame];
	}
	// Bind directly: the outer pass restores both actual GL and its state cache.
	qglBindTexture( GL_TEXTURE_2D, image->texnum );
	qglColor4ubv( entity->shaderRGBA );
	unsigned blend = stage ? stage->stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) : 0;
	int mode = 0;
	if ( blend == ( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE ) ) {
		mode = 3; // Additive textures store coverage in RGB, not necessarily alpha.
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE );
	} else if ( blend == ( GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE ) ) {
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE );
	} else if ( blend == ( GLS_SRCBLEND_ZERO | GLS_DSTBLEND_ONE_MINUS_SRC_COLOR ) ) {
		mode = 1;
		qglBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR );
	} else if ( blend == ( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO ) ) {
		mode = 2;
		qglBlendFunc( GL_DST_COLOR, GL_ZERO );
	} else {
		qglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	rpUniform1i( screenProgram->blendMode, mode );
}
