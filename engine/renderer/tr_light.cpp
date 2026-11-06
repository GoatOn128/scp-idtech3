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

// tr_light.c

#include "../precompiled.h"

#include "tr_local.h"

#define DLIGHT_AT_RADIUS        16
// at the edge of a dlight's influence, this amount of light will be added

#define DLIGHT_MINIMUM_RADIUS   16
// never calculate a range less than this to prevent huge light numbers


/*
===============
R_TransformDlights

Transforms the origins of an array of dlights.
Used by both the front end (for DlightBmodel) and
the back end (before doing the lighting calculation)
===============
*/
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *orent ) {
	int i;
	idVec3 temp;

	for ( i = 0 ; i < count ; i++, dl++ ) {
		temp = dl->origin - orent->origin;
		dl->transformed[0] = ( temp ) * ( orent->axis[0] );
		dl->transformed[1] = ( temp ) * ( orent->axis[1] );
		dl->transformed[2] = ( temp ) * ( orent->axis[2] );
	}
}

/*
=============
R_DlightBmodel

Determine which dynamic lights may effect this bmodel
=============
*/
void R_DlightBmodel( bmodel_t *bmodel ) {
	int i, j;
	dlight_t    *dl;
	int mask;
	msurface_t  *surf;

	// transform all the lights
	R_TransformDlights( tr.refdef.num_dlights, tr.refdef.dlights, &tr.orent );

	mask = 0;
	for ( i = 0 ; i < tr.refdef.num_dlights ; i++ ) {
		dl = &tr.refdef.dlights[i];

		// see if the point is close enough to the bounds to matter
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( dl->transformed[j] - bmodel->bounds[1][j] > dl->radius ) {
				break;
			}
			if ( bmodel->bounds[0][j] - dl->transformed[j] > dl->radius ) {
				break;
			}
		}
		if ( j < 3 ) {
			continue;
		}

		// we need to check this light
		mask |= 1 << i;
	}

	// RF, this is why some dlights wouldn't light up bmodels

	//tr.currentEntity->needDlights = (mask != 0);

	// (SA) isn't this dangerous to do to an enumerated type? (setting it to an int)
	//		meaning, shouldn't ->needDlights be changed to an int rather than a qbool?

	tr.currentEntity->needDlights = mask;


	// set the dlight bits in all the surfaces
	for ( i = 0 ; i < bmodel->numSurfaces ; i++ ) {
		surf = bmodel->firstSurface + i;

		if ( *surf->data == SF_FACE ) {
			( (srfSurfaceFace_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_GRID ) {
			( (srfGridMesh_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
		} else if ( *surf->data == SF_TRIANGLES ) {
			( (srfTriangles_t *)surf->data )->dlightBits[ tr.smpFrame ] = mask;
		}
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

extern cvar_t  *r_ambientScale;
extern cvar_t  *r_directedScale;
extern cvar_t  *r_debugLight;

/*
=================
R_SetupEntityLightingGrid

=================
*/
static void R_SetupEntityLightingGrid( trRefEntity_t *ent ) {
	idVec3 lightOrigin;
	int pos[3];
	int i, j;
	byte    *gridData;
	float frac[3];
	int gridStep[3];
	idVec3 direction;
	float totalFactor;

	if ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) {
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		lightOrigin = ent->e.lightingOrigin;
	} else {
		lightOrigin = ent->e.origin;
	}

	lightOrigin = lightOrigin - tr.world->lightGridOrigin;
	for ( i = 0 ; i < 3 ; i++ ) {
		float v;

		v = lightOrigin[i] * tr.world->lightGridInverseSize[i];
		pos[i] = floor( v );
		frac[i] = v - pos[i];
		if ( pos[i] < 0 ) {
			pos[i] = 0;
		} else if ( pos[i] >= tr.world->lightGridBounds[i] - 1 ) {
			pos[i] = tr.world->lightGridBounds[i] - 1;
		}
	}

	ent->ambientLight.Zero();
	ent->directedLight.Zero();
	direction.Zero();

	assert( tr.world->lightGridData ); // bk010103 - NULL with -nolight maps

	// trilerp the light value
	gridStep[0] = 8;
	gridStep[1] = 8 * tr.world->lightGridBounds[0];
	gridStep[2] = 8 * tr.world->lightGridBounds[0] * tr.world->lightGridBounds[1];
	gridData = tr.world->lightGridData + pos[0] * gridStep[0]
			   + pos[1] * gridStep[1] + pos[2] * gridStep[2];

	totalFactor = 0;
	for ( i = 0 ; i < 8 ; i++ ) {
		float factor;
		byte    *data;
		int lat, lng;
		idVec3 normal;

		factor = 1.0;
		data = gridData;
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( i & ( 1 << j ) ) {
				factor *= frac[j];
				data += gridStep[j];
			} else {
				factor *= ( 1.0f - frac[j] );
			}
		}

		if ( !( data[0] + data[1] + data[2] ) ) {
			continue;   // ignore samples in walls
		}
		totalFactor += factor;

		ent->ambientLight[0] += factor * data[0];
		ent->ambientLight[1] += factor * data[1];
		ent->ambientLight[2] += factor * data[2];

		ent->directedLight[0] += factor * data[3];
		ent->directedLight[1] += factor * data[4];
		ent->directedLight[2] += factor * data[5];

		lat = data[7];
		lng = data[6];
		lat *= ( FUNCTABLE_SIZE / 256 );
		lng *= ( FUNCTABLE_SIZE / 256 );

		// decode X as cos( lat ) * sin( long )
		// decode Y as sin( lat ) * sin( long )
		// decode Z as cos( long )

		normal[0] = tr.sinTable[( lat + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK] * tr.sinTable[lng];
		normal[1] = tr.sinTable[lat] * tr.sinTable[lng];
		normal[2] = tr.sinTable[( lng + ( FUNCTABLE_SIZE / 4 ) ) & FUNCTABLE_MASK];

		direction = direction + normal * factor;
	}

	if ( totalFactor > 0 && totalFactor < 0.99 ) {
		totalFactor = 1.0f / totalFactor;
		ent->ambientLight = ent->ambientLight * totalFactor;
		ent->directedLight = ent->directedLight * totalFactor;
	}

	ent->ambientLight = ent->ambientLight * r_ambientScale->value;
	ent->directedLight = ent->directedLight * r_directedScale->value;

//----(SA)	added
	// cheats?  check for single player?
	if ( tr.lightGridMulDirected ) {
		ent->directedLight = ent->directedLight * tr.lightGridMulDirected;
	}
	if ( tr.lightGridMulAmbient ) {
		ent->ambientLight = ent->ambientLight * tr.lightGridMulAmbient;
	}
//----(SA)	end

	( ent->lightDir = direction, ent->lightDir.Normalize() );
}


/*
===============
LogLight
===============
*/
static void LogLight( trRefEntity_t *ent ) {
	int max1, max2;

	if ( !( ent->e.renderfx & RF_FIRST_PERSON ) ) {
		return;
	}

	max1 = ent->ambientLight[0];
	if ( ent->ambientLight[1] > max1 ) {
		max1 = ent->ambientLight[1];
	} else if ( ent->ambientLight[2] > max1 ) {
		max1 = ent->ambientLight[2];
	}

	max2 = ent->directedLight[0];
	if ( ent->directedLight[1] > max2 ) {
		max2 = ent->directedLight[1];
	} else if ( ent->directedLight[2] > max2 ) {
		max2 = ent->directedLight[2];
	}

	ri.Printf( PRINT_ALL, "amb:%i  dir:%i\n", max1, max2 );
}

/*
=================
R_SetupEntityLighting

Calculates all the lighting values that will be used
by the Calc_* functions
=================
*/
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent ) {
	int i;
	dlight_t        *dl;
	float power;
	idVec3 dir;
	float d;
	idVec3 lightDir;
	idVec3 lightOrigin;
//	qboolean		highlighted = qfalse; // TTimo: unused

	// lighting calculations
	if ( ent->lightingCalculated ) {
		return;
	}
	ent->lightingCalculated = qtrue;

	//
	// trace a sample point down to find ambient light
	//
	if ( ent->e.renderfx & RF_LIGHTING_ORIGIN ) {
		// seperate lightOrigins are needed so an object that is
		// sinking into the ground can still be lit, and so
		// multi-part models can be lit identically
		lightOrigin = ent->e.lightingOrigin;
	} else {
		lightOrigin = ent->e.origin;
	}

	// if NOWORLDMODEL, only use dynamic lights (menu system, etc)
	if ( !( refdef->rdflags & RDF_NOWORLDMODEL )
		 && tr.world->lightGridData ) {
		R_SetupEntityLightingGrid( ent );
	} else {
		ent->ambientLight[0] = ent->ambientLight[1] =
								   ent->ambientLight[2] = tr.identityLight * 150;
		ent->directedLight[0] = ent->directedLight[1] =
									ent->directedLight[2] = tr.identityLight * 150;
		ent->lightDir = tr.sunDirection;
	}

	if ( ent->e.hilightIntensity ) {
		// level of intensity was set because the item was looked at
		ent->ambientLight[0] += tr.identityLight * 128 * ent->e.hilightIntensity;
		ent->ambientLight[1] += tr.identityLight * 128 * ent->e.hilightIntensity;
		ent->ambientLight[2] += tr.identityLight * 128 * ent->e.hilightIntensity;
	} else if ( ent->e.renderfx & RF_MINLIGHT ) {
		// give everything a minimum light add
		ent->ambientLight[0] += tr.identityLight * 32;
		ent->ambientLight[1] += tr.identityLight * 32;
		ent->ambientLight[2] += tr.identityLight * 32;
	}


	if ( ent->e.entityNum < MAX_CLIENTS && ( refdef->rdflags & RDF_SNOOPERVIEW ) ) {
		ent->ambientLight.Set( 245, 245, 245 );    // allow a little room for flicker from directed light
	}


	//
	// modify the light by dynamic lights
	//
	d = ent->directedLight.Length();
	lightDir = ent->lightDir * d;

	for ( i = 0 ; i < refdef->num_dlights ; i++ ) {
		dl = &refdef->dlights[i];

		if ( dl->dlshader ) {  //----(SA)	if the dlight has a diff shader specified, you don't know what it does, so don't let it affect entities lighting
			continue;
		}

		dir = dl->origin - lightOrigin;
		d = dir.Normalize();

		power = DLIGHT_AT_RADIUS * ( dl->radius * dl->radius );
		if ( d < DLIGHT_MINIMUM_RADIUS ) {
			d = DLIGHT_MINIMUM_RADIUS;
		}

		d = power / ( d * d );

		ent->directedLight = ent->directedLight + dl->color * d;
		lightDir = lightDir + dir * d;
	}

	// clamp ambient
	for ( i = 0 ; i < 3 ; i++ ) {
		if ( ent->ambientLight[i] > tr.identityLightByte ) {
			ent->ambientLight[i] = tr.identityLightByte;
		}
	}

	if ( r_debugLight->integer ) {
		LogLight( ent );
	}

	// save out the byte packet version
	( (byte *)&ent->ambientLightInt )[0] = idMath::Ftoi( ent->ambientLight[0] );
	( (byte *)&ent->ambientLightInt )[1] = idMath::Ftoi( ent->ambientLight[1] );
	( (byte *)&ent->ambientLightInt )[2] = idMath::Ftoi( ent->ambientLight[2] );
	( (byte *)&ent->ambientLightInt )[3] = 0xff;

	// transform the direction to local space
	lightDir.Normalize();
	ent->lightDir[0] = ( lightDir ) * ( ent->e.axis[0] );
	ent->lightDir[1] = ( lightDir ) * ( ent->e.axis[1] );
	ent->lightDir[2] = ( lightDir ) * ( ent->e.axis[2] );
}

/*
=================
R_LightForPoint
=================
*/
int R_LightForPoint( const idVec3 &point, idVec3 &ambientLight, idVec3 &directedLight, idVec3 &lightDir ) {
	trRefEntity_t ent;

	// bk010103 - this segfaults with -nolight maps
	if ( tr.world->lightGridData == NULL ) {
		return qfalse;
	}

	memset( &ent, 0, sizeof( ent ) );
	ent.e.origin = point;
	R_SetupEntityLightingGrid( &ent );
	ambientLight = ent.ambientLight;
	directedLight = ent.directedLight;
	lightDir = ent.lightDir;

	return qtrue;
}
