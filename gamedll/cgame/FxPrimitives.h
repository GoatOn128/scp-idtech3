
#if !defined(FX_SYSTEM_H_INC)
	#include "FxSystem.h"
#endif

#ifndef FX_PRIMITIVES_H_INC
#define FX_PRIMITIVES_H_INC


#define MAX_EFFECTS			16384


// Generic group flags, used by parser, then get converted to the appropriate specific flags
#define FX_PARM_MASK		0xC	// use this to mask off any transition types that use a parm
#define FX_GENERIC_MASK		0xF
#define FX_LINEAR			0x1
#define FX_RAND				0x2
#define FX_NONLINEAR		0x4
#define FX_WAVE				0x8
#define FX_CLAMP			0xC

// Group flags
#define FX_ALPHA_SHIFT		0
#define FX_ALPHA_PARM_MASK	0x0000000C
#define	FX_ALPHA_LINEAR		0x00000001
#define FX_ALPHA_RAND		0x00000002
#define	FX_ALPHA_NONLINEAR	0x00000004
#define FX_ALPHA_WAVE		0x00000008
#define FX_ALPHA_CLAMP		0x0000000C

#define FX_RGB_SHIFT		4
#define FX_RGB_PARM_MASK	0x000000C0
#define	FX_RGB_LINEAR		0x00000010
#define FX_RGB_RAND			0x00000020
#define	FX_RGB_NONLINEAR	0x00000040
#define FX_RGB_WAVE			0x00000080
#define FX_RGB_CLAMP		0x000000C0

#define FX_SIZE_SHIFT		8
#define FX_SIZE_PARM_MASK	0x00000C00
#define	FX_SIZE_LINEAR		0x00000100
#define FX_SIZE_RAND		0x00000200
#define	FX_SIZE_NONLINEAR	0x00000400
#define FX_SIZE_WAVE		0x00000800
#define FX_SIZE_CLAMP		0x00000C00

#define FX_LENGTH_SHIFT		12
#define FX_LENGTH_PARM_MASK	0x0000C000
#define	FX_LENGTH_LINEAR	0x00001000
#define FX_LENGTH_RAND		0x00002000
#define	FX_LENGTH_NONLINEAR	0x00004000
#define FX_LENGTH_WAVE		0x00008000
#define FX_LENGTH_CLAMP		0x0000C000

#define FX_SIZE2_SHIFT		16
#define FX_SIZE2_PARM_MASK	0x000C0000
#define	FX_SIZE2_LINEAR		0x00010000
#define FX_SIZE2_RAND		0x00020000
#define	FX_SIZE2_NONLINEAR	0x00040000
#define FX_SIZE2_WAVE		0x00080000
#define FX_SIZE2_CLAMP		0x000C0000

// Feature flags
#define	FX_DEPTH_HACK		0x00100000
#define	FX_RELATIVE			0x00200000
#define	FX_SET_SHADER_TIME	0x00400000		// by having the effects system set the shader time, we can make animating textures start at the correct time
#define FX_EXPENSIVE_PHYSICS 0x00800000		

#define FX_ATTACHED_MODEL	0x01000000

#define FX_APPLY_PHYSICS	0x02000000
#define FX_USE_BBOX			0x04000000		// can make physics more accurate at the expense of speed

#define FX_USE_ALPHA		0x08000000		// the FX system actually uses RGB to do fades, but this will override that
											//	and cause it to fill in the alpha.

#define FX_EMIT_FX			0x10000000		// emitters technically don't have to emit stuff, but when they do
											//	this flag needs to be set
#define FX_DEATH_RUNS_FX	0x20000000		// Normal death triggers effect, but not kill_on_impact
#define FX_KILL_ON_IMPACT	0x40000000		// works just like it says, but only when physics are on.
#define FX_IMPACT_RUNS_FX	0x80000000		// an effect can call another effect when it hits something.

// Lightning flags, duplicates of existing flags, but lightning doesn't use those flags in that context...and nothing will ever use these in this context..so we are safe.
#define FX_TAPER			0x01000000		// tapers as it moves towards its endpoint
#define FX_BRANCH			0x02000000		// enables lightning branching
#define FX_GROW				0x04000000		// lightning grows from start point to end point over the course of its life

//------------------------------
class CEffect
{
protected:

	idVec3		mOrigin1;

	int			mTimeStart;
	int			mTimeEnd;

	unsigned int	mFlags;

	// Size of our object, useful for things that have physics
	idVec3		mMin;
	idVec3		mMax;

	int			mImpactFxID;		// if we have an impact event, we may have to call an effect
	int			mDeathFxID;			// if we have a death event, we may have to call an effect

	FxRefEntity	mRefEnt;


public:

	CEffect() : mTimeStart(0), mTimeEnd(0), mFlags(0), mImpactFxID(0), mDeathFxID(0) { mOrigin1.Zero(); mMin.Zero(); mMax.Zero(); memset( &mRefEnt, 0, sizeof( FxRefEntity )); }
	virtual ~CEffect() {}
	virtual void Die() {}
	virtual void GroupMessage( int group, int action, const idVec3 &value ) {}

	virtual bool Update()
	{	// Game pausing can cause dumb time things to happen, so kill the effect in this instance
		if ( mTimeStart > theFxHelper.mTime ) {
			return false;
		}	
		return true;
	}

	inline void SetSTScale(float s,float t)	{ mRefEnt.shaderTexCoord[0]=s;mRefEnt.shaderTexCoord[1]=t;}

	inline void SetMin( const idVec3 &min )		{ mMin = min;			}
	inline void SetMin( const idVec3 *min )		{ if ( min ) { mMin = *min; } else { mMin.Zero(); }		}
	inline void SetMax( const idVec3 &max )		{ mMax = max;			}
	inline void SetMax( const idVec3 *max )		{ if ( max ) { mMax = *max; } else { mMax.Zero(); }		}
	inline void SetFlags( int flags )		{ mFlags = flags;				}
	inline void AddFlags( int flags )		{ mFlags |= flags;				}
	inline void ClearFlags( int flags )		{ mFlags &= ~flags;				}
	inline void SetOrigin1( const idVec3 &org )	{ mOrigin1 = org;	}
	inline void SetOrigin1( const idVec3 *org )	{ if ( org ) { mOrigin1 = *org; } else { mOrigin1.Zero(); }	}
	inline void SetTimeStart( int time )	{ mTimeStart = time; if (mFlags&FX_SET_SHADER_TIME) { mRefEnt.shaderTime = cg.time * 0.001f; }}
	inline void	SetTimeEnd( int time )		{ mTimeEnd = time;				}
	inline void SetImpactFxID( int id )		{ mImpactFxID = id;				}
	inline void SetDeathFxID( int id )		{ mDeathFxID = id;				}
};


class CLight : public CEffect
{
protected:

	float		mSizeStart;
	float		mSizeEnd;
	float		mSizeParm;

	idVec3		mRGBStart;
	idVec3		mRGBEnd;
	float		mRGBParm;


	void		UpdateSize();
	void		UpdateRGB();

	void Draw()
	{
		theFxHelper.AddLightToScene( mOrigin1, mRefEnt.radius, 
			mRefEnt.lightingOrigin[0], mRefEnt.lightingOrigin[1], mRefEnt.lightingOrigin[2] );
	}

public:

	CLight() {}
	virtual ~CLight() {}
	virtual bool Update();
	
	inline void SetSizeStart( float sz )	{ mSizeStart = sz;			}
	inline void SetSizeEnd( float sz )		{ mSizeEnd = sz;			}
	inline void SetSizeParm( float parm )	{ mSizeParm = parm;			}

	inline void SetRGBStart( const idVec3 &rgb )	{ mRGBStart = rgb;	}
	inline void SetRGBStart( const idVec3 *rgb )	{ if ( rgb ) { mRGBStart = *rgb; } else { mRGBStart.Zero(); }	}
	inline void SetRGBEnd( const idVec3 &rgb )		{ mRGBEnd = rgb;		}
	inline void SetRGBEnd( const idVec3 *rgb )		{ if ( rgb ) { mRGBEnd = *rgb; } else { mRGBEnd.Zero(); }		}
	inline void SetRGBParm( float parm )	{ mRGBParm = parm;			}
};

//------------------------------
class CFlash : public CLight
{
protected:

	virtual void Draw();

public:

	CFlash() {}
	virtual ~CFlash() {}

	virtual bool Update();

	inline void SetShader( qhandle_t sh )	{ mRefEnt.customShader = sh;				}
	void		Init( void );
};

//------------------------------
class CPrimitiveTemplate;

class CParticle : public CEffect
{
protected:
    std::vector<qhandle_t> mAnimation;
    int mRepeat = 0, mGroup = -1, mRemoveAt = 0;
    float mHeightMin = 0, mHeightMax = 0;
    bool mVisible = true;
	int mCollisionMask = MASK_SHOT | CONTENTS_WATER;
	float mFadeIn = 0;
    void UpdateAnimation();


	idVec3		mOrgOffset;

	idVec3		mVel;
	idVec3		mAccel;
	float		mGravity;

	float		mSizeStart;
	float		mSizeEnd;
	float		mSizeParm;

	idVec3		mRGBStart;
	idVec3		mRGBEnd;
	float		mRGBParm;
	
	float		mAlphaStart;
	float		mAlphaEnd;
	float		mAlphaParm;

	float		mRotationDelta;
	float		mElasticity;

	int			mClientID;

	bool		UpdateOrigin();
	void		UpdateVelocity() {( mVel = mVel + mAccel * theFxHelper.mFloatFrameTime ); }

	void		UpdateSize();
	void		UpdateRGB();
	void		UpdateAlpha();
	void		UpdateRotation() { mRefEnt.rotation += theFxHelper.mFrameTime * 0.01f * mRotationDelta; }

	virtual bool Cull();
	virtual void Draw();
	
public:

	void Configure( const CPrimitiveTemplate &definition );
    void GroupMessage( int group, int action, const idVec3 &value ) override;
    inline CParticle() { mRefEnt.reType = RT_SPRITE; mClientID = -1; }
	virtual ~CParticle() {}

	virtual void Die();
	virtual bool Update();

	inline void SetShader( qhandle_t sh )	{ mRefEnt.customShader = sh;				}

	inline void SetOrgOffset( const idVec3 &o )	{ mOrgOffset = o; }
	inline void SetOrgOffset( const idVec3 *o )	{ if ( o ) { mOrgOffset = *o; } else { mOrgOffset.Zero(); }}
	inline void SetVel( const idVec3 &vel )		{ mVel = vel;			}
	inline void SetVel( const idVec3 *vel )		{ if ( vel ) { mVel = *vel; } else { mVel.Zero(); }	}
	inline void SetAccel( const idVec3 &ac )		{ mAccel = ac;			}
	inline void SetAccel( const idVec3 *ac )		{ if ( ac ) { mAccel = *ac; } else { mAccel.Zero(); }	}
	inline void SetGravity( float grav )	{ mGravity = grav;			}

	inline void SetSizeStart( float sz )	{ mSizeStart = sz;			}
	inline void SetSizeEnd( float sz )		{ mSizeEnd = sz;			}
	inline void SetSizeParm( float parm )	{ mSizeParm = parm;			}

	inline void SetRGBStart( const idVec3 &rgb )	{ mRGBStart = rgb;	}
	inline void SetRGBStart( const idVec3 *rgb )	{ if ( rgb ) { mRGBStart = *rgb; } else { mRGBStart.Zero(); }	}
	inline void SetRGBEnd( const idVec3 &rgb )		{ mRGBEnd = rgb;		}
	inline void SetRGBEnd( const idVec3 *rgb )		{ if ( rgb ) { mRGBEnd = *rgb; } else { mRGBEnd.Zero(); }		}
	inline void SetRGBParm( float parm )	{ mRGBParm = parm;			}

	inline void SetAlphaStart( float al )	{ mAlphaStart = al;			}
	inline void SetAlphaEnd( float al )		{ mAlphaEnd = al;			}
	inline void SetAlphaParm( float parm )	{ mAlphaParm = parm;		}

	inline void SetRotation( float rot )		{ mRefEnt.rotation = rot;	}
	inline void SetRotationDelta( float rot )	{ mRotationDelta = rot;		}
	inline void SetElasticity( float el )		{ mElasticity = el;			}

	inline void SetClient( int clientID )	{ mClientID = clientID;				}
};


//------------------------------
class CLine : public CParticle
{
protected:

	idVec3	mOrigin2;

	virtual void Draw();

public:

	CLine() { mRefEnt.reType = RT_LINE;}
	virtual ~CLine() {}
	virtual void Die() {}
	virtual void GroupMessage( int group, int action, const idVec3 &value ) {}
	virtual bool Update();


	inline void SetOrigin2( const idVec3 &org2 )	{ mOrigin2 = org2; }
};

//------------------------------
class CBezier : public CLine
{
protected:

	idVec3	mControl1;
	idVec3	mControl1Vel;

	idVec3	mControl2;
	idVec3	mControl2Vel;

	bool	mInit;

	virtual void Draw();

public:

	CBezier(){ mInit = false; }
	virtual ~CBezier() {}
	virtual void Die() {}
	virtual void GroupMessage( int group, int action, const idVec3 &value ) {}

	virtual bool Update();
	
	void DrawSegment( const idVec3 &start, const idVec3 &end, float texcoord1, float texcoord2 );

	inline void SetControlPoints( const idVec3 &ctrl1, const idVec3 &ctrl2 )	{ mControl1 = ctrl1; mControl2 = ctrl2; }
	inline void SetControlVel( const idVec3 &ctrl1v, const idVec3 &ctrl2v )	{ mControl1Vel = ctrl1v; mControl2Vel = ctrl2v; }
};


//------------------------------
class CElectricity : public CLine
{
protected:

	float	mChaos;

	virtual void Draw();

public:

	CElectricity() { mRefEnt.reType = RT_ELECTRICITY; }
	virtual ~CElectricity() {}
	virtual void Die() {}
	virtual void GroupMessage( int group, int action, const idVec3 &value ) {}

	virtual bool Update();

	void Initialize();

	inline void SetChaos( float chaos )		{ mChaos = chaos; }
};


// Oriented quad
//------------------------------
class COrientedParticle : public CParticle
{
protected:

	idVec3	mNormal;

	virtual bool Cull();
	virtual void Draw();

public:

	COrientedParticle() { mRefEnt.reType = RT_ORIENTED_QUAD; }
	virtual ~COrientedParticle() {}

	virtual bool Update();

	inline void SetNormal( const idVec3 &norm )	{ mNormal = norm;	}
};

//------------------------------
class CTail : public CParticle
{
protected:

	idVec3	mOldOrigin;

	float	mLengthStart;
	float	mLengthEnd;
	float	mLengthParm;

	float	mLength;

	void	UpdateLength();
	void	CalcNewEndpoint();

	virtual void Draw();
	virtual bool Cull();

public:

	CTail() { mRefEnt.reType = RT_LINE; }
	virtual ~CTail() {}

	virtual bool Update();

	inline void SetLengthStart( float len )	{ mLengthStart = len;	}
	inline void SetLengthEnd( float len )	{ mLengthEnd = len;	}
	inline void SetLengthParm( float len )	{ mLengthParm = len;	}
};


//------------------------------
class CCylinder : public CTail
{
protected:

	float		mSize2Start;
	float		mSize2End;
	float		mSize2Parm;

	void	UpdateSize2();

	virtual void Draw();

public:

	CCylinder() { mRefEnt.reType = RT_CYLINDER; }
	virtual ~CCylinder() {}

	virtual bool Update();

	inline void SetSize2Start( float sz )	{ mSize2Start = sz;			}
	inline void SetSize2End( float sz )		{ mSize2End = sz;			}
	inline void SetSize2Parm( float parm )	{ mSize2Parm = parm;		}

	inline void SetNormal( const idVec3 &norm )	{ mRefEnt.axis[0] = norm; }
};


//------------------------------
// Emitters are derived from particles because, although they don't draw, any effect called
//	from them can borrow an initial or ending value from the emitters current alpha, rgb, etc..
class CEmitter : public CParticle
{
protected:

	idVec3		mOldOrigin;		// we use these to do some nice
	idVec3		mLastOrigin;	//	tricks...
	idVec3		mOldVelocity;	//	
	int			mOldTime;

	idVec3		mAngles;		// for a rotating thing, using a delta  
	idVec3		mAngleDelta;	//	as opposed to an end angle is probably much easier

	int			mEmitterFxID;	// if we have emitter fx, this is our id

	float		mDensity;		// controls how often emitter chucks an effect
	float		mVariance;		// density sloppiness

	void		UpdateAngles();

	virtual void Draw();

public:

	CEmitter() {
		// There may or may not be a model, but if there isn't one, 
		//	we just won't bother adding the refEnt in our Draw func
		mRefEnt.reType = RT_MODEL;
	}
	virtual ~CEmitter() {}

	virtual bool Update();

	inline void SetModel( qhandle_t model )		{ mRefEnt.hModel = model;	}
	inline void SetAngles( const idVec3 &ang )	{ mAngles = ang;			}
	inline void SetAngles( const idVec3 *ang )	{ if ( ang ) { mAngles = *ang; } else { mAngles.Zero(); }		}
	inline void SetAngleDelta( const idVec3 &ang){ mAngleDelta = ang;	}
	inline void SetAngleDelta( const idVec3 *ang){ if ( ang ) { mAngleDelta = *ang; } else { mAngleDelta.Zero(); }	}
	inline void SetEmitterFxID( int id )		{ mEmitterFxID = id;		}
	inline void SetDensity( float density )		{ mDensity = density;		}
	inline void SetVariance( float var )		{ mVariance = var;			}
	inline void SetOldTime( int time )			{ mOldTime = time;			}
	inline void SetLastOrg( const idVec3 &org )	{ mLastOrigin = org;	}
	inline void SetLastOrg( const idVec3 *org )	{ if ( org ) { mLastOrigin = *org; } else { mLastOrigin.Zero(); }	}
	inline void SetLastVel( const idVec3 &vel )	{ mOldVelocity = vel;	}
	inline void SetLastVel( const idVec3 *vel )	{ if ( vel ) { mOldVelocity = *vel; } else { mOldVelocity.Zero(); }}

};

// We're getting pretty low level here, not the kind of thing to abuse considering how much overhead this
//	adds to a SINGLE triangle or quad....
// The editor doesn't need to see or do anything with this
//------------------------------
#define MAX_CPOLY_VERTS	5

class CPoly : public CParticle
{
protected:

	int		mCount;
	idVec3	mRotDelta;
	int		mTimeStamp;

	virtual bool Cull();
	virtual void Draw();

public:

	idVec3	mOrg[MAX_CPOLY_VERTS];
	idVec2	mST[MAX_CPOLY_VERTS];

	float	mRot[3][3];
	int		mLastFrameTime;


	CPoly()				{}
	virtual ~CPoly()	{}

	virtual bool Update();

	void PolyInit();
	void CalcRotateMatrix();
	void Rotate();

	inline void SetNumVerts( int c )					{ mCount = c;			}
	inline void SetRot( const idVec3 &r )						{ mRotDelta = r; }
	inline void SetRot( const idVec3 *r )						{ if ( r ) { mRotDelta = *r; } else { mRotDelta.Zero(); }}
	inline void SetMotionTimeStamp( int t )				{ mTimeStamp = theFxHelper.mTime + t; }
	inline int	GetMotionTimeStamp()					{ return mTimeStamp; }
};


#endif //FX_PRIMITIVES_H_INC