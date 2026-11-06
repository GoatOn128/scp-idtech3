#ifndef __SDL_LOCAL_H__
#define __SDL_LOCAL_H__

// sys_local.h -- declarations shared between the SDL3 system layer
// translation units in sys/sdl

#include <SDL3/SDL.h>

#include "../../client/client.h"
#include "../../qcommon/qcommon.h"

// the one and only SDL window, created by the video backend (sys_glimp.cpp)
extern SDL_Window *sdl_window;

// sys_main.cpp
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
char *Sys_ConsoleInput( void );

// sys_input.cpp
void IN_Init( void );
void IN_Shutdown( void );
void IN_Activate( qboolean active );
void IN_Frame( void );
void IN_JoystickCommands( void );
void IN_ClearStates( void );

// snd_openal.cpp (sys/openal)
void SNDDMA_Activate( void );

// sys_net.cpp
qboolean Sys_GetPacket( netadr_t *net_from, idBitMsg *net_message );

// sys_glimp.cpp
void GLimp_UpdateWindowSize( void );

#endif
