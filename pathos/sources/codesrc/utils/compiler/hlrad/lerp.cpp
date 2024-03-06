#include "qrad.h"

#pragma warning(disable: 4018)

int             g_lerp_enabled = DEFAULT_LERP_ENABLED;
#ifdef HLRAD_LERP_VL
static bool		LerpTriangle(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
					vec3_t &result_direction, 
	#endif
					int pt1, int pt2, int pt3, int style);
static bool		LerpEdge(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
					vec3_t &result_direction, 
	#endif
					int pt1, int pt2, int style);
static bool		LerpNearest(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
					vec3_t &result_direction, 
	#endif
					int pt1, int style);
#define LERP_EPSILON 0.5
#endif

#ifndef HLRAD_LERP_VL
// =====================================================================================
//  TestWallIntersectTri
//      Returns true if wall polygon intersects patch polygon
// =====================================================================================
static bool     TestWallIntersectTri(const lerpTriangulation_t* const trian, const vec3_t p1, const vec3_t p2, const vec3_t p3)
{
#ifdef HLRAD_LERP_VL
	int x;
    const lerpWall_t* wall;
	const vec_t* normal = trian->plane->normal;
	{
		vec3_t d1, d2, n;
		VectorSubtract (p3, p2, d1);
		VectorSubtract (p1, p2, d2);
		CrossProduct (d1, d2, n);
		if (DotProduct (n, normal) < 0)
		{
			const vec_t* tmp;
			tmp = p2;
			p2 = p3;
			p3 = tmp;
		}
	}
	for (x = 0, wall = trian->walls; x < trian->numwalls; x++, wall++)
	{
		if (point_in_tri (wall->vertex0, trian->plane, p1, p2, p3))
		{
			return true;
		}
		if (point_in_tri (wall->vertex1, trian->plane, p1, p2, p3))
		{
			return true;
		}
	}
	return false;
#else
    int             x;
    const lerpWall_t* wall = trian->walls;
    dplane_t        plane;

    plane_from_points(p1, p2, p3, &plane);

    // Try first 'vertical' side
    // Since we test each of the 3 segments from patch against wall, only one side of wall needs testing inside 
    // patch (since they either dont intersect at all at this point, or both line segments intersect inside)
    for (x = 0; x < trian->numwalls; x++, wall++)
    {
        vec3_t          point;

        // Try side A
        if (intersect_linesegment_plane(&plane, wall->vertex[0], wall->vertex[3], point))
        {
            if (point_in_tri(point, &plane, p1, p2, p3))
            {
#if 0
                Verbose
                    ("Wall side A point @ (%4.3f %4.3f %4.3f) inside patch (%4.3f %4.3f %4.3f) (%4.3f %4.3f %4.3f) (%4.3f %4.3f %4.3f)\n",
                     point[0], point[1], point[2], p1[0], p1[1], p1[2], p2[0], p2[1], p2[2], p3[0], p3[1], p3[2]);
#endif
                return true;
            }
        }
    }
    return false;
#endif
}
#endif

// =====================================================================================
//  TestLineSegmentIntersectWall
//      Returns true if line would hit the 'wall' (to fix light streaking)
// =====================================================================================
static bool     TestLineSegmentIntersectWall(const lerpTriangulation_t* const trian, const vec3_t p1, const vec3_t p2)
{
    int             x;
    const lerpWall_t* wall = trian->walls;

    for (x = 0; x < trian->numwalls; x++, wall++)
    {
#ifdef HLRAD_LERP_VL
		vec_t front, back, frac;
		vec3_t mid;
		front = DotProduct (p1, wall->plane.normal) - wall->plane.dist;
		back = DotProduct (p2, wall->plane.normal) - wall->plane.dist;
		if (front > ON_EPSILON/2 && back > ON_EPSILON/2 || front < -ON_EPSILON/2 && back < -ON_EPSILON/2)
		{
			continue;
		}
		if (fabs (front) <= ON_EPSILON && fabs (back) <= ON_EPSILON)
		{
			if (DotProduct (p1, wall->increment) < DotProduct (wall->vertex0, wall->increment) - ON_EPSILON &&
				DotProduct (p2, wall->increment) < DotProduct (wall->vertex0, wall->increment) - ON_EPSILON )
			{
				continue;
			}
			if (DotProduct (p1, wall->increment) > DotProduct (wall->vertex1, wall->increment) + ON_EPSILON &&
				DotProduct (p2, wall->increment) > DotProduct (wall->vertex1, wall->increment) + ON_EPSILON )
			{
				continue;
			}
			return true;
		}
		frac = front / (front - back);
		mid[0] = p1[0] + (p2[0] - p1[0]) * frac;
		mid[1] = p1[1] + (p2[1] - p1[1]) * frac;
		mid[2] = p1[2] + (p2[2] - p1[2]) * frac;
		if (DotProduct (mid, wall->increment) < DotProduct (wall->vertex0, wall->increment) - ON_EPSILON ||
			DotProduct (mid, wall->increment) > DotProduct (wall->vertex1, wall->increment) + ON_EPSILON )
		{
			continue;
		}
		return true;
#else
        vec3_t          point;

        if (intersect_linesegment_plane(&wall->plane, p1, p2, point))
        {
            if (point_in_wall(wall, point))
            {
#if 0
                Verbose
                    ("Tested point @ (%4.3f %4.3f %4.3f) blocks segment from (%4.3f %4.3f %4.3f) to (%4.3f %4.3f %4.3f) intersects wall\n",
                     point[0], point[1], point[2], p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
#endif
                return true;
            }
        }
#endif
    }
    return false;
}

#ifndef HLRAD_LERP_VL
// =====================================================================================
//  TestTriIntersectWall
//      Returns true if line would hit the 'wall' (to fix light streaking)
// =====================================================================================
static bool     TestTriIntersectWall(const lerpTriangulation_t* trian, const vec3_t p1, const vec3_t p2,
                                     const vec3_t p3)
{
    if (TestLineSegmentIntersectWall(trian, p1, p2) || TestLineSegmentIntersectWall(trian, p1, p3)
        || TestLineSegmentIntersectWall(trian, p2, p3))
    {
        return true;
    }
    return false;
}
#endif

// =====================================================================================
//  LerpTriangle
//      pt1 must be closest point
// =====================================================================================
#ifdef HLRAD_LERP_VL
static bool LerpTriangle(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
				vec3_t &result_direction, 
	#endif
				int pt1, int pt2, int pt3, int style)
{
    patch_t *p1;
    patch_t *p2;
    patch_t *p3;
	const vec_t *o1;
	const vec_t *o2;
	const vec_t *o3;
	vec3_t v;

    p1 = trian->points[pt1];
    p2 = trian->points[pt2];
    p3 = trian->points[pt3];
#ifdef HLRAD_LERP_TEXNORMAL
	o1 = trian->points_pos[pt1];
	o2 = trian->points_pos[pt2];
	o3 = trian->points_pos[pt3];
#else
	o1 = p1->origin;
	o2 = p2->origin;
	o3 = p3->origin;
#endif

	dplane_t edge12, edge13, edge23;

	VectorSubtract (o2, o1, v);
	CrossProduct (trian->plane->normal, v, edge12.normal);
	VectorNormalize (edge12.normal);
	edge12.dist = DotProduct (o1, edge12.normal);

	VectorSubtract (o3, o1, v);
	CrossProduct (trian->plane->normal, v, edge13.normal);
	VectorNormalize (edge13.normal);
	edge13.dist = DotProduct (o1, edge13.normal);

	VectorSubtract (o3, o2, v);
	CrossProduct (trian->plane->normal, v, edge23.normal);
	VectorNormalize (edge23.normal);
	edge23.dist = DotProduct (o2, edge23.normal);

	vec_t dist12 = DotProduct (point, edge12.normal) - edge12.dist;
	vec_t dist13 = DotProduct (point, edge13.normal) - edge13.dist;
	vec_t dist23 = DotProduct (point, edge23.normal) - edge23.dist;

	// the point could be on the edge.
	if (fabs (dist12) < LERP_EPSILON)
	{
		if (LerpEdge (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt1, pt2, style))
		{
			return true;
		}
	}
	if (fabs (dist13) < LERP_EPSILON)
	{
		if (LerpEdge (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt1, pt3, style))
		{
			return true;
		}
	}
	if (fabs (dist23) < LERP_EPSILON)
	{
		if (LerpEdge (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt2, pt3, style))
		{
			return true;
		}
	}

	// now that the point is not on the edge, we should check the shape of the triangle and make sure that the point is precisely inside the triangle.
	vec_t maxdist12 = DotProduct (o3, edge12.normal) - edge12.dist;
	vec_t maxdist13 = DotProduct (o2, edge13.normal) - edge13.dist;
	vec_t maxdist23 = DotProduct (o1, edge23.normal) - edge23.dist;
	if (fabs (maxdist12) <= LERP_EPSILON || fabs (maxdist13) <= LERP_EPSILON || fabs (maxdist23) <= LERP_EPSILON)
	{
		return false;
	}
	if (dist12 / maxdist12 < 0 || dist12 / maxdist12 > 1 ||
		dist13 / maxdist13 < 0 || dist13 / maxdist13 > 1 ||
		dist23 / maxdist23 < 0 || dist23 / maxdist23 > 1 )
	{
		return false;
	}

	vec3_t testpoint;
	VectorScale (p1->origin, 1 - dist13 / maxdist13 - dist12 / maxdist12, testpoint);
	VectorMA (testpoint, dist13 / maxdist13, p2->origin, testpoint);
	VectorMA (testpoint, dist12 / maxdist12, p3->origin, testpoint);
	if (TestLineSegmentIntersectWall (trian, testpoint, p1->origin) ||
		TestLineSegmentIntersectWall (trian, testpoint, p2->origin) ||
		TestLineSegmentIntersectWall (trian, testpoint, p3->origin) ||
		TestLineSegmentIntersectWall (trian, p2->origin, p1->origin) ||
		TestLineSegmentIntersectWall (trian, p3->origin, p2->origin) ||
		TestLineSegmentIntersectWall (trian, p1->origin, p3->origin) )
	{
		return false;
	}

	const vec3_t *l1, *l2, *l3;
	#ifdef ZHLT_XASH
	const vec3_t *l1_d, *l2_d, *l3_d;
	#endif
	l1 = GetTotalLight(p1, style
	#ifdef ZHLT_XASH
			, l1_d
	#endif
			);
	l2 = GetTotalLight(p2, style
	#ifdef ZHLT_XASH
			, l2_d
	#endif
			);
	l3 = GetTotalLight(p3, style
	#ifdef ZHLT_XASH
			, l3_d
	#endif
			);

	VectorScale (*l1, 1 - dist13 / maxdist13 - dist12 / maxdist12, result);
	VectorMA (result, dist13 / maxdist13, *l2, result);
	VectorMA (result, dist12 / maxdist12, *l3, result);
	#ifdef ZHLT_XASH
	VectorScale (*l1_d, 1 - dist13 / maxdist13 - dist12 / maxdist12, result_direction);
	VectorMA (result_direction, dist13 / maxdist13, *l2_d, result_direction);
	VectorMA (result_direction, dist12 / maxdist12, *l3_d, result_direction);
	#endif

	return true;
}
#else
#ifdef ZHLT_TEXLIGHT
static void     LerpTriangle(const lerpTriangulation_t* const trian, const vec3_t point, vec3_t result, const unsigned pt1, const unsigned pt2, const unsigned pt3, int style) //LRC
#else
static void     LerpTriangle(const lerpTriangulation_t* const trian, const vec3_t point, vec3_t result, const unsigned pt1, const unsigned pt2, const unsigned pt3)
#endif
{
    patch_t*        p1;
    patch_t*        p2;
    patch_t*        p3;
    vec3_t          base;
    vec3_t          d1;
    vec3_t          d2;
    vec_t           x;
    vec_t           y;
    vec_t           y1;
    vec_t           x2;
    vec3_t          v;
    dplane_t        ep1;
    dplane_t        ep2;

    p1 = trian->points[pt1];
    p2 = trian->points[pt2];
    p3 = trian->points[pt3];

#ifdef ZHLT_TEXLIGHT
    VectorCopy(*GetTotalLight(p1, style), base); //LRC
    VectorSubtract(*GetTotalLight(p2, style), base, d1); //LRC
    VectorSubtract(*GetTotalLight(p3, style), base, d2); //LRC
#else
    VectorCopy(p1->totallight, base);
    VectorSubtract(p2->totallight, base, d1);
    VectorSubtract(p3->totallight, base, d2);
#endif

    // Get edge normals
    VectorSubtract(p1->origin, p2->origin, v);
    VectorNormalize(v);
    CrossProduct(v, trian->plane->normal, ep1.normal);
    ep1.dist = DotProduct(p1->origin, ep1.normal);

    VectorSubtract(p1->origin, p3->origin, v);
    VectorNormalize(v);
    CrossProduct(v, trian->plane->normal, ep2.normal);
    ep2.dist = DotProduct(p1->origin, ep2.normal);

    x = DotProduct(point, ep1.normal) - ep1.dist;
    y = DotProduct(point, ep2.normal) - ep2.dist;

    y1 = DotProduct(p2->origin, ep2.normal) - ep2.dist;
    x2 = DotProduct(p3->origin, ep1.normal) - ep1.dist;

    VectorCopy(base, result);
    if (fabs(x2) >= ON_EPSILON)
    {
        int             i;

        for (i = 0; i < 3; i++)
        {
            result[i] += x * d2[i] / x2;
        }
    }
    if (fabs(y1) >= ON_EPSILON)
    {
        int             i;

        for (i = 0; i < 3; i++)
        {
            result[i] += y * d1[i] / y1;
        }
    }
}
#endif

// =====================================================================================
//  LerpNearest
// =====================================================================================
#ifdef HLRAD_LERP_VL
static bool LerpNearest(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
				vec3_t &result_direction, 
	#endif
				int pt1, int style)
{
    patch_t *patch;
	patch = trian->points[pt1];
	const vec3_t *l;
	#ifdef ZHLT_XASH
	const vec3_t *l_d;
	#endif
	l = GetTotalLight(patch, style
	#ifdef ZHLT_XASH
			, l_d
	#endif
			);
	VectorCopy (*l, result);
	#ifdef ZHLT_XASH
	VectorCopy (*l_d, result_direction);
	#endif
	return true;
}
#else
#ifdef ZHLT_TEXLIGHT
static void     LerpNearest(const lerpTriangulation_t* const trian, vec3_t result, int style) //LRC
#else
static void     LerpNearest(const lerpTriangulation_t* const trian, vec3_t result)
#endif
{
    unsigned        x;
    unsigned        numpoints = trian->numpoints;
    patch_t*        patch;

    // Find nearest in original face
    for (x = 0; x < numpoints; x++)
    {
        patch = trian->points[trian->dists[x].patch];

        if (patch->faceNumber == trian->facenum)
        {
	#ifdef ZHLT_TEXLIGHT
            VectorCopy(*GetTotalLight(patch, style), result); //LRC
	#else
            VectorCopy(patch->totallight, result);
	#endif
            return;
        }
    }

    // If none in nearest face, settle for nearest
    if (numpoints)
    {
	#ifdef ZHLT_TEXLIGHT 
        VectorCopy(*GetTotalLight(trian->points[trian->dists[0].patch], style), result); //LRC
	#else
        VectorCopy(trian->points[trian->dists[0].patch]->totallight, result);
	#endif
    }
    else
    {
        VectorClear(result);
    }
}
#endif

// =====================================================================================
//  LerpEdge
// =====================================================================================
#ifdef HLRAD_LERP_VL
static bool		LerpEdge(const lerpTriangulation_t* trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
					vec3_t &result_direction, 
	#endif
					int pt1, int pt2, int style)
{
    patch_t *p1;
    patch_t *p2;
	const vec_t *o1;
	const vec_t *o2;
    vec3_t increment;
	vec3_t normal;
	vec_t x;
    vec_t x1;
	vec3_t base;
	vec3_t d;
	#ifdef ZHLT_XASH
	vec3_t base_direction;
	vec3_t d_direction;
	#endif
	p1 = trian->points[pt1];
	p2 = trian->points[pt2];
#ifdef HLRAD_LERP_TEXNORMAL
	o1 = trian->points_pos[pt1];
	o2 = trian->points_pos[pt2];
#else
	o1 = p1->origin;
	o2 = p2->origin;
#endif
    if (TestLineSegmentIntersectWall(trian, p1->origin, p2->origin))
		return false;
	VectorSubtract (o2, o1, increment);
	CrossProduct (trian->plane->normal, increment, normal);
	CrossProduct (normal, trian->plane->normal, increment);
	VectorNormalize (increment);
	x = DotProduct (o2, increment) - DotProduct (o1, increment);
	x1 = DotProduct (point, increment) - DotProduct (o1, increment);
	if (x1 < -LERP_EPSILON || x1 > x + LERP_EPSILON)
	{
		return false;
	}
	const vec3_t *l;
	#ifdef ZHLT_XASH
	const vec3_t *l_d;
	#endif
	l = GetTotalLight(p1, style
	#ifdef ZHLT_XASH
			, l_d
	#endif
			);
	VectorCopy (*l, base);
	#ifdef ZHLT_XASH
	VectorCopy (*l_d, base_direction);
	#endif
	l = GetTotalLight(p2, style
	#ifdef ZHLT_XASH
			, l_d
	#endif
			);
	VectorSubtract (*l, base, d);
	VectorCopy (base, result);
	#ifdef ZHLT_XASH
	VectorSubtract (*l_d, base_direction, d_direction);
	VectorCopy (base_direction, result_direction);
	#endif
	if (x > LERP_EPSILON)
	{
		vec_t frac = x1 / x;
		if (frac < 0) frac = 0;
		if (frac > 1) frac = 1;
		VectorMA (result, frac, d, result);
	#ifdef ZHLT_XASH
		VectorMA (result_direction, frac, d_direction, result_direction);
	#endif
	}
	return true;
}
#else
#ifdef ZHLT_TEXLIGHT
static bool     LerpEdge(const lerpTriangulation_t* const trian, const vec3_t point, vec3_t result, int style) //LRC
#else
static bool     LerpEdge(const lerpTriangulation_t* const trian, const vec3_t point, vec3_t result)
#endif
{
    patch_t*        p1;
    patch_t*        p2;
#ifndef HLRAD_LERP_VL
    patch_t*        p3;
#endif
    vec3_t          v1;
    vec3_t          v2;
    vec_t           d;

#ifdef HLRAD_LERP_VL
	p1 = trian->points[pt1];
	p2 = trian->points[pt2];
#else
    p1 = trian->points[trian->dists[0].patch];
    p2 = trian->points[trian->dists[1].patch];
    p3 = trian->points[trian->dists[2].patch];
#endif

#ifndef HLRAD_LERP_FIX
    VectorSubtract(point, p1->origin, v2);
    VectorNormalize(v2);
#endif

    // Try nearest and 2
    if (!TestLineSegmentIntersectWall(trian, p1->origin, p2->origin))
    {
#ifdef HLRAD_LERP_FIX
		vec_t total_length, length1, length2;
		VectorSubtract (p2->origin, p1->origin, v1);
		CrossProduct (trian->plane->normal, v1, v2);
		CrossProduct (v2, trian->plane->normal, v1);
		length1 = DotProduct (v1, point) - DotProduct (v1, p1->origin);
		length2 = DotProduct (v1, p2->origin) - DotProduct (v1, point);
		total_length = DotProduct (v1, p2->origin) - DotProduct (v1, p1->origin);
		if (total_length > 0 && length1 >= 0 && length2 >= 0)
		{
            int             i;
#else
        VectorSubtract(p2->origin, p1->origin, v1);
        VectorNormalize(v1);
        d = DotProduct(v2, v1);
        if (d >= ON_EPSILON)
        {
            int             i;
            vec_t           length1;
            vec_t           length2;
            vec3_t          segment;
            vec_t           total_length;

            VectorSubtract(point, p1->origin, segment);
            length1 = VectorLength(segment);
            VectorSubtract(point, p2->origin, segment);
            length2 = VectorLength(segment);
            total_length = length1 + length2;
#endif

            for (i = 0; i < 3; i++)
            {
#ifdef ZHLT_TEXLIGHT
	#ifdef HLRAD_LERP_FIX
                result[i] = (((*GetTotalLight(p1, style))[i] * length2) + ((*GetTotalLight(p2, style))[i] * length1)) / total_length; //LRC
	#else
				result[i] = (((*GetTotalLight(p1, style))[i] * length2) + ((*GetTotalLight(p1, style))[i] * length1)) / total_length; //LRC
	#endif
#else
                result[i] = ((p1->totallight[i] * length2) + (p2->totallight[i] * length1)) / total_length;
#endif
            }
            return true;
        }
    }

#ifndef HLRAD_LERP_VL
    // Try nearest and 3
    if (!TestLineSegmentIntersectWall(trian, p1->origin, p3->origin))
    {
#ifdef HLRAD_LERP_FIX
		vec_t total_length, length1, length2;
		VectorSubtract (p3->origin, p1->origin, v1);
		CrossProduct (trian->plane->normal, v1, v2);
		CrossProduct (v2, trian->plane->normal, v1);
		length1 = DotProduct (v1, point) - DotProduct (v1, p1->origin);
		length2 = DotProduct (v1, p3->origin) - DotProduct (v1, point);
		total_length = DotProduct (v1, p3->origin) - DotProduct (v1, p1->origin);
		if (total_length > 0 && length1 >= 0 && length2 >= 0)
		{
            int             i;
#else
        VectorSubtract(p3->origin, p1->origin, v1);
        VectorNormalize(v1);
        d = DotProduct(v2, v1);
        if (d >= ON_EPSILON)
        {
            int             i;
            vec_t           length1;
            vec_t           length2;
            vec3_t          segment;
            vec_t           total_length;

            VectorSubtract(point, p1->origin, segment);
            length1 = VectorLength(segment);
            VectorSubtract(point, p3->origin, segment);
            length2 = VectorLength(segment);
            total_length = length1 + length2;
#endif

            for (i = 0; i < 3; i++)
            {
	#ifdef ZHLT_TEXLIGHT
                result[i] = (((*GetTotalLight(p1, style))[i] * length2) + ((*GetTotalLight(p3, style))[i] * length1)) / total_length; //LRC
	#else
                result[i] = ((p1->totallight[i] * length2) + (p3->totallight[i] * length1)) / total_length;
	#endif
            }
            return true;
        }
    }
#endif
    return false;
}
#endif


// =====================================================================================
//
//  SampleTriangulation
//
// =====================================================================================

// =====================================================================================
//  dist_sorter
// =====================================================================================
static int CDECL dist_sorter(const void* p1, const void* p2)
{
    lerpDist_t*     dist1 = (lerpDist_t*) p1;
    lerpDist_t*     dist2 = (lerpDist_t*) p2;

#ifdef HLRAD_LERP_VL
	if (dist1->invalid < dist2->invalid)
		return -1;
	if (dist2->invalid < dist1->invalid)
		return 1;
	if (dist1->dist + ON_EPSILON < dist2->dist)
		return -1;
	if (dist2->dist + ON_EPSILON < dist1->dist)
		return 1;
	if (dist1->pos[0] + ON_EPSILON < dist2->pos[0])
		return -1;
	if (dist2->pos[0] + ON_EPSILON < dist1->pos[0])
		return 1;
	if (dist1->pos[1] + ON_EPSILON < dist2->pos[1])
		return -1;
	if (dist2->pos[1] + ON_EPSILON < dist1->pos[1])
		return 1;
	if (dist1->pos[2] + ON_EPSILON < dist2->pos[2])
		return -1;
	if (dist2->pos[2] + ON_EPSILON < dist1->pos[2])
		return 1;
	return 0;
#else
    if (dist1->dist < dist2->dist)
    {
        return -1;
    }
    else if (dist1->dist > dist2->dist)
    {
        return 1;
    }
    else
    {
        return 0;
    }
#endif
}

// =====================================================================================
//  FindDists
// =====================================================================================
static void     FindDists(const lerpTriangulation_t* const trian, const vec3_t point)
{
    unsigned        x;
    unsigned        numpoints = trian->numpoints;
    patch_t**       patch = trian->points;
    lerpDist_t*     dists = trian->dists;
    vec3_t          delta;
#ifdef HLRAD_LERP_VL
	vec3_t testpoint;
	{
		VectorCopy (point, testpoint);
		Winding *w = new Winding (*trian->face);
		int i;
		for (i = 0; i < w->m_NumPoints; i++)
		{
			VectorAdd (w->m_Points[i], g_face_offset[trian->facenum], w->m_Points[i]);
		}
		if (!point_in_winding_noedge (*w, *trian->plane, testpoint, DEFAULT_EDGE_WIDTH))
		{
			snap_to_winding_noedge (*w, *trian->plane, testpoint, DEFAULT_EDGE_WIDTH);
		}
		delete w;
		if (!TestLineSegmentIntersectWall (trian, point, testpoint))
		{
			VectorCopy (point, testpoint);
		}
	}
#endif

    for (x = 0; x < numpoints; x++, patch++, dists++)
    {
#ifdef HLRAD_LERP_TEXNORMAL
		VectorSubtract (trian->points_pos[x], point, delta);
#else
        VectorSubtract((*patch)->origin, point, delta);
#endif
#ifdef HLRAD_LERP_VL
		vec3_t normal;
		CrossProduct (trian->plane->normal, delta, normal);
		CrossProduct (normal, trian->plane->normal, delta);
#endif
        dists->dist = VectorLength(delta);
        dists->patch = x;
#ifdef HLRAD_LERP_VL
		dists->invalid = TestLineSegmentIntersectWall (trian, testpoint, (*patch)->origin);
		VectorCopy ((*patch)->origin, dists->pos);
#endif
    }

    qsort((void*)trian->dists, (size_t) numpoints, sizeof(lerpDist_t), dist_sorter);
}

// =====================================================================================
//  SampleTriangulation
// =====================================================================================
#ifdef ZHLT_TEXLIGHT
#ifdef HLRAD_LERP_VL
void            SampleTriangulation(const lerpTriangulation_t* const trian, const vec3_t point, vec3_t result, 
	#ifdef ZHLT_XASH
					vec3_t &result_direction, 
	#endif
					int style)
#else
void            SampleTriangulation(const lerpTriangulation_t* const trian, vec3_t point, vec3_t result, int style) //LRC
#endif
#else
void            SampleTriangulation(const lerpTriangulation_t* const trian, vec3_t point, vec3_t result)
#endif
{
    FindDists(trian, point);

#ifdef HLRAD_LERP_VL
	VectorClear(result);
	#ifdef ZHLT_XASH
	VectorClear (result_direction);
	#endif
	if (trian->numpoints >= 3 && trian->dists[2].invalid <= 0 && g_lerp_enabled)
	{
		int pt1 = trian->dists[0].patch;
		int pt2 = trian->dists[1].patch;
		int pt3 = trian->dists[2].patch;
		if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt1, pt2, pt3, style))
		{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
			if (g_drawlerp)
				result[0] = 100, result[1] = 100, result[2] = 100;
	#endif
			return;
		}
#ifdef HLRAD_LERP_TRY5POINTS
		if (trian->numpoints >= 4 && trian->dists[3].invalid <= 0)
		{
			int pt4 = trian->dists[3].patch;
			if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
					result_direction, 
	#endif
					pt1, pt2, pt4, style))
			{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
				if (g_drawlerp)
					result[0] = 100, result[1] = 100, result[2] = 0;
				return;
	#endif
			}
			if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
					result_direction, 
	#endif
					pt1, pt3, pt4, style))
			{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
				if (g_drawlerp)
					result[0] = 100, result[1] = 0, result[2] = 100;
	#endif
				return;
			}
			if (trian->numpoints >= 5 && trian->dists[4].invalid <= 0)
			{
				int pt5 = trian->dists[4].patch;
				if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
						result_direction, 
	#endif
						pt1, pt2, pt5, style))
				{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
					if (g_drawlerp)
						result[0] = 100, result[1] = 0, result[2] = 0;
	#endif
					return;
				}
				if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
						result_direction, 
	#endif
						pt1, pt3, pt5, style))
				{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
					if (g_drawlerp)
						result[0] = 100, result[1] = 0, result[2] = 0;
	#endif
					return;
				}
				if (LerpTriangle (trian, point, result, 
	#ifdef ZHLT_XASH
						result_direction, 
	#endif
						pt1, pt4, pt5, style))
				{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
					if (g_drawlerp)
						result[0] = 100, result[1] = 0, result[2] = 0;
	#endif
					return;
				}
			}
		}
#endif
	}
	if (trian->numpoints >= 2 && trian->dists[1].invalid <= 0  && g_lerp_enabled)
	{
		int pt1 = trian->dists[0].patch;
		int pt2 = trian->dists[1].patch;
		if (LerpEdge (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt1, pt2, style))
		{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
			if (g_drawlerp)
				result[0] = 0, result[1] = 100, result[2] = 100;
	#endif
			return;
		}
		if (trian->numpoints >= 3 && trian->dists[2].invalid <= 0 )
		{
			int pt3 = trian->dists[2].patch;
			if (LerpEdge (trian, point, result, 
	#ifdef ZHLT_XASH
					result_direction, 
	#endif
					pt1, pt3, style))
			{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
				if (g_drawlerp)
					result[0] = 0, result[1] = 100, result[2] = 0;
	#endif
				return;
			}
		}
	}
	if (trian->numpoints >= 1)
	{
		int pt1 = trian->dists[0].patch;
		if (LerpNearest (trian, point, result, 
	#ifdef ZHLT_XASH
				result_direction, 
	#endif
				pt1, style))
		{
	#ifdef HLRAD_DEBUG_DRAWPOINTS
			if (g_drawlerp)
				result[0] = 0, result[1] = 0, result[2] = 100;
	#endif
			return;
		}
	}
#else
    if ((trian->numpoints > 3) && (g_lerp_enabled))
    {
        unsigned        pt1;
        unsigned        pt2;
        unsigned        pt3;
        vec_t*          p1;
        vec_t*          p2;
        vec_t*          p3;
        dplane_t        plane;

        pt1 = trian->dists[0].patch;
        pt2 = trian->dists[1].patch;
        pt3 = trian->dists[2].patch;

        p1 = trian->points[pt1]->origin;
        p2 = trian->points[pt2]->origin;
        p3 = trian->points[pt3]->origin;

        plane_from_points(p1, p2, p3, &plane);
        SnapToPlane(&plane, point, 0.0);
        if (point_in_tri(point, &plane, p1, p2, p3))
        {                                                  // TODO check edges/tri for blocking by wall
            if (!TestWallIntersectTri(trian, p1, p2, p3) && !TestTriIntersectWall(trian, p1, p2, p3))
            {
#ifdef ZHLT_TEXLIGHT
                LerpTriangle(trian, point, result, pt1, pt2, pt3, style); //LRC
#else
                LerpTriangle(trian, point, result, pt1, pt2, pt3);
#endif
                return;
            }
        }
        else
        {
#ifdef ZHLT_TEXLIGHT
            if (LerpEdge(trian, point, result, style)) //LRC
#else
            if (LerpEdge(trian, point, result))
#endif
            {
                return;
            }
        }
    }

#ifdef ZHLT_TEXLIGHT
    LerpNearest(trian, result, style); //LRC
#else
    LerpNearest(trian, result);
#endif
#endif
}

// =====================================================================================
//  AddPatchToTriangulation
// =====================================================================================
static void     AddPatchToTriangulation(lerpTriangulation_t* trian, patch_t* patch)
{
#ifdef HLRAD_PATCHBLACK_FIX
	if (patch->flags != ePatchFlagOutside)
#else
    if (!(patch->flags & ePatchFlagOutside))
#endif
    {
        int             pnum = trian->numpoints;

        if (pnum >= trian->maxpoints)
        {
            trian->points = (patch_t**)realloc(trian->points, sizeof(patch_t*) * (trian->maxpoints + DEFAULT_MAX_LERP_POINTS));

            hlassume(trian->points != NULL, assume_NoMemory);
            memset(trian->points + trian->maxpoints, 0, sizeof(patch_t*) * DEFAULT_MAX_LERP_POINTS);   // clear the new block
#ifdef HLRAD_LERP_TEXNORMAL
			trian->points_pos = (vec3_t *)realloc(trian->points_pos, sizeof (vec3_t) * (trian->maxpoints + DEFAULT_MAX_LERP_POINTS));
			hlassume (trian->points_pos != NULL, assume_NoMemory);
			memset (trian->points_pos + trian->maxpoints, 0, sizeof (vec3_t) * DEFAULT_MAX_LERP_POINTS);
#endif

            trian->maxpoints += DEFAULT_MAX_LERP_POINTS;
        }

        trian->points[pnum] = patch;
#ifdef HLRAD_LERP_TEXNORMAL
		VectorCopy (patch->origin, trian->points_pos[pnum]);
		if (patch->faceNumber != trian->facenum)
		{
			vec3_t snapdir;
			dplane_t p1 = *trian->plane;
			p1.dist += DotProduct (p1.normal, g_face_offset[trian->facenum]);
			dplane_t p2 = *getPlaneFromFaceNumber (patch->faceNumber);
			p2.dist += DotProduct (p2.normal, g_face_offset[patch->faceNumber]);
			VectorCopy (p1.normal, snapdir);
			if (!GetIntertexnormal (patch->faceNumber, trian->facenum, snapdir))
			{
				Warning ("AddPatchToTriangulation: internal error 1.");
			}
			VectorMA (trian->points_pos[pnum], -PATCH_HUNT_OFFSET, p2.normal, trian->points_pos[pnum]);
			vec_t dist = (DotProduct (trian->points_pos[pnum], p1.normal) - p1.dist) / DotProduct (snapdir, p1.normal);
			VectorMA (trian->points_pos[pnum], -dist, snapdir, trian->points_pos[pnum]);
		}
#endif
        trian->numpoints++;
    }
}

// =====================================================================================
//  CreateWalls
// =====================================================================================
#ifdef HLRAD_LERP_VL
static void		AddWall (lerpTriangulation_t *trian, const vec_t *v1, const vec_t *v2) // v1 & v2 should be without offset
{
	int facenum = trian->facenum;
    const dplane_t* p = trian->plane;
	vec3_t delta;
	vec3_t p0;
	vec3_t p1;
	vec3_t normal;
	lerpWall_t *wall;

	if (trian->numwalls >= trian->maxwalls)
	{
		trian->walls =
			(lerpWall_t*)realloc(trian->walls, sizeof(lerpWall_t) * (trian->maxwalls + DEFAULT_MAX_LERP_WALLS));
		hlassume(trian->walls != NULL, assume_NoMemory);
		memset(trian->walls + trian->maxwalls, 0, sizeof(lerpWall_t) * DEFAULT_MAX_LERP_WALLS);     // clear the new block
		trian->maxwalls += DEFAULT_MAX_LERP_WALLS;
	}

	wall = &trian->walls[trian->numwalls];
	trian->numwalls++;
	VectorAdd (v1, g_face_offset[facenum], p0);
	VectorAdd (v2, g_face_offset[facenum], p1);
	VectorSubtract (p1, p0, delta);
	CrossProduct (p->normal, delta, normal);
	if (VectorNormalize (normal) == 0.0)
	{
		trian->numwalls--;
		return;
	}
	VectorCopy (normal, wall->plane.normal);
	wall->plane.dist = DotProduct (normal, p0);
	CrossProduct (normal, p->normal, wall->increment);
	VectorCopy (p0, wall->vertex0);
	VectorCopy (p1, wall->vertex1);
}
#endif
#ifndef HLRAD_LERP_FACELIST
static void     CreateWalls(lerpTriangulation_t* trian, const dface_t* const face)
{
#ifdef HLRAD_LERP_VL
    const dplane_t* p = getPlaneFromFace(face);
    int             facenum = face - g_dfaces;
    int             x;

    for (x = 0; x < face->numedges; x++)
    {
        edgeshare_t*    es;
        dface_t*        f2;
        int             edgenum = g_dsurfedges[face->firstedge + x];

        if (edgenum > 0)
        {
            es = &g_edgeshare[edgenum];
            f2 = es->faces[1];
        }
        else
        {
            es = &g_edgeshare[-edgenum];
            f2 = es->faces[0];
        }
		if (!es->smooth)
		{
			AddWall (trian, g_dvertexes[g_dedges[abs(edgenum)].v[0]].point, g_dvertexes[g_dedges[abs(edgenum)].v[1]].point);
		}
		else
		{
			int facenum2 = f2 - g_dfaces;
			int x2;
			for (x2 = 0; x2 < f2->numedges; x2++)
			{
				edgeshare_t *es2;
				dface_t *f3;
				int edgenum2 = g_dsurfedges[f2->firstedge + x2];
				if (edgenum2 > 0)
				{
					es2 = &g_edgeshare[edgenum2];
					f3 = es2->faces[1];
				}
				else
				{
					es2 = &g_edgeshare[-edgenum2];
					f3 = es2->faces[0];
				}
				if (!es2->smooth)
				{
					AddWall (trian, g_dvertexes[g_dedges[abs(edgenum2)].v[0]].point, g_dvertexes[g_dedges[abs(edgenum2)].v[1]].point);
				}
			}
		}
	}
#else
    const dplane_t* p = getPlaneFromFace(face);
    int             facenum = face - g_dfaces;
    int             x;

    for (x = 0; x < face->numedges; x++)
    {
        edgeshare_t*    es;
        dface_t*        f2;
        int             edgenum = g_dsurfedges[face->firstedge + x];

        if (edgenum > 0)
        {
            es = &g_edgeshare[edgenum];
            f2 = es->faces[1];
        }
        else
        {
            es = &g_edgeshare[-edgenum];
            f2 = es->faces[0];
        }

        // Build Wall for non-coplanar neighbhors
        if (f2 && !es->coplanar && VectorCompare(vec3_origin, es->interface_normal))
        {
            const dplane_t* plane = getPlaneFromFace(f2);

            // if plane isn't facing us, ignore it
	#ifdef HLRAD_DPLANEOFFSET_MISCFIX
            if (DotProduct(plane->normal, g_face_centroids[facenum]) < plane->dist + DotProduct(plane->normal, g_face_offset[facenum]))
	#else
            if (DotProduct(plane->normal, g_face_centroids[facenum]) < plane->dist)
	#endif
            {
                continue;
            }

            {
                vec3_t          delta;
                vec3_t          p0;
                vec3_t          p1;
                lerpWall_t*     wall;

                if (trian->numwalls >= trian->maxwalls)
                {
                    trian->walls =
                        (lerpWall_t*)realloc(trian->walls, sizeof(lerpWall_t) * (trian->maxwalls + DEFAULT_MAX_LERP_WALLS));
                    hlassume(trian->walls != NULL, assume_NoMemory);
                    memset(trian->walls + trian->maxwalls, 0, sizeof(lerpWall_t) * DEFAULT_MAX_LERP_WALLS);     // clear the new block
                    trian->maxwalls += DEFAULT_MAX_LERP_WALLS;
                }

                wall = &trian->walls[trian->numwalls];
                trian->numwalls++;

                VectorScale(p->normal, 10000.0, delta);

                VectorCopy(g_dvertexes[g_dedges[abs(edgenum)].v[0]].point, p0);
                VectorCopy(g_dvertexes[g_dedges[abs(edgenum)].v[1]].point, p1);

                // Adjust for origin-based models
                // technically we should use the other faces g_face_offset entries
                // If they are nonzero, it has to be from the same model with
                // the same offset, so we are cool
                VectorAdd(p0, g_face_offset[facenum], p0);
                VectorAdd(p1, g_face_offset[facenum], p1);

                VectorAdd(p0, delta, wall->vertex[0]);
                VectorAdd(p1, delta, wall->vertex[1]);
                VectorSubtract(p1, delta, wall->vertex[2]);
                VectorSubtract(p0, delta, wall->vertex[3]);

                {
                    vec3_t          delta1;
                    vec3_t          delta2;
                    vec3_t          normal;
                    vec_t           dist;

                    VectorSubtract(wall->vertex[2], wall->vertex[1], delta1);
                    VectorSubtract(wall->vertex[0], wall->vertex[1], delta2);
                    CrossProduct(delta1, delta2, normal);
                    VectorNormalize(normal);
                    dist = DotProduct(normal, p0);

                    VectorCopy(normal, wall->plane.normal);
                    wall->plane.dist = dist;
                }
            }
        }
    }
#endif
}
#endif

// =====================================================================================
//  AllocTriangulation
// =====================================================================================
static lerpTriangulation_t* AllocTriangulation()
{
    lerpTriangulation_t* trian = (lerpTriangulation_t*)calloc(1, sizeof(lerpTriangulation_t));
#ifdef HLRAD_HLASSUMENOMEMORY
	hlassume (trian != NULL, assume_NoMemory);
#endif

    trian->maxpoints = DEFAULT_MAX_LERP_POINTS;
    trian->maxwalls = DEFAULT_MAX_LERP_WALLS;

    trian->points = (patch_t**)calloc(DEFAULT_MAX_LERP_POINTS, sizeof(patch_t*));
#ifdef HLRAD_LERP_TEXNORMAL
	trian->points_pos = (vec3_t *)calloc (DEFAULT_MAX_LERP_POINTS, sizeof(vec3_t));
	hlassume (trian->points_pos != NULL, assume_NoMemory);
#endif

    trian->walls = (lerpWall_t*)calloc(DEFAULT_MAX_LERP_WALLS, sizeof(lerpWall_t));

    hlassume(trian->points != NULL, assume_NoMemory);
    hlassume(trian->walls != NULL, assume_NoMemory);

    return trian;
}

// =====================================================================================
//  FreeTriangulation
// =====================================================================================
void            FreeTriangulation(lerpTriangulation_t* trian)
{
    free(trian->dists);
    free(trian->points);
#ifdef HLRAD_LERP_TEXNORMAL
	free(trian->points_pos);
#endif
    free(trian->walls);
#ifdef HLRAD_LERP_FACELIST
	for (facelist_t *next; trian->allfaces; trian->allfaces = next)
	{
		next = trian->allfaces->next;
		free (trian->allfaces);
	}
#endif
    free(trian);
}

#ifdef HLRAD_LERP_FACELIST
void AddFaceToTrian (facelist_t **faces, dface_t *face)
{
	for (; *faces; faces = &(*faces)->next)
	{
		if ((*faces)->face == face)
		{
			return;
		}
	}
	*faces = (facelist_t *)malloc (sizeof (facelist_t));
	hlassume (*faces != NULL, assume_NoMemory);
	(*faces)->face = face;
	(*faces)->next = NULL;
}
void FindFaces (lerpTriangulation_t *trian)
{
	int i, j;
	AddFaceToTrian (&trian->allfaces, &g_dfaces[trian->facenum]);
	for (j = 0; j < trian->face->numedges; j++)
	{
		int edgenum = g_dsurfedges[trian->face->firstedge + j];
		edgeshare_t *es = &g_edgeshare[abs (edgenum)];
		dface_t *f2;
		if (!es->smooth)
		{
			continue;
		}
		if (edgenum > 0)
		{
			f2 = es->faces[1];
		}
		else
		{
			f2 = es->faces[0];
		}
		AddFaceToTrian (&trian->allfaces, f2);
	}
	for (j = 0; j < trian->face->numedges; j++)
	{
		int edgenum = g_dsurfedges[trian->face->firstedge + j];
		edgeshare_t *es = &g_edgeshare[abs (edgenum)];
		if (!es->smooth)
		{
			continue;
		}
		for (i = 0; i < 2; i++)
		{
			facelist_t *fl;
			for (fl = es->vertex_facelist[i]; fl; fl = fl->next)
			{
				dface_t *f2 = fl->face;
				AddFaceToTrian (&trian->allfaces, f2);
			}
		}
	}
}
#endif
// =====================================================================================
//  CreateTriangulation
// =====================================================================================
lerpTriangulation_t* CreateTriangulation(const unsigned int facenum)
{
    const dface_t*  f = g_dfaces + facenum;
    const dplane_t* p = getPlaneFromFace(f);
    lerpTriangulation_t* trian = AllocTriangulation();
    patch_t*        patch;
    unsigned int    j;
    dface_t*        f2;

    trian->facenum = facenum;
    trian->plane = p;
    trian->face = f;

#ifdef HLRAD_LERP_FACELIST
	FindFaces (trian);
	facelist_t *fl;
	for (fl = trian->allfaces; fl; fl = fl->next)
	{
		f2 = fl->face;
		int facenum2 = fl->face - g_dfaces;
		for (patch = g_face_patches[facenum2]; patch; patch = patch->next)
		{
			AddPatchToTriangulation (trian, patch);
		}
		for (j = 0; j < f2->numedges; j++)
		{
			int edgenum = g_dsurfedges[f2->firstedge + j];
			edgeshare_t *es = &g_edgeshare[abs(edgenum)];
			if (!es->smooth)
			{
				AddWall (trian, g_dvertexes[g_dedges[abs(edgenum)].v[0]].point, g_dvertexes[g_dedges[abs(edgenum)].v[1]].point);
			}
		}
	}
#else
    for (patch = g_face_patches[facenum]; patch; patch = patch->next)
    {
        AddPatchToTriangulation(trian, patch);
    }

    CreateWalls(trian, f);

    for (j = 0; j < f->numedges; j++)
    {
        edgeshare_t*    es;
        int             edgenum = g_dsurfedges[f->firstedge + j];

        if (edgenum > 0)
        {
            es = &g_edgeshare[edgenum];
            f2 = es->faces[1];
        }
        else
        {
            es = &g_edgeshare[-edgenum];
            f2 = es->faces[0];
        }

        if (!es->coplanar && VectorCompare(vec3_origin, es->interface_normal))
        {
            continue;
        }

        for (patch = g_face_patches[f2 - g_dfaces]; patch; patch = patch->next)
        {
            AddPatchToTriangulation(trian, patch);
        }
    }
#endif

    trian->dists = (lerpDist_t*)calloc(trian->numpoints, sizeof(lerpDist_t));
#ifdef HLRAD_HULLU
    //Get rid off error that seems to happen with some opaque faces (when opaque face have all edges 'out' of map)
    if(trian->numpoints != 0) // this line should be removed. --vluzacn
#endif
    hlassume(trian->dists != NULL, assume_NoMemory);

    return trian;
}
