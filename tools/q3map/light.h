
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "polylib.h"
#include "imagelib.h"
#include "threads.h"
#include "scriplib.h"

#include "shaders.h"
#include "mesh.h"



typedef enum
{
	emit_point,
	emit_area,
	emit_spotlight,
	emit_sun
} emittype_t;

#define	MAX_LIGHT_EDGES		8
typedef struct light_s
{
	struct light_s *next;
	emittype_t	type;
	struct shaderInfo_s	*si;

	idVec3		origin;
	idVec3		normal;		// for surfaces, spotlights, and suns
	float		dist;		// plane location along normal

	qboolean	linearLight;
	int			photons;
	int			style;
	idVec3		color;
	float		radiusByDist;	// for spotlights

	qboolean	twosided;		// fog lights both sides

	winding_t	*w;
	idVec3		emitColor;		// full out-of-gamut value
} light_t;


extern	float	lightscale;
extern	float	ambient;
extern	float	maxlight;
extern	float	direct_scale;
extern	float	entity_scale;

extern	qboolean	noSurfaces;

//===============================================================

// light_trace.c

// a facet is a subdivided element of a patch aproximation or model
typedef struct cFacet_s {
	idVec4	surface;
	int		numBoundaries;		// either 3 or 4, anything less is degenerate
	idVec4	boundaries[4];	// positive is outside the bounds

	idVec3	points[4];			// needed for area light subdivision

	float	textureMatrix[2][4];	// compute texture coordinates at point of impact for translucency
} cFacet_t;

typedef struct {
	idVec3		mins, maxs;
	idVec3		origin;
	float		radius;

	qboolean	patch;

	int			numFacets;
	cFacet_t	*facets;

	shaderInfo_t	*shader;		// for translucency
} surfaceTest_t;


typedef struct {
	idVec3		filter;				// starts out 1.0, 1.0, 1.0, may be reduced if
									// transparent surfaces are crossed

	idVec3		hit;				// the impact point of a completely opaque surface
	float		hitFraction;		// 0 = at start, 1.0 = at end
	qboolean	passSolid;
} idQ3MapTrace;

extern	surfaceTest_t	*surfaceTest[MAX_MAP_DRAW_SURFS];

void	InitTrace( void );

// traceWork_t is only a parameter to crutch up poor large local allocations on
// winNT and macOS.  It should be allocated in the worker function, but never
// looked at.
typedef struct {
	idVec3		start, end;
	int			numOpenLeafs;
	int			openLeafNumbers[MAX_MAP_LEAFS];
	idQ3MapTrace		*trace;
	int			patchshadows;
} traceWork_t;

void TraceLine( const idVec3 start, const idVec3 stop, idQ3MapTrace *trace,
			   qboolean testAll, traceWork_t *tw );
qboolean PointInSolid( idVec3 start );

//===============================================================

//===============================================================


typedef struct {
	int		textureNum;
	int		x, y, width, height;

	// for patches
	qboolean	patch;
	mesh_t		mesh;

	// for faces
	idVec3	origin;
	idVec3	vecs[3];
} lightmap_t;
