#ifndef __INCLUDE_GUARD_19715045_A7D7_4BC2_B7A5_674B2D36055C
#define __INCLUDE_GUARD_19715045_A7D7_4BC2_B7A5_674B2D36055C
#ifdef _MSC_VER
	#pragma once
#endif


#include "ray.h"

//An axis aligned bounding box
struct BBox
{
	Point min, max;

	//Returns the entry and exit distances of the ray with the
	//	bounding box.
	//If the first returned distance > the second, than
	//	the ray does not intersect the bounding box at all
	std::pair<float, float> intersect(const Ray &_ray) const
	{
		float4 v1 = min, v2 = max;

		const float EPS = 0.0000001f;

		float4 div = _ray.d;
		int4 cmp = (div > float4::rep(-EPS)) & (div < float4::rep(EPS));
		for(int i = 0; i < 4; i++)
			if(cmp[i] != 0)
			div[i] = EPS;
		//div = (cmp & float4::rep(EPS)) | (~cmp & div);

		float4 t1 = (v1 - _ray.o) / div;
		float4 t2 = (v2 - _ray.o) / div;

		float4 tMin = float4::min(t1, t2);
		float4 tMax = float4::max(t1, t2);

		std::pair<float, float> ret;

		ret.first = std::max(std::max(tMin.x, tMin.y), tMin.z);
		ret.second = std::min(std::min(tMax.x, tMax.y), tMax.z);

		return ret;
	}

	//Extend the bounding box with a point
	void extend(const Point &_point)
	{
		float4 newMin = float4::min(float4(min), _point);
		float4 newMax = float4::max(float4(max), _point);
		min = *(Point*)&newMin;
		max = *(Point*)&newMax;
	}

	//Extend the bounding box with another bounding box
	void extend(const BBox &_bbox)
	{
		float4 newMin = float4::min(float4(min), _bbox.min);
		float4 newMax = float4::max(float4(max), _bbox.max);
		min = *(Point*)&newMin;
		max = *(Point*)&newMax;
	}

	//Returns an "empty" bounding box. Extending such a bounding
	//	box with a point will always create a bbox around the point
	//	and with a bbox - will simply copy the bbox.
	static BBox empty() 
	{
		BBox ret;
		ret.min = Point(FLT_MAX, FLT_MAX, FLT_MAX);
		ret.max = Point(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		return ret;
	}

	const Vector diagonal() const 
	{
		return max - min;
	}
};


#endif //__INCLUDE_GUARD_19715045_A7D7_4BC2_B7A5_674B2D36055C
