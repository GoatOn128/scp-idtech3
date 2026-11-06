// this include must remain at the top of every CPP file
#include "../precompiled.h"

#include "FxWolf.h"


#if !defined(FX_SCHEDULER_H_INC)
	#include "FxScheduler.h"
#endif





#if !defined(__Q_SHARED_H)
	#include "../game/q_shared.h"
#endif




CFxScheduler	theFxScheduler;

//-----------------------------------------------------------
void CMediaHandles::operator=(const CMediaHandles &that )
{
	mMediaList.Clear();

	for ( int i = 0; i < that.mMediaList.Num(); i++ )
	{
		mMediaList.Append( that.mMediaList[i] );
	}
}

//------------------------------------------------------
CFxScheduler::CFxScheduler()
{ 
	memset( &mEffectTemplates, 0, sizeof( mEffectTemplates ));
}

//-----------------------------------------------------------
void SEffectTemplate::operator=(const SEffectTemplate &that)
{
	mCopy = true;

	strcpy( mEffectName, that.mEffectName );

	mPrimitiveCount = that.mPrimitiveCount;

	for( int i = 0; i < mPrimitiveCount; i++ )
	{
		mPrimitives[i] = new CPrimitiveTemplate;
		*(mPrimitives[i]) = *(that.mPrimitives[i]);
		// Mark use as a copy so that we know that we should be chucked when used up
		mPrimitives[i]->mCopy = true;
	}
}

//------------------------------------------------------
// Clean
//	Free up any memory we've allocated so we aren't leaking memory
//
// Input:
//	Whether to clean everything or just stop the playing (active) effects
//
// Return:
//	None
//
//------------------------------------------------------
void CFxScheduler::Clean(bool bRemoveTemplates /*= true*/, int idToPreserve /*= 0*/)
{
	int								i, j;

	// Ditch any scheduled effects
	for ( i = 0; i < mFxSchedule.Num(); ++i ) {
		CPrimitiveTemplate *primitive = mFxSchedule[i]->mpTemplate;
		if ( primitive->mCopy && --primitive->mRefCount == 0 ) {
			delete primitive;
		}
		delete mFxSchedule[i];
	}
	mFxSchedule.Clear();

	if (bRemoveTemplates)
	{
		// Ditch any effect templates
		for ( i = 1; i < FX_MAX_EFFECTS; i++ )
		{
			if ( i == idToPreserve)
			{
				continue;
			}

			if ( mEffectTemplates[i].mInUse )
			{
				// Ditch the primitives
				for (j = 0; j < mEffectTemplates[i].mPrimitiveCount; j++)
				{
					delete mEffectTemplates[i].mPrimitives[j];
				}
			} 

			mEffectTemplates[i].mInUse = false;
		}

		if (idToPreserve == 0)
		{
			mEffectIDs.Clear();
		}
		else
		{
			// Clear the effect names, but first get the name of the effect to preserve,
			// and restore it after clearing.
			idStr str;
			for ( i = 0; i < mEffectIDs.Num(); ++i ) {
				const int *effectID = mEffectIDs.GetIndex( i );
				idStr name;
				if ( effectID && *effectID == idToPreserve && mEffectIDs.GetIndexKey( i, name ) ) {
					str = name;
					break;
				}
			}

			mEffectIDs.Clear();
			if ( !str.IsEmpty() )
				mEffectIDs.Set( str, idToPreserve );
		}
	}
}

//------------------------------------------------------
// RegisterEffect
//	Attempt to open the specified effect file, if 
//	file read succeeds, parse the file.
//
// Input:
//	path or filename to open
//
// Return:
//	int handle to the effect
//------------------------------------------------------
int CFxScheduler::RegisterEffect( const char *file, bool bHasCorrectPath /*= false*/ )
{
	if ( !file || !file[0] || strlen(file) >= MAX_QPATH - sizeof("effects/.efx") ) {
		theFxHelper.Print( "FX: invalid effect path\n" );
		return 0;
	}
	// Dealing with file names:
	// File names can come from two places - the editor, in which case we should use the given
	// path as is, and the effect file, in which case we should add the correct path and extension.
	// In either case we create a stripped file name to use for naming effects.
	//

	char sfile[MAX_QPATH];

	// Get an extension stripped version of the file
	if (bHasCorrectPath)
	{
		const char *last = file, *p = file;

		while (*p != '\0')
		{
			if ((*p == '/') || (*p == '\\'))
			{
				last = p + 1;
			}

			p++;
		}

		idStr sfileStr( last );
		sfileStr.StripFileExtension();
		idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );
	}
	else
	{
		idStr sfileStr( file );
		sfileStr.StripFileExtension();
		idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );
	}

	// see if the specified file is already registered.  If it is, just return the id of that file
	const int *registeredID = nullptr;
	if ( mEffectIDs.Get( sfile, &registeredID ) )
		return *registeredID;

	idParseTree		parser;
	int				len = 0;
	fileHandle_t	fh;
	char			data[65536];
	char			temp[MAX_QPATH];
	const char		*pfile;

	if (bHasCorrectPath)
	{
		pfile = file;
	}
	else
	{
		// Add on our extension and prepend the file with the default path
		Com_sprintf( temp, sizeof(temp), "%s/%s.efx", FX_FILE_PATH, sfile );
		pfile = temp;
	}

	len = theFxHelper.OpenFile( pfile, &fh, FS_READ );

	if ( len < 0 )
	{
		theFxHelper.Print( "Effect file load failed: %s\n", pfile );
		return 0;
	}

	if (len == 0)
	{
		theFxHelper.Print( "INVALID Effect file: %s\n", pfile );
		theFxHelper.CloseFile( fh );
		return 0;
	}

	// If we'll overflow our buffer, bail out--not a particularly elegant solution
	if (len >= sizeof(data) - 1 )
	{
		theFxHelper.CloseFile( fh );
		return 0;
	}

	// Get the goods and ensure Null termination
	theFxHelper.ReadFile( data, len, fh );
	data[len] = '\0';

	if ( !parser.Parse( data, len, pfile ) ) {
		theFxHelper.CloseFile( fh );
		theFxHelper.Print( "Malformed effect: %s\n", pfile );
		return 0;
	}

	theFxHelper.CloseFile( fh );

	// Lets convert the effect file into something that we can work with
	return ParseEffect( sfile, parser.GetRoot() );
}


//------------------------------------------------------
// ParseEffect
//	Starts at ground zero, using each group header to 
//	determine which kind of effect we are working with.
//	Then we call the appropriate function to parse the
//	specified effect group.
//
// Input:
//	base group, essentially the whole files contents
//
// Return:
//	int handle of the effect
//------------------------------------------------------
int CFxScheduler::ParseEffect( const char *file, const idParseGroup *base )
{
	const idParseGroup	*primitiveGroup;
	CPrimitiveTemplate	*prim;
	const char			*grpName;
	SEffectTemplate		*effect = 0;
	EPrimType			type;
	int					handle;

	effect = GetNewEffectTemplate( &handle, file );

	if ( !handle || !effect )
	{
		// failure
		return 0;
	}

	primitiveGroup = base->GetSubGroup( 0 );

	for ( int i = 0; i < base->GetNumSubGroups(); i++, primitiveGroup = base->GetSubGroup( i ) )
	{
		grpName = primitiveGroup->GetName();

		// Huge stricmp lists suxor
		if ( !idStr::Icmp( grpName, "controller" ) ) { type = Controller; }
		else if ( !idStr::Icmp( grpName, "ribbon" ) ) { type = Ribbon; }
		else if ( ( !idStr::Icmp( grpName, "particle" ) || !idStr::Icmp( grpName, "screenParticle" ) ))
		{
			type = Particle;
		}
		else if ( !idStr::Icmp( grpName, "line" ))
		{
			type = Line;
		}
		else if ( !idStr::Icmp( grpName, "tail" ))
		{
			type = Tail;
		}
		else if ( !idStr::Icmp( grpName, "sound" ))
		{
			type = Sound;
		}
		else if ( !idStr::Icmp( grpName, "cylinder" ))
		{
			type = Cylinder;
		}
		else if ( !idStr::Icmp( grpName, "electricity" ))
		{
			type = Electricity;
		}
		else if ( !idStr::Icmp( grpName, "emitter" ))
		{
			type = Emitter;
		}
		else if ( !idStr::Icmp( grpName, "decal" ))
		{
			type = Decal;
		}
		else if ( !idStr::Icmp( grpName, "orientedparticle" ) || !idStr::Icmp( grpName, "screenDecal" ) )
		{
			type = OrientedParticle;
		}
		else if ( !idStr::Icmp( grpName, "fxrunner" ))
		{
			type = FxRunner;
		}
		else if ( !idStr::Icmp( grpName, "light" ))
		{
			type = Light;
		}
		else if ( !idStr::Icmp( grpName, "cameraShake" ))
		{
			type = CameraShake;
		}
		else if ( !idStr::Icmp( grpName, "flash" ))
		{
			type = ScreenFlash;
		}
		else 
		{
			type = None;
		}
		
		if ( type != None )
		{
			prim = new CPrimitiveTemplate;

			prim->mType = type;
			if ( !prim->ParsePrimitive( primitiveGroup ) ) {
				delete prim;
				for ( int index = 0; index < effect->mPrimitiveCount; index++ ) {
					delete effect->mPrimitives[index];
				}
				effect->mPrimitiveCount = 0;
				effect->mInUse = false;
				mEffectIDs.Remove( file );
				return 0;
			}

			if ( !idStr::Icmp( grpName, "screenParticle" ) ) { prim->mRenderType = RT_SCREEN_PARTICLE; prim->mFlags |= FX_USE_ALPHA; }
			if ( !idStr::Icmp( grpName, "screenDecal" ) ) { prim->mRenderType = RT_SCREEN_DECAL; prim->mFlags |= FX_USE_ALPHA; }

			// Add our primitive template to the effect list
			AddPrimitiveToEffect( effect, prim );
		}

	}

	return handle;
}


//------------------------------------------------------
// AddPrimitiveToEffect
//	Takes a primitive and attaches it to the effect.
//	
// Input:
//	Effect template that we tack the primitive on to
//	Primitive to add to the effect template
//
// Return:
//	None
//------------------------------------------------------
void CFxScheduler::AddPrimitiveToEffect( SEffectTemplate *fx, CPrimitiveTemplate *prim )
{
	int ct = fx->mPrimitiveCount;

	if ( ct >= FX_MAX_EFFECT_COMPONENTS )
	{
		theFxHelper.Print( "FxScheduler:  Error--too many primitives in an effect\n" );
		delete prim;
	}
	else
	{
		fx->mPrimitives[ct] = prim;
		fx->mPrimitiveCount++;
	}
}

//------------------------------------------------------
// GetNewEffectTemplate
//	Finds an unused effect template and returns it to the
//	caller.
//	
// Input:
//	pointer to an id that will be filled in, 
//	file name-- should be NULL when requesting a copy
//
// Return:
//	the id of the added effect template
//------------------------------------------------------
SEffectTemplate *CFxScheduler::GetNewEffectTemplate( int *id, const char *file )
{
	SEffectTemplate *effect;

	// wanted zero to be a bogus effect ID, so we just skip it.
	for ( int i = 1; i < FX_MAX_EFFECTS; i++ )
	{
		effect = &mEffectTemplates[i];

		if ( !effect->mInUse )
		{
			*id = i;
			memset( effect, 0, sizeof( SEffectTemplate ));

			// If we are a copy, we really won't have a name that we care about saving for later
			if ( file )
			{
				mEffectIDs.Set( file, i );
				strcpy( effect->mEffectName, file );
			}

			effect->mInUse = true;
			return effect;
		}
	}

	theFxHelper.Print( "FxScheduler:  Error--reached max effects\n" );
	*id = 0;
	return 0;
}

//------------------------------------------------------
// GetEffectCopy
//	Returns a copy of the desired effect so that it can
//	easily be modified run-time.
//	
// Input:
//	file-- the name of the effect file that you want a copy of
//	newHandle-- will actually be the returned handle to the new effect
//				you have to hold onto this if you intend to call it again
//
// Return:
//	the pointer to the copy
//------------------------------------------------------
SEffectTemplate *CFxScheduler::GetEffectCopy( const char *file, int *newHandle )
{
	const int *effectID = nullptr;
	mEffectIDs.Get( file, &effectID );
	return GetEffectCopy( effectID ? *effectID : 0, newHandle );
}

//------------------------------------------------------
// GetEffectCopy
//	Returns a copy of the desired effect so that it can
//	easily be modified run-time.
//	
// Input:
//	fxHandle-- the handle to the effect that you want a copy of
//	newHandle-- will actually be the returned handle to the new effect
//				you have to hold onto this if you intend to call it again
//
// Return:
//	the pointer to the copy
//------------------------------------------------------
SEffectTemplate *CFxScheduler::GetEffectCopy( int fxHandle, int *newHandle )
{
	if ( fxHandle < 1 || fxHandle >= FX_MAX_EFFECTS || !mEffectTemplates[fxHandle].mInUse )
	{
		// Didn't even request a valid effect to copy!!!
		theFxHelper.Print( "FxScheduler: Bad effect file copy request\n" );

		*newHandle = 0;
		return 0;
	}

	// never get a copy when time is frozen
	if ( fx_freeze.integer )
	{
		return 0;
	}

	// Copies shouldn't have names, otherwise they could trash our stl map used for getting ID from name
	SEffectTemplate *copy = GetNewEffectTemplate( newHandle, NULL );

	if ( copy && *newHandle )
	{
		// do the effect copy and mark us as what we are
		*copy = mEffectTemplates[fxHandle];
		copy->mCopy = true;

		// the user had better hold onto this handle if they ever hope to call this effect.
		return copy;
	}

	// No space left to return an effect
	*newHandle = 0;
	return 0;
}

//------------------------------------------------------
// GetPrimitiveCopy
//	Helper function that returns a copy of the desired primitive
//	
// Input:
//	fxHandle - the pointer to the effect copy you want to override
//	componentName - name of the component to find
//
// Return:
//	the pointer to the desired primitive
//------------------------------------------------------
CPrimitiveTemplate *CFxScheduler::GetPrimitiveCopy( SEffectTemplate *effectCopy, const char *componentName )
{
	if ( !effectCopy || !effectCopy->mInUse )
	{
		return NULL;
	}

	for ( int i = 0; i < effectCopy->mPrimitiveCount; i++ )
	{
		if ( !idStr::Icmp( effectCopy->mPrimitives[i]->mName, componentName ))
		{
			// we found a match, so return it
			return effectCopy->mPrimitives[i];
		}
	}

	// bah, no good.
	return NULL;
}

//------------------------------------------------------
static void ReportPlayEffectError(int id)
{
#ifdef _DEBUG
		theFxHelper.Print( "CFxScheduler::PlayEffect called with invalid effect ID: %i\n", id );
#endif
}


//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Applies a default up
//	axis.
//	
// Input:
//	Effect file id and the origin
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( int id, const idVec3 &origin )
{
	idVec3	axis[3];

	axis[0].Set( 0, 0, 1 );
	axis[1].Set( 1, 0, 0 );
	axis[2].Set( 0, 1, 0 );

	PlayEffect( id, origin, axis );
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Takes a fwd vector
//	and builds a right and up vector
//	
// Input:
//	Effect file id, the origin, and a fwd vector
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( int id, const idVec3 &origin, const idVec3 &forward )
{
	idVec3	axis[3];

	// Take the forward vector and create two arbitrary but perpendicular vectors
	( axis[0] = forward );
	forward.MakeNormalVectors( axis[1], axis[2] );

	PlayEffect( id, origin, axis );
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Uses the specified axis
//	
// Input:
//	Effect file name, the origin, and axis.
//  Optional entity number to be used by a cheap entity origin bolt (defaults to -1)
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( const char *file, const idVec3 &origin, const idVec3 *axis, const int entNum )
{
	char	sfile[MAX_QPATH];

	// Get an extenstion stripped version of the file
	idStr sfileStr( file );
	sfileStr.StripFileExtension();
	idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );

	// This is a horribly dumb thing to have to do, but QuakeIII might not have calc'd the lerpOrigin
	//	for the entity we may be trying to bolt onto.  We like having the correct origin, so we are
	//	forced to call this function....
	if ( entNum != -1 )
	{
		// Packet entities have already computed their interpolated positions.
	}

	const int *effectID = nullptr;
	mEffectIDs.Get( sfile, &effectID );
	const int id = effectID ? *effectID : 0;
#ifndef FINAL_BUILD
	if ( id == 0 )
	{
		theFxHelper.Print( "CFxScheduler::PlayEffect unregistered/non-existent effect: %s\n", sfile );		
	}
#endif

	PlayEffect( id, origin, axis, entNum );
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Uses the specified axis
//	
// Input:
//	Effect file name, the origin, and axis.
//  Optional entity number to be used by a cheap entity origin bolt (defaults to -1)
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( const char *file, int clientID )
{
	char	sfile[MAX_QPATH];
	int		id;

	// Get an extenstion stripped version of the file
	idStr sfileStr( file );
	sfileStr.StripFileExtension();
	idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );
	const int *effectID = nullptr;
	id = mEffectIDs.Get( sfile, &effectID ) ? *effectID : 0;

#ifndef FINAL_BUILD
	if ( id == 0 )
	{
		theFxHelper.Print( "CFxScheduler::PlayEffect unregistered/non-existent effect: %s\n", file );		
	}
#endif

	SEffectTemplate			*fx;
	CPrimitiveTemplate		*prim;
	int						i = 0;
	int						count = 0, delay = 0;
	SScheduledEffect		*sfx;
	float					factor = 0.0f;

	if ( id < 1 || id >= FX_MAX_EFFECTS || !mEffectTemplates[id].mInUse )
	{
		// Now you've done it!
		ReportPlayEffectError(id);
		return;
	}

	// Don't bother scheduling the effect if the system is currently frozen
 
	// Get the effect.
	fx = &mEffectTemplates[id];

	// Loop through the primitives and schedule each bit
	for ( i = 0; i < fx->mPrimitiveCount; i++ )
	{
		prim = fx->mPrimitives[i];

		count = prim->mSpawnCount.GetRoundedVal();
		if ( count <= 0 ) {
			if ( prim->mCopy ) { delete prim; }
			continue;
		}

		if ( prim->mCopy )
		{
			// If we are a copy, we need to store a "how many references count" so that we
			//	can keep the primitive template around for the correct amount of time.
			prim->mRefCount = count;
		}

		if ( prim->mSpawnFlags & FX_EVEN_DISTRIBUTION )
		{
			factor = abs(prim->mSpawnDelay.GetMax() - prim->mSpawnDelay.GetMin()) / (float)count;
		}

		// Schedule the random number of bits
		for ( int t = 0; t < count; t++ )
		{
			if ( prim->mSpawnFlags & FX_EVEN_DISTRIBUTION )
			{
				delay = t * factor;
			}
			else
			{
				delay = prim->mSpawnDelay.GetVal();
			}

			// if the delay is so small, we may as well just create this bit right now
			if ( delay < 1 )
			{
				CreateEffect( prim, clientID, -delay );
			}
			else
			{
				// We have to create a new scheduled effect so that we can create it at a later point
				//	you should avoid this because it's much more expensive
				sfx = new SScheduledEffect;
				sfx->mStartTime = theFxHelper.mTime + delay;
				sfx->mpTemplate = prim;
				sfx->mClientID = clientID;

				mFxSchedule.Insert( sfx, 0 );
			}
		}
	}

	// We track effect templates and primitive templates separately.
	if ( fx->mCopy )
	{
		// We don't use dynamic memory allocation, so just mark us as dead
		fx->mInUse = false;
	}
}

//------------------------------------------------------
// CreateEffect
//	Creates the specified fx taking into account the
//	multitude of different ways it could be spawned.
//	
// Input:
//	template used to build the effect, desired effect origin, 
//	desired orientation and how late the effect is so that
//	it can be moved to the correct spot
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::CreateEffect( CPrimitiveTemplate *fx, int clientID, int delay )
{
	idVec3	sRGB, eRGB;
	idVec3	vel, accel;
	idVec3	org;
	int		flags = 0;

	// Origin calculations -- completely ignores most things
	//-------------------------------------
	org.Set( fx->mOrigin1X.GetVal(), fx->mOrigin1Y.GetVal(), fx->mOrigin1Z.GetVal() );

	// handle RGB color
	if ( fx->mSpawnFlags & FX_RGB_COMPONENT_INTERP )
	{
		float perc = cgRandom.RandomFloat();

		sRGB.Set( fx->mRedStart.GetVal( perc ), fx->mGreenStart.GetVal( perc ), fx->mBlueStart.GetVal( perc ) );
		eRGB.Set( fx->mRedEnd.GetVal( perc ), fx->mGreenEnd.GetVal( perc ), fx->mBlueEnd.GetVal( perc ) );
	}
	else
	{
		sRGB.Set( fx->mRedStart.GetVal(), fx->mGreenStart.GetVal(), fx->mBlueStart.GetVal() );
		eRGB.Set( fx->mRedEnd.GetVal(), fx->mGreenEnd.GetVal(), fx->mBlueEnd.GetVal() );
	}

	// NOTE: This completely disregards a few specialty flags.
	vel.Set( fx->mVelX.GetVal( ), fx->mVelY.GetVal( ), fx->mVelZ.GetVal( ) );
	accel.Set( fx->mAccelX.GetVal( ), fx->mAccelY.GetVal( ), fx->mAccelZ.GetVal( ) );

	// If depth hack ISN'T already on, then turn it on.  Otherwise, we treat a pre-existing depth_hack flag as NOT being depth_hack.
	//	This is done because muzzle flash fx files are shared amongst all shooters, but for the player we need to do depth hack in first person....
	if ( !( fx->mFlags & FX_DEPTH_HACK ) && !cg.renderingThirdPerson ) // hack!
	{
		flags = fx->mFlags | FX_RELATIVE | FX_DEPTH_HACK;
	}
	else
	{
		flags = (fx->mFlags | FX_RELATIVE) & ~FX_DEPTH_HACK;
	}

	// We only support particles for now
	//------------------------
	switch( fx->mType )
	{
	//---------
	case Particle:
	//---------

		FX_AddParticle( clientID, org, vel, accel, fx->mGravity.GetVal(),
					fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
					fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
					sRGB, eRGB, fx->mRGBParm.GetVal(),
					fx->mRotation.GetVal(), fx->mRotationDelta.GetVal(),
					fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), flags );
		break;

	//---------
	case Line:
	//---------

		FX_AddLine( clientID, org, 
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(), flags );
		break;

	//---------
	case Tail:
	//---------

		FX_AddTail( clientID, org, vel, accel,
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mLengthStart.GetVal(), fx->mLengthEnd.GetVal(), fx->mLengthParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mMin, fx->mMax, fx->mElasticity.GetVal(), 
						fx->mDeathFxHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), flags );
		break;


	//---------
	case Sound:
	//---------

		// bolted sounds actually play on the client....
		theFxHelper.PlaySound( NULL, clientID, CHAN_WEAPON, fx->mMediaHandles.GetHandle() );
		break;

	//---------
	case Light:
	//---------

		// don't much care if the light stays bolted...so just add it.  
		if ( clientID >= 0 && clientID < ENTITYNUM_WORLD )
		{
			// ..um, ok.....
			centity_t *cent = &cg_entities[clientID];

			if ( cent->currentValid )
			{
				FX_AddLight( cent->lerpOrigin, fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mLife.GetVal(), fx->mFlags );
			}
		}
		break;

	//---------
	case CameraShake:
	//---------

		if ( clientID >= 0 && clientID < ENTITYNUM_WORLD )
		{
			// ..um, ok.....
			centity_t *cent = &cg_entities[clientID];

			if ( cent->currentValid )
			{
				theFxHelper.CameraShake( cent->lerpOrigin, fx->mElasticity.GetVal(), fx->mRadius.GetVal(), fx->mLife.GetVal() );
			}
		}
		break;

	default:
		break;
	}

	// Track when we need to clean ourselves up if we are a copy
	if ( fx->mCopy )
	{
		fx->mRefCount--;

		if ( fx->mRefCount <= 0 )
		{
			delete fx;
		}
	}
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Uses the specified axis
//	
// Input:
//	Effect id, the origin, and axis.
//  Optional entity number to be used by a cheap entity origin bolt (defaults to -1)
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( int id, const idVec3 &origin, const idVec3 *axis, const int entNum )
{
	SEffectTemplate			*fx;
	CPrimitiveTemplate		*prim;
	int						i = 0;
	int						count = 0, delay = 0;
	SScheduledEffect		*sfx;
	float					factor = 0.0f;
	bool					forceScheduling = false;

	if ( id < 1 || id >= FX_MAX_EFFECTS || !mEffectTemplates[id].mInUse )
	{
		// Now you've done it!
		ReportPlayEffectError(id);
		return;
	}

	// Don't bother scheduling the effect if the system is currently frozen
	if ( fx_freeze.integer )
	{
		return;
	}

    if ( entNum < -1 || entNum >= MAX_GENTITIES ) { return; }

	// Get the effect.
	fx = &mEffectTemplates[id];

	// Loop through the primitives and schedule each bit
	for ( i = 0; i < fx->mPrimitiveCount; i++ )
	{
		prim = fx->mPrimitives[i];

		if ( prim->mCullRange )
		{
			if ( ( origin - cg.refdef.vieworg ).LengthSqr() > prim->mCullRange )  // cull range has already been squared
			{
				// Copies culled before scheduling have no references to release later.
				if ( prim->mCopy ) { delete prim; }
				continue;
			}
		}

		count = prim->mSpawnCount.GetRoundedVal();
		if ( count <= 0 ) {
			if ( prim->mCopy ) { delete prim; }
			continue;
		}

		if ( prim->mCopy )
		{
			// If we are a copy, we need to store a "how many references count" so that we
			//	can keep the primitive template around for the correct amount of time.
			prim->mRefCount = count;
		}

		if ( prim->mSpawnFlags & FX_EVEN_DISTRIBUTION )
		{
			factor = abs(prim->mSpawnDelay.GetMax() - prim->mSpawnDelay.GetMin()) / (float)count;
		}

		// Schedule the random number of bits
		for ( int t = 0; t < count; t++ )
		{
			if ( prim->mSpawnFlags & FX_EVEN_DISTRIBUTION )
			{
				delay = t * factor;
			}
			else
			{
				delay = prim->mSpawnDelay.GetVal();
			}

			// if the delay is so small, we may as well just create this bit right now
			if ( delay < 1 && !forceScheduling )
			{
				if ( entNum != -1 )
				{
					// Find out where the entity currently is
					CreateEffect( prim, cg_entities[entNum].lerpOrigin, axis, -delay );
				}
				else
				{
					CreateEffect( prim, origin, axis, -delay );
				}
			}
			else
			{
				// We have to create a new scheduled effect so that we can create it at a later point
				//	you should avoid this because it's much more expensive
				sfx = new SScheduledEffect;
				sfx->mStartTime = theFxHelper.mTime + delay;
				sfx->mpTemplate = prim;
				sfx->mClientID = -1;

                sfx->mEntNum = entNum;
                sfx->mOrigin = origin;
                sfx->mAxis[0] = axis[0];
                sfx->mAxis[1] = axis[1];
                sfx->mAxis[2] = axis[2];

				mFxSchedule.Insert( sfx, 0 );
			}
		}
	}

	// We track effect templates and primitive templates separately.
	if ( fx->mCopy )
	{
		// We don't use dynamic memory allocation, so just mark us as dead
		fx->mInUse = false;
	}
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Applies a default up
//	axis.
//	
// Input:
//	Effect file name and the origin
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( const char *file, const idVec3 &origin )
{
	char	sfile[MAX_QPATH];

	// Get an extenstion stripped version of the file
	idStr sfileStr( file );
	sfileStr.StripFileExtension();
	idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );

	const int *effectID = nullptr;
	const int id = mEffectIDs.Get( sfile, &effectID ) ? *effectID : 0;
	PlayEffect( id, origin );

#ifndef FINAL_BUILD
	if ( id == 0 )
	{
		theFxHelper.Print( "CFxScheduler::PlayEffect unregistered/non-existent effect: %s\n", file );		
	}
#endif
}

//------------------------------------------------------
// PlayEffect
//	Handles scheduling an effect so all the components
//	happen at the specified time.  Takes a forward vector
//	and uses this to complete the axis field.
//	
// Input:
//	Effect file name, the origin, and a forward vector
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::PlayEffect( const char *file, const idVec3 &origin, const idVec3 &forward )
{
	char	sfile[MAX_QPATH];

	// Get an extenstion stripped version of the file
	idStr sfileStr( file );
	sfileStr.StripFileExtension();
	idStr::Copynz( sfile, sfileStr.c_str(), sizeof( sfile ) );

	const int *effectID = nullptr;
	const int id = mEffectIDs.Get( sfile, &effectID ) ? *effectID : 0;
	PlayEffect( id, origin, forward );

#ifndef FINAL_BUILD
	if ( id == 0 )
	{
		theFxHelper.Print( "CFxScheduler::PlayEffect unregistered/non-existent effect: %s\n", file );		
	}
#endif
}

//------------------------------------------------------
// AddScheduledEffects
//	Handles determining if a scheduled effect should
//	be created or not.  If it should it handles converting
//	the template effect into a real one.
//	
// Input:
//	none
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::AddScheduledEffects( void )
{
	for ( int index = 0; index < mFxSchedule.Num(); ) {
		SScheduledEffect *scheduled = mFxSchedule[index];
		if ( *scheduled <= theFxHelper.mTime )
		{
			if ( scheduled->mClientID >= 0 )
			{
				CreateEffect( scheduled->mpTemplate, scheduled->mClientID,
								theFxHelper.mTime - scheduled->mStartTime );
			}
			else
			{// ok, are we spawning a bolt on effect or a normal one?
				if ( scheduled->mEntNum != -1 )
				{
					// Find out where the entity currently is
					CreateEffect( scheduled->mpTemplate,
								cg_entities[scheduled->mEntNum].lerpOrigin, scheduled->mAxis,
								theFxHelper.mTime - scheduled->mStartTime );
				}
				else
				{
					CreateEffect( scheduled->mpTemplate,
								scheduled->mOrigin, scheduled->mAxis,
								theFxHelper.mTime - scheduled->mStartTime );
				}
			}
			// Get 'em out of there.
			delete scheduled;
			mFxSchedule.RemoveIndex( index );
		} else {
			++index;
		}
	}

	// The caller submits active primitives after scheduling.
}

//------------------------------------------------------
// CreateEffect
//	Creates the specified fx taking into account the
//	multitude of different ways it could be spawned.
//	
// Input:
//	template used to build the effect, desired effect origin, 
//	desired orientation and how late the effect is so that
//	it can be moved to the correct spot
//
// Return:
//	none
//------------------------------------------------------
void CFxScheduler::CreateEffect( CPrimitiveTemplate *fx, const idVec3 &origin, const idVec3 *axis, int lateTime )
{
	idVec3	org, org2, temp,
				vel, accel,
				sRGB, eRGB,
				ang, angDelta,
				ax[3];
	idAngles tempAngles;
	trace_t	tr;
	int		emitterModel;

	// We may modify the axis, so make a work copy
	ax[0] = axis[0];
	ax[1] = axis[1];
	ax[2] = axis[2];

	if( fx->mSpawnFlags & FX_RAND_ROT_AROUND_FWD )
	{
		ax[1].RotatePointAroundVector( ax[0], axis[1], cgRandom.RandomFloat() * 360.0f );
		( ax[2] = ax[0].Cross( ax[1] ) );
	}

	// Origin calculations
	//-------------------------------------
	if ( fx->mSpawnFlags & FX_CHEAP_ORG_CALC )
	{ // let's take the easy way out
		org.Set( fx->mOrigin1X.GetVal(), fx->mOrigin1Y.GetVal(), fx->mOrigin1Z.GetVal() );
	}
	else
	{ // time for some extra work
		( org = ax[0] * fx->mOrigin1X.GetVal() );
		( org = org + ax[1] * fx->mOrigin1Y.GetVal() );
		( org = org + ax[2] * fx->mOrigin1Z.GetVal() );
	}

	// We always add our calculated offset to the passed in origin...
	( org = org + origin );

	// Now, we may need to calc a point on a sphere/ellipsoid/cylinder/disk and add that to it
	//----------------------------------------------------------------
	if ( fx->mSpawnFlags & FX_ORG_ON_SPHERE )
	{
		float x, y;
		float width, height;
		
		x = DEG2RAD( cgRandom.RandomFloat() * 360.0f );
		y = DEG2RAD( cgRandom.RandomFloat() * 180.0f );

		width = fx->mRadius.GetVal();
		height = fx->mHeight.GetVal();

		// calculate point on ellipse
		temp.Set( idMath::Sin( x ) * width * idMath::Sin( y ), idMath::Cos( x ) * width * idMath::Sin( y ), idMath::Cos( y ) * height ); // sinx * siny, cosx * siny, cosy
		( org = org + temp );

		if ( fx->mSpawnFlags & FX_AXIS_FROM_SPHERE )
		{ 
			// well, we will now override the axis at the users request
			( ax[0] = temp ).Normalize();
			ax[0].MakeNormalVectors( ax[1], ax[2] );
		}
	}
	else if ( fx->mSpawnFlags & FX_ORG_ON_CYLINDER )
	{
		idVec3	pt;

		// set up our point, then rotate around the current direction to.  Make unrotated cylinder centered around 0,0,0
		( pt = ax[1] * fx->mRadius.GetVal() );
		( pt = pt + ax[0] * cgRandom.CRandomFloat() * 0.5f * fx->mHeight.GetVal() );
		temp.RotatePointAroundVector( ax[0], pt, cgRandom.RandomFloat() * 360.0f );

		( org = org + temp );

		if ( fx->mSpawnFlags & FX_AXIS_FROM_SPHERE )
		{ 
			idVec3 up(0,0,1);

			// well, we will now override the axis at the users request
			( ax[0] = temp ).Normalize();

			if ( ax[0][2] == 1.0f )
			{
				// readjust up
				up.Set( 0, 1, 0 );
			}

			( ax[1] = up.Cross( ax[0] ) );
			( ax[2] = ax[0].Cross( ax[1] ) );
		}
	}

	// There are only a few types that really use velocity and acceleration, so do extra work for those types
	//--------------------------------------------------------------------------------------------------------
	if ( fx->mType == Particle || fx->mType == OrientedParticle || fx->mType == Tail || fx->mType == Emitter )
	{
		// Velocity calculations
		//-------------------------------------
		if ( fx->mSpawnFlags & FX_VEL_IS_ABSOLUTE )
		{  
			vel.Set( fx->mVelX.GetVal(), fx->mVelY.GetVal(), fx->mVelZ.GetVal() );
		}
		else
		{ // bah, do some extra work to coerce it
			( vel = ax[0] * fx->mVelX.GetVal() );
			( vel = vel + ax[1] * fx->mVelY.GetVal() );
			( vel = vel + ax[2] * fx->mVelZ.GetVal() );
		}

		// Acceleration calculations
		//-------------------------------------
		if ( fx->mSpawnFlags & FX_ACCEL_IS_ABSOLUTE )
		{ 
			accel.Set( fx->mAccelX.GetVal(), fx->mAccelY.GetVal(), fx->mAccelZ.GetVal() );
		}
		else
		{
			( accel = ax[0] * fx->mAccelX.GetVal() );
			( accel = accel + ax[1] * fx->mAccelY.GetVal() );
			( accel = accel + ax[2] * fx->mAccelZ.GetVal() );
		}

		// Gravity is completely decoupled from acceleration since it is __always__ absolute
		// NOTE: I only effect Z ( up/down in the Quake world )
		accel[2] += fx->mGravity.GetVal();
	
		// There may be a lag between when the effect should be created and when it actually gets created.  
		//	Since we know what the discrepancy is, we can attempt to compensate...
		if ( lateTime > 0 )
		{
			// Calc the time differences
			float ftime = lateTime * 0.001f;
			float time2 = ftime * ftime * 0.5f;

			// Advance from the original velocity, then apply acceleration to velocity.
			for ( int i = 0 ; i < 3 ; i++ ) {
				org[i] += ftime * vel[i] + time2 * accel[i];
			}
			( vel = vel + accel * ftime );
		}
	} // end moving types

	// Line type primitives work with an origin2, so do the extra work for them
	//--------------------------------------------------------------------------
	if ( fx->mType == Line || fx->mType == Electricity )
	{
		// We may have to do a trace to find our endpoint
		if ( fx->mSpawnFlags & FX_ORG2_FROM_TRACE )
		{
			( temp = org + ax[0] * FX_MAX_TRACE_DIST );

			if ( fx->mSpawnFlags & FX_ORG2_IS_OFFSET )
			{ // add a random flair to the endpoint...note: org2 will have to be pretty large to affect this much
				// we also do this pre-trace as opposed to post trace since we may have to render an impact effect
				//	and we will want the normal at the exact endpos...
				if ( fx->mSpawnFlags & FX_CHEAP_ORG2_CALC )
				{ 
					org2.Set( fx->mOrigin2X.GetVal(), fx->mOrigin2Y.GetVal(), fx->mOrigin2Z.GetVal() );
					( temp = org2 + temp );
				}
				else
				{ // I can only imagine a few cases where you might want to do this...
					( temp = temp + ax[0] * fx->mOrigin2X.GetVal() );
					( temp = temp + ax[1] * fx->mOrigin2Y.GetVal() );
					( temp = temp + ax[2] * fx->mOrigin2Z.GetVal() );
				}
			}

			theFxHelper.Trace( &tr, org, vec3_origin, vec3_origin, temp, -1, CONTENTS_SOLID | CONTENTS_SOLID );//MASK_SHOT );

			if ( tr.startsolid || tr.allsolid )
			{
				( org2 = org ); // this is not a very good solution
			}
			else
			{
				( org2 = tr.endpos );
			}

			if ( fx->mSpawnFlags & FX_TRACE_IMPACT_FX )
			{
				PlayEffect( fx->mImpactFxHandles.GetHandle(), org2, tr.plane.normal );
			}
		}
		else
		{
			if ( fx->mSpawnFlags & FX_CHEAP_ORG2_CALC )
			{
				org2.Set( fx->mOrigin2X.GetVal(), fx->mOrigin2Y.GetVal(), fx->mOrigin2Z.GetVal() );
			}
			else
			{
				( org2 = ax[0] * fx->mOrigin2X.GetVal() );
				( org2 = org2 + ax[1] * fx->mOrigin2Y.GetVal() );
				( org2 = org2 + ax[2] * fx->mOrigin2Z.GetVal() );

				( org2 = org2 + origin );
			}

		}
	} // end special org2 types

	// handle RGB color, but only for types that will use it
	//---------------------------------------------------------------------------
	if ( fx->mType != Sound && fx->mType != FxRunner && fx->mType != CameraShake )
	{
		if ( fx->mSpawnFlags & FX_RGB_COMPONENT_INTERP )
		{
			float perc = cgRandom.RandomFloat();

			sRGB.Set( fx->mRedStart.GetVal( perc ), fx->mGreenStart.GetVal( perc ), fx->mBlueStart.GetVal( perc ) );
			eRGB.Set( fx->mRedEnd.GetVal( perc ), fx->mGreenEnd.GetVal( perc ), fx->mBlueEnd.GetVal( perc ) );
		}
		else
		{
			sRGB.Set( fx->mRedStart.GetVal(), fx->mGreenStart.GetVal(), fx->mBlueStart.GetVal() );
			eRGB.Set( fx->mRedEnd.GetVal(), fx->mGreenEnd.GetVal(), fx->mBlueEnd.GetVal() );
		}
	}

	// Now create the appropriate effect entity
	//------------------------
	switch( fx->mType )
	{
	//---------
	case Particle:
	//---------

		{ CParticle *particle = FX_AddParticle( org, vel, accel, 
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mRotation.GetVal(), fx->mRotationDelta.GetVal(),
						fx->mMin, fx->mMax, fx->mElasticity.GetVal(), 
						fx->mDeathFxHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
        if ( particle ) { particle->Configure(*fx); } }
		break;

	//---------
	case Line:
	//---------

		FX_AddLine( org, org2, 
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
		break;

	//---------
	case Tail:
	//---------

		{ CTail *particle = FX_AddTail( org, vel, accel,
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mLengthStart.GetVal(), fx->mLengthEnd.GetVal(), fx->mLengthParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mMin, fx->mMax, fx->mElasticity.GetVal(), 
						fx->mDeathFxHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
		if ( particle ) { particle->Configure( *fx ); } }
		break;

	//----------------
	case Electricity:
	//----------------

		FX_AddElectricity( org, org2, 
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mElasticity.GetVal(), fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
		break;

	//---------
	case Cylinder:
	//---------

		FX_AddCylinder( org, ax[0],
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mSize2Start.GetVal(), fx->mSize2End.GetVal(), fx->mSize2Parm.GetVal(),
						fx->mLengthStart.GetVal(), fx->mLengthEnd.GetVal(), fx->mLengthParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
		break;

	//---------
	case Emitter:
	//---------

		// for chunk angles, you don't really need much control over the end result...you just want variation..
		ang.Set( fx->mAngle1.GetVal(), fx->mAngle2.GetVal(), fx->mAngle3.GetVal() );

		tempAngles = ax[0].ToAngles();
		temp.Set( tempAngles.pitch, tempAngles.yaw, tempAngles.roll );
		( ang = ang + temp );

		angDelta.Set( fx->mAngle1Delta.GetVal(), fx->mAngle2Delta.GetVal(), fx->mAngle3Delta.GetVal() );

		emitterModel = fx->mMediaHandles.GetHandle();

		FX_AddEmitter( org, vel, accel,
						fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						ang, angDelta,
						fx->mMin, fx->mMax, fx->mElasticity.GetVal(),
						fx->mDeathFxHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(),
						fx->mEmitterFxHandles.GetHandle(),
						fx->mDensity.GetVal(), fx->mVariance.GetVal(),
						fx->mLife.GetVal(), emitterModel, fx->mFlags );
		break;

	//---------
	case Decal:
	//---------

		// I'm calling this function ( at least for now ) because it handles projecting
		//	the decal mark onto the surfaces properly.  This is especially important for large marks.
		// The downside is that it's much less flexible....
		CG_ImpactMark( fx->mMediaHandles.GetHandle(), org, ax[0], fx->mRotation.GetVal(), 
			sRGB[0], sRGB[1], sRGB[2], fx->mAlphaStart.GetVal(), 
			qtrue, fx->mSizeStart.GetVal(), qfalse, fx->mLife.GetRoundedVal() );
		break;

	//-------------------
	case OrientedParticle:
	//-------------------

		{ CParticle *particle = FX_AddOrientedParticle( org, ax[0], vel, accel,
					fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
					fx->mAlphaStart.GetVal(), fx->mAlphaEnd.GetVal(), fx->mAlphaParm.GetVal(),
					sRGB, eRGB, fx->mRGBParm.GetVal(),
					fx->mRotation.GetVal(), fx->mRotationDelta.GetVal(),
					fx->mMin, fx->mMax, fx->mElasticity.GetVal(),
					fx->mDeathFxHandles.GetHandle(), fx->mImpactFxHandles.GetHandle(),
					fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
        if ( particle ) { particle->Configure(*fx); } }
		break;

	//---------
	case Sound:
	//---------

		if ( fx->mSpawnFlags & FX_SND_LESS_ATTENUATION )
		{
			theFxHelper.PlaySound( &org, ENTITYNUM_NONE, CHAN_LESS_ATTEN, fx->mMediaHandles.GetHandle() );
		}
		else
		{
			theFxHelper.PlaySound( &org, ENTITYNUM_NONE, CHAN_AUTO, fx->mMediaHandles.GetHandle() );
		}
		break;

	//---------
	case FxRunner:
	//---------
		
		PlayEffect( fx->mPlayFxHandles.GetHandle(), org, ax );
		break;

	//---------
	case Light:
	//---------

		FX_AddLight( org, fx->mSizeStart.GetVal(), fx->mSizeEnd.GetVal(), fx->mSizeParm.GetVal(),
						sRGB, eRGB, fx->mRGBParm.GetVal(),
						fx->mLife.GetVal(), fx->mFlags );
		break;

	//---------
	case CameraShake:
	//---------
		// It calculates how intense the shake should be based on how close you are to the origin you pass in here
		//	elasticity is actually the intensity...radius is the distance in which the shake will have some effect
		//	life is how long the effect lasts.
		theFxHelper.CameraShake( org, fx->mElasticity.GetVal(), fx->mRadius.GetVal(), fx->mLife.GetVal() );
		break;

	//--------------
	case ScreenFlash:
	//--------------

		FX_AddFlash( org, 
					sRGB, eRGB, fx->mRGBParm.GetVal(),
					fx->mLife.GetVal(), fx->mMediaHandles.GetHandle(), fx->mFlags );
		break;

	default:
		break;
	}

	// Track when we need to clean ourselves up if we are a copy
	if ( fx->mCopy )
	{
		fx->mRefCount--;

		if ( fx->mRefCount <= 0 )
		{
			delete fx;
		}
	}
}
const CPrimitiveTemplate *CFxScheduler::Definition( const char *name ) {
	int handle = RegisterEffect( name );
	if ( !handle || !mEffectTemplates[handle].mPrimitiveCount ) { return NULL; }
	return mEffectTemplates[handle].mPrimitives[0];
}
