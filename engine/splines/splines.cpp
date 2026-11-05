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

#include "q_splineshared.h"
#include "splines.h"

int FS_Write( const void *buffer, int len, fileHandle_t h );
int FS_ReadFile( const char *qpath, void **buffer );
void FS_FreeFile( void *buffer );
fileHandle_t FS_FOpenFileWrite( const char *filename );
void FS_FCloseFile( fileHandle_t f );
void Cbuf_AddText( const char *text );
void Cbuf_Execute( void );

// (SA) making a list of cameras so I can use
//		the splines as targets for other things.
//		Certainly better ways to do this, but this lets
//		me get underway quickly with ents that need spline
//		targets.
#define MAX_CAMERAS 64

idCameraDef camera[MAX_CAMERAS];

qboolean loadCamera( int camNum, const char *name ) {
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return qfalse;
	}
	camera[camNum].clear();
	// TTimo static_cast confused gcc, went for C-style casting
	return (qboolean)( camera[camNum].load( name ) );
}

qboolean getCameraInfo( int camNum, int time, idVec3 *origin, idVec3 *angles, float *fov ) {
	idVec3 dir, org;
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return qfalse;
	}
	org = *origin;
	if ( camera[camNum].getCameraInfo( time, org, dir, fov ) ) {
		*origin = org;
		( *angles )[1] = atan2( dir[1], dir[0] ) * 180 / 3.14159;
		( *angles )[0] = asin( dir[2] ) * 180 / 3.14159;
		return qtrue;
	}
	return qfalse;
}

void startCamera( int camNum, int time ) {
	if ( camNum < 0 || camNum >= MAX_CAMERAS ) {
		return;
	}
	camera[camNum].startCamera( time );
}

//#include "../shared/windings.h"
//#include "../qcommon/qcommon.h"
//#include "../sys/sys_public.h"
//#include "../game/game_entity.h"

idCameraDef splineList;
idCameraDef *g_splineList = &splineList;

idVec3 idSplineList::zero( 0,0,0 );

void glLabeledPoint( idVec3 &color, idVec3 &point, float size, const char *label ) {
	qglColor3fv( $1.ToFloatPtr() );colorqglColor3fv( $1.ToFloatPtr() );
	qglPointSize( size );
	qglBegin( GL_POINTS );
	qglVertex3fv( point.ToFloatPtr() );
	qglEnd();
	idVec3 v = point;
	v.x += 1;
	v.y += 1;
	v.z += 1;
	qglRasterPos3fv( v.ToFloatPtr() );
	qglCallLists( strlen( label ), GL_UNSIGNED_BYTE, label );
}


void glBox( idVec3 &color, idVec3 &point, float size ) {
	idVec3 mins( point );
	idVec3 maxs( point );
	mins[0] -= size;
	mins[1] += size;
	mins[2] -= size;
	maxs[0] += size;
	maxs[1] -= size;
	maxs[2] += size;
	qglColor3fv( $1.ToFloatPtr() );colorqglColor3fv( $1.ToFloatPtr() );
	qglBegin( GL_LINE_LOOP );
	qglVertex3f( mins[0],mins[1],mins[2] );
	qglVertex3f( maxs[0],mins[1],mins[2] );
	qglVertex3f( maxs[0],maxs[1],mins[2] );
	qglVertex3f( mins[0],maxs[1],mins[2] );
	qglEnd();
	qglBegin( GL_LINE_LOOP );
	qglVertex3f( mins[0],mins[1],maxs[2] );
	qglVertex3f( maxs[0],mins[1],maxs[2] );
	qglVertex3f( maxs[0],maxs[1],maxs[2] );
	qglVertex3f( mins[0],maxs[1],maxs[2] );
	qglEnd();

	qglBegin( GL_LINES );
	qglVertex3f( mins[0],mins[1],mins[2] );
	qglVertex3f( mins[0],mins[1],maxs[2] );
	qglVertex3f( mins[0],maxs[1],maxs[2] );
	qglVertex3f( mins[0],maxs[1],mins[2] );
	qglVertex3f( maxs[0],mins[1],mins[2] );
	qglVertex3f( maxs[0],mins[1],maxs[2] );
	qglVertex3f( maxs[0],maxs[1],maxs[2] );
	qglVertex3f( maxs[0],maxs[1],mins[2] );
	qglEnd();

}

void splineTest() {
	//g_splineList->load("p:/doom/base/maps/test_base1.camera");
}

void splineDraw() {
	//g_splineList->addToRenderer();
}


//extern void D_DebugLine( const idVec3 &color, const idVec3 &start, const idVec3 &end );

void debugLine( idVec3 &color, float x, float y, float z, float x2, float y2, float z2 ) {
//	idVec3 from(x, y, z);
//	idVec3 to(x2, y2, z2);
	//D_DebugLine(color, from, to);
}

void idSplineList::addToRenderer() {

	if ( controlPoints.Num() == 0 ) {
		return;
	}

	idVec3 mins, maxs;
	idVec3 yellow( 1.0, 1.0, 0 );
	idVec3 white( 1.0, 1.0, 1.0 );
	int i;

	for ( i = 0; i < controlPoints.Num(); i++ ) {
		mins = *controlPoints[i];
		maxs = mins;
		mins[0] -= 8;
		mins[1] += 8;
		mins[2] -= 8;
		maxs[0] += 8;
		maxs[1] -= 8;
		maxs[2] += 8;
		debugLine( yellow, mins[0], mins[1], mins[2], maxs[0], mins[1], mins[2] );
		debugLine( yellow, maxs[0], mins[1], mins[2], maxs[0], maxs[1], mins[2] );
		debugLine( yellow, maxs[0], maxs[1], mins[2], mins[0], maxs[1], mins[2] );
		debugLine( yellow, mins[0], maxs[1], mins[2], mins[0], mins[1], mins[2] );

		debugLine( yellow, mins[0], mins[1], maxs[2], maxs[0], mins[1], maxs[2] );
		debugLine( yellow, maxs[0], mins[1], maxs[2], maxs[0], maxs[1], maxs[2] );
		debugLine( yellow, maxs[0], maxs[1], maxs[2], mins[0], maxs[1], maxs[2] );
		debugLine( yellow, mins[0], maxs[1], maxs[2], mins[0], mins[1], maxs[2] );

	}

	int step = 0;
	idVec3 step1;
	for ( i = 3; i < controlPoints.Num(); i++ ) {
		for ( float tension = 0.0f; tension < 1.001f; tension += 0.1f ) {
			float x = 0;
			float y = 0;
			float z = 0;
			for ( int j = 0; j < 4; j++ ) {
				x += controlPoints[i - ( 3 - j )]->x * calcSpline( j, tension );
				y += controlPoints[i - ( 3 - j )]->y * calcSpline( j, tension );
				z += controlPoints[i - ( 3 - j )]->z * calcSpline( j, tension );
			}
			if ( step == 0 ) {
				step1[0] = x;
				step1[1] = y;
				step1[2] = z;
				step = 1;
			} else {
				debugLine( white, step1[0], step1[1], step1[2], x, y, z );
				step = 0;
			}

		}
	}
}

void idSplineList::buildSpline() {
	//int start = Sys_Milliseconds();
	clearSpline();
	for ( int i = 3; i < controlPoints.Num(); i++ ) {
		for ( float tension = 0.0f; tension < 1.001f; tension += granularity ) {
			float x = 0;
			float y = 0;
			float z = 0;
			for ( int j = 0; j < 4; j++ ) {
				x += controlPoints[i - ( 3 - j )]->x * calcSpline( j, tension );
				y += controlPoints[i - ( 3 - j )]->y * calcSpline( j, tension );
				z += controlPoints[i - ( 3 - j )]->z * calcSpline( j, tension );
			}
			splinePoints.Append( new idVec3( x, y, z ) );
		}
	}
	dirty = false;
	//Com_Printf("Spline build took %f seconds\n", (float)(Sys_Milliseconds() - start) / 1000);
}


void idSplineList::draw( bool edit ) {
	int i;
	idVec4 yellow( 1, 1, 0, 1 );

	if ( controlPoints.Num() == 0 ) {
		return;
	}

	if ( dirty ) {
		buildSpline();
	}


	qglColor3fv( $1.ToFloatPtr() );controlColorqglColor3fv( $1.ToFloatPtr() );
	qglPointSize( 5 );

	qglBegin( GL_POINTS );
	for ( i = 0; i < controlPoints.Num(); i++ ) {
		qglVertex3fv( (*$1[$2]).ToFloatPtr() );controlPointsqglVertex3fv( (*$1[$2]).ToFloatPtr() );iqglVertex3fv( (*$1[$2]).ToFloatPtr() );
	}
	qglEnd();

	if ( edit ) {
		for ( i = 0; i < controlPoints.Num(); i++ ) {
			glBox( activeColor, *controlPoints[i], 4 );
		}
	}

	//Draw the curve
	qglColor3fv( $1.ToFloatPtr() );pathColorqglColor3fv( $1.ToFloatPtr() );
	qglBegin( GL_LINE_STRIP );
	int count = splinePoints.Num();
	for ( i = 0; i < count; i++ ) {
		qglVertex3fv( (*$1[$2]).ToFloatPtr() );splinePointsqglVertex3fv( (*$1[$2]).ToFloatPtr() );iqglVertex3fv( (*$1[$2]).ToFloatPtr() );
	}
	qglEnd();

	if ( edit ) {
		qglColor3fv( $1.ToFloatPtr() );segmentColorqglColor3fv( $1.ToFloatPtr() );
		qglPointSize( 3 );
		qglBegin( GL_POINTS );
		for ( i = 0; i < count; i++ ) {
			qglVertex3fv( (*$1[$2]).ToFloatPtr() );splinePointsqglVertex3fv( (*$1[$2]).ToFloatPtr() );iqglVertex3fv( (*$1[$2]).ToFloatPtr() );
		}
		qglEnd();
	}
	if ( count > 0 ) {
		//assert(activeSegment >=0 && activeSegment < count);
		if ( activeSegment >= 0 && activeSegment < count ) {
			glBox( activeColor, *splinePoints[activeSegment], 6 );
			glBox( yellow.ToVec3(), *splinePoints[activeSegment], $1 )8glBox( yellow.ToVec3(), *splinePoints[activeSegment], $1 );
		}
	}

}

float idSplineList::totalDistance() {

	// FIXME: save dist and return
	//
	if ( controlPoints.Num() == 0 ) {
		return 0.0;
	}

	if ( dirty ) {
		buildSpline();
	}

	float dist = 0.0;
	idVec3 temp;
	int count = splinePoints.Num();
	for ( int i = 1; i < count; i++ ) {
		temp = *splinePoints[i - 1];
		temp -= *splinePoints[i];
		dist += temp.Length();
	}
	return dist;
}

void idSplineList::initPosition( long bt, long totalTime ) {

	if ( dirty ) {
		buildSpline();
	}

	if ( splinePoints.Num() == 0 ) {
		return;
	}

	baseTime = bt;
	time = totalTime;

	// calc distance to travel ( this will soon be broken into time segments )
	splineTime.Clear();
	splineTime.Append( bt );
	double dist = totalDistance();
	double distSoFar = 0.0;
	idVec3 temp;
	int count = splinePoints.Num();
	//for(int i = 2; i < count - 1; i++) {
	for ( int i = 1; i < count; i++ ) {
		temp = *splinePoints[i - 1];
		temp -= *splinePoints[i];
		distSoFar += temp.Length();
		double percent = distSoFar / dist;
		percent *= totalTime;
		splineTime.Append( percent + bt );
	}
	assert( splineTime.Num() == splinePoints.Num() );
	activeSegment = 0;
}



float idSplineList::calcSpline( int step, float tension ) {
	switch ( step ) {
	case 0: return ( pow( 1 - tension, 3 ) ) / 6;
	case 1: return ( 3 * pow( tension, 3 ) - 6 * pow( tension, 2 ) + 4 ) / 6;
	case 2: return ( -3 * pow( tension, 3 ) + 3 * pow( tension, 2 ) + 3 * tension + 1 ) / 6;
	case 3: return pow( tension, 3 ) / 6;
	}
	return 0.0;
}



void idSplineList::updateSelection( const idVec3 &move ) {
	if ( selected ) {
		dirty = true;
		*selected += move;
	}
}


void idSplineList::setSelectedPoint( idVec3 *p ) {
	if ( p ) {
		p->Snap();
		for ( int i = 0; i < controlPoints.Num(); i++ ) {
			if ( *p == *controlPoints[i] ) {
				selected = controlPoints[i];
			}
		}
	} else {
		selected = NULL;
	}
}

const idVec3 *idSplineList::getPosition( long t ) {
	static idVec3 interpolatedPos;
//	static long lastTime = -1; // TTimo unused

	int count = splineTime.Num();
	if ( count == 0 ) {
		return &zero;
	}

//	Com_Printf("Time: %d\n", t);
	assert( splineTime.Num() == splinePoints.Num() );

	while ( activeSegment < count ) {
		if ( splineTime[activeSegment] >= t ) {
			if ( activeSegment > 0 && activeSegment < count - 1 ) {
				double timeHi = splineTime[activeSegment + 1];
				double timeLo = splineTime[activeSegment - 1];
				double percent = ( timeHi - t ) / ( timeHi - timeLo );
				// pick two bounding points
				idVec3 v1 = *splinePoints[activeSegment - 1];
				idVec3 v2 = *splinePoints[activeSegment + 1];
				v2 *= ( 1.0 - percent );
				v1 *= percent;
				v2 += v1;
				interpolatedPos = v2;
				return &interpolatedPos;
			}
			return splinePoints[activeSegment];
		} else {
			activeSegment++;
		}
	}
	return splinePoints[count - 1];
}

void idSplineList::parse( idLexer &src ) {
	idToken token;
	//src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Icmp( token, "}" ) ) {
			break;
		}

		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Icmp( token, "(" ) || !idStr::Icmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "granularity" ) == 0 ) {
				granularity = $1( token.c_str() )atof$1( token.c_str() );
			} else if ( idStr::Icmp( key.c_str(), "name" ) == 0 ) {
				name = token.c_str();
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Icmp( token, "}" ) ) {
			break;
		}
$1src.UnreadToken( &token );
		$1src.UnreadToken( &token );
		// read the control point
		idVec3 point;
		src.Parse1DMatrix( 3, $1.ToFloatPtr() );pointsrc.Parse1DMatrix( 3, $1.ToFloatPtr() );
		addPoint( point.x, point.y, point.z );
	} while ( 1 );

	//Com_UngetToken();
	//src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
	dirty = true;
}

void idSplineList::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	//s = va("\t\tname %s\n", name.c_str());
	//FS_Write(s.c_str(), s.length(), file);
	s = va( "\t\t\tgranularity %f\n", granularity );
	FS_Write( s.c_str(), s.length(), file );
	int count = controlPoints.Num();
	for ( int i = 0; i < count; i++ ) {
		s = va( "\t\t\t( %f %f %f )\n", controlPoints[i]->x, controlPoints[i]->y, controlPoints[i]->z );
		FS_Write( s.c_str(), s.length(), file );
	}
	s = "\t\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


void idCameraDef::getActiveSegmentInfo( int segment, idVec3 &origin, idVec3 &direction, float *fovOut ) {
#if 0
	if ( !cameraSpline.validTime() ) {
		buildCamera();
	}
	double d = (double)segment / numSegments();
	getCameraInfo( d * totalTime * 1000, origin, direction, fovOut );
#endif
/*
	if (!cameraSpline.validTime()) {
		buildCamera();
	}
	origin = *cameraSpline.getSegmentPoint(segment);


	idVec3 temp;

	int numTargets = getTargetSpline()->controlPoints.Num();
	int count = cameraSpline.splineTime.Num();
	if (numTargets == 0) {
		// follow the path
		if (cameraSpline.getActiveSegment() < count - 1) {
			temp = *cameraSpline.splinePoints[cameraSpline.getActiveSegment()+1];
		}
	} else if (numTargets == 1) {
		temp = *getTargetSpline()->controlPoints[0];
	} else {
		temp = *getTargetSpline()->getSegmentPoint(segment);
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;
*/
}

bool idCameraDef::getCameraInfo( long time, idVec3 &origin, idVec3 &direction, float *fv ) {

	char buff[1024];

	if ( ( time - startTime ) / 1000 > totalTime ) {
		return false;
	}


	for ( int i = 0; i < events.Num(); i++ ) {
		if ( time >= startTime + events[i]->getTime() && !events[i]->getTriggered() ) {
			events[i]->setTriggered( true );
			if ( events[i]->getType() == idCameraEvent::EVENT_TARGET ) {
				setActiveTargetByName( events[i]->getParam() );
				getActiveTarget()->start( startTime + events[i]->getTime() );
				//Com_Printf("Triggered event switch to target: %s\n",events[i]->getParam());
			} else if ( events[i]->getType() == idCameraEvent::EVENT_TRIGGER ) {
				// empty!
			} else if ( events[i]->getType() == idCameraEvent::EVENT_FOV ) {
				memset( buff, 0, sizeof( buff ) );
				strcpy( buff, events[i]->getParam() );
				const char *param1 = strtok( buff, " \t,\0" );
				const char *param2 = strtok( NULL, " \t,\0" );
				float len = ( param2 ) ? atof( param2 ) : 0;
				float newfov = ( param1 ) ? atof( param1 ) : 90;
				fov.reset( fov.getFOV( time ), newfov, time, len );
				//*fv = fov = atof(events[i]->getParam());
			} else if ( events[i]->getType() == idCameraEvent::EVENT_FADEIN ) {
				float fadeTime = atof( events[i]->getParam() );
				Cbuf_AddText( va( "fade 0 0 0 0 %f", fadeTime ) );
				Cbuf_Execute();
			} else if ( events[i]->getType() == idCameraEvent::EVENT_FADEOUT ) {
				float fadeTime = atof( events[i]->getParam() );
				Cbuf_AddText( va( "fade 0 0 0 255 %f", fadeTime ) );
				Cbuf_Execute();
			} else if ( events[i]->getType() == idCameraEvent::EVENT_CAMERA ) {
				memset( buff, 0, sizeof( buff ) );
				strcpy( buff, events[i]->getParam() );
				const char *param1 = strtok( buff, " \t,\0" );
				const char *param2 = strtok( NULL, " \t,\0" );

				if ( param2 ) {
					loadCamera( atoi( param1 ), va( "cameras/%s.camera", param2 ) );
					startCamera( time );
				} else {
					loadCamera( 0, va( "cameras/%s.camera", events[i]->getParam() ) );
					startCamera( time );
				}
				return true;
			} else if ( events[i]->getType() == idCameraEvent::EVENT_STOP ) {
				return false;
			}
		}
	}

	origin = *cameraPosition->getPosition( time );

	CHECK_NAN_VEC( origin );

	*fv = fov.getFOV( time );

	idVec3 temp = origin;

	int numTargets = targetPositions.Num();
	if ( numTargets == 0 ) {
		// empty!
	} else {
		temp = *getActiveTarget()->getPosition( time );
	}

	temp -= origin;
	temp.Normalize();
	direction = temp;

	return true;
}

bool idCameraDef::waitEvent( int index ) {
	//for (int i = 0; i < events.Num(); i++) {
	//	if (events[i]->getSegment() == index && events[i]->getType() == idCameraEvent::EVENT_WAIT) {
	//		return true;
	//	}
	//}
	return false;
}


#define NUM_CCELERATION_SEGS 10
#define CELL_AMT 5

void idCameraDef::buildCamera() {
	int i;
	//int lastSwitch = 0; // TTimo: unused
	idList<float> waits;
	idList<int> targets;

	totalTime = baseTime;
	cameraPosition->setTime( totalTime * 1000 );
	// we have a base time layout for the path and the target path
	// now we need to layer on any wait or speed changes
	for ( i = 0; i < events.Num(); i++ ) {
		//idCameraEvent *ev = events[i]; // TTimo: unused
		events[i]->setTriggered( false );
		switch ( events[i]->getType() ) {
		case idCameraEvent::EVENT_TARGET: {
			targets.Append( i );
			break;
		}
		case idCameraEvent::EVENT_FEATHER: {
			long featherStart = 0;
			float speed = 0;
			long loopTime = 10;
			float stepGoal = cameraPosition->getBaseVelocity() / ( 1000 / loopTime );
			while ( featherStart <= 1000 ) {
				cameraPosition->addVelocity( featherStart, loopTime, speed );
				speed += stepGoal;
				if ( speed > cameraPosition->getBaseVelocity() ) {
					speed = cameraPosition->getBaseVelocity();
				}
				featherStart += loopTime;
			}

			// TTimo gcc warns: assignment to `long int' from `float'
			// more efficient to do (long int)(totalTime) * 1000 - 1000
			// safer to (long int)(totalTime * 1000 - 1000)
			featherStart = ( long int )( totalTime * 1000 - 1000 );
			long endTime = featherStart + 1000;
			speed = cameraPosition->getBaseVelocity();
			while ( featherStart < endTime ) {
				speed -= stepGoal;
				if ( speed < 0 ) {
					speed = 0;
				}
				cameraPosition->addVelocity( featherStart, loopTime, speed );
				featherStart += loopTime;
			}
			break;

		}
		case idCameraEvent::EVENT_WAIT: {
			waits.Append( atof( events[i]->getParam() ) );

			//FIXME: this is quite hacky for Wolf E3, accel and decel needs
			// do be parameter based etc..
			long waitStart = events[i]->getTime() - 1000;
			if ( waitStart < 0 ) {
				waitStart = 0;
			}
			float speed = cameraPosition->getBaseVelocity();
			long loopTime = 10;
			float steps = speed / ( ( events[i]->getTime() - waitStart ) / loopTime );
			while ( waitStart <= events[i]->getTime() - loopTime ) {
				cameraPosition->addVelocity( waitStart, loopTime, speed );
				speed -= steps;
				waitStart += loopTime;
			}
			cameraPosition->addVelocity( events[i]->getTime(), atof( events[i]->getParam() ) * 1000, 0 );

			waitStart = ( long int )( events[i]->getTime() + atof( events[i]->getParam() ) * 1000 );
			long endTime = waitStart + 1000;
			speed = 0;
			while ( waitStart <= endTime ) {
				cameraPosition->addVelocity( waitStart, loopTime, speed );
				speed += steps;
				waitStart += loopTime;
			}
			break;
		}
		case idCameraEvent::EVENT_TARGETWAIT: {
			//targetWaits.Append(i);
			break;
		}
		case idCameraEvent::EVENT_SPEED: {
/*
				// take the average delay between up to the next five segments
				float adjust = atof(events[i]->getParam());
				int index = events[i]->getSegment();
				total = 0;
				count = 0;

				// get total amount of time over the remainder of the segment
				for (j = index; j < cameraSpline.numSegments() - 1; j++) {
					total += cameraSpline.getSegmentTime(j + 1) - cameraSpline.getSegmentTime(j);
					count++;
				}

				// multiply that by the adjustment
				double newTotal = total * adjust;
				// what is the difference..
				newTotal -= total;
				totalTime += newTotal / 1000;

				// per segment difference
				newTotal /= count;
				int additive = newTotal;

				// now propogate that difference out to each segment
				for (j = index; j < cameraSpline.numSegments(); j++) {
					cameraSpline.addSegmentTime(j, additive);
					additive += newTotal;
				}
				break;
*/
		default:
			break;
		}
		}
	}


	for ( i = 0; i < waits.Num(); i++ ) {
		totalTime += waits[i];
	}

	// on a new target switch, we need to take time to this point ( since last target switch )
	// and allocate it across the active target, then reset time to this point
	long timeSoFar = 0;
	long total = ( long int )( totalTime * 1000 );
	for ( i = 0; i < targets.Num(); i++ ) {
		long t;
		if ( i < targets.Num() - 1 ) {
			t = events[targets[i + 1]]->getTime();
		} else {
			t = total - timeSoFar;
		}
		// t is how much time to use for this target
		setActiveTargetByName( events[targets[i]]->getParam() );
		getActiveTarget()->setTime( t );
		timeSoFar += t;
	}


}

void idCameraDef::startCamera( long t ) {
	cameraPosition->clearVelocities();
	cameraPosition->start( t );
	buildCamera();
	fov.reset( 90, 90, t, 0 );
	//for (int i = 0; i < targetPositions.Num(); i++) {
	//	targetPositions[i]->
	//}
	startTime = t;
	cameraRunning = true;
}


void idCameraDef::parse( idLexer &src ) {

	idToken token;
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Icmp( token, "}" ) ) {
			break;
		}

		if ( idStr::Icmp( token, "time" ) == 0 ) {
			baseTime = src.ParseFloat();
		} else if ( idStr::Icmp( token, "camera_fixed" ) == 0 )        {
			cameraPosition = new idFixedPosition();
			cameraPosition$1parse( src )->$1parse( src );
		} else if ( idStr::Icmp( token, "camera_interpolated" ) == 0 )        {
			cameraPosition = new idInterpolatedPosition();
			cameraPosition$1parse( src )->$1parse( src );
		} else if ( idStr::Icmp( token, "camera_spline" ) == 0 )        {
			cameraPosition = new idSplinePosition();
			cameraPosition$1parse( src )->$1parse( src );
		} else if ( idStr::Icmp( token, "target_fixed" ) == 0 )        {
			idFixedPosition *pos = new idFixedPosition();
			pos$1parse( src )->$1parse( src );
			targetPositions.Append( pos );
		} else if ( idStr::Icmp( token, "target_interpolated" ) == 0 )        {
			idInterpolatedPosition *pos = new idInterpolatedPosition();
			pos$1parse( src )->$1parse( src );
			targetPositions.Append( pos );
		} else if ( idStr::Icmp( token, "target_spline" ) == 0 )        {
			idSplinePosition *pos = new idSplinePosition();
			pos$1parse( src )->$1parse( src );
			targetPositions.Append( pos );
		} else if ( idStr::Icmp( token, "fov" ) == 0 )        {
			fov$1parse( src ).$1parse( src );
		} else if ( idStr::Icmp( token, "event" ) == 0 )        {
			idCameraEvent *event = new idCameraEvent();
			event$1parse( src )->$1parse( src );
			addEvent( event );
		}


	} while ( 1 );

	if ( !cameraPosition ) {
		Com_Printf( "no camera position specified\n" );
		// prevent a crash later on
		cameraPosition = new idFixedPosition();
	}
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );

}

bool idCameraDef::load( const char *filename ) {
	char *buf;
	int len = FS_ReadFile( filename, (void **)&buf );
	if ( !buf || len <= 0 ) {
		return false;
	}

	clear();
	idLexer src;
	src.LoadMemory( buf, len, filename );
	parse( src );
	FS_FreeFile( buf );

	return true;
}

void idCameraDef::save( const char *filename ) {
	fileHandle_t file = FS_FOpenFileWrite( filename );
	if ( file ) {
		int i;
		idStr s = "cameraPathDef { \n";
		FS_Write( s.c_str(), s.length(), file );
		s = va( "\ttime %f\n", baseTime );
		FS_Write( s.c_str(), s.length(), file );

		cameraPosition->write( file, va( "camera_%s",cameraPosition->typeStr() ) );

		for ( i = 0; i < numTargets(); i++ ) {
			targetPositions[i]->write( file, va( "target_%s", targetPositions[i]->typeStr() ) );
		}

		for ( i = 0; i < events.Num(); i++ ) {
			events[i]->write( file, "event" );
		}

		fov.write( file, "fov" );

		s = "}\n";
		FS_Write( s.c_str(), s.length(), file );
	}
	FS_FCloseFile( file );
}

int idCameraDef::sortEvents( const void *p1, const void *p2 ) {
	idCameraEvent *ev1 = ( idCameraEvent* )( p1 );
	idCameraEvent *ev2 = ( idCameraEvent* )( p2 );

	if ( ev1->getTime() > ev2->getTime() ) {
		return -1;
	}
	if ( ev1->getTime() < ev2->getTime() ) {
		return 1;
	}
	return 0;
}

void idCameraDef::addEvent( idCameraEvent *event ) {
	events.Append( event );
	//events.Sort(&sortEvents);

}
void idCameraDef::addEvent( idCameraEvent::eventType t, const char *param, long time ) {
	addEvent( new idCameraEvent( t, param, time ) );
	buildCamera();
}


const char *idCameraEvent::eventStr[] = {
	"NA",
	"WAIT",
	"TARGETWAIT",
	"SPEED",
	"TARGET",
	"SNAPTARGET",
	"FOV",
	"CMD",
	"TRIGGER",
	"STOP",
	"CAMERA",
	"FADEOUT",
	"FADEIN",
	"FEATHER"
};

void idCameraEvent::parse( idLexer &src ) {
	idToken token;
	src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Cmp( token, "(" ) || !idStr::Cmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "type" ) == 0 ) {
				type = static_cast<idCameraEvent::eventType>( $1( token.c_str() )atoi$1( token.c_str() ) );
			} else if ( idStr::Icmp( key.c_str(), "param" ) == 0 ) {
				paramStr = token.c_str();
			} else if ( idStr::Icmp( key.c_str(), "time" ) == 0 ) {
				time = $1( token.c_str() )atoi$1( token.c_str() );
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
}

void idCameraEvent::write( fileHandle_t file, const char *name ) {
	idStr s = va( "\t%s {\n", name );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\ttype %d\n", static_cast<int>( type ) );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\tparam \"%s\"\n", paramStr.c_str() );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\ttime %d\n", time );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


const char *idCameraPosition::positionStr[] = {
	"Fixed",
	"Interpolated",
	"Spline",
};



const idVec3 *idInterpolatedPosition::getPosition( long t ) {
	static idVec3 interpolatedPos;
	float percent = 0.0;
	float velocity = getVelocity( t );
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	idVec3 temp = startPos;
	temp -= endPos;
	float distance = temp.Length();

	distSoFar += distToTravel;

	// TTimo
	// show_bug.cgi?id=409
	// avoid NaN on fixed cameras
	if ( distance != 0.0 ) {   //DAJ added to protect DBZ
		percent = (float)( distSoFar ) / distance;
	}

	if ( percent > 1.0 ) {
		percent = 1.0;
	} else if ( percent < 0.0 ) {
		percent = 0.0;
	}

	// the following line does a straigt calc on percentage of time
	// float percent = (float)(startTime + time - t) / time;

	idVec3 v1 = startPos;
	idVec3 v2 = endPos;
	v1 *= ( 1.0 - percent );
	v2 *= percent;
	v1 += v2;
	interpolatedPos = v1;
	return &interpolatedPos;
}


void idCameraFOV::parse( idLexer &src ) {
	idToken token;
	src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Cmp( token, "(" ) || !idStr::Cmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "fov" ) == 0 ) {
				fov = $1( token.c_str() )atof$1( token.c_str() );
			} else if ( idStr::Icmp( key.c_str(), "startFOV" ) == 0 ) {
				startFOV = $1( token.c_str() )atof$1( token.c_str() );
			} else if ( idStr::Icmp( key.c_str(), "endFOV" ) == 0 ) {
				endFOV = $1( token.c_str() )atof$1( token.c_str() );
			} else if ( idStr::Icmp( key.c_str(), "time" ) == 0 ) {
				time = $1( token.c_str() )atoi$1( token.c_str() );
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
}

bool idCameraPosition::parseToken( const char *key, idLexer &src ) {
	idToken token;
	if ( !src.ReadToken( &token ) ) {
		token.Empty();
	}
	if ( idStr::Icmp( key, "time" ) == 0 ) {
		time = $1( token.c_str() )atol$1( token.c_str() );
		return true;
	} else if ( idStr::Icmp( key, "type" ) == 0 ) {
		type = static_cast<idCameraPosition::positionType>( $1( token.c_str() )atoi$1( token.c_str() ) );
		return true;
	} else if ( idStr::Icmp( key, "velocity" ) == 0 ) {
		long t = $1( token.c_str() )atol$1( token.c_str() );
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
		long d = $1( token.c_str() )atol$1( token.c_str() );
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
		float s = $1( token.c_str() )atof$1( token.c_str() );
		addVelocity( t, d, s );
		return true;
	} else if ( idStr::Icmp( key, "baseVelocity" ) == 0 ) {
		baseVelocity = $1( token.c_str() )atof$1( token.c_str() );
		return true;
	} else if ( idStr::Icmp( key, "name" ) == 0 ) {
		name = token.c_str();
		return true;
	} else if ( idStr::Icmp( key, "time" ) == 0 ) {
		time = $1( token.c_str() )atoi$1( token.c_str() );
		return true;
	}
$1src.UnreadToken( &token );	$1src.UnreadToken( &token );
	return false;
}



void idFixedPosition::parse( idLexer &src ) {
	idToken token;
	src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Cmp( token, "(" ) || !idStr::Cmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "pos" ) == 0 ) {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				src.Parse1DMatrix( 3, $1.ToFloatPtr() );possrc.Parse1DMatrix( 3, $1.ToFloatPtr() );
			} else {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				idCameraPosition::parseToken( key.c_str(), src );
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
}

void idInterpolatedPosition::parse( idLexer &src ) {
	idToken token;
	src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Cmp( token, "(" ) || !idStr::Cmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "startPos" ) == 0 ) {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				src.Parse1DMatrix( 3, $1.ToFloatPtr() );startPossrc.Parse1DMatrix( 3, $1.ToFloatPtr() );
			} else if ( idStr::Icmp( key.c_str(), "endPos" ) == 0 ) {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				src.Parse1DMatrix( 3, $1.ToFloatPtr() );endPossrc.Parse1DMatrix( 3, $1.ToFloatPtr() );
			} else {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				idCameraPosition::parseToken( key.c_str(), src );
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
}


void idSplinePosition::parse( idLexer &src ) {
	idToken token;
	src.ExpectTokenString( "$1" );{src.ExpectTokenString( "$1" );
	do {
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}		$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		if ( !token[0] ) {
			break;
		}
		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

		// here we may have to jump over brush epairs ( only used in editor )
		do {
			// if token is not a brace, it is a key for a key/value pair
			if ( !token[0] || !idStr::Cmp( token, "(" ) || !idStr::Cmp( token, "}" ) ) {
				break;
			}
$1src.UnreadToken( &token );
			$1src.UnreadToken( &token );
$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}			$1idToken key;
$1if ( !src.ReadTokenOnLine( &key ) ) {
$1	key.Empty();
$1}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}
			if ( idStr::Icmp( key.c_str(), "target" ) == 0 ) {
				target$1parse( src ).$1parse( src );
			} else {
$1src.UnreadToken( &token );				$1src.UnreadToken( &token );
				idCameraPosition::parseToken( key.c_str(), src );
			}
$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}			$1if ( !src.ReadToken( &token ) ) {
$1	token.Empty();
$1}

		} while ( 1 );

		if ( !idStr::Cmp( token, "}" ) ) {
			break;
		}

	} while ( 1 );
$1src.UnreadToken( &token );
	$1src.UnreadToken( &token );
	src.ExpectTokenString( "$1" );}src.ExpectTokenString( "$1" );
}



void idCameraFOV::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tfov %f\n", fov );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tstartFOV %f\n", startFOV );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tendFOV %f\n", endFOV );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\ttime %i\n", time );
	FS_Write( s.c_str(), s.length(), file );

	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}


void idCameraPosition::write( fileHandle_t file, const char *p ) {

	idStr s = va( "\t\ttime %i\n", time );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\ttype %i\n", static_cast<int>( type ) );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tname %s\n", name.c_str() );
	FS_Write( s.c_str(), s.length(), file );

	s = va( "\t\tbaseVelocity %f\n", baseVelocity );
	FS_Write( s.c_str(), s.length(), file );

	for ( int i = 0; i < velocities.Num(); i++ ) {
		s = va( "\t\tvelocity %i %i %f\n", velocities[i]->startTime, velocities[i]->time, velocities[i]->speed );
		FS_Write( s.c_str(), s.length(), file );
	}

}

void idFixedPosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	s = va( "\t\tpos ( %f %f %f )\n", pos.x, pos.y, pos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idInterpolatedPosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	s = va( "\t\tstartPos ( %f %f %f )\n", startPos.x, startPos.y, startPos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = va( "\t\tendPos ( %f %f %f )\n", endPos.x, endPos.y, endPos.z );
	FS_Write( s.c_str(), s.length(), file );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idSplinePosition::write( fileHandle_t file, const char *p ) {
	idStr s = va( "\t%s {\n", p );
	FS_Write( s.c_str(), s.length(), file );
	idCameraPosition::write( file, p );
	target.write( file, "target" );
	s = "\t}\n";
	FS_Write( s.c_str(), s.length(), file );
}

void idCameraDef::addTarget( const char *n, idCameraPosition::positionType type ) {
	// TTimo: unused
	//const char *text = (name == NULL) ? va("target0%d", numTargets()+1) : name;
	idCameraPosition *pos = newFromType( type );
	if ( pos ) {
		pos->setName( n );
		targetPositions.Append( pos );
		activeTarget = numTargets() - 1;
		if ( activeTarget == 0 ) {
			// first one
			addEvent( idCameraEvent::EVENT_TARGET, n, 0 );
		}
	}
}

const idVec3 *idSplinePosition::getPosition( long t ) {
	static idVec3 interpolatedPos;

	float velocity = getVelocity( t );
	float timePassed = t - lastTime;
	lastTime = t;

	// convert to seconds
	timePassed /= 1000;

	float distToTravel = timePassed * velocity;

	distSoFar += distToTravel;
	double tempDistance = target.totalDistance();

	double percent = (double)( distSoFar ) / tempDistance;

	double targetDistance = percent * tempDistance;
	tempDistance = 0;

	double lastDistance1,lastDistance2;
	lastDistance1 = lastDistance2 = 0;
	//FIXME: calc distances on spline build
	idVec3 temp;
	int count = target.numSegments();
	// TTimo fixed MSVCism
	int i;
	for ( i = 1; i < count; i++ ) {
		temp = *target.getSegmentPoint( i - 1 );
		temp -= *target.getSegmentPoint( i );
		tempDistance += temp.Length();
		if ( i & 1 ) {
			lastDistance1 = tempDistance;
		} else {
			lastDistance2 = tempDistance;
		}
		if ( tempDistance >= targetDistance ) {
			break;
		}
	}

	if ( i >= count - 1 ) {
		interpolatedPos = *target.getSegmentPoint( i - 1 );
	} else {
#if 0
		double timeHi = target.getSegmentTime( i + 1 );
		double timeLo = target.getSegmentTime( i - 1 );
		double percent = ( timeHi - t ) / ( timeHi - timeLo );
		idVec3 v1 = *target.getSegmentPoint( i - 1 );
		idVec3 v2 = *target.getSegmentPoint( i + 1 );
		v2 *= ( 1.0 - percent );
		v1 *= percent;
		v2 += v1;
		interpolatedPos = v2;
#else
		if ( lastDistance1 > lastDistance2 ) {
			double d = lastDistance2;
			lastDistance2 = lastDistance1;
			lastDistance1 = d;
		}

		idVec3 v1 = *target.getSegmentPoint( i - 1 );
		idVec3 v2 = *target.getSegmentPoint( i );
		double frac = ( lastDistance2 - targetDistance ) / ( lastDistance2 - lastDistance1 );
		v2 *= ( 1.0 - frac );
		v1 *= frac;
		v2 += v1;
		interpolatedPos = v2;
#endif
	}
	return &interpolatedPos;

}

