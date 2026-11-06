// this include must remain at the top of every CPP file
#include "FxWolf.h"

#if !defined(FX_SCHEDULER_H_INC)
	#include "FxScheduler.h"
#endif



extern int drawnFx;
extern int mParticles;
extern int mOParticles;
extern int mLines;
extern int mTails;

// Helper function
//-------------------------
void ClampVec( const idVec3 &dat, byte *res )
{
	int r;

	// clamp all vec values, then multiply the normalized values by 255 to maximize the result
	for ( int i = 0; i < 3; i++ )
	{
		r = idMath::Ftoi( dat[i] * 255.0f );

		if ( r < 0 )
		{
			r = 0;
		}
		else if ( r > 255 )
		{
			r = 255;
		}

		res[i] = (unsigned char)r;
	}	
}

void GetOrigin( int clientID, idVec3 &org )
{
	if ( clientID >=0 )
	{
		centity_t *cent = &cg_entities[clientID];

		if ( cent->currentValid )
		{
			( org = cent->lerpOrigin );
		}
	}
}

void GetDir( int clientID, idVec3 &org )
{
	if ( clientID >=0 )
	{
		centity_t *cent = &cg_entities[clientID];

		if ( cent->currentValid )
		{
			idAngles( cent->lerpAngles ).ToVectors( &( org ), NULL, NULL );
		}
	}
}

//--------------------------
//
// Base Effect Class
//
//--------------------------


//--------------------------
//
// Derived Particle Class
//
//--------------------------

void CParticle::Die()
{
	if ( mFlags & FX_DEATH_RUNS_FX && !(mFlags & FX_KILL_ON_IMPACT) )
	{
		idVec3	norm;

		// Man, this just seems so, like, uncool and stuff...
		norm.Set( Q_flrand( -1.0f, 1.0f ), Q_flrand( -1.0f, 1.0f ), Q_flrand( -1.0f, 1.0f ) );
		norm.Normalize();

		theFxScheduler.PlayEffect( mDeathFxID, mOrigin1, norm );
	}
}

//----------------------------
bool CParticle::Cull()
{
	if ( mRefEnt.reType == RT_SCREEN_PARTICLE ) { return false; }
	idVec3	dir;

	// Get the direction to the view
	( dir = mOrigin1 - cg.refdef.vieworg );

	// Check if it's behind the viewer
	if ( (( cg.refdef.viewaxis[0] * dir )) < 0 )
	{
		return true;
	}

	float len = ( dir.LengthSqr() );

	// Can't be too close
	if ( len < 16 * 16 )
	{
		return true;
	}

	return false;
}

//----------------------------
void CParticle::Draw()
{
	UpdateAnimation();
	if ( !mVisible ) { return; }
	if ( mFlags & FX_DEPTH_HACK )
	{
		// Not sure if first person needs to be set, but it can't hurt?
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	// Add our refEntity to the scene
	( mRefEnt.origin = mOrigin1 );
	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
	mParticles++;
}

//----------------------------
// Update
//----------------------------
bool CParticle::Update()
{
    if ( mRemoveAt && theFxHelper.mTime >= mRemoveAt ) { return false; }
    if ( mRepeat == 1 && mHeightMax > mHeightMin ) {
        if ( mOrigin1[2] < mHeightMin ) { mOrigin1[2] = mHeightMax; }
        if ( mOrigin1[2] > mHeightMax ) { mOrigin1[2] = mHeightMin; }
    }

	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}

	if ( mFlags & FX_RELATIVE )
	{
		if ( mClientID < 0 || mClientID >= ENTITYNUM_WORLD )
		{
			// we are somehow not bolted even though the flag is on?
			return false;
		}

		idVec3	dir, org;
		idVec3 	realVel, realAccel;
		float	time = (theFxHelper.mTime - mTimeStart) * 0.001f;

		// Get our current position and direction
		GetOrigin( mClientID, org );
		GetDir( mClientID, dir );
	
		idVec3 ang, ax[3];

		vectoangles( dir, ang );
		idAngles( ang ).ToVectors( &( ax[0] ), &( ax[1] ), &( ax[2] ) );
//		VectorCopy( dir, ax[0] );

//		CrossProduct( up, ax[0], ax[1] );
//		VectorNormalize( ax[1] );
//		CrossProduct( ax[0], ax[1], ax[2] );

		( org = org + ax[0] * mOrgOffset[0] );
		( org = org + ax[1] * mOrgOffset[1] );
		( org = org + ax[2] * mOrgOffset[2] );

		// calc the real velocity and accel vectors
		( realVel = ax[0] * mVel[0] );
		( realVel = realVel + ax[1] * mVel[1] );
		( realVel = realVel + ax[2] * mVel[2] );
		realVel[2] += 0.5f * mGravity * time;

		( realAccel = ax[0] * mAccel[0] );
		( realAccel = realAccel + ax[1] * mAccel[1] );
		( realAccel = realAccel + ax[2] * mAccel[2] );

		// Get our real velocity at the current time, taking into account the effects of acceleartion.  NOTE: not sure if this is even 100% correct math-wise
		( realVel = realVel + realAccel * time );

		// Now move us to where we should be at the given time
		( mOrigin1 = org + realVel * time );
		
	}
	else if (( mTimeStart < theFxHelper.mTime ) && UpdateOrigin() == false )
	{
		// we are marked for death
		return false;
	}

	if ( !Cull())
	{
		UpdateSize();
		UpdateRGB();
		UpdateAlpha();
		UpdateRotation();

		Draw();
	}

	return true;
}

//----------------------------
// Update Origin
//----------------------------
bool CParticle::UpdateOrigin()
{
	idVec3	new_origin;
//	float	ftime, time2;

	UpdateVelocity();

	// Calc the time differences
//	ftime = theFxHelper.mFrameTime * 0.001f;
	//time2 = ftime * ftime * 0.5f;
//	time2=0;

	// Predict the new position
	new_origin[0] = mOrigin1[0] + theFxHelper.mFloatFrameTime * mVel[0];// + time2 * mVel[0];
	new_origin[1] = mOrigin1[1] + theFxHelper.mFloatFrameTime * mVel[1];// + time2 * mVel[1];
	new_origin[2] = mOrigin1[2] + theFxHelper.mFloatFrameTime * mVel[2];// + time2 * mVel[2];

	// Only perform physics if this object is tagged to do so
	if ( (mFlags & FX_APPLY_PHYSICS) )
	{
		bool solid;

		if ( mFlags & FX_EXPENSIVE_PHYSICS )
		{
			solid = true; // by setting this to true, we force a real trace to happen
		}
		else
		{
			// if this returns solid, we need to do a trace
			solid = !!(CG_PointContents( new_origin, ENTITYNUM_WORLD ) & mCollisionMask);
		}

		if ( solid )
		{
			trace_t	trace;
			float	dot;

			if ( mFlags & FX_USE_BBOX )
			{
				theFxHelper.Trace( &trace, mOrigin1, mMin, mMax, new_origin, -1, mCollisionMask );
			}
			else
			{
				theFxHelper.Trace( &trace, mOrigin1, vec3_origin, vec3_origin, new_origin, -1, mCollisionMask );
			}

			// Hit something
			if ( trace.fraction < 1.0f )//|| trace.startsolid || trace.allsolid )
			{
				if ( mFlags & FX_IMPACT_RUNS_FX && !trace.startsolid && !trace.allsolid && !(trace.surfaceFlags & ( SURF_NOIMPACT | SURF_SKY )) )
				{
					theFxScheduler.PlayEffect( mImpactFxID, trace.endpos, trace.plane.normal );
				}

				if ( mFlags & FX_KILL_ON_IMPACT	)
				{
					// time to die
					return false;
				}

				( mVel = mVel + mAccel * theFxHelper.mFloatFrameTime * trace.fraction );

				dot = ( mVel * trace.plane.normal );

				( mVel = mVel + trace.plane.normal * -2 * dot );

				( mVel = mVel * mElasticity );

				// If the velocity is too low, make it stop moving, rotating, and turn off physics to avoid
				//	doing expensive operations when they aren't needed
				if ( trace.plane.normal[2] > 0 && mVel[2] < 4 )
				{
					mVel.Zero();
					mAccel.Zero();

					mFlags &= ~(FX_APPLY_PHYSICS|FX_IMPACT_RUNS_FX);
				}

				// Set the origin to the exact impact point
				( mOrigin1 = trace.endpos );
				return true;
			}
		}
	}

	// No physics were done to this object, move it
	( mOrigin1 = new_origin );

	return true;
}

//----------------------------
// Update Size
//----------------------------
void CParticle::UpdateSize()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 

	if ( (mFlags & FX_SIZE_LINEAR) )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)(theFxHelper.mTime - mTimeStart) 
						/ (float)(mTimeEnd - mTimeStart);
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR, FX_WAVE, or FX_CLAMP
	if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_NONLINEAR )
	{
		if ( theFxHelper.mTime > mSizeParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)(theFxHelper.mTime - mSizeParm) 
							/ (float)(mTimeEnd - mSizeParm);
		}

		if ( mFlags & FX_SIZE_LINEAR )
		{
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( (theFxHelper.mTime - mTimeStart) * mSizeParm );
	}
	else if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_CLAMP )
	{
		if ( theFxHelper.mTime < mSizeParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mSizeParm - theFxHelper.mTime) 
							/ (float)(mSizeParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( (mFlags & FX_SIZE_LINEAR) )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if (( mFlags & FX_SIZE_RAND ))
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	mRefEnt.radius = (mSizeStart * perc1) + (mSizeEnd * (1.0f - perc1));
}

//----------------------------
// Update RGB
//----------------------------
void CParticle::UpdateRGB()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 
	idVec3	res;

	if ( (mFlags & FX_RGB_LINEAR) )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)( theFxHelper.mTime - mTimeStart ) 
						/ (float)( mTimeEnd - mTimeStart );
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR, FX_WAVE, or FX_CLAMP
	if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_NONLINEAR )
	{
		if ( theFxHelper.mTime > mRGBParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)( theFxHelper.mTime - mRGBParm ) 
							/ (float)( mTimeEnd - mRGBParm );
		}

		if ( (mFlags & FX_RGB_LINEAR) )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( ( theFxHelper.mTime - mTimeStart ) * mRGBParm );
	}
	else if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_CLAMP )
	{
		if ( theFxHelper.mTime < mRGBParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mRGBParm - theFxHelper.mTime) 
							/ (float)(mRGBParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if (( mFlags & FX_RGB_LINEAR ))
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if (( mFlags & FX_RGB_RAND ))
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	// Now get the correct color 
	( res = mRGBStart * perc1 );
	( mRefEnt.angles = res + mRGBEnd * (1.0f - perc1) ); // angles is a temp storage, will get clamped to a byte in the UpdateAlpha section
}


//----------------------------
// Update Alpha
//----------------------------
void CParticle::UpdateAlpha()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f;

	if ( mFlags & FX_ALPHA_LINEAR )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)(theFxHelper.mTime - mTimeStart) 
						/ (float)(mTimeEnd - mTimeStart);
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR, FX_WAVE, or FX_CLAMP
	if (( mFlags & FX_ALPHA_PARM_MASK ) == FX_ALPHA_NONLINEAR )
	{
		if ( theFxHelper.mTime > mAlphaParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)(theFxHelper.mTime - mAlphaParm) 
							/ (float)(mTimeEnd - mAlphaParm);
		}

		if ( mFlags & FX_ALPHA_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_ALPHA_PARM_MASK ) == FX_ALPHA_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( (theFxHelper.mTime - mTimeStart) * mAlphaParm );
	}
	else if (( mFlags & FX_ALPHA_PARM_MASK ) == FX_ALPHA_CLAMP )
	{
		if ( theFxHelper.mTime < mAlphaParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mAlphaParm - theFxHelper.mTime) 
							/ (float)(mAlphaParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( mFlags & FX_ALPHA_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	perc1 = (mAlphaStart * perc1) + (mAlphaEnd * (1.0f - perc1));

	// We should be in the right range, but clamp to ensure
	if ( perc1 < 0.0f )
	{
		perc1 = 0.0f;
	}
	else if ( perc1 > 1.0f )
	{
		perc1 = 1.0f;
	}

	if ( mFadeIn > 0 && theFxHelper.mTime - mTimeStart < mFadeIn ) {
		perc1 *= (theFxHelper.mTime - mTimeStart) / mFadeIn;
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if ( (mFlags & FX_ALPHA_RAND) )
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	if ( mFlags & FX_USE_ALPHA )
	{
		// should use this when using art that has an alpha channel
		 ClampVec( mRefEnt.angles, (byte*)(&mRefEnt.shaderRGBA) );
		 mRefEnt.shaderRGBA[3] = (byte)(perc1 * 0xff);
	}
	else
	{
		// Modulate the rgb fields by the alpha value to do the fade, works fine for additive blending
		( mRefEnt.angles = mRefEnt.angles * perc1 );
		ClampVec( mRefEnt.angles, (byte*)(&mRefEnt.shaderRGBA) );
	}
}

//--------------------------------
//
// Derived Oriented Particle Class
//
//--------------------------------


//----------------------------
bool COrientedParticle::Cull()
{
	if ( mRefEnt.reType == RT_SCREEN_DECAL ) { return false; }
	idVec3	dir;

	// Get the direction to the view
	( dir = mOrigin1 - cg.refdef.vieworg );

	// Check if it's behind the viewer
	if ( (( cg.refdef.viewaxis[0] * dir )) < 0 )
	{
		return true;
	}

	float len = ( dir.LengthSqr() );

	// Can't be too close
	if ( len < 24 * 24 )
	{
		return true;
	}

	return false;
}

//----------------------------
void COrientedParticle::Draw()
{
	UpdateAnimation();
	if ( !mVisible ) { return; }
	if ( mFlags & FX_DEPTH_HACK )
	{
		// Not sure if first person needs to be set
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	// Add our refEntity to the scene
	( mRefEnt.origin = mOrigin1 );
	( mRefEnt.axis[0] = mNormal );
	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
	mOParticles++;
}

//----------------------------
// Update
//----------------------------
bool COrientedParticle::Update()
{
	if ( mRemoveAt && theFxHelper.mTime >= mRemoveAt ) { return false; }
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	if (( mTimeStart < theFxHelper.mTime ) && UpdateOrigin() == false )
	{
		// we are marked for death
		return false;
	}

	if ( !Cull())
	{
		UpdateSize();
		UpdateRGB();
		UpdateAlpha();
		UpdateRotation();

		Draw();
	}

	return true;
}


//----------------------------
//
// Derived Line Class
//
//----------------------------

//----------------------------
void CLine::Draw()
{
	if ( mFlags & FX_DEPTH_HACK )
	{
		// Not sure if first person needs to be set, but it can't hurt?
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	( mRefEnt.origin = mOrigin1 );
	( mRefEnt.oldorigin = mOrigin2 );

	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
	mLines++;
}

//----------------------------
bool CLine::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	if ( mFlags & FX_RELATIVE )
	{
		if ( mClientID < 0 || mClientID >= ENTITYNUM_WORLD )
		{
			// we are somehow not bolted even though the flag is on?
			return false;
		}

		idVec3	dir, end;
		trace_t	trace;

		// Get our current position and direction
		GetOrigin( mClientID, mOrigin1 );
		GetDir( mClientID, dir );

		if ( mFlags & FX_APPLY_PHYSICS )
		{
			( end = mOrigin1 + dir * 2048 );

			theFxHelper.Trace( &trace, mOrigin1, vec3_origin, vec3_origin, end, mClientID, MASK_SHOT );

			( mOrigin2 = trace.endpos );

			if ( mImpactFxID > 0 )
			{
				theFxScheduler.PlayEffect( mImpactFxID, trace.endpos, trace.plane.normal );
			}
		}
		else
		{
			( mOrigin2 = mOrigin1 + dir * mOrgOffset[0] );
		}
	}

	UpdateSize();
	UpdateRGB();
	UpdateAlpha();

	Draw();

	return true;
}


//----------------------------
//
// Derived Electricity Class
//
//----------------------------
void CElectricity::Initialize()
{
	mRefEnt.frame = Q_flrand( 0.0f, 1.0f ) * 1265536;
	mRefEnt.endTime = cg.time + (mTimeEnd - mTimeStart);

	if ( mFlags & FX_DEPTH_HACK )
	{
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	if ( mFlags & FX_BRANCH )
	{
		mRefEnt.renderfx |= RF_FORKED;	
	}

	if ( mFlags & FX_TAPER )
	{
		mRefEnt.renderfx |= RF_TAPERED;
	}

	if ( mFlags & FX_GROW )
	{
		mRefEnt.renderfx |= RF_GROW;
	}
}

//----------------------------
void CElectricity::Draw()
{
	( mRefEnt.origin = mOrigin1 );
	( mRefEnt.oldorigin = mOrigin2 );
	mRefEnt.angles[0] = mChaos;
	mRefEnt.angles[1] = mTimeEnd - mTimeStart;

	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
	mLines++; // NOT REALLY A LINE!
}

//----------------------------
bool CElectricity::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	UpdateSize();
	UpdateRGB();
	UpdateAlpha();

	Draw();

	return true;
}


//----------------------------
//
// Derived Tail Class
//
//----------------------------
bool CTail::Cull()
{
	idVec3	dir;

	// Get the direction to the view
	( dir = mOrigin1 - cg.refdef.vieworg );

	// Check if it's behind the viewer
	if ( (( cg.refdef.viewaxis[0] * dir )) < 0 )
	{
		return true;
	}

	return false;
}

//----------------------------
void CTail::Draw()
{
	if ( mFlags & FX_DEPTH_HACK )
	{
		// Not sure if first person needs to be set
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	( mRefEnt.origin = mOrigin1 );

	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
	mTails++;
}

//----------------------------
bool CTail::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
	
	if ( !fx_freeze.integer )
	{
		( mOldOrigin = mOrigin1 );
	}

	if ( mFlags & FX_RELATIVE )
	{
		if ( mClientID < 0 || mClientID >= ENTITYNUM_WORLD )
		{
			// the thing we are bolted to is no longer valid, so we may as well just die.
			return false;
		}

		idVec3	dir, org;
//		idVec3	right, up;
		idVec3 	realVel, realAccel;

		// Get our current position and direction
		GetOrigin( mClientID, org );
		GetDir( mClientID, dir );

		idVec3 ang, ax[3];

		vectoangles( dir, ang );
		idAngles( ang ).ToVectors( &( ax[0] ), &( ax[1] ), &( ax[2] ) );

		( org = org + ax[0] * mOrgOffset[0] );
		( org = org + ax[1] * mOrgOffset[1] );
		( org = org + ax[2] * mOrgOffset[2] );

		// calc the real velocity and accel vectors
		// FIXME: if you want right and up movement in addition to the forward movement, you'll have to convert dir into a set of perp. axes and do some extra work
		( realVel = ax[0] * mVel[0] );
		( realVel = realVel + ax[1] * mVel[1] );
		( realVel = realVel + ax[2] * mVel[2] );

		( realAccel = ax[0] * mAccel[0] );
		( realAccel = realAccel + ax[1] * mAccel[1] );
		( realAccel = realAccel + ax[2] * mAccel[2] );

		// Get our real velocity at the current time, taking into account the effects of acceleration.  NOTE: not sure if this is even 100% correct math-wise
		( realVel = realVel + realAccel * (theFxHelper.mTime - mTimeStart) * 0.001f );

		// Now move us to where we should be at the given time
		( mOrigin1 = org + realVel * (theFxHelper.mTime - mTimeStart) * 0.001f );

		// Just calc an old point some time in the past, doesn't really matter when
		( mOldOrigin = org + realVel * ((theFxHelper.mTime - mTimeStart) - 3) * 0.001f );
	}
	else if (( mTimeStart < theFxHelper.mTime ) && UpdateOrigin() == false )
	{
		// we are marked for death
		return false;
	}

	if ( !Cull() )
	{
		UpdateSize();
		UpdateLength();
		UpdateRGB();
		UpdateAlpha();

		CalcNewEndpoint();

		Draw();
	}

	return true;
}

//----------------------------
void CTail::UpdateLength()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 

	if ( mFlags & FX_LENGTH_LINEAR )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)(theFxHelper.mTime - mTimeStart) 
						/ (float)(mTimeEnd - mTimeStart);
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR or FX_WAVE
	if (( mFlags & FX_LENGTH_PARM_MASK ) == FX_LENGTH_NONLINEAR )
	{
		if ( theFxHelper.mTime > mLengthParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)(theFxHelper.mTime - mLengthParm) 
							/ (float)(mTimeEnd - mLengthParm);
		}

		if ( mFlags & FX_LENGTH_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_LENGTH_PARM_MASK ) == FX_LENGTH_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( (theFxHelper.mTime - mTimeStart) * mLengthParm );
	}
	else if (( mFlags & FX_LENGTH_PARM_MASK ) == FX_LENGTH_CLAMP )
	{
		if ( theFxHelper.mTime < mLengthParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mLengthParm - theFxHelper.mTime) 
							/ (float)(mLengthParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( mFlags & FX_LENGTH_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if ( mFlags & FX_LENGTH_RAND )
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	mLength = (mLengthStart * perc1) + (mLengthEnd * (1.0f - perc1));
}

//----------------------------
void CTail::CalcNewEndpoint()
{
	idVec3 temp;

	// FIXME:  Hmmm, this looks dumb when physics are on and a bounce happens
	( temp = mOldOrigin - mOrigin1 );

	// I wish we didn't have to do a VectorNormalize every frame.....
	temp.Normalize();

	( mRefEnt.oldorigin = mOrigin1 + temp * mLength );
}


//----------------------------
//
// Derived Cylinder Class
//
//----------------------------
void CCylinder::Draw()
{
	if ( mFlags & FX_DEPTH_HACK )
	{
		// Not sure if first person needs to be set, but it can't hurt?
		mRefEnt.renderfx |= RF_DEPTHHACK;
	}

	( mRefEnt.origin = mOrigin1 );
	( mRefEnt.oldorigin = mOrigin1 + mRefEnt.axis[0] * mLength );

	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
}

//----------------------------
// Update Size2
//----------------------------
void CCylinder::UpdateSize2()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 

	if ( mFlags & FX_SIZE2_LINEAR )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)(theFxHelper.mTime - mTimeStart) 
						/ (float)(mTimeEnd - mTimeStart);
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR or FX_WAVE
	if (( mFlags & FX_SIZE2_PARM_MASK ) == FX_SIZE2_NONLINEAR )
	{
		if ( theFxHelper.mTime > mSize2Parm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)(theFxHelper.mTime - mSize2Parm) 
							/ (float)(mTimeEnd - mSize2Parm);
		}

		if ( (mFlags & FX_SIZE2_LINEAR) )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_SIZE2_PARM_MASK ) == FX_SIZE2_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( (theFxHelper.mTime - mTimeStart) * mSize2Parm );
	}
	else if (( mFlags & FX_SIZE2_PARM_MASK ) == FX_SIZE2_CLAMP )
	{
		if ( theFxHelper.mTime < mSize2Parm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mSize2Parm - theFxHelper.mTime) 
							/ (float)(mSize2Parm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( mFlags & FX_SIZE2_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if ( mFlags & FX_SIZE2_RAND )
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	mRefEnt.backlerp = (mSize2Start * perc1) + (mSize2End * (1.0f - perc1));
}

//----------------------------
bool CCylinder::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	UpdateSize();
	UpdateSize2();
	UpdateLength();
	UpdateRGB();
	UpdateAlpha();

	Draw();

	return true;
}


//----------------------------
//
// Derived Emitter Class
//
//----------------------------

//----------------------------
// Draw
//----------------------------
void CEmitter::Draw()
{
	// Emitters don't draw themselves, but they may need to add an attached model
	if ( mFlags & FX_ATTACHED_MODEL )
	{
		mRefEnt.nonNormalizedAxes = qtrue;

		( mRefEnt.origin = mOrigin1 );

		// ensure that we are sized
		for ( int i = 0; i < 3; i++ )
		{
			( mRefEnt.axis[i] = mRefEnt.axis[i] * mRefEnt.radius );
		}

		theFxHelper.AddFxToScene( &mRefEnt );
	}

	// If we are emitting effects, we had better be careful because just calling it every cgame frame could
	//	either choke up the effects system on a fast machine, or look really nasty on a low end one.
	if ( mFlags & FX_EMIT_FX )
	{
		idVec3	org, v;
		float	ftime, time2, 
				step;
		int		i, t, dif;

#define TRAIL_RATE		8 // we "think" at about a 60hz rate

		// Pick a target step distance and square it
		step = mDensity + Q_flrand( -1.0f, 1.0f ) * mVariance;
		step *= step;

		dif = 0;

		for ( t = mOldTime; t <= theFxHelper.mTime; t += TRAIL_RATE ) 
		{
			dif += TRAIL_RATE;

			// ?Not sure if it's better to update this before or after updating the origin
			( v = mOldVelocity + mAccel * dif * 0.001f );

			// Calc the time differences
			ftime = dif * 0.001f;
			time2 = ftime * ftime * 0.5f;

			// Predict the new position
			for ( i = 0 ; i < 3 ; i++ ) 
			{
				org[i] = mOldOrigin[i] + ftime * v[i] + time2 * v[i];
			}
	
			// Only perform physics if this object is tagged to do so
			if ( (mFlags & FX_APPLY_PHYSICS) )
			{
				bool solid;

				if ( mFlags & FX_EXPENSIVE_PHYSICS )
				{
					solid = true; // by setting this to true, we force a real trace to happen
				}
				else
				{
					// if this returns solid, we need to do a trace
					solid = !!(CG_PointContents( org, ENTITYNUM_WORLD ) & MASK_SHOT);
				}

				if ( solid )
				{
					trace_t	trace;

					if ( mFlags & FX_USE_BBOX )
					{
						theFxHelper.Trace( &trace, mOldOrigin, mMin, mMax, org, -1, MASK_SHOT );
					}
					else
					{
						theFxHelper.Trace( &trace, mOldOrigin, vec3_origin, vec3_origin, org, -1, MASK_SHOT );
					}

					// Hit something
					if ( trace.fraction < 1.0f || trace.startsolid || trace.allsolid )
					{
						return;
					}
				}
			}

			// Is it time to draw an effect?
			if ( DistanceSquared( org, mOldOrigin ) >= step )
			{ 
				// Pick a new target step distance and square it
				step = mDensity + Q_flrand( -1.0f, 1.0f ) * mVariance;
				step *= step;

				// We met the step criteria so, we should add in the effect
				theFxScheduler.PlayEffect( mEmitterFxID, org, mRefEnt.axis );

				( mOldOrigin = org );
				( mOldVelocity = v );
				dif = 0;
				mOldTime = t;
			}
		}
	}

	drawnFx++;
}

//----------------------------
bool CEmitter::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	// Use this to track if we've stopped moving
	( mOldOrigin = mOrigin1 );
	( mOldVelocity = mVel );

	if (( mTimeStart < theFxHelper.mTime ) && UpdateOrigin() == false )
	{
		// we are marked for death
		return false;
	}

	// If the thing is no longer moving, kill the angle delta, but don't do it too quickly or it will
	//	look very artificial.  Don't do it too slowly or it will look like there is no friction.
	if ( ( mOldOrigin == mOrigin1 ))
	{
		( mAngleDelta = mAngleDelta * 0.7f );
	}

	UpdateAngles();
	UpdateSize();
//	UpdateRGB();	// had wanted to do something slick whereby an emitted effect could somehow inherit these
//	UpdateAlpha();	// values, but it's not a priority right now.

	Draw();

	return true;
}

//----------------------------
void CEmitter::UpdateAngles()
{
	( mAngles = mAngles + mAngleDelta * theFxHelper.mFrameTime * 0.01f ); // was 0.001f, but then you really have to jack up the delta to even notice anything
	AnglesToAxis( mAngles, mRefEnt.axis );
}


//--------------------------
//
// Derived Light Class
//
//--------------------------
//----------------------------
// Update
//----------------------------
bool CLight::Update()
{
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}
		
	UpdateSize();
	UpdateRGB();

	Draw();

	return true;
}

//----------------------------
// Update Size
//----------------------------
void CLight::UpdateSize()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 

	if ( mFlags & FX_SIZE_LINEAR )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)(theFxHelper.mTime - mTimeStart) 
						/ (float)(mTimeEnd - mTimeStart);
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR or FX_WAVE
	if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_NONLINEAR )
	{
		if ( theFxHelper.mTime > mSizeParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)(theFxHelper.mTime - mSizeParm) 
							/ (float)(mTimeEnd - mSizeParm);
		}

		if ( (mFlags & FX_SIZE_LINEAR) )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( (theFxHelper.mTime - mTimeStart) * mSizeParm );
	}
	else if (( mFlags & FX_SIZE_PARM_MASK ) == FX_SIZE_CLAMP )
	{
		if ( theFxHelper.mTime < mSizeParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mSizeParm - theFxHelper.mTime) 
							/ (float)(mSizeParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( mFlags & FX_SIZE_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if ( mFlags & FX_SIZE_RAND )
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	mRefEnt.radius = (mSizeStart * perc1) + (mSizeEnd * (1.0f - perc1));
}

//----------------------------
// Update RGB
//----------------------------
void CLight::UpdateRGB()
{
	// completely biased towards start if it doesn't get overridden
	float	perc1 = 1.0f, perc2 = 1.0f; 
	idVec3	res;

	if ( mFlags & FX_RGB_LINEAR )
	{ 
		// calculate element biasing
		perc1 = 1.0f - (float)( theFxHelper.mTime - mTimeStart ) 
						/ (float)( mTimeEnd - mTimeStart );
	}

	// We can combine FX_LINEAR with _either_ FX_NONLINEAR or FX_WAVE
	if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_NONLINEAR )
	{
		if ( theFxHelper.mTime > mRGBParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = 1.0f - (float)( theFxHelper.mTime - mRGBParm ) 
							/ (float)( mTimeEnd - mRGBParm );
		}

		if ( mFlags & FX_RGB_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}
	else if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_WAVE )
	{ 
		// wave gen, with parm being the frequency multiplier
		perc1 = perc1 * (float)idMath::Cos( ( theFxHelper.mTime - mTimeStart ) * mRGBParm );
	}
	else if (( mFlags & FX_RGB_PARM_MASK ) == FX_RGB_CLAMP )
	{
		if ( theFxHelper.mTime < mRGBParm )
		{ 
			// get percent done, using parm as the start of the non-linear fade
			perc2 = (float)(mRGBParm - theFxHelper.mTime) 
							/ (float)(mRGBParm - mTimeStart);
		}
		else
		{
			perc2 = 0.0f; // make it full size??
		}

		if ( mFlags & FX_RGB_LINEAR )
		{ 
			// do an even blend
			perc1 = perc1 * 0.5f + perc2 * 0.5f;
		}
		else
		{ 
			// just copy it over...sigh
			perc1 = perc2;
		}
	}

	// If needed, RAND can coexist with linear and either non-linear or wave.
	if ( mFlags & FX_RGB_RAND )
	{ 
		// Random simply modulates the existing value
		perc1 = Q_flrand( 0.0f, 1.0f ) * perc1;
	}

	// Now get the correct color 
	( res = mRGBStart * perc1 );

	mRefEnt.lightingOrigin[0] = res[0] + ( 1.0f - perc1 ) * mRGBEnd[0];
	mRefEnt.lightingOrigin[1] = res[1] + ( 1.0f - perc1 ) * mRGBEnd[1];
	mRefEnt.lightingOrigin[2] = res[2] + ( 1.0f - perc1 ) * mRGBEnd[2];
}


//--------------------------
//
// Derived Trail Class
//
//--------------------------
//--------------------------
//
// Derived Poly Class
//
//--------------------------
bool CPoly::Cull()
{
	idVec3	dir;

	// Get the direction to the view
	( dir = mOrigin1 - cg.refdef.vieworg );

	// Check if it's behind the viewer
	if ( (( cg.refdef.viewaxis[0] * dir )) < 0 )
	{
		return true;
	}

	float len = ( dir.LengthSqr() );

	// Can't be too close
	if ( len < 24 * 24 )
	{
		return true;
	}

	return false;
}

//----------------------------
void CPoly::Draw()
{
	polyVert_t	verts[MAX_CPOLY_VERTS];

	for ( int i = 0; i < mCount; i++ )
	{
		// Add our midpoint and vert offset to get the actual vertex
		( verts[i].xyz = mOrigin1 + mOrg[i] );

		// Assign the same color to each vert
		verts[i].modulate[0] = mRefEnt.shaderRGBA[0];
		verts[i].modulate[1] = mRefEnt.shaderRGBA[1];
		verts[i].modulate[2] = mRefEnt.shaderRGBA[2];
		verts[i].modulate[3] = mRefEnt.shaderRGBA[3];

		// Copy the ST coords
		verts[i].st[0] = mST[i].x;
		verts[i].st[1] = mST[i].y;
	}

	// Add this poly
	theFxHelper.AddPolyToScene( mRefEnt.customShader, mCount, verts );

	drawnFx++;
}

//----------------------------
void CPoly::CalcRotateMatrix()
{
	float	cosX, cosZ;
	float	sinX, sinZ;
	float	rad;

	// rotate around Z
	rad = DEG2RAD( mRotDelta[YAW] * theFxHelper.mFrameTime * 0.01f );
	cosZ = idMath::Cos( rad );
	sinZ = idMath::Sin( rad );
	// rotate around X
	rad = DEG2RAD( mRotDelta[PITCH] * theFxHelper.mFrameTime * 0.01f );
	cosX = idMath::Cos( rad );
	sinX = idMath::Sin( rad );

/*Pitch - aroundx  Yaw - around z
1 0  0			 c -s 0
0 c -s			 s  c 0 
0 s  c			 0  0 1
*/
	mRot[0][0] = cosZ;
	mRot[1][0] = -sinZ;
	mRot[2][0] = 0;
	mRot[0][1] = cosX * sinZ;
	mRot[1][1] = cosX * cosZ;
	mRot[2][1] = -sinX;
	mRot[0][2] = sinX * sinZ;
	mRot[1][2] = sinX * cosZ;
	mRot[2][2] = cosX;
/*
// ROLL is not supported unless anyone complains, if it needs to be added, use this format
Roll

 c 0 s
 0 1 0 
-s 0 c
*/
	mLastFrameTime = theFxHelper.mFrameTime;
}

//--------------------------------
void CPoly::Rotate()
{
	idVec3	temp[MAX_CPOLY_VERTS];
	float	dif = idMath::Fabs( mLastFrameTime - theFxHelper.mFrameTime );

	// Very generous check with frameTimes
	if ( dif > 0.5f * mLastFrameTime )
	{
		CalcRotateMatrix();
	}

	// Multiply our rotation matrix by each of the offset verts to get their new position
	for ( int i = 0; i < mCount; i++ )
	{
		// rotate by the matrix rows, matching Q3's VectorRotate
		for ( int j = 0; j < 3; j++ )
		{
			temp[i][j] = mOrg[i][0] * mRot[j][0] + mOrg[i][1] * mRot[j][1] + mOrg[i][2] * mRot[j][2];
		}
		mOrg[i] = temp[i];
	}
}

//----------------------------
// Update
//----------------------------
bool CPoly::Update()
{
	idVec3 mOldOrigin;
	
	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
	if ( mTimeStart > theFxHelper.mTime )
	{
		return false;
	}

	// If our timestamp hasn't exired yet, we won't even consider doing any kind of motion
	if ( theFxHelper.mTime > mTimeStamp )
	{
		( mOldOrigin = mOrigin1 );

		if (( mTimeStart < theFxHelper.mTime ) && UpdateOrigin() == false )
		{
			// we are marked for death
			return false;
		}
	}

	if ( !Cull() )
	{
		// only rotate when our start timestamp has expired
		if ( theFxHelper.mTime > mTimeStamp )
		{
			// Only rotate whilst moving
			if ( !( mOldOrigin == mOrigin1 ))
			{
				Rotate();
			}
		}

		UpdateRGB();
		UpdateAlpha();

		Draw();
	}

	return true;
}

//----------------------------
void CPoly::PolyInit()
{
	if ( mCount < 3 )
	{
		return;
	}

	int		i;
	idVec3	org(0,0,0);

	// Find our midpoint
	for ( i = 0; i < mCount; i++ )
	{
		( org = org + mOrg[i] );
	}

	( org = org * (float)(1.0f / mCount) );

	// now store our midpoint for physics purposes
	( mOrigin1 = org );

	// Now we process the passed in points and make it so that they aren't actually the point...
	//	rather, they are the offset from mOrigin1.
	for ( i = 0; i < mCount; i++ )
	{
		( mOrg[i] = mOrg[i] - mOrigin1 );
	}

	CalcRotateMatrix();
}

/*
-------------------------
CBezier

Bezier curve line
-------------------------
*/
//----------------------------
bool CBezier::Update( void )
{
	float	ftime, time2;

	ftime = cg.frametime * 0.001f;
	time2 = ftime * ftime * 0.5f;

	for ( int i = 0; i < 3; i++ ) 
	{
		mControl1[i] = mControl1[i] + ftime * mControl1Vel[i] + time2 * mControl1Vel[i];
		mControl2[i] = mControl2[i] + ftime * mControl2Vel[i] + time2 * mControl2Vel[i];
	}

	UpdateSize();
	UpdateRGB();
	UpdateAlpha();

	Draw();

	return true;
}

//----------------------------
inline void CBezier::DrawSegment( const idVec3 &start, const idVec3 &end, float texcoord1, float texcoord2 )
{
	idVec3			lineDir, cross, viewDir;
	static idVec3	lastEnd[2];
	polyVert_t		verts[4];
	float			scale;

	( lineDir = end - start );
	( viewDir = end - cg.refdef.vieworg );
	( cross = lineDir.Cross( viewDir ) );
	cross.Normalize();

	scale = mRefEnt.radius * 0.5f;

	//Construct the oriented quad
	if ( mInit )
	{
		( verts[0].xyz = lastEnd[0] );
		( verts[1].xyz = lastEnd[1] );
	}
	else
	{
		( verts[0].xyz = start + cross * -scale );
		( verts[1].xyz = start + cross * scale );
	}

	verts[0].st[0] = 0.0f;
	verts[0].st[1] = texcoord1;

	verts[0].modulate[0] = mRefEnt.shaderRGBA[0] * ( 1.0f - texcoord1 );
	verts[0].modulate[1] = mRefEnt.shaderRGBA[1] * ( 1.0f - texcoord1 );
	verts[0].modulate[2] = mRefEnt.shaderRGBA[2] * ( 1.0f - texcoord1 );
	verts[0].modulate[3] = mRefEnt.shaderRGBA[3];

	verts[1].st[0] = 1.0f;
	verts[1].st[1] = texcoord1;
	verts[1].modulate[0] = mRefEnt.shaderRGBA[0] * ( 1.0f - texcoord1 );
	verts[1].modulate[1] = mRefEnt.shaderRGBA[1] * ( 1.0f - texcoord1 );
	verts[1].modulate[2] = mRefEnt.shaderRGBA[2] * ( 1.0f - texcoord1 );
	verts[1].modulate[3] = mRefEnt.shaderRGBA[3];

	if ( texcoord1 == 0.0f )
	{
		verts[0].modulate[0] = 0;
		verts[0].modulate[1] = 0;
		verts[0].modulate[2] = 0;
		verts[0].modulate[3] = 0;
		verts[1].modulate[0] = 0;
		verts[1].modulate[1] = 0;
		verts[1].modulate[2] = 0;
		verts[1].modulate[3] = 0;
	}

	( verts[2].xyz = end + cross * scale );
	verts[2].st[0] = 1.0f;
	verts[2].st[1] = texcoord2;
	verts[2].modulate[0] = mRefEnt.shaderRGBA[0] * ( 1.0f - texcoord2 );
	verts[2].modulate[1] = mRefEnt.shaderRGBA[1] * ( 1.0f - texcoord2 );
	verts[2].modulate[2] = mRefEnt.shaderRGBA[2] * ( 1.0f - texcoord2 );
	verts[2].modulate[3] = mRefEnt.shaderRGBA[3];

	( verts[3].xyz = end + cross * -scale );
	verts[3].st[0] = 0.0f;
	verts[3].st[1] = texcoord2;
	verts[3].modulate[0] = mRefEnt.shaderRGBA[0] * ( 1.0f - texcoord2 );
	verts[3].modulate[1] = mRefEnt.shaderRGBA[1] * ( 1.0f - texcoord2 );
	verts[3].modulate[2] = mRefEnt.shaderRGBA[2] * ( 1.0f - texcoord2 );
	verts[3].modulate[3] = mRefEnt.shaderRGBA[3];

	trap_R_AddPolyToScene( mRefEnt.customShader, 4, verts );

	( lastEnd[1] = verts[2].xyz );
	( lastEnd[0] = verts[3].xyz );

	mInit = true;
}

const	float	BEZIER_RESOLUTION	= 16.0f;

//----------------------------
void CBezier::Draw( void )	
{
	idVec3	pos, old_pos;
    float	mu, mum1;
	float	incr = 1.0f / BEZIER_RESOLUTION, tex = 1.0f, tc1, tc2;
	int		i;

	( old_pos = mOrigin1 );

	mInit = false;	//Signify a new batch for vert gluing

	// Calculate the texture coords so the texture can stretch along the whole bezier
//	if ( mFlags & FXF_WRAP )
//	{
//		tex = m_stScale / 1.0f;
//	}

	float mum13, mu3, group1, group2;

	tc1 = 0.0f;

	for ( mu = incr; mu <= 1.0f; mu += incr )
	{
		//Four point curve
		mum1	= 1 - mu;
		mum13	= mum1 * mum1 * mum1;
		mu3		= mu * mu * mu;
		group1	= 3 * mu * mum1 * mum1;
		group2	= 3 * mu * mu *mum1;

		for ( i = 0; i < 3; i++ )
		{
			pos[i] = mum13 * mOrigin1[i] + group1 * mControl1[i] + group2 * mControl2[i] + mu3 * mOrigin2[i];
		}

//		if ( m_flags & FXF_WRAP ) 
//		{
			tc2 = mu * tex;
//		}
//		else
//		{
//			// Texture will get mapped onto each segement
//			tc1 = 0.0f;
//			tc2 = 1.0f;
//		}

		//Draw it
		DrawSegment( old_pos, pos, tc1, tc2 );

		( old_pos = pos );
		tc1 = tc2;
	}

	drawnFx++;
	mLines++; // NOT REALLY A LINE
}

/*
-------------------------
CFlash

Full screen flash
-------------------------
*/

//----------------------------
bool CFlash::Update( void )
{
	UpdateRGB();
	Draw();

	return true;
}

//----------------------------
void CFlash::Init( void )
{
	idVec3	dif;
	float	mod = 1.0f, dis;

	( dif = mOrigin1 - cg.refdef.vieworg );
	dis = dif.Normalize();

	mod = ( dif * cg.refdef.viewaxis[0] );

	if ( dis > 600 || ( mod < 0.5f && dis > 100 ))
	{
		mod = 0.0f;
	}
	else if ( mod < 0.5f && dis <= 100 )
	{
		mod += 1.1f;
	}

	mod *= (1.0f - ((dis * dis) / (600.0f * 600.0f)));

	( mRGBStart = mRGBStart * mod );
	( mRGBEnd = mRGBEnd * mod );
}

//----------------------------
void CFlash::Draw( void )	
{
	mRefEnt.reType = RT_SPRITE;

	for ( int i = 0; i < 3; i++ )
	{
		if ( mRefEnt.lightingOrigin[i] > 1.0f )
		{
			mRefEnt.lightingOrigin[i] = 1.0f;
		}
		else if ( mRefEnt.lightingOrigin[i] < 0.0f )
		{
			mRefEnt.lightingOrigin[i] = 0.0f;
		}
	}
	mRefEnt.shaderRGBA[0] = mRefEnt.lightingOrigin[0] * 255; 
	mRefEnt.shaderRGBA[1] = mRefEnt.lightingOrigin[1] * 255; 
	mRefEnt.shaderRGBA[2] = mRefEnt.lightingOrigin[2] * 255; 
	mRefEnt.shaderRGBA[3] = 255;

	( mRefEnt.origin = cg.refdef.vieworg );
	( mRefEnt.origin = mRefEnt.origin + cg.refdef.viewaxis[0] * 8 );
	mRefEnt.radius = 12.0f;

	theFxHelper.AddFxToScene( &mRefEnt );

	drawnFx++;
}

void CParticle::Configure( const CPrimitiveTemplate &definition ) {
    mAnimation = definition.mAnimation;
    mRefEnt.aspectRatio = definition.mAspect;
	mCollisionMask = definition.mCollisionMask;
	mRefEnt.scale[0] = definition.mAspect;
	mRefEnt.shaderTexCoord[0] = definition.mSoftness;
	mRefEnt.shaderTexCoord[1] = definition.mProjectionDepth;
	mFadeIn = definition.mFadeIn;
	if ( definition.mRenderType >= 0 ) { mRefEnt.reType = static_cast<refEntityType_t>(definition.mRenderType); }
    mRepeat = definition.mRepeat;
    mGroup = definition.mGroup;
    mHeightMin = definition.mHeightMin;
    mHeightMax = definition.mHeightMax;
}

void CParticle::UpdateAnimation() {
    if ( !mAnimation.empty() ) {
        int count = static_cast<int>(mAnimation.size());
        float age = static_cast<float>(theFxHelper.mTime - mTimeStart) / (mTimeEnd - mTimeStart);
        int frame = static_cast<int>(age * count);
        frame = frame < 0 ? 0 : (frame >= count ? count - 1 : frame);
        mRefEnt.customShader = mAnimation[frame];
    }
    if ( mRemoveAt ) {
        float fade = (mRemoveAt - theFxHelper.mTime) / 100.0f;
        mRefEnt.shaderRGBA[3] = static_cast<byte>(mRefEnt.shaderRGBA[3] * (fade > 0 ? fade : 0));
    }
}

void CParticle::GroupMessage( int group, int action, const idVec3 &value ) {
    if ( mGroup < 0 || mGroup != group ) { return; }
    if ( action == 0 && mRepeat == 1 ) { mVisible = value[0] != 0; }
    if ( action == 1 && mRepeat == 2 ) { SetVel(value); }
    if ( action == 2 && mRepeat == 3 ) { mRemoveAt = theFxHelper.mTime + 100; }
}
