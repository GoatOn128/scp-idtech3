// Connected Wolf trails, owned and updated by the FX primitive runtime.
#include "FxScheduler.h"
#include "cg_fx.h"
#include <deque>
#include <algorithm>

struct FxTrailPoint {
	vec3_t origin, rgbStart, rgbEnd;
	float widthStart, widthEnd, alphaStart, alphaEnd, s;
	int start, life;
};

class CWolfTrail;
static std::map<int, CWolfTrail *> fxTrails;
static int fxNextTrail = 1;

class CWolfTrail : public CEffect {
	std::deque<FxTrailPoint> points;
	qhandle_t shader;
	int flags, textureMode, handle;
public:
	CWolfTrail( qhandle_t material, int spawnFlags, int mapping ) :
		shader( material ), flags( spawnFlags ), textureMode( mapping ), handle( 0 ) {
		SetFlags( 0 );
	}
	~CWolfTrail() { fxTrails.erase( handle ); }
	int Append( FxTrailPoint point, float ratio, float speed ) {
		if ( points.empty() ) {
			point.s = ratio > 0 ? speed * ( 1 - ( cg.time % 1000 ) / 1000.0f ) / ratio : 0;
		} else {
			float scale = ratio * point.widthEnd;
			point.s = points.back().s + ( scale > 0 ? Distance( point.origin, points.back().origin ) / scale : 0 );
		}
		points.push_back( point );
		if ( points.size() > 4096 ) { points.pop_front(); }
		fxTrails.erase( handle );
		// IDs are never reused while the module is loaded, including map_restart.
		if ( fxNextTrail == 0x7fffffff ) { fxNextTrail = 1; }
		while ( fxTrails.count( fxNextTrail ) ) { fxNextTrail++; }
		handle = fxNextTrail++;
		fxTrails[handle] = this;
		return handle;
	}
	void VertexPair( const FxTrailPoint &point, const vec3_t side, float s, polyVert_t *verts ) {
		float fraction = ( theFxHelper.mTime - point.start ) / static_cast<float>( point.life );
		fraction = fraction < 0 ? 0 : ( fraction > 1 ? 1 : fraction );
		float width = point.widthStart + fraction * ( point.widthEnd - point.widthStart );
		float alpha = point.alphaStart + fraction * ( point.alphaEnd - point.alphaStart );
		if ( flags & TJFL_FADEIN ) { alpha *= fraction < 0.1f ? fraction * 10 : 1; }
		if ( !( flags & TJFL_NOCULL ) && width > 0 ) {
			float distance = Distance( point.origin, cg.refdef.vieworg );
			alpha *= distance < width * 2 ? distance / ( width * 2 ) : 1;
		}
		alpha = alpha < 0 ? 0 : ( alpha > 1 ? 1 : alpha );
		for ( int i = 0; i < 2; i++ ) {
			VectorMA( point.origin, ( i ? -0.5f : 0.5f ) * width, side, verts[i].xyz );
			verts[i].st[0] = s;
			verts[i].st[1] = static_cast<float>( i );
			for ( int j = 0; j < 3; j++ ) {
				float color = point.rgbStart[j] + fraction * ( point.rgbEnd[j] - point.rgbStart[j] );
				verts[i].modulate[j] = static_cast<byte>( 255 * ( color < 0 ? 0 : ( color > 1 ? 1 : color ) ) );
			}
			verts[i].modulate[3] = static_cast<byte>( 255 * alpha );
		}
	}
	bool Update() override {
		while ( !points.empty() && theFxHelper.mTime >= points.front().start + points.front().life ) {
			points.pop_front();
		}
		if ( points.empty() ) { return false; }
		for ( size_t i = 1; i < points.size(); i++ ) {
			vec3_t direction, view, side;
			VectorSubtract( points[i].origin, points[i - 1].origin, direction );
			if ( VectorNormalize( direction ) == 0 ) { continue; }
			VectorSubtract( cg.refdef.vieworg, points[i].origin, view );
			CrossProduct( direction, view, side );
			if ( VectorNormalize( side ) == 0 ) { PerpendicularVector( side, direction ); }
			int planes = ( flags & TJFL_CROSSOVER ) ? 2 : 1;
			for ( int plane = 0; plane < planes; plane++ ) {
				polyVert_t ends[4], polygon[4];
				float startS = textureMode == STYPE_REPEAT ? points[i - 1].s : ( i - 1.0f ) / ( points.size() - 1 );
				float endS = textureMode == STYPE_REPEAT ? points[i].s : static_cast<float>( i ) / ( points.size() - 1 );
				VertexPair( points[i - 1], side, startS, ends );
				VertexPair( points[i], side, endS, ends + 2 );
				polygon[0] = ends[0]; polygon[1] = ends[1]; polygon[2] = ends[3]; polygon[3] = ends[2];
				if ( flags & TJFL_FIXDISTORT ) {
					polyVert_t middle = {};
					float color[4] = {};
					for ( int corner = 0; corner < 4; corner++ ) {
						VectorMA(middle.xyz, 0.25f, polygon[corner].xyz, middle.xyz);
						for ( int axis = 0; axis < 2; axis++ ) { middle.st[axis] += polygon[corner].st[axis] * 0.25f; }
						for ( int channel = 0; channel < 4; channel++ ) { color[channel] += polygon[corner].modulate[channel] * 0.25f; }
					}
					for ( int channel = 0; channel < 4; channel++ ) { middle.modulate[channel] = static_cast<byte>(color[channel]); }
					for ( int corner = 0; corner < 4; corner++ ) {
						polyVert_t triangle[3] = { polygon[corner], polygon[(corner + 1) % 4], middle };
						theFxHelper.AddPolyToScene(shader, 3, triangle);
					}
				} else {
					theFxHelper.AddPolyToScene( shader, 4, polygon );
				}
				vec3_t perpendicular;
				CrossProduct( direction, side, perpendicular );
				VectorCopy( perpendicular, side );
			}
		}
		return true;
	}
};


static bool TrailBinds( const CPrimitiveTemplate &definition, const char *field ) {
	return (" " + definition.mBindings + " ").find(" " + std::string(field) + " ") != std::string::npos;
}

int CG_FxTrail( const char *name, int head, const vec3_t position, const FxTrailInputs &input ) {
	if ( theFxHelper.mTimeFrozen ) { return head; }
	const CPrimitiveTemplate *definition = theFxScheduler.Definition( name );
	if ( !definition || definition->mType != Ribbon ) { return head; }
	const CPrimitiveTemplate &d = *definition;
	int life = TrailBinds(d, "life") && input.life >= 0 ? input.life : d.mLife.GetVal();
	qhandle_t shader = TrailBinds(d, "shader") && input.shader ? input.shader : d.mMediaHandles.GetHandle();
	if ( life <= 0 || !shader ) { return head; }
	int flags = TrailBinds(d, "flags") && input.flags >= 0 ? input.flags : CG_FxValue(name, "flags");
	int mapping = TrailBinds(d, "mapping") && input.mapping >= 0 ? input.mapping : CG_FxValue(name, "mapping");
	float ratio = TrailBinds(d, "ratio") && input.ratio >= 0 ? input.ratio : CG_FxValue(name, "ratio");
	float speed = TrailBinds(d, "speed") && input.speed >= 0 ? input.speed : CG_FxValue(name, "speed");
	FxTrailPoint point;
	VectorCopy( position, point.origin );
	VectorSet( point.rgbStart, d.mRedStart.GetVal(), d.mGreenStart.GetVal(), d.mBlueStart.GetVal() );
	VectorSet( point.rgbEnd, d.mRedEnd.GetVal(), d.mGreenEnd.GetVal(), d.mBlueEnd.GetVal() );
	if ( TrailBinds(d, "color") && input.color[0] >= 0 ) { VectorCopy(input.color, point.rgbStart); }
	if ( TrailBinds(d, "colorEnd") && input.colorEnd[0] >= 0 ) { VectorCopy(input.colorEnd, point.rgbEnd); }
	point.start = theFxHelper.mTime - ( input.time < 0 ? 0 : cg.time - input.time );
	point.life = life;
	point.widthStart = TrailBinds(d, "width") && input.width >= 0 ? input.width : d.mSizeStart.GetVal();
	point.widthEnd = TrailBinds(d, "widthEnd") && input.widthEnd >= 0 ? input.widthEnd : d.mSizeEnd.GetVal();
	point.alphaStart = TrailBinds(d, "alpha") && input.alpha >= 0 ? input.alpha : d.mAlphaStart.GetVal();
	point.alphaEnd = TrailBinds(d, "alphaEnd") && input.alphaEnd >= 0 ? input.alphaEnd : d.mAlphaEnd.GetVal();
	if ( CG_FxValue(name, "sparkColor") ) {
		point.rgbStart[1] += CG_FxValue(name, "sparkGreen") * point.alphaStart;
		point.rgbEnd[1] += CG_FxValue(name, "sparkGreen") * point.alphaEnd;
		point.rgbStart[2] += CG_FxValue(name, "sparkBlue") * point.alphaStart;
		point.rgbEnd[2] += CG_FxValue(name, "sparkBlue") * point.alphaEnd;
	}
	float alphaScale = CG_FxValue(name, "alphaScale");
	point.alphaStart *= alphaScale;
	point.alphaEnd *= alphaScale;
	CWolfTrail *trail;
	auto existing = fxTrails.find( head );
	if ( existing == fxTrails.end() ) {
		trail = new CWolfTrail( shader, flags, mapping );
		CEffect *effect = trail;
		FX_AddPrimitive( &effect, 24 * 60 * 60 * 1000 );
	} else {
		trail = existing->second;
	}
	return trail->Append( point, ratio, speed );
}
