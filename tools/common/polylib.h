
typedef struct
{
	int		numpoints;
	idVec3	p[4];		// variable sized
} winding_t;

#define	MAX_POINTS_ON_WINDING	64

// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1
#endif

winding_t	*AllocWinding (int points);
vec_t	WindingArea (winding_t *w);
void	WindingCenter (winding_t *w, idVec3 center);
void	ClipWindingEpsilon (winding_t *in, idVec3 normal, vec_t dist, 
				vec_t epsilon, winding_t **front, winding_t **back);
winding_t	*ChopWinding (winding_t *in, idVec3 normal, vec_t dist);
winding_t	*CopyWinding (winding_t *w);
winding_t	*ReverseWinding (winding_t *w);
winding_t	*BaseWindingForPlane (idVec3 normal, vec_t dist);
void	CheckWinding (winding_t *w);
void	WindingPlane (winding_t *w, idVec3 normal, vec_t *dist);
void	RemoveColinearPoints (winding_t *w);
int		WindingOnPlaneSide (winding_t *w, idVec3 normal, vec_t dist);
void	FreeWinding (winding_t *w);
void	WindingBounds (winding_t *w, idVec3 mins, idVec3 maxs);

void	AddWindingToConvexHull( winding_t *w, winding_t **hull, idVec3 normal );

void	ChopWindingInPlace (winding_t **w, idVec3 normal, vec_t dist, vec_t epsilon);
// frees the original if clipped

void pw(winding_t *w);
