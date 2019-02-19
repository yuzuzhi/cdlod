#include <d3dx9math.h>
#include <xnamath.h>
#pragma once

struct IntersectRes
{
	enum Type {
		eOutside,
		eIntersect,
		eInside,
	};
};
struct AABB
{
	D3DXVECTOR3       Min;
	D3DXVECTOR3       Max;
	D3DXVECTOR3       Center()   { return (Min + Max) * 0.5f; }
	D3DXVECTOR3       Size()     { return Max - Min; }

	void Expand( float percentage )
	{
		D3DXVECTOR3 offset = Size() * percentage;
		Min -= offset;
		Max += offset;
	}

	void GetCornerPoints( D3DXVECTOR3 corners [] )
	{
		corners[0].x = Min.x;
		corners[0].y = Min.y;
		corners[0].z = Min.z;

		corners[1].x = Min.x;
		corners[1].y = Max.y;
		corners[1].z = Min.z;

		corners[2].x = Max.x;
		corners[2].y = Min.y;
		corners[2].z = Min.z;

		corners[3].x = Max.x;
		corners[3].y = Max.y;
		corners[3].z = Min.z;

		corners[4].x = Min.x;
		corners[4].y = Min.y;
		corners[4].z = Max.z;

		corners[5].x = Min.x;
		corners[5].y = Max.y;
		corners[5].z = Max.z;

		corners[6].x = Max.x;
		corners[6].y = Min.y;
		corners[6].z = Max.z;

		corners[7].x = Max.x;
		corners[7].y = Max.y;
		corners[7].z = Max.z;
	}

	IntersectRes::Type TestInBoundingPlanes( const D3DXPLANE planes[] )
	{
		D3DXVECTOR3 corners[9];
		GetCornerPoints(corners);
		corners[8] = Center();

		D3DXVECTOR3 boxSize = Size();
		float size = D3DXVec3Length( &boxSize );
		const float halfsize = size / 2;
		
		//中心点为圆心半径为halfsize的圆
		for(int p = 0; p < 6; p++) 
		{
			float centDist = D3DXPlaneDotCoord( &planes[p], &corners[8] );
			if( centDist < -halfsize )
				return IntersectRes::eOutside;
		}

		int totalIn = 0;
		size /= 6.0f; //reduce size to 1/4 (half of radius) for more precision!! // tweaked to 1/6, more sensible

		// test all 8 corners and 9th center point against the planes
		// if all points are behind 1 specific plane, we are out
		// if we are in with all points, then we are fully in
		for(int p = 0; p < 6; p++) 
		{
			int inCount = 9;
			int ptIn = 1;

			for(int i = 0; i < 9; ++i) 
			{

				// test this point against the planes
				float distance = D3DXPlaneDotCoord( &planes[p], &corners[i] );
				if (distance < -size) 
				{
					ptIn = 0;
					inCount--;
				}
			}

			// were all the points outside of plane p?
			if (inCount == 0) 
			{
				//assert( completelyIn == false );
				return IntersectRes::eOutside;
			}

			// check if they were all on the right side of the plane
			totalIn += ptIn;
		}

		if( totalIn == 6 )
			return IntersectRes::eInside;

		return IntersectRes::eIntersect;
	}


	bool	IntersectSphereSq( const D3DXVECTOR3 & center, float radiusSq )
	{
		return MinDistanceFromPointSq( center ) <= radiusSq;
	}

	float MinDistanceFromPointSq( const D3DXVECTOR3 & point )
	{
		float dist = 0.0f;

		if (point.x < Min.x)
		{
			float d=point.x-Min.x;
			dist+=d*d;
		}
		else if (point.x > Max.x)
		{
			float d=point.x-Max.x;
			dist+=d*d;
		}

		if (point.y < Min.y)
		{
			float d=point.y-Min.y;
			dist+=d*d;
		}
		else if (point.y > Max.y)
		{
			float d=point.y-Max.y;
			dist+=d*d;
		}

		if (point.z < Min.z)
		{
			float d=point.z-Min.z;
			dist+=d*d;
		}
		else if (point.z > Max.z)
		{
			float d=point.z-Max.z;
			dist+=d*d;
		}

		return dist;
	}
};

