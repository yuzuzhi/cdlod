#pragma once

#ifndef PI
#define PI 3.141592653
#endif

#define FLT_MAX         3.402823466e+38F        /* max value */
#define FLT_MIN         1.175494351e-38F        /* min positive value */

template<class T>
inline T clamp(T const & v, T const & b, T const & c)
{
	if( v < b ) return b;
	if( v > c ) return c;
	return v;
}

// short for clamp( a, 0, 1 )
template<class T>
inline T saturate( T const & a )
{
	return ::clamp( a, (T)0.0, (T)1.0 );
}

template<class T>
inline T lerp(T const & a, T const & b, T const & f)
{
	return a + (b-a)*f;
}

template<class T>
inline void swap(T & a, T & b)
{
	T temp = b;
	b = a;
	a = temp;
}

template<class T>
inline T sqr(T & a)
{
	return a * a;
}

inline float randf( )       { return rand() / (float)RAND_MAX; }


struct Frustum
{
	XMFLOAT3 Origin;            // Origin of the frustum (and projection).
	XMFLOAT4 Orientation;       // Unit quaternion representing rotation.

	FLOAT RightSlope;           // Positive X slope (X/Z).
	FLOAT LeftSlope;            // Negative X slope.
	FLOAT TopSlope;             // Positive Y slope (Y/Z).
	FLOAT BottomSlope;          // Negative Y slope.
	FLOAT Near, Far;            // Z of the near plane and far plane.
};
