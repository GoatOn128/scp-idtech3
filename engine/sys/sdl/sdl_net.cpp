// sdl_net.cpp -- portable UDP network layer, replaces win_net.cpp.
// Uses winsock2 on Windows and BSD sockets everywhere else. IPv4 only,
// the old IPX and SOCKS relay code is gone but the cvars stay registered
// for config compatibility. Networking remains disabled for single
// player exactly like the old win32 code.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "sdl_local.h"

#ifndef _WIN32
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef int SOCKET;
#define INVALID_SOCKET ( SOCKET ) - 1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#define MAX_IPS     32

#ifdef _WIN32
#define NET_GetLastError()     WSAGetLastError()
#define NET_EWOULDBLOCK       WSAEWOULDBLOCK
#define NET_ECONNRESET        WSAECONNRESET
#define NET_EADDRNOTAVAIL     WSAEADDRNOTAVAIL
#define NET_EAFNOSUPPORT      WSAEAFNOSUPPORT
#else
#define NET_GetLastError()     ( errno )
#define NET_EWOULDBLOCK       EWOULDBLOCK
#define NET_ECONNRESET        ECONNRESET
#define NET_EADDRNOTAVAIL     EADDRNOTAVAIL
#define NET_EAFNOSUPPORT      EAFNOSUPPORT
#endif

static qboolean winsockInitialized = qfalse;
static qboolean networkingEnabled = qfalse;

static cvar_t   *net_noudp;
static cvar_t   *net_noipx;

static cvar_t   *net_socksEnabled;
static cvar_t   *net_socksServer;
static cvar_t   *net_socksPort;
static cvar_t   *net_socksUsername;
static cvar_t   *net_socksPassword;

static byte localIP[MAX_IPS][4];
static int numIP = 0;

static SOCKET ip_socket = INVALID_SOCKET;

/*
====================
NET_ErrorString
====================
*/
char *NET_ErrorString( void ) {
#ifdef _WIN32
	int code;

	code = WSAGetLastError();
	switch ( code ) {
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSAECONNABORTED";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
#else
	return (char *)strerror( errno );
#endif
}

/*
====================
NET_WouldBlock

Silent errors that happen on every frame when a non blocking socket
has nothing pending
====================
*/
static qboolean NET_WouldBlock( int err ) {
	if ( err == NET_EWOULDBLOCK || err == NET_ECONNRESET ) {
		return qtrue;
	}
#ifndef _WIN32
	if ( err == EAGAIN ) {
		return qtrue;
	}
#endif
	return qfalse;
}

static void NetadrToSockadr( netadr_t *a, struct sockaddr *s ) {
	memset( s, 0, sizeof( struct sockaddr_in ) );

	if ( a->type == NA_BROADCAST ) {
		( (struct sockaddr_in *)s )->sin_family = AF_INET;
		( (struct sockaddr_in *)s )->sin_port = a->port;
		( (struct sockaddr_in *)s )->sin_addr.s_addr = INADDR_BROADCAST;
	} else if ( a->type == NA_IP ) {
		( (struct sockaddr_in *)s )->sin_family = AF_INET;
		( (struct sockaddr_in *)s )->sin_addr.s_addr = *(int *)&a->ip;
		( (struct sockaddr_in *)s )->sin_port = a->port;
	}
}

static void SockadrToNetadr( struct sockaddr *s, netadr_t *a ) {
	if ( s->sa_family == AF_INET ) {
		a->type = NA_IP;
		*(int *)&a->ip = ( (struct sockaddr_in *)s )->sin_addr.s_addr;
		a->port = ( (struct sockaddr_in *)s )->sin_port;
	}
}

qboolean Sys_StringToSockaddr( const char *s, struct sockaddr *sadr ) {
	struct addrinfo hints;
	struct addrinfo *res = NULL;

	memset( sadr, 0, sizeof( *sadr ) );
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ( getaddrinfo( s, NULL, &hints, &res ) != 0 || res == NULL ) {
		return qfalse;
	}

	if ( res->ai_family == AF_INET ) {
		memcpy( sadr, res->ai_addr, sizeof( struct sockaddr_in ) );
		freeaddrinfo( res );
		return qtrue;
	}

	freeaddrinfo( res );
	return qfalse;
}

/*
=============
Sys_StringToAdr

idnewt
192.246.40.70
=============
*/
qboolean Sys_StringToAdr( const char *s, netadr_t *a ) {
	struct sockaddr sadr;

	if ( !Sys_StringToSockaddr( s, &sadr ) ) {
		return qfalse;
	}

	SockadrToNetadr( &sadr, a );
	return qtrue;
}

/*
==================
Sys_GetPacket

Never called by the game logic, just the system event queing
==================
*/
qboolean Sys_GetPacket( netadr_t *net_from, msg_t *net_message ) {
	int ret;
	struct sockaddr_in from;
#ifdef _WIN32
	int fromlen;
#else
	socklen_t fromlen;
#endif
	int err;

	if ( !ip_socket || ip_socket == INVALID_SOCKET ) {
		return qfalse;
	}

	fromlen = sizeof( from );
	ret = recvfrom( ip_socket, (char *)net_message->data, net_message->maxsize, 0, (struct sockaddr *)&from, &fromlen );
	if ( ret == SOCKET_ERROR ) {
		err = NET_GetLastError();

		if ( NET_WouldBlock( err ) ) {
			return qfalse;
		}
		Com_Printf( "NET_GetPacket: %s\n", NET_ErrorString() );
		return qfalse;
	}

	memset( ( (struct sockaddr_in *)&from )->sin_zero, 0, 8 );

	SockadrToNetadr( (struct sockaddr *)&from, net_from );
	net_message->readcount = 0;

	if ( ret == net_message->maxsize ) {
		Com_Printf( "Oversize packet from %s\n", NET_AdrToString( *net_from ) );
		return qfalse;
	}

	net_message->cursize = ret;
	return qtrue;
}

/*
==================
Sys_SendPacket
==================
*/
void Sys_SendPacket( int length, const void *data, netadr_t to ) {
	int ret;
	int err;
	struct sockaddr addr;

	if ( to.type != NA_BROADCAST && to.type != NA_IP ) {
		Com_Error( ERR_FATAL, "Sys_SendPacket: bad address type" );
		return;
	}

	if ( !ip_socket || ip_socket == INVALID_SOCKET ) {
		return;
	}

	NetadrToSockadr( &to, &addr );

	ret = sendto( ip_socket, (const char *)data, length, 0, &addr, sizeof( addr ) );
	if ( ret == SOCKET_ERROR ) {
		err = NET_GetLastError();

		// wouldblock is silent
		if ( err == NET_EWOULDBLOCK ) {
			return;
		}

		// some PPP links do not allow broadcasts and return an error
		if ( err == NET_EADDRNOTAVAIL && to.type == NA_BROADCAST ) {
			return;
		}

		Com_Printf( "NET_SendPacket: %s\n", NET_ErrorString() );
	}
}

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
qboolean Sys_IsLANAddress( netadr_t adr ) {
	int i;

	if ( adr.type == NA_LOOPBACK ) {
		return qtrue;
	}

	if ( adr.type != NA_IP ) {
		return qfalse;
	}

	// choose which comparison to use based on the class of the address being tested
	// any local adresses of a different class than the address being tested will fail based on the first byte

	if ( adr.ip[0] == 127 && adr.ip[1] == 0 && adr.ip[2] == 0 && adr.ip[3] == 1 ) {
		return qtrue;
	}

	// Class A
	if ( ( adr.ip[0] & 0x80 ) == 0x00 ) {
		for ( i = 0 ; i < numIP ; i++ ) {
			if ( adr.ip[0] == localIP[i][0] ) {
				return qtrue;
			}
		}
		// the RFC1918 class a block will pass the above test
		return qfalse;
	}

	// Class B
	if ( ( adr.ip[0] & 0xc0 ) == 0x80 ) {
		for ( i = 0 ; i < numIP ; i++ ) {
			if ( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] ) {
				return qtrue;
			}
			// also check against the RFC1918 class b blocks
			if ( adr.ip[0] == 172 && localIP[i][0] == 172 && ( adr.ip[1] & 0xf0 ) == 16 && ( localIP[i][1] & 0xf0 ) == 16 ) {
				return qtrue;
			}
		}
		return qfalse;
	}

	// Class C
	for ( i = 0 ; i < numIP ; i++ ) {
		if ( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] && adr.ip[2] == localIP[i][2] ) {
			return qtrue;
		}
		// also check against the RFC1918 class c blocks
		if ( adr.ip[0] == 192 && localIP[i][0] == 192 && adr.ip[1] == 168 && localIP[i][1] == 168 ) {
			return qtrue;
		}
	}
	return qfalse;
}

/*
==================
Sys_ShowIP
==================
*/
void Sys_ShowIP( void ) {
	int i;

	for ( i = 0; i < numIP; i++ ) {
		Com_Printf( "IP: %i.%i.%i.%i\n", localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3] );
	}
}

//=============================================================================

/*
====================
NET_IPSocket
====================
*/
static int NET_IPSocket( char *net_interface, int port ) {
	SOCKET newsocket;
	struct sockaddr_in address;
	unsigned long _true = 1;
	int i = 1;
	int err;

	if ( net_interface ) {
		Com_Printf( "Opening IP socket: %s:%i\n", net_interface, port );
	} else {
		Com_Printf( "Opening IP socket: localhost:%i\n", port );
	}

	if ( ( newsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET ) {
		err = NET_GetLastError();
		if ( err != NET_EAFNOSUPPORT ) {
			Com_Printf( "WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString() );
		}
		return 0;
	}

	// make it non-blocking
#ifdef _WIN32
	if ( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR ) {
#else
	if ( fcntl( newsocket, F_SETFL, fcntl( newsocket, F_GETFL, 0 ) | O_NONBLOCK ) == -1 ) {
#endif
		Com_Printf( "WARNING: UDP_OpenSocket: nonblocking: %s\n", NET_ErrorString() );
		return 0;
	}

	// make it broadcast capable
	if ( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof( i ) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString() );
		return 0;
	}

	if ( !net_interface || !net_interface[0] || !idStr::Icmp( net_interface, "localhost" ) ) {
		memset( &address, 0, sizeof( address ) );
		address.sin_addr.s_addr = INADDR_ANY;
	} else {
		Sys_StringToSockaddr( net_interface, (struct sockaddr *)&address );
	}

	if ( port == PORT_ANY ) {
		address.sin_port = 0;
	} else {
		address.sin_port = htons( (short)port );
	}

	address.sin_family = AF_INET;

	if ( bind( newsocket, (const sockaddr *)&address, sizeof( address ) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return 0;
	}

	return (int)newsocket;
}

/*
=====================
NET_GetLocalAddress
=====================
*/
static void NET_GetLocalAddress( void ) {
	char hostname[256];
	struct addrinfo hints;
	struct addrinfo *res = NULL;
	struct addrinfo *p;
	struct sockaddr_in *sin;
	const byte *b;
	int ip;
	int ret;

	ret = gethostname( hostname, sizeof( hostname ) );
	if ( ret == SOCKET_ERROR ) {
		return;
	}

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	if ( getaddrinfo( hostname, NULL, &hints, &res ) != 0 || res == NULL ) {
		return;
	}

	Com_Printf( "Hostname: %s\n", hostname );

	numIP = 0;
	for ( p = res; p != NULL && numIP < MAX_IPS; p = p->ai_next ) {
		sin = (struct sockaddr_in *)p->ai_addr;
		b = (const byte *)&sin->sin_addr;
		ip = ntohl( sin->sin_addr.s_addr );

		localIP[ numIP ][0] = b[0];
		localIP[ numIP ][1] = b[1];
		localIP[ numIP ][2] = b[2];
		localIP[ numIP ][3] = b[3];
		Com_Printf( "IP: %i.%i.%i.%i\n", ( ip >> 24 ) & 0xff, ( ip >> 16 ) & 0xff, ( ip >> 8 ) & 0xff, ip & 0xff );
		numIP++;
	}

	freeaddrinfo( res );
}

/*
====================
NET_OpenIP
====================
*/
static void NET_OpenIP( void ) {
	cvar_t  *ip;
	int port;
	int i;

	ip = Cvar_Get( "net_ip", "localhost", CVAR_LATCH );
	port = Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH )->integer;

	// automatically scan for a valid port, so multiple
	// dedicated servers can be started without requiring
	// a different net_port for each one
	for ( i = 0 ; i < 10 ; i++ ) {
		ip_socket = NET_IPSocket( ip->string, port + i );
		if ( ip_socket ) {
			Cvar_SetValue( "net_port", port + i );
			NET_GetLocalAddress();
			return;
		}
	}
	Com_Printf( "WARNING: Couldn't allocate IP port\n" );
}

/*
====================
NET_GetCvars
====================
*/
static qboolean NET_GetCvars( void ) {
	qboolean modified;

	modified = qfalse;

	if ( net_noudp && net_noudp->modified ) {
		modified = qtrue;
	}
	net_noudp = Cvar_Get( "net_noudp", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_noipx && net_noipx->modified ) {
		modified = qtrue;
	}
	net_noipx = Cvar_Get( "net_noipx", "0", CVAR_LATCH | CVAR_ARCHIVE );

	// the SOCKS relay is gone, but the cvars stay registered so old
	// configs keep loading
	if ( net_socksEnabled && net_socksEnabled->modified ) {
		modified = qtrue;
	}
	net_socksEnabled = Cvar_Get( "net_socksEnabled", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksServer && net_socksServer->modified ) {
		modified = qtrue;
	}
	net_socksServer = Cvar_Get( "net_socksServer", "", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksPort && net_socksPort->modified ) {
		modified = qtrue;
	}
	net_socksPort = Cvar_Get( "net_socksPort", "1080", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksUsername && net_socksUsername->modified ) {
		modified = qtrue;
	}
	net_socksUsername = Cvar_Get( "net_socksUsername", "", CVAR_LATCH | CVAR_ARCHIVE );

	if ( net_socksPassword && net_socksPassword->modified ) {
		modified = qtrue;
	}
	net_socksPassword = Cvar_Get( "net_socksPassword", "", CVAR_LATCH | CVAR_ARCHIVE );

	return modified;
}

/*
====================
NET_Config
====================
*/
void NET_Config( qboolean enableNetworking ) {
	qboolean modified;
	qboolean stop;
	qboolean start;

	// get any latched changes to cvars
	modified = NET_GetCvars();

	if ( net_noudp->integer && net_noipx->integer ) {
		enableNetworking = qfalse;
	}

	// if enable state is the same and no cvars were modified, we have nothing to do
	if ( enableNetworking == networkingEnabled && !modified ) {
		return;
	}

	if ( enableNetworking == networkingEnabled ) {
		if ( enableNetworking ) {
			stop = qtrue;
			start = qtrue;
		} else {
			stop = qfalse;
			start = qfalse;
		}
	} else {
		if ( enableNetworking ) {
			stop = qfalse;
			start = qtrue;
		} else {
			stop = qtrue;
			start = qfalse;
		}
		networkingEnabled = enableNetworking;
	}

	if ( stop ) {
		if ( ip_socket && ip_socket != INVALID_SOCKET ) {
			closesocket( ip_socket );
			ip_socket = 0;
		}
	}

	if ( start ) {
		if ( !net_noudp->integer ) {
			NET_OpenIP();
		}
	}
}

/*
====================
NET_Init
====================
*/
void NET_Init( void ) {
	return; //----(SA)	disabled networking for SP

#ifdef _WIN32
	{
		int r;
		WSADATA winsockdata;

		r = WSAStartup( MAKEWORD( 2, 2 ), &winsockdata );
		if ( r ) {
			Com_Printf( "WARNING: Winsock initialization failed, returned %i\n", r );
			return;
		}
	}
#endif
	winsockInitialized = qtrue;
	Com_Printf( "Winsock Initialized\n" );

	// this is really just to get the cvars registered
	NET_GetCvars();

	//FIXME testing!
	NET_Config( qtrue );
}

/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown( void ) {
	if ( !winsockInitialized ) {
		return;
	}
	NET_Config( qfalse );
#ifdef _WIN32
	WSACleanup();
#endif
	winsockInitialized = qfalse;
}

/*
====================
NET_Sleep

sleeps msec or until net socket is ready
====================
*/
void NET_Sleep( int msec ) {
	(void)msec;
}

/*
====================
NET_Restart_f
====================
*/
void NET_Restart( void ) {
	NET_Config( networkingEnabled );
}
