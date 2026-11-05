#ifndef __SDL_LOCAL_H__
#define __SDL_LOCAL_H__

// sdl_local.h -- declarations shared between the SDL3 system layer
// translation units in sys/sdl

#include <SDL3/SDL.h>

#include "../../client/client.h"
#include "../../qcommon/qcommon.h"

// the one and only SDL window, created by the video backend (sdl_glimp.cpp)
extern SDL_Window *sdl_window;

// sdl_main.cpp
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
char *Sys_ConsoleInput( void );

// sdl_input.cpp
void IN_Init( void );
void IN_Shutdown( void );
void IN_Activate( qboolean active );
void IN_Frame( void );
void IN_JoystickCommands( void );
void IN_ClearStates( void );

// sdl_snd.cpp
void SNDDMA_Activate( void );

// sdl_net.cpp
qboolean Sys_GetPacket( netadr_t *net_from, msg_t *net_message );

#endif
