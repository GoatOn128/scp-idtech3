// sdl_input.cpp -- SDL3 mouse and gamepad input, replaces win_input.cpp

#include "sdl_local.h"

static cvar_t *in_mouse;
static cvar_t *in_joystick;
static cvar_t *in_joyBallScale;
static cvar_t *in_debugJoystick;
static cvar_t *joy_threshold;
static cvar_t *k_language;

qboolean in_appactive = qfalse;

static qboolean s_mouseActive = qfalse;
static SDL_Gamepad *s_gamepad = NULL;
static int s_oldButtonState = 0;
static int s_oldPovState = 0;

// array of keys to use for the pov buttons, listed in the order that
// the bits are set in povstate
static int joyDirectionKeys[16] = {
	K_LEFTARROW, K_RIGHTARROW,
	K_UPARROW, K_DOWNARROW,
	K_JOY16, K_JOY17,
	K_JOY18, K_JOY19,
	K_JOY20, K_JOY21,
	K_JOY22, K_JOY23,
	K_JOY24, K_JOY25,
	K_JOY26, K_JOY27
};

static void IN_StartupJoystick( void ) {
	int count = 0;
	SDL_GamepadID *gamepads;

	if ( !in_joystick->integer ) {
		return;
	}

	gamepads = SDL_GetGamepads( &count );
	if ( gamepads == NULL || count <= 0 ) {
		Com_Printf( "Joystick not found\n" );
		if ( gamepads != NULL ) {
			SDL_free( gamepads );
		}
		return;
	}

	s_gamepad = SDL_OpenGamepad( gamepads[0] );
	SDL_free( gamepads );
	if ( s_gamepad == NULL ) {
		Com_Printf( "Joystick not found\n" );
		return;
	}

	Com_DPrintf( "Joystick found.\n" );
	Com_DPrintf( "Name: %s\n", SDL_GetGamepadName( s_gamepad ) );

	in_mouse->modified = qfalse;
	in_joystick->modified = qfalse;
}

void IN_Init( void ) {
	// variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_ARCHIVE | CVAR_LATCH );
	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_ARCHIVE | CVAR_LATCH );
	in_joyBallScale = Cvar_Get( "in_joyBallScale", "0.02", CVAR_ARCHIVE );
	in_debugJoystick = Cvar_Get( "in_debugJoystick", "0", CVAR_TEMP );
	joy_threshold = Cvar_Get( "joy_threshold", "0.15", CVAR_ARCHIVE );

	// keyboard input translation, kept registered for config compatibility
	k_language = Cvar_Get( "k_language", "american", CVAR_ARCHIVE | CVAR_NORESTART );

	IN_StartupJoystick();
}

void IN_Shutdown( void ) {
	if ( s_gamepad != NULL ) {
		SDL_CloseGamepad( s_gamepad );
		s_gamepad = NULL;
	}
}

static void IN_ActivateMouse( void ) {
	if ( !in_mouse->integer || sdl_window == NULL ) {
		return;
	}
	if ( s_mouseActive ) {
		return;
	}

	s_mouseActive = qtrue;
	SDL_SetWindowRelativeMouseMode( sdl_window, true );
	SDL_HideCursor();
}

static void IN_DeactivateMouse( void ) {
	if ( !s_mouseActive ) {
		return;
	}

	s_mouseActive = qfalse;
	if ( sdl_window != NULL ) {
		SDL_SetWindowRelativeMouseMode( sdl_window, false );
	}
	SDL_ShowCursor();
}

void IN_Activate( qboolean active ) {
	in_appactive = active;
	if ( !active ) {
		IN_DeactivateMouse();
	}
}

void IN_ClearStates( void ) {
	s_oldButtonState = 0;
	s_oldPovState = 0;
}

void IN_JoystickCommands( void ) {
	int buttonstate = 0;
	int povstate = 0;
	int i;
	int pressed;
	float fValue;
	int x, y;

	if ( s_gamepad == NULL ) {
		return;
	}

	// button key events
	for ( i = 0; i < 16; i++ ) {
		if ( SDL_GetGamepadButton( s_gamepad, (SDL_GamepadButton)( SDL_GAMEPAD_BUTTON_SOUTH + i ) ) ) {
			buttonstate |= 1 << i;
		}
	}
	for ( i = 0; i < 16; i++ ) {
		pressed = ( buttonstate & ( 1 << i ) ) != 0;
		if ( pressed != ( ( s_oldButtonState & ( 1 << i ) ) != 0 ) ) {
			Sys_QueEvent( 0, SE_KEY, K_JOY1 + i, pressed, 0, NULL );
		}
	}
	s_oldButtonState = buttonstate;

	// pov: the left stick maps to the first pov bits, the same way the
	// old win32 code mapped the first joystick axes
	for ( i = 0; i < 2; i++ ) {
		fValue = SDL_GetGamepadAxis( s_gamepad, (SDL_GamepadAxis)( SDL_GAMEPAD_AXIS_LEFTX + i ) ) / 32768.0f;
		if ( fValue < -joy_threshold->value ) {
			povstate |= 1 << ( i * 2 );
		} else if ( fValue > joy_threshold->value ) {
			povstate |= 1 << ( i * 2 + 1 );
		}
	}

	// the dpad maps to the same bits the old pov hat used
	if ( SDL_GetGamepadButton( s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP ) ) {
		povstate |= 1 << 12;
	}
	if ( SDL_GetGamepadButton( s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN ) ) {
		povstate |= 1 << 13;
	}
	if ( SDL_GetGamepadButton( s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT ) ) {
		povstate |= 1 << 14;
	}
	if ( SDL_GetGamepadButton( s_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT ) ) {
		povstate |= 1 << 15;
	}

	// pov button key events
	for ( i = 0; i < 16; i++ ) {
		pressed = ( povstate & ( 1 << i ) ) != 0;
		if ( pressed != ( ( s_oldPovState & ( 1 << i ) ) != 0 ) ) {
			Sys_QueEvent( 0, SE_KEY, joyDirectionKeys[i], pressed, 0, NULL );
		}
	}
	s_oldPovState = povstate;

	// treat the right stick like the old trackball input: it moves the mouse
	x = (int)( SDL_GetGamepadAxis( s_gamepad, SDL_GAMEPAD_AXIS_RIGHTX ) * in_joyBallScale->value );
	y = (int)( SDL_GetGamepadAxis( s_gamepad, SDL_GAMEPAD_AXIS_RIGHTY ) * in_joyBallScale->value );
	if ( x || y ) {
		Sys_QueEvent( 0, SE_MOUSE, x, y, 0, NULL );
	}
}

void IN_Frame( void ) {
	float fx = 0.0f, fy = 0.0f;
	int mx, my;

	IN_JoystickCommands();

	if ( !in_mouse->integer ) {
		return;
	}

	// release the mouse when the console is up and we are windowed
	if ( ( cls.keyCatchers & KEYCATCH_CONSOLE ) && Cvar_VariableIntegerValue( "r_fullscreen" ) == 0 ) {
		IN_DeactivateMouse();
		return;
	}

	// release the mouse when the app is not active
	if ( !in_appactive ) {
		IN_DeactivateMouse();
		return;
	}

	IN_ActivateMouse();

	SDL_GetRelativeMouseState( &fx, &fy );
	mx = (int)fx;
	my = (int)fy;
	if ( mx || my ) {
		Sys_QueEvent( 0, SE_MOUSE, mx, my, 0, NULL );
	}
}
