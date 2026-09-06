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


// cg_flamethrower.c - special code for the flamethrower effects
//
//	the flameChunks behave similarly to the trailJunc's, except they are rendered differently, and
//	also interact with the environment
//
// NOTE: some AI's are treated different, mostly for aesthetical reasons.

#include "FxScheduler.h"
#include "cg_fx.h"

class CWolfFlameStream;

// a flameChunk is a ball or section of fuel which goes from fuel->blue ignition->flame ball
// optimization is necessary, since lots of these will be spawned, but as they grow, they can be
// merged so that less overdraw occurs
typedef struct flameChunk_s
{
	CWolfFlameStream *stream;
	struct flameChunk_s *nextFlameChunk;            // next junction in the trail

	qboolean inuse;
	qboolean dead;          // set when a chunk is effectively inactive, but waiting to be freed
	int ownerCent;              // cent that spawned us

	int timeStart, timeEnd;
	float sizeStart, sizeMax;               // start small, increase if we slow down
	float sizeRand;
	float sizeRate;                 // rate per ms, variable according to speed (larger if moving slower)
	int sizeTime;
	vec3_t baseOrg;
	int baseOrgTime;
	vec3_t velDir;
	float velSpeed;                 // flame chunks should start with a fast velocity, then slow down if there is nothing behind them pushing them along
	float rollAngle;
	qboolean ignitionOnly;
	int blueLife;
	float gravity;
	vec3_t startVelDir;
	float speedScale;

	// current variables
	vec3_t org;
	float size;
	float lifeFrac;                 // 0.0 (baby) -> 1.0 (aged)

	int lastFriction, lastFrictionTake;
	vec3_t parentFwd;
} flameChunk_t;

// Stream ownership is handled by FX. This index is only for sound updates.
static std::list<CWolfFlameStream *> fxFlameStreams;

class CWolfFlameStream : public CEffect {
    std::list<flameChunk_t *> chunks;
    std::list<CWolfFlameStream *>::iterator entry;
public:
    flameChunk_t *head;
    CWolfFlameStream() : head( NULL ) {
        SetFlags( 0 );
        fxFlameStreams.push_front( this );
        entry = fxFlameStreams.begin();
    }
    ~CWolfFlameStream();
    flameChunk_t *Append() {
        flameChunk_t *chunk = new flameChunk_t();
        chunk->stream = this;
        chunk->inuse = qtrue;
        chunk->nextFlameChunk = head;
        head = chunk;
        chunks.push_front( chunk );
        return chunk;
    }
    bool Update() override;
};

// this structure stores information relevant to each cent in the game, this way we keep
// the flamethrower data seperate to the rest of the code, which helps if we decide against
// using this weapon in the game
typedef struct centFlameInfo_s
{
	int lastClientFrame;            // client frame that we last fired the flamethrower
	vec3_t lastAngles;              // angles at last firing
	vec3_t lastOrigin;              // origin at last firing
	flameChunk_t
	*lastFlameChunk;                // flame chunk we last spawned
	int lastSoundUpdate;

	qboolean lastFiring;

	qboolean silent;                //----(SA)	added

	int lastDmgUpdate;              // time we last told server about this ent's flame damage
	int lastDmgCheck;               // only check once per 100ms
	int lastDmgEnemy;               // entity that inflicted the damage
} centFlameInfo_t;

static centFlameInfo_t centFlameInfo[MAX_GENTITIES];

typedef struct
{
//	float fireVolume;	// not needed, since we add individual loop sources for the flame, so it gets spacialized
	float blowVolume;
	float streamVolume;
} flameSoundStatus_t;

static flameSoundStatus_t centFlameStatus[MAX_GENTITIES];

// procedure defs
flameChunk_t *CG_SpawnFlameChunk( flameChunk_t *headFlameChunk );
void CG_FlameCalcOrg( flameChunk_t *f, int time, vec3_t outOrg );
void CG_FlameGetMuzzlePoint( vec3_t org, vec3_t fwd, vec3_t right, vec3_t up, vec3_t outPos );

// these must be globals, since they cannot expand or contract, since that might result in them getting
//	stuck in geometry. therefore when a chunk hits a surface, we should deflect it away from the surface
//	slightly, rather than running along it, so that as the chink grows, the sprites don't sink into the
//	wall too much.
static vec3_t flameChunkMins = {-4, -4, -4};
static vec3_t flameChunkMaxs = { 4,  4,  4};

// Continuous motion is Wolf-specific; every authored setting is loaded from FX data.
struct FxFlameSettings {
	float startSize;
	float startMaxSize;
	float startMaxSizeRandom;
	float maxSize;
	float minMaxSize;
	float startSpeed;
	float minSpeed;
	float chunkDistance;
	float blueLength;
	float blueMaxAlpha;
	float fuelLength;
	float fuelMaxAlpha;
	float fuelMinWidth;
	float rangePadding;
	float frictionScale;
	float fuelFadeIn;
	float blueFadeIn;
	float blueFadeOut;
	float blueGrowth;
	float fireGrowth;
	float spriteStartBlueScale;
	float spriteMinRadius;
	float spriteAspect;
	float rotate;
	float nozzleFrameTime;
	float soundRange;
	float ignitionGravity;
	float rollRange;
	float speedBase;
	float speedWeight;
	float lightMin;
	float lightMax;
	float lightOffset;
	float lightBlue;
	float blueTimeScale;
	float spriteAlphaScale;
	float spriteCullBase;
	float spriteCullWeight;
	float spriteCullSize;
	float lightWeightSize;
	float lightFlicker;
	float lightFlickerX;
	float lightFlickerY;
	float ignitionWidthScale;
	float fuelWidthDivisor;
	float fuelMaxWidthDivisor;
	float Lifetime() const { return (FLAMETHROWER_RANGE + rangePadding) / startSpeed * 1000.0f; }
	float BlueLife() const { return blueLength / startSpeed * 1000.0f; }
	void Load() {
		startSize = CG_FxValue( "wolf/flame_stream", "startSize" );
		startMaxSize = CG_FxValue( "wolf/flame_stream", "startMaxSize" );
		startMaxSizeRandom = CG_FxValue( "wolf/flame_stream", "startMaxSizeRandom" );
		maxSize = CG_FxValue( "wolf/flame_stream", "maxSize" );
		minMaxSize = CG_FxValue( "wolf/flame_stream", "minMaxSize" );
		startSpeed = CG_FxValue( "wolf/flame_stream", "startSpeed" );
		minSpeed = CG_FxValue( "wolf/flame_stream", "minSpeed" );
		chunkDistance = CG_FxValue( "wolf/flame_stream", "chunkDistance" );
		blueLength = CG_FxValue( "wolf/flame_stream", "blueLength" );
		blueMaxAlpha = CG_FxValue( "wolf/flame_stream", "blueMaxAlpha" );
		fuelLength = CG_FxValue( "wolf/flame_stream", "fuelLength" );
		fuelMaxAlpha = CG_FxValue( "wolf/flame_stream", "fuelMaxAlpha" );
		fuelMinWidth = CG_FxValue( "wolf/flame_stream", "fuelMinWidth" );
		rangePadding = CG_FxValue( "wolf/flame_stream", "rangePadding" );
		frictionScale = CG_FxValue( "wolf/flame_stream", "frictionScale" );
		fuelFadeIn = CG_FxValue( "wolf/flame_stream", "fuelFadeIn" );
		blueFadeIn = CG_FxValue( "wolf/flame_stream", "blueFadeIn" );
		blueFadeOut = CG_FxValue( "wolf/flame_stream", "blueFadeOut" );
		blueGrowth = CG_FxValue( "wolf/flame_stream", "blueGrowth" );
		fireGrowth = CG_FxValue( "wolf/flame_stream", "fireGrowth" );
		spriteStartBlueScale = CG_FxValue( "wolf/flame_stream", "spriteStartBlueScale" );
		spriteMinRadius = CG_FxValue( "wolf/flame_stream", "spriteMinRadius" );
		spriteAspect = CG_FxValue( "wolf/flame_stream", "spriteAspect" );
		rotate = CG_FxValue( "wolf/flame_stream", "rotate" );
		nozzleFrameTime = CG_FxValue( "wolf/flame_stream", "nozzleFrameTime" );
		soundRange = CG_FxValue( "wolf/flame_stream", "soundRange" );
		ignitionGravity = CG_FxValue( "wolf/flame_stream", "ignitionGravity" );
		rollRange = CG_FxValue( "wolf/flame_stream", "rollRange" );
		speedBase = CG_FxValue( "wolf/flame_stream", "speedBase" );
		speedWeight = CG_FxValue( "wolf/flame_stream", "speedWeight" );
		lightMin = CG_FxValue( "wolf/flame_stream", "lightMin" );
		lightMax = CG_FxValue( "wolf/flame_stream", "lightMax" );
		lightOffset = CG_FxValue( "wolf/flame_stream", "lightOffset" );
		lightBlue = CG_FxValue( "wolf/flame_stream", "lightBlue" );
		blueTimeScale = CG_FxValue( "wolf/flame_stream", "blueTimeScale" );
		spriteAlphaScale = CG_FxValue( "wolf/flame_stream", "spriteAlphaScale" );
		spriteCullBase = CG_FxValue( "wolf/flame_stream", "spriteCullBase" );
		spriteCullWeight = CG_FxValue( "wolf/flame_stream", "spriteCullWeight" );
		spriteCullSize = CG_FxValue( "wolf/flame_stream", "spriteCullSize" );
		lightWeightSize = CG_FxValue( "wolf/flame_stream", "lightWeightSize" );
		lightFlicker = CG_FxValue( "wolf/flame_stream", "lightFlicker" );
		lightFlickerX = CG_FxValue( "wolf/flame_stream", "lightFlickerX" );
		lightFlickerY = CG_FxValue( "wolf/flame_stream", "lightFlickerY" );
		ignitionWidthScale = CG_FxValue( "wolf/flame_stream", "ignitionWidthScale" );
		fuelWidthDivisor = CG_FxValue( "wolf/flame_stream", "fuelWidthDivisor" );
		fuelMaxWidthDivisor = CG_FxValue( "wolf/flame_stream", "fuelMaxWidthDivisor" );
		if ( startSpeed <= 0 || fireGrowth <= 0 || blueGrowth <= 0 || chunkDistance <= 0 || spriteAspect <= 0 || nozzleFrameTime < 1 ) {
			CG_Error( "FX: invalid flame_stream dimensions or timing" );
		}
	}
};
static FxFlameSettings flameSettings;

static float FlameBlueFadeIn( float time ) { return flameSettings.blueFadeIn * time; }
static float FlameBlueFadeOut( float time ) { return flameSettings.blueFadeOut * time; }
static float FlameBlueSizeSpeed( float size ) { return size / flameSettings.Lifetime() / flameSettings.blueGrowth; }
static float FlameSizeSpeed( float size ) { return size / flameSettings.Lifetime() / flameSettings.fireGrowth; }

/*
===============
CG_FlameLerpVec
===============
*/
void CG_FlameLerpVec( const vec3_t oldV, const vec3_t newV, float backLerp, vec3_t outV ) {
	VectorScale( newV, ( 1.0 - backLerp ), outV );
	VectorMA( outV, backLerp, oldV, outV );
}

/*
===============
CG_FlameAdjustSpeed
===============
*/
void CG_FlameAdjustSpeed( flameChunk_t *f, float change ) {
	if ( !f->velSpeed && !change ) {
		return;
	}

	f->velSpeed += change;
	if ( f->velSpeed < flameSettings.minSpeed ) {
		f->velSpeed = flameSettings.minSpeed;
	}
/*
	f->sizeMax = flameSettings.startMaxSize + ((1.0 - (f->velSpeed / flameSettings.startSpeed)) * (flameSettings.maxSize - flameSettings.startMaxSize));

	if (!f->ignitionOnly) {
		if ((cg.time - f->timeStart) < f->blueLife) {
			f->sizeRate = FlameBlueSizeSpeed(f->sizeMax);
		} else {
			f->sizeMax += f->sizeRand;
			if (f->sizeMax < flameSettings.minMaxSize)
				f->sizeMax = flameSettings.minMaxSize;
			f->sizeRate = FlameSizeSpeed(f->sizeMax);
		}
	}
*/
}

/*
===============
CG_FireFlameChunks

  The given entity is firing a flamethrower

  flags:
	1 - use 'angles'
	2 - silent
===============
*/
void CG_FireFlameChunks( centity_t *cent, vec3_t origin, vec3_t angles, float speedScale, qboolean firing, int flags ) {
	centFlameInfo_t *centInfo;
	flameChunk_t *f, *of;
	vec3_t lastFwd, thisFwd, fwd;
	vec3_t lastUp, thisUp, up;
	vec3_t lastRight, thisRight, right;
	vec3_t thisOrg, lastOrg, org;
	double timeInc, backLerp, fracInc;
	int t;
	double ft;
	trace_t trace;
	vec3_t parentFwd;
	int flameLife;
#define MAX_CHUNKS_PER_CALL     30


	centInfo = &centFlameInfo[cent->currentState.number];

//----(SA)	added
	if ( flags & 2 ) { // silent
		centInfo->silent = qtrue;
	}
//----(SA)	end

	// only do one ignition flame for zombie
	if ( cent->currentState.aiChar == AICHAR_ZOMBIE && !firing ) {
		if ( !centInfo->lastFiring ) {
			return;
		}
	}

	// if this point is in water, no flames
	if ( CG_PointContents( origin, cent->currentState.number ) & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// for any other character or in 3rd person view, use entity angles for friction
	if ( cent->currentState.number != cg.snap->ps.clientNum || cg_thirdPerson.integer ) {
		if ( flags & 1 ) { // use 'angles'
			AngleVectors( angles, parentFwd, NULL, NULL );
		} else {
			AngleVectors( cent->currentState.angles, parentFwd, NULL, NULL );
		}
	} else {
		AngleVectors( angles, parentFwd, NULL, NULL );
	}

	// tweak the speed of the flame
	//if (cent->currentState.aiChar)
	//	speedScale *= 0.8;
	//else
	//	speedScale *= 1.0;

	AngleVectors( angles, thisFwd, thisRight, thisUp );
	VectorCopy( origin, thisOrg );

	// if this entity was firing last frame, interpolate the angles as we spawn the chunks that
	// fired over the last frame
	if (    ( centInfo->lastClientFrame == cent->currentState.frame ) &&
			( centInfo->lastFlameChunk && centInfo->lastFiring == firing ) ) {
		AngleVectors( centInfo->lastAngles, lastFwd, lastRight, lastUp );
		VectorCopy( centInfo->lastOrigin, lastOrg );
		centInfo->lastFiring = firing;

		flameLife = flameSettings.Lifetime() * ( 1.0 / ( 0.5 + 0.5 * speedScale ) );
		of = centInfo->lastFlameChunk;
		timeInc = 1000.0 * ( firing ? 1.0 : 0.5 ) * ( flameSettings.chunkDistance /**(1.0+crandom()*0.1)*/ / ( flameSettings.startSpeed * speedScale ) );
		ft = ( (double)of->timeStart );
		// never spawn too many chunk in one call (framerate preservation)
		if ( ( ( (double)cg.time - ft ) / timeInc ) > MAX_CHUNKS_PER_CALL ) {
			timeInc = ( (double)cg.time - ft ) / MAX_CHUNKS_PER_CALL;
		}
		ft += timeInc;
		t = (int)ft;
		fracInc = timeInc / (double)( cg.time - of->timeStart );
		backLerp = 1.0 - fracInc;
		while ( t <= cg.time ) {
			// spawn a new chunk
			CG_FlameLerpVec( lastOrg, thisOrg, backLerp, org );

			CG_Trace( &trace, org, flameChunkMins, flameChunkMaxs, org, cent->currentState.number, MASK_SHOT );
			if ( trace.startsolid && trace.entityNum >= cgs.maxclients ) {
				centInfo->lastFlameChunk = NULL;
				return;     // don't spawn inside a wall
			}

			f = CG_SpawnFlameChunk( of );

			if ( !f ) {
				CG_Printf( "Out of flame chunks\n" );
				return;
			}

			CG_FlameLerpVec( lastFwd, thisFwd, backLerp, fwd );
			VectorNormalize( fwd );
			CG_FlameLerpVec( lastRight, thisRight, backLerp, right );
			VectorNormalize( right );
			CG_FlameLerpVec( lastUp, thisUp, backLerp, up );
			VectorNormalize( up );

			f->timeStart = t;
			f->timeEnd = t + flameLife; // * (firing ? 1.0 : 0.2);
			f->sizeStart = f->size = flameSettings.startSize * speedScale; // * (firing ? 1.0 : 0.2);
			f->sizeMax = ( flameSettings.speedBase + flameSettings.speedWeight * speedScale ) * ( flameSettings.startMaxSize + f->sizeRand * ( firing ? 1.0 : 0.0 ) );
			//if (cent->currentState.aiChar == AICHAR_ZOMBIE) {
			//	f->sizeRand = crandom() * flameSettings.startMaxSizeRandom;
			//	f->sizeMax = 20;
			//} else {
			f->sizeRand = 0;
			//}
			if ( f->sizeMax > flameSettings.maxSize ) {
				f->sizeMax = flameSettings.maxSize;
			}
			f->sizeRate = FlameBlueSizeSpeed( f->sizeMax * speedScale * ( 1.0 + ( 0.5 * (float)!firing ) ) );
			f->sizeTime = t;
			VectorCopy( org, f->baseOrg );
			f->baseOrgTime = t;
			VectorCopy( fwd, f->velDir );
			VectorCopy( fwd, f->startVelDir );
			f->speedScale = speedScale;
			// randomize the direction slightly
			/*
			if (!firing) {
				f->velDir[0] += 0.03*sin((float)cg.time/134);
				f->velDir[1] += 0.03*sin((float)cg.time/567);
				f->velDir[2] += 0.03*cos((float)cg.time/3435);
			}
			*/
			VectorNormalize( f->velDir );
			f->velSpeed = flameSettings.startSpeed * ( flameSettings.speedBase + flameSettings.speedWeight * speedScale );
			f->ownerCent = cent->currentState.number;
			f->rollAngle = crandom() * flameSettings.rollRange;
			f->ignitionOnly = !firing;
			if ( !firing ) {
				f->gravity = flameSettings.ignitionGravity;
				f->blueLife = flameSettings.BlueLife() * 0.5;
//				f->blueLife = (int)(0.3*(1.0/speedScale)*(float)flameSettings.BlueLife());
			} else {
				f->gravity = 0;
				f->blueLife = flameSettings.BlueLife();
			}
			f->lastFriction = cg.time;
			f->lastFrictionTake = cg.time;
			VectorCopy( parentFwd, f->parentFwd );

			ft += timeInc;
			// always spawn a chunk right on the current time
			if ( (int)ft > cg.time && t < cg.time ) {
				ft = (double)cg.time;
				t = cg.time;
				backLerp = 0;
			} else {
				t = (int)floor( ft );
				backLerp -= fracInc;
			}
			centInfo->lastFlameChunk = of = f;
		}
	} else {

		centInfo->lastFiring = firing;
		t = cg.time;

		VectorCopy( thisOrg, org );

		// if igniting, play start sound
		//if (firing) {
		//	trap_S_StartSound( org, cent->currentState.number, CHAN_AUTO, cgs.media.flameStartSound );
		//}

		CG_Trace( &trace, org, flameChunkMins, flameChunkMaxs, org, cent->currentState.number, MASK_SHOT );
		if ( trace.startsolid && trace.entityNum >= cgs.maxclients ) {
			centInfo->lastFlameChunk = NULL;
			return;     // don't spawn inside a wall
		}

		// just fire a single chunk to get us started
		f = CG_SpawnFlameChunk( NULL );

		if ( !f ) {
			CG_Printf( "Out of flame chunks\n" );
			return;
		}

		VectorCopy( thisFwd, fwd );
		VectorCopy( thisUp, up );
		VectorCopy( thisRight, right );

		f->timeStart = cg.time;
		f->timeEnd = cg.time + flameSettings.Lifetime() * ( 1.0 / ( 0.5 + 0.5 * speedScale ) ); // * (firing ? 1.0 : 0.2);
		//f->timeEnd = cg.time + flameSettings.Lifetime() * (1.0/speedScale);
		f->sizeStart = f->size = flameSettings.startSize * speedScale;
		f->sizeMax = flameSettings.startMaxSize * ( flameSettings.speedBase + flameSettings.speedWeight * speedScale );
		if ( f->sizeMax > flameSettings.maxSize ) {
			f->sizeMax = flameSettings.maxSize;
		}
		//if (cent->currentState.aiChar == AICHAR_ZOMBIE) {
		//	f->sizeRand = crandom() * flameSettings.startMaxSizeRandom;
		//	f->sizeMax = 20;
		//} else {
		f->sizeRand = 0;
		//}
		f->sizeRate = FlameBlueSizeSpeed( f->sizeMax * speedScale );
		f->sizeTime = cg.time;
		VectorCopy( org, f->baseOrg );
		f->baseOrgTime = cg.time;
		VectorCopy( fwd, f->velDir );
		VectorCopy( fwd, f->startVelDir );
		f->velSpeed = flameSettings.startSpeed * ( flameSettings.speedBase + flameSettings.speedWeight * speedScale );
		f->ownerCent = cent->currentState.number;
		f->rollAngle = crandom() * flameSettings.rollRange;
		f->ignitionOnly = !firing;
		f->speedScale = speedScale;
		if ( !firing ) {
			f->gravity = -100;
			f->blueLife = (int)( 0.3 * ( 1.0 / speedScale ) * (float)flameSettings.BlueLife() );
		} else {
			f->gravity = 0;
			f->blueLife = flameSettings.BlueLife();
		}
		f->lastFriction = cg.time;
		f->lastFrictionTake = cg.time;
		VectorCopy( parentFwd, f->parentFwd );

		centInfo->lastFlameChunk = f;
	}

	/*if (centInfo->lastFlameChunk && !cent->currentState.aiChar)*/ {
		// push them along
		float frametime, dot;

		f = centInfo->lastFlameChunk;
		while ( f ) {

			if ( f->lastFriction < cg.time - 50 ) {
				frametime = (float)( cg.time - f->lastFriction ) / 1000.0;
				f->lastFriction = cg.time;
				dot = DotProduct( parentFwd, f->parentFwd );
				if ( dot >= 0.99 ) {
					dot -= 0.99;
					dot *= ( 1.0 / ( 1.0 - 0.99 ) );
					CG_FlameAdjustSpeed( f, /*(0.3+0.7*f->speedScale) **/ 0.5 * frametime * (flameSettings.frictionScale * flameSettings.startSpeed) * pow( dot,4 ) );
				}
			}

			f = f->nextFlameChunk;
		}
	}

	VectorCopy( angles, centInfo->lastAngles );
	VectorCopy( origin, centInfo->lastOrigin );
	centInfo->lastClientFrame = cent->currentState.frame;
}

/*
===============
CG_ClearFlameChunks
===============
*/
void CG_ClearFlameChunks( void ) {
    memset( centFlameInfo, 0, sizeof( centFlameInfo ) );
    memset( centFlameStatus, 0, sizeof( centFlameStatus ) );
}

/*
===============
CG_SpawnFlameChunk
===============
*/
flameChunk_t *CG_SpawnFlameChunk( flameChunk_t *previous ) {
    if ( theFxHelper.mTimeFrozen ) { return NULL; }
    CWolfFlameStream *stream = previous && previous->inuse ? previous->stream : NULL;
    if ( !stream ) {
        stream = new CWolfFlameStream;
        CEffect *effect = stream;
        FX_AddPrimitive( &effect, 24 * 60 * 60 * 1000 );
    }
    return stream->Append();
}

/*
===========
CG_FreeFlameChunk
===========
*/
void CG_FreeFlameChunk( flameChunk_t *chunk ) {
    while ( chunk ) {
        chunk->inuse = qfalse;
        chunk->dead = qtrue;
        chunk = chunk->nextFlameChunk;
    }
}

/*
===============
CG_MergeFlameChunks

  Assumes f1 comes before f2
===============
*/
void CG_MergeFlameChunks( flameChunk_t *f1, flameChunk_t *f2 ) {
	if ( f1->nextFlameChunk != f2 ) {
		CG_Error( "CG_MergeFlameChunks: f2 doesn't follow f1, cannot merge\n" );
	}

	f1->nextFlameChunk = f2->nextFlameChunk;
	f2->nextFlameChunk = NULL;

	//VectorAdd( f1->velDir, f2->velDir, f1->velDir );
	//VectorScale( f1->velDir, 0.5, f1->velDir );
	VectorCopy( f2->velDir, f1->velDir );

	//VectorAdd( f1->baseOrg, f2->baseOrg, f1->baseOrg );
	//VectorScale( f1->baseOrg, 0.5, f1->baseOrg );
	VectorCopy( f2->baseOrg, f1->baseOrg );
	f1->baseOrgTime = f2->baseOrgTime;

	f1->velSpeed = f2->velSpeed; //(f1->velSpeed + f2->velSpeed) / 2.0;
	f1->sizeMax = f2->sizeMax; //(f1->sizeMax + f2->sizeMax) / 2.0;
	f1->size = f2->size; //(f1->size + f2->size) / 2.0;
	f1->timeStart = f2->timeStart;
	f1->timeEnd = f2->timeEnd;

	CG_FreeFlameChunk( f2 );
}

/*
===============
CG_FlameCalcOrg
===============
*/
void CG_FlameCalcOrg( flameChunk_t *f, int time, vec3_t outOrg ) {
	VectorMA( f->baseOrg, f->velSpeed * ( (float)( time - f->baseOrgTime ) / 1000 ), f->velDir, outOrg );
	outOrg[2] -= f->gravity * ( (float)( time - f->timeStart ) / 1000.0 ) * ( (float)( time - f->timeStart ) / 1000.0 );
}

void CG_AdjustFlameSize( flameChunk_t *f, int t ) {
	// adjust size
	if ( f->size < f->sizeMax ) {
		//f->sizeMax = flameSettings.startMaxSize + ((1.0 - (f->velSpeed / flameSettings.startSpeed)) * (flameSettings.maxSize - flameSettings.startMaxSize));
		if ( ( t - f->timeStart ) < f->blueLife ) {
			f->sizeRate = FlameBlueSizeSpeed( flameSettings.startMaxSize );  // use a constant so the blue flame doesn't distort
		} else {
			f->sizeRate = FlameSizeSpeed( f->sizeMax );
		}

		f->size += f->sizeRate * (float)( t - f->sizeTime );
		if ( f->size > f->sizeMax ) {
			f->size = f->sizeMax;
		}
	}
	f->sizeTime = t;
}

/*
===============
CG_MoveFlameChunk
===============
*/
void CG_MoveFlameChunk( flameChunk_t *f, int t ) {
	vec3_t newOrigin, sOrg;
	trace_t trace;
	int jiggleCount;
	float dot;
	//static vec3_t	umins = {-1,-1,-1}; // TTimo: unused
	//static vec3_t	umaxs = { 1, 1, 1}; // TTimo: unused

	// subtract friction from speed
	if ( f->velSpeed > 1 && f->lastFrictionTake < t - 50 ) {
		CG_FlameAdjustSpeed( f, /*(0.3+0.7*f->speedScale) **/ -( (float)( t - f->lastFrictionTake ) / 1000.0 ) * (flameSettings.frictionScale * flameSettings.startSpeed) );
		f->lastFrictionTake = t;
	}
/*
	// adjust size
	if (f->size < f->sizeMax) {
		//f->sizeMax = flameSettings.startMaxSize + ((1.0 - (f->velSpeed / flameSettings.startSpeed)) * (flameSettings.maxSize - flameSettings.startMaxSize));
		if ((t - f->timeStart) < f->blueLife)
			f->sizeRate = FlameBlueSizeSpeed(flameSettings.startMaxSize);	// use a constant so the blue flame doesn't distort
		else
			f->sizeRate = FlameSizeSpeed(f->sizeMax);

		f->size += f->sizeRate * (float)(t - f->baseOrgTime);
		if (f->size > f->sizeMax) {
			f->size = f->sizeMax;
		}
	}
*/

	jiggleCount = 0;
	VectorCopy( f->baseOrg, sOrg );
	while ( f->velSpeed > 1 && ( ( t - f->baseOrgTime ) > 20 ) ) {
		CG_FlameCalcOrg( f, t, newOrigin );

		//if (cg_entities[f->ownerCent].currentState.aiChar != AICHAR_ZOMBIE) {
		// trace a line from previous position to new position
		CG_Trace( &trace, sOrg, flameChunkMins, flameChunkMaxs, newOrigin, f->ownerCent, MASK_SHOT );
		//} else {
		//	trace.startsolid = 0;
		//	trace.surfaceFlags = 0;
		//	trace.fraction = 1.0;
		//	VectorCopy( newOrigin, trace.endpos );
		//}

		if ( trace.startsolid ) {
			// if it's young, let it go through solids briefly
			if ( f->lifeFrac < 0.4 ) {
				trace.fraction = 1.0;
			} else {
				f->velSpeed = 0;
				break;
			}
		}

		if ( trace.surfaceFlags & SURF_NOIMPACT ) {
			break;
		}

		// moved some distance
		VectorCopy( trace.endpos, f->baseOrg );
		f->baseOrgTime += (int)( (float)( t - f->baseOrgTime ) * trace.fraction );

		if ( trace.fraction == 1.0 ) {
			// check for hitting client
			if ( ( f->ownerCent != cg.snap->ps.clientNum ) && !( cg.snap->ps.eFlags & EF_DEAD ) && VectorDistance( newOrigin, cg.snap->ps.origin ) < 32 ) {
				VectorNegate( f->velDir, trace.plane.normal );
			} else {
				break;
			}
		}

		// reflect off surface
		dot = DotProduct( f->velDir, trace.plane.normal );
		VectorMA( f->velDir, -1.3 * dot, trace.plane.normal, f->velDir );
		VectorNormalize( f->velDir );
		// subtract some speed
		f->velSpeed *= 0.5 * ( 0.25 + 0.75 * ( ( dot + 1.0 ) * 0.5 ) );
		if ( f->velSpeed > 20 ) {
			f->velSpeed = 20;
		}
		// adjust size
		//f->sizeMax = flameSettings.startMaxSize + ((/*1.0 -*/ (f->velSpeed / flameSettings.startSpeed)) * (flameSettings.maxSize - flameSettings.startMaxSize));
		//f->sizeRate = FlameSizeSpeed(f->sizeMax);
		VectorCopy( f->velDir, f->parentFwd );

		VectorCopy( f->baseOrg, sOrg );
	}

	CG_FlameCalcOrg( f, t, f->org );
	//f->baseOrgTime = t;	// incase we skipped the movement
}

/*
===============
CG_AddFlameSpriteToScene
===============
*/
static vec3_t vright, vup;
static vec3_t rright, rup;

static std::vector<qhandle_t> flameShaders;
static std::vector<qhandle_t> nozzleShaders;

#define MAX_CLIPPED_FLAMES  2       // dont draw more than this many per frame
static int numClippedFlames;

void CG_FlameDamage( int owner, vec3_t org, float radius ) {
	centity_t   *cent;
	int i;
	trace_t tr;
	static vec3_t lastPos;
	static int lastTime;

	if ( cg.time == lastTime ) {
		if ( VectorDistance( org, lastPos ) < radius ) {
			return;
		}
	}
	lastTime = cg.time;
	VectorCopy( org, lastPos );

	// check for damage this flame is inflicting on something
	if ( radius < 24 ) {
		radius = 24;
	}

	// check the local playing client
	if ( ( centFlameInfo[cg.snap->ps.clientNum].lastDmgCheck < cg.time - 100 ) &&
		 ( Distance( org, cg.snap->ps.origin ) < radius * ( cg.snap->ps.clientNum == owner ? 0.1 : 1.0 ) ) ) {
		// trace to make sure it's not going through geometry
		CG_Trace( &tr, org, NULL, NULL, cg.snap->ps.origin, -1, MASK_SHOT );
		//
		if ( tr.fraction == 1.0 || tr.entityNum == cg.snap->ps.clientNum ) {
			centFlameInfo[cg.snap->ps.clientNum].lastDmgCheck = cg.time;
			centFlameInfo[cg.snap->ps.clientNum].lastDmgEnemy = owner;
		}
	}

	if ( cgs.localServer && cgs.gametype == GT_SINGLE_PLAYER ) {

		// flaming zombie's only hurt the player
		//if (cg_entities[owner].currentState.aiChar == AICHAR_ZOMBIE)
		//	return;

		// check for AI's getting hurt (TODO: bot support?)
		for ( cent = cg_entities, i = 0; i < cgs.maxclients; cent++, i++ ) {
			if ( cent->currentState.aiChar &&
				 cent->currentState.aiChar != AICHAR_VENOM &&   // Venom has no flame damage
				 centFlameInfo[i].lastDmgCheck < ( cg.time - 100 ) &&
				 cent->currentValid &&  // is in the visible frame
				 ( Distance( org, cent->lerpOrigin ) < radius ) ) {
				// trace to make sure it's not going through geometry
				CG_Trace( &tr, org, NULL, NULL, cent->lerpOrigin, -1, MASK_SHOT );
				//
				if ( tr.fraction == 1.0 || tr.entityNum == cent->currentState.number ) {
					centFlameInfo[i].lastDmgCheck = cg.time;
					centFlameInfo[i].lastDmgEnemy = owner;
				}
			}
		}
	}
}

void CG_AddFlameSpriteToScene( flameChunk_t *f, float lifeFrac, float alpha ) {
	vec3_t point, p2, projVec, sProj;
	polyVert_t verts[4];
	float radius, sdist, x;
	int frameNum;
	vec3_t vec, rotate_ang;
	unsigned char alphaChar;
	vec2_t fovRadius;
	vec2_t rdist, rST;
	int rollAngleClamped;
	static vec3_t lastPos;

	CG_FlameDamage( f->ownerCent, f->org, f->size );

	if ( alpha < 0 ) {
		return; // we dont want to see this

	}
	radius = ( f->size / flameSettings.fuelWidthDivisor );
	if ( radius < flameSettings.spriteMinRadius ) {
		radius = flameSettings.spriteMinRadius;
	}
	rST[0] = radius * 1.0;
	rST[1] = radius * 1.0 / flameSettings.spriteAspect;
	alphaChar = ( unsigned char )( 255.0 * alpha );

	verts[0].modulate[0] = alphaChar;
	verts[0].modulate[1] = alphaChar;
	verts[0].modulate[2] = alphaChar;
	verts[0].modulate[3] = alphaChar;
	verts[1] = verts[0];
	verts[2] = verts[0];
	verts[3] = verts[0];

	// find the projected distance from the eye to the projection of the flame origin
	// onto the view direction vector
	VectorMA( cg.refdef.vieworg, 1024, cg.refdef.viewaxis[0], p2 );
	ProjectPointOntoVector( f->org, cg.refdef.vieworg, p2, sProj );

	// make sure its infront of us
	VectorSubtract( sProj, cg.refdef.vieworg, vec );
	sdist = VectorNormalize( vec );
	if ( !sdist || DotProduct( vec, cg.refdef.viewaxis[0] ) < 0 ) {
		return;
	}

	// if we are "inside" this sprite, clip it to our view frustum
	if ( sdist < f->size * 0.6 ) {
		// clip the sprite to the viewport, avoiding rendering off-screen pixels which
		// is the main cause of slow-downs

		// if it's close to the previous sprite, ignore
		//if (VectorDistance(lastPos, f->org) < 6)
		//	return;

		if ( ( sdist < f->size * 0.6 ) && ( numClippedFlames++ > MAX_CLIPPED_FLAMES ) ) {
			return;
		}

		// OPTIMIZATION: clamp to 90 degree rotations
		rollAngleClamped = 0;
		if ( flameSettings.rotate ) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += ( rollAngleClamped = ( (int)( f->rollAngle ) / 90 ) * 90 );
			AngleVectors( rotate_ang, NULL, rright, rup );
		} else {
			VectorCopy( vright, rright );
			VectorCopy( vup, rup );
		}

		VectorSubtract( sProj, f->org, projVec );

		// find the distances from the sprite origin to the projection along the refdef axis
		rdist[0] = -1.0 * DotProduct( rright, projVec );
		rdist[1] = -1.0 * DotProduct( rup, projVec );

		if ( fabs( rdist[0] ) > radius || fabs( rdist[1] ) > radius ) {
			return; // completely off-screen

		}
		// now set the bounds for clipping
		fovRadius[0] = tan( DEG2RAD( ( rollAngleClamped % 2 == 0 ? cg.refdef.fov_x : cg.refdef.fov_x ) * 0.52 ) ) * sdist;
		fovRadius[1] = tan( DEG2RAD( ( rollAngleClamped % 2 == 0 ? cg.refdef.fov_x : cg.refdef.fov_x ) * 0.52 ) ) * sdist;

		// BOTTOM LEFT
		x = ( -radius + rdist[0] );
		if ( x < -fovRadius[0] ) {    // clip
			VectorMA( f->org, -radius + ( -fovRadius[0] - x ), rright, point );
			verts[0].st[0] = 0.0 + ( -fovRadius[0] - x ) / ( radius * 2 );
		} else {
			VectorMA( f->org, -radius, rright, point );
			verts[0].st[0] = 0.0;
		}
		x = ( -radius + rdist[1] );
		if ( x < -fovRadius[1] ) {
			VectorMA( point, -radius + ( -fovRadius[1] - x ), rup, point );
			verts[0].st[1] = 0.0 + ( -fovRadius[1] - x ) / ( radius * 2 );
		} else {
			VectorMA( point, -radius, rup, point );
			verts[0].st[1] = 0.0;
		}
		VectorCopy( point, verts[0].xyz );

		// TOP LEFT
		x = ( -radius + rdist[0] );
		if ( x < -fovRadius[0] ) {    // clip
			VectorMA( f->org, -radius + ( -fovRadius[0] - x ), rright, point );
			verts[1].st[0] = 0.0 + ( -fovRadius[0] - x ) / ( radius * 2 );
		} else {
			VectorMA( f->org, -radius, rright, point );
			verts[1].st[0] = 0.0;
		}
		x = ( radius + rdist[1] );
		if ( x > fovRadius[1] ) {
			VectorMA( point, radius + ( fovRadius[1] - x ), rup, point );
			verts[1].st[1] = 1.0 + ( fovRadius[1] - x ) / ( radius * 2 );
		} else {
			VectorMA( point, radius, rup, point );
			verts[1].st[1] = 1.0;
		}
		VectorCopy( point, verts[1].xyz );

		// TOP RIGHT
		x = ( radius + rdist[0] );
		if ( x > fovRadius[0] ) {
			VectorMA( f->org, radius + ( fovRadius[0] - x ), rright, point );
			verts[2].st[0] = 1.0 + ( fovRadius[0] - x ) / ( radius * 2 );
		} else {
			VectorMA( f->org, radius, rright, point );
			verts[2].st[0] = 1.0;
		}
		x = ( radius + rdist[1] );
		if ( x > fovRadius[1] ) {
			VectorMA( point, radius + ( fovRadius[1] - x ), rup, point );
			verts[2].st[1] = 1.0 + ( fovRadius[1] - x ) / ( radius * 2 );
		} else {
			VectorMA( point, radius, rup, point );
			verts[2].st[1] = 1.0;
		}
		VectorCopy( point, verts[2].xyz );

		// BOTTOM RIGHT
		x = ( radius + rdist[0] );
		if ( x > fovRadius[0] ) {
			VectorMA( f->org, radius + ( fovRadius[0] - x ), rright, point );
			verts[3].st[0] = 1.0 + ( fovRadius[0] - x ) / ( radius * 2 );
		} else {
			VectorMA( f->org, radius, rright, point );
			verts[3].st[0] = 1.0;
		}
		x = ( -radius + rdist[1] );
		if ( x < -fovRadius[1] ) {
			VectorMA( point, -radius + ( -fovRadius[1] - x ), rup, point );
			verts[3].st[1] = 0.0 + ( -fovRadius[1] - x ) / ( radius * 2 );
		} else {
			VectorMA( point, -radius, rup, point );
			verts[3].st[1] = 0.0;
		}
		VectorCopy( point, verts[3].xyz );

	} else {    // set default values for no clipping

		if ( flameSettings.rotate ) {
			vectoangles( cg.refdef.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += f->rollAngle;
			AngleVectors( rotate_ang, NULL, rright, rup );
		} else {
			VectorCopy( vright, rright );
			VectorCopy( vup, rup );
		}

		VectorMA( f->org, -rST[1], rup, point );
		VectorMA( point, -rST[0], rright, point );
		VectorCopy( point, verts[0].xyz );
		verts[0].st[0] = 0;
		verts[0].st[1] = 0;

		VectorMA( point, rST[1] * 2, rup, point );
		VectorCopy( point, verts[1].xyz );
		verts[1].st[0] = 0;
		verts[1].st[1] = 1;

		VectorMA( point, rST[0] * 2, rright, point );
		VectorCopy( point, verts[2].xyz );
		verts[2].st[0] = 1;
		verts[2].st[1] = 1;

		VectorMA( point, -rST[1] * 2, rup, point );
		VectorCopy( point, verts[3].xyz );
		verts[3].st[0] = 1;
		verts[3].st[1] = 0;
	}

	frameNum = (int)floor( lifeFrac * static_cast<int>(flameShaders.size()) );
	if ( frameNum < 0 ) {
		frameNum = 0;
	} else if ( frameNum > static_cast<int>(flameShaders.size()) - 1 )  {
		frameNum = static_cast<int>(flameShaders.size()) - 1;
	}

	trap_R_AddPolyToScene( flameShaders[frameNum], 4, verts );
	VectorCopy( f->org, lastPos );
}

/*
===============
CG_AddFlameToScene
===============
*/
void CG_AddFlameToScene( flameChunk_t *fHead ) {
	flameChunk_t *f, *fNext;
	int blueTrailHead = 0, fuelTrailHead = 0;
	static vec3_t whiteColor = {1,1,1};
	vec3_t c;
	float alpha;
	float lived;
	int headTimeStart;
	//qboolean	firstFuel = qtrue; // TTimo
	float vdist, bdot;

	//flameChunk_t *lastSoundFlameChunk=NULL; // TTimo: unused
	flameChunk_t *lastBlowChunk = NULL;
	qboolean isClientFlame, firing;
	int shader;

	flameChunk_t *lastBlueChunk = NULL;
	qboolean skip = qfalse, droppedTrail;
	vec3_t v, lastDrawPos;



	vec3_t lightOrg;                // origin to place light at
	float lightSize;
	float lightFlameCount;

	float lastFuelAlpha;


	//if (fHead->ownerCent == cg.snap->ps.clientNum)
	isClientFlame = ( fHead == centFlameInfo[fHead->ownerCent].lastFlameChunk );
	//else
	//	isClientFlame = qfalse;	// always optimize enemy flames

	if ( ( cg_entities[fHead->ownerCent].currentState.eFlags & EF_FIRING ) /*(centFlameInfo[fHead->ownerCent].lastClientFrame == cg_entities[fHead->ownerCent].lastWeaponClientFrame)*/ && ( centFlameInfo[fHead->ownerCent].lastFlameChunk == fHead ) ) {
		headTimeStart = fHead->timeStart;
		firing = qtrue;
	} else {
		//isClientFlame = qfalse;
		headTimeStart = cg.time;
		firing = qfalse;
	}

	// Zombie ignition is just to make on/off transitions look better
	//if (cg_entities[fHead->ownerCent].currentState.aiChar == AICHAR_ZOMBIE && !centFlameInfo[fHead->ownerCent].lastFiring) {
	//	return;
	//}

	VectorClear( lightOrg );
	lightSize = 0;
	lightFlameCount = 0;

	lastFuelAlpha = 1.0;
	VectorClear( lastDrawPos );

	f = fHead;
	while ( f ) {

		if ( f->nextFlameChunk && f->nextFlameChunk->dead ) {
			// kill it
			CG_FreeFlameChunk( f->nextFlameChunk );
			f->nextFlameChunk = NULL;
		}

		// draw this chunk

		fNext = f->nextFlameChunk;
		lived = (float)( headTimeStart - f->timeStart );

		// update the "blow" sound volume (louder as we sway it)
		vdist = Distance( cg.refdef.vieworg, f->org );  // NOTE: this needs to be here or the flameSound code further below won't work
		if (    lastBlowChunk && ( centFlameStatus[f->ownerCent].blowVolume < 1.0 ) &&
				( ( bdot = DotProduct( lastBlowChunk->startVelDir, f->startVelDir ) ) < 1.0 ) ) {
			if ( vdist < flameSettings.soundRange ) {
				centFlameStatus[f->ownerCent].blowVolume += 500.0 * ( 1.0 - bdot ) * ( 1.0 - ( vdist / flameSettings.soundRange ) );
				if ( centFlameStatus[f->ownerCent].blowVolume > 1.0 ) {
					centFlameStatus[f->ownerCent].blowVolume = 1.0;
				}
			}
		}
		lastBlowChunk = f;

		VectorMA( lightOrg, f->size / flameSettings.lightWeightSize, f->org, lightOrg );
		lightSize += f->size;
		lightFlameCount += f->size / flameSettings.lightWeightSize;

		droppedTrail = qfalse;

		// is it a stream chunk? (no special handling)
		if ( !f->ignitionOnly && f->velSpeed < 1 ) {

			CG_AddFlameSpriteToScene( f, f->lifeFrac, 1.0 );

			// is it in the blue ignition section of the flame?
		} else if ( isClientFlame && f->blueLife > ( lived / 2.0 ) ) {

			skip = qfalse;

			// if this is backwards from the last chunk, then skip it
			if ( /*!f->ignitionOnly &&*/ fNext && f != fHead && lastBlueChunk ) {
				VectorSubtract( f->org, lastBlueChunk->org, v );
				if ( VectorNormalize( v ) < f->size / flameSettings.fuelWidthDivisor ) {
					skip = qtrue;
				} else if ( DotProduct( v, f->velDir ) < 0 ) {
					skip = qtrue;
				}
			}

			// stream sound
			if ( !f->ignitionOnly && ( cg_entities[f->ownerCent].currentState.number >= MAX_CLIENTS || cg_entities[f->ownerCent].currentState.weapon == WP_FLAMETHROWER ) ) {
				centFlameStatus[f->ownerCent].streamVolume += 0.05;
				if ( centFlameStatus[f->ownerCent].streamVolume > 1.0 ) {
					centFlameStatus[f->ownerCent].streamVolume = 1.0;
				}
			}


			if ( !skip ) {

				// just call this for damage checking
				if ( !f->ignitionOnly ) {
					CG_AddFlameSpriteToScene( f, f->lifeFrac, -1 );
				}

				lastBlueChunk = f;

				alpha = 1.0;    // new nozzle sprite
				VectorScale( whiteColor, alpha, c );

				if ( f->blueLife > lived * ( f->ignitionOnly ? 3.0 : 3.0 ) ) {

					//if (cg_entities[f->ownerCent].currentState.aiChar == AICHAR_ZOMBIE)
					//	shader = cgs.media.funnelFireShader[(cg.time/50 + (cg.time/50 >> 1))%NUM_FUNNEL_SPRITES];
					//else
					shader = nozzleShaders[( cg.time / static_cast<int>(flameSettings.nozzleFrameTime) + ( cg.time / static_cast<int>(flameSettings.nozzleFrameTime) >> 1 ) ) % static_cast<int>(nozzleShaders.size())];

					blueTrailHead = CG_FxTrail( "wolf/trail_flame_nozzle", blueTrailHead, f->org,
			FxTrailInputs().Alpha( alpha ).AlphaEnd( alpha ).Width( f->size * ( f->ignitionOnly ? flameSettings.ignitionWidthScale : 1.0 ) ).Color( c ).ColorEnd( c ).Shader( shader ) );
				}

				// fire stream
//if (qfalse)
				if ( !f->ignitionOnly ) { // && isClientFlame) {
					float bscale;
					qboolean fskip = qfalse;

					bscale = 1.0;

					if ( !f->nextFlameChunk ) {
						alpha = 0;
					} else if ( lived / flameSettings.blueTimeScale < bscale * FlameBlueFadeIn( f->blueLife ) ) {
						alpha = flameSettings.blueMaxAlpha * ( ( lived / flameSettings.blueTimeScale ) / ( bscale * FlameBlueFadeIn( f->blueLife ) ) );
					} else if ( lived / flameSettings.blueTimeScale < ( f->blueLife - FlameBlueFadeOut( f->blueLife ) ) ) {
						alpha = flameSettings.blueMaxAlpha;
					} else {
						alpha = flameSettings.blueMaxAlpha * ( 1.0 - ( ( lived / flameSettings.blueTimeScale - ( f->blueLife - FlameBlueFadeOut( f->blueLife ) ) ) / ( FlameBlueFadeOut( f->blueLife ) ) ) );
					}
					if ( alpha <= 0.0 ) {
						alpha = 0.0;
						if ( lastFuelAlpha <= 0.0 ) {
							fskip = qtrue;
						}
					}

					if ( !fskip ) {
						lastFuelAlpha = alpha;

						VectorScale( whiteColor, alpha, c );

						droppedTrail = qtrue;

						fuelTrailHead = CG_FxTrail( "wolf/trail_flame_fuel", fuelTrailHead, f->org,
			FxTrailInputs().Alpha( alpha ).AlphaEnd( alpha ).Width( ( f->size / flameSettings.fuelWidthDivisor < f->sizeMax / flameSettings.fuelMaxWidthDivisor ? f->size / flameSettings.fuelWidthDivisor : f->sizeMax / flameSettings.fuelMaxWidthDivisor ) ).Mapping( ( f->ignitionOnly ? STYPE_STRETCH : STYPE_REPEAT ) ).Color( c ).ColorEnd( c ) );
					}
				}

			}

		}


		if ( !f->ignitionOnly &&
			 //!(cg_entities[f->ownerCent].currentState.aiChar == AICHAR_ZOMBIE) &&
			 ( (float)( flameSettings.spriteStartBlueScale * f->blueLife ) < (float)lived ) ) {

			float alpha, lifeFrac;
			qboolean skip = qfalse;
			vec3_t v1, v2;
			float fDist, vDist;

			// should we merge it with the next sprite?
			while ( fNext && !droppedTrail ) {
				if (    ( Distance( f->org, fNext->org ) < ( ( 0.2 + 0.8 * f->lifeFrac ) * f->size * ( isClientFlame ? 0.2 : 0.1 ) ) )
						&&  ( fabs( f->size - fNext->size ) < ( 40.0 ) )
						&&  ( fabs( f->timeStart - fNext->timeStart ) < 50 )
						&&  ( DotProduct( f->velDir, fNext->velDir ) > 0.999 )
						) {
					if ( !droppedTrail ) {
						CG_MergeFlameChunks( f, fNext );
						fNext = f->nextFlameChunk;      // it may have changed
					} else {
						skip = qtrue;
						break;
					}
				} else {
					break;
				}
			}

			lifeFrac = ( lived - flameSettings.spriteStartBlueScale * f->blueLife ) / ( flameSettings.Lifetime() - flameSettings.spriteStartBlueScale * f->blueLife );

			alpha = ( 1.0 - lifeFrac ) * flameSettings.spriteAlphaScale;
			if ( alpha > 1.0 ) {
				alpha = 1.0;
			}

			// should we draw this sprite?
			if ( !skip && VectorLengthSquared( lastDrawPos ) ) {
				VectorSubtract( cg.refdef.vieworg, lastDrawPos, v1 );
				VectorSubtract( f->org, lastDrawPos, v2 );
				fDist = VectorNormalize( v2 );
				vDist = VectorNormalize( v1 );
				if ( ( vDist / f->size ) * fDist * ( flameSettings.spriteCullBase + flameSettings.spriteCullWeight * ( 1.0 - fabs( DotProduct( v1, v2 ) ) ) ) < ( 2.0 * ( f->size / flameSettings.spriteCullSize < 1.0 ? f->size / flameSettings.spriteCullSize : 1.0 ) ) ) {
					skip = qtrue;
				}
			}

			if ( !skip ) {
				// draw the sprite
				CG_AddFlameSpriteToScene( f, lifeFrac, alpha );
				VectorCopy( f->org, lastDrawPos );
			}
			// update the sizeRate
			f->sizeRate = FlameSizeSpeed( f->sizeMax );



		}

		f = fNext;
	}

	if ( lightSize < flameSettings.lightMin ) {
		lightSize = flameSettings.lightMin;
	}

	if ( lightSize > flameSettings.lightMax ) {
		lightSize = flameSettings.lightMax;
	}
	lightSize *= 1.0 + flameSettings.lightFlicker * ( sin( cg.time / flameSettings.lightFlickerX ) * cos( cg.time / flameSettings.lightFlickerY ) );
	// set the alpha
	alpha = lightSize / flameSettings.lightMax;
	if ( alpha > 1.0 ) {
		alpha = 1.0;
	}
	if ( lightFlameCount <= 0 ) { return; }
	VectorScale( lightOrg, 1.0 / lightFlameCount, lightOrg );
	// if it's only a nozzle, make it blue
	if ( fHead->ignitionOnly ) {
		if ( lightSize > flameSettings.lightMin ) {
			lightSize = flameSettings.lightMin;
		}
		trap_R_AddLightToScene( lightOrg, flameSettings.lightOffset + lightSize, 0, 0, alpha * flameSettings.lightBlue, 0 );
	} else if ( isClientFlame || ( fHead->ownerCent == cg.snap->ps.clientNum ) ) {
		//trap_R_AddLightToScene( lightOrg, flameSettings.lightOffset + lightSize, 1.000000*alpha, 0.603922*alpha, 0.207843*alpha, 2/*isClientFlame * (fHead->ownerCent == cg.snap->ps.clientNum)*/  );
	}
}



/*
===============
CG_InitFlameChunks
===============
*/
void CG_InitFlameChunks( void ) {
	CG_ClearFlameChunks();
	flameSettings.Load();
	const CPrimitiveTemplate *body = theFxScheduler.Definition( "wolf/flame_stream" );
	const CPrimitiveTemplate *nozzle = theFxScheduler.Definition( "wolf/flame_nozzle" );
	if ( !body || body->mAnimation.empty() || !nozzle || nozzle->mAnimation.empty() ) {
		CG_Error( "FX: flame animation sequences are missing" );
	}
	flameShaders = body->mAnimation;
	nozzleShaders = nozzle->mAnimation;
}

/*
===============
CG_FxBeginFlames
===============
*/
void CG_FxBeginFlames( void ) {
    VectorCopy( cg.refdef.viewaxis[1], vright );
    VectorCopy( cg.refdef.viewaxis[2], vup );
    memset( centFlameStatus, 0, sizeof( centFlameStatus ) );
    numClippedFlames = 0;
}

CWolfFlameStream::~CWolfFlameStream() {
    for ( flameChunk_t *chunk : chunks ) {
        if ( centFlameInfo[chunk->ownerCent].lastFlameChunk == chunk ) {
            centFlameInfo[chunk->ownerCent].lastFlameChunk = NULL;
        }
        delete chunk;
    }
    fxFlameStreams.erase( entry );
}

bool CWolfFlameStream::Update() {
    for ( flameChunk_t *f : chunks ) {
        if ( !f->inuse || f->dead ) { continue; }
        if ( cg.time > f->timeEnd || ( f->ignitionOnly && f->blueLife < cg.time - f->timeStart ) ) {
            f->dead = qtrue;
        } else {
            if ( ( f->sizeTime - f->timeStart < f->blueLife ) != ( cg.time - f->timeStart < f->blueLife ) ) {
                CG_AdjustFlameSize( f, f->timeStart + f->blueLife );
            }
            CG_AdjustFlameSize( f, cg.time );
            CG_MoveFlameChunk( f, cg.time );
            f->lifeFrac = static_cast<float>( f->baseOrgTime - f->timeStart ) / ( f->timeEnd - f->timeStart );
        }
    }
    if ( head && head->dead ) {
        CG_FreeFlameChunk( head );
    } else if ( head && ( !head->ignitionOnly || centFlameInfo[head->ownerCent].lastFlameChunk == head ) ) {
        CG_AddFlameToScene( head );
    }
    for ( auto it = chunks.begin(); it != chunks.end(); ) {
        flameChunk_t *chunk = *it;
        if ( chunk->inuse ) { ++it; continue; }
        if ( centFlameInfo[chunk->ownerCent].lastFlameChunk == chunk ) {
            centFlameInfo[chunk->ownerCent].lastFlameChunk = NULL;
            centFlameInfo[chunk->ownerCent].lastClientFrame = 0;
        }
        if ( head == chunk ) { head = NULL; }
        delete chunk;
        it = chunks.erase( it );
    }
    return head != NULL;
}

/*
===============
CG_UpdateFlamethrowerSounds
===============
*/
void CG_UpdateFlamethrowerSounds( void ) {
	flameChunk_t *f, *trav;
	//flameChunk_t *lastSoundFlameChunk=NULL; // TTimo: unused
	int i;
	centity_t *cent;
	#define MIN_BLOW_VOLUME     30

	// draw each of the headFlameChunk's
	for ( CWolfFlameStream *stream : fxFlameStreams ) {
		f = stream->head;
		if ( !f ) { continue; }

		if ( centFlameInfo[f->ownerCent].silent ) {
			continue;
		}

		// update this entity?
		if ( centFlameInfo[f->ownerCent].lastSoundUpdate != cg.time ) {
			// blow/ignition sound
			if ( centFlameStatus[f->ownerCent].blowVolume * 255.0 > MIN_BLOW_VOLUME ) {
				trap_S_AddLoopingSound( f->ownerCent, f->org, vec3_origin, cgs.media.flameBlowSound, (int)( 255.0 * centFlameStatus[f->ownerCent].blowVolume ) );
			} else {
				trap_S_AddLoopingSound( f->ownerCent, f->org, vec3_origin, cgs.media.flameBlowSound, MIN_BLOW_VOLUME );
			}

			if ( centFlameStatus[f->ownerCent].streamVolume ) {
				if ( cg_entities[f->ownerCent].currentState.aiChar != AICHAR_ZOMBIE ) {
					trap_S_AddLoopingSound( f->ownerCent, f->org, vec3_origin, cgs.media.flameStreamSound, (int)( 255.0 /**centFlameStatus[f->ownerCent].streamVolume*/ ) );
				} else {
					trap_S_AddLoopingSound( f->ownerCent, f->org, vec3_origin, cgs.media.flameCrackSound, (int)( 255.0 * centFlameStatus[f->ownerCent].streamVolume ) );
				}
			}

			centFlameInfo[f->ownerCent].lastSoundUpdate = cg.time;
		}

		// traverse the chunks, spawning flame sound sources as we go
		for ( trav = f; trav; trav = trav->nextFlameChunk ) {
			// update the sound volume
			if ( trav->blueLife + 100 < ( cg.time - trav->timeStart ) ) {
				//if (!lastSoundFlameChunk || Distance( lastSoundFlameChunk->org, trav->org ) > lastSoundFlameChunk->size) {
				trap_S_AddLoopingSound( trav->ownerCent, trav->org, vec3_origin, cgs.media.flameSound, (int)( 255.0 * ( 0.2 * ( trav->size / flameSettings.maxSize ) ) ) );
				//	lastSoundFlameChunk = trav;
				//}
			}
		}

	}

	// send client damage updates if required
	for ( cent = cg_entities, i = 0; i < cgs.maxclients; cent++, i++ ) {
		if ( centFlameInfo[i].lastDmgCheck > centFlameInfo[i].lastDmgUpdate &&
			 centFlameInfo[i].lastDmgUpdate < cg.time - 50 ) { // JPW NERVE (cgs.gametype == GT_SINGLE_PLAYER ? 50 : 50)) -- sean changed clientdamage so this isn't a saturation issue any longer
			if ( ( cg.snap->ps.pm_flags & PMF_LIMBO ) || ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) ) { // JPW NERVE
				return; // JPW NERVE don't do flame damage to guys in limbo or spectator, they drop out of the game

			}
			CG_ClientDamage( i, centFlameInfo[i].lastDmgEnemy, CLDMG_FLAMETHROWER );
			centFlameInfo[i].lastDmgUpdate = cg.time;
		}
	}
}
