///////////////////////////////////////////////////////////////////////////////////////////////////
//  
//      __   __                              __    
//     |  |_|  |--.-----. .----.--.--.-----.|  |--.
//   __|   _|     |  -__| |   _|  |  |__ --||     |
//  |__|____|__|__|_____| |__| |_____|_____||__|__|
//                                                 
//  Copyright (C) 2007-2013 Cedric Guillemet
//
// This file is part of .the rush//.
//
//    .the rush// is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    .the rush// is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with .the rush//.  If not, see <http://www.gnu.org/licenses/>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>
///////////////////////////////////////////////////////////////////////////////////////////////////

struct matrix_t;

inline void FPU_MatrixF_x_MatrixF(const float *a, const float *b, float *r)
{
	r[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8]  + a[3]*b[12];
	r[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9]  + a[3]*b[13];
	r[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	r[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];

	r[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8]  + a[7]*b[12];
	r[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9]  + a[7]*b[13];
	r[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	r[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];

	r[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	r[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	r[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10]+ a[11]*b[14];
	r[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11]+ a[11]*b[15];

	r[12]= a[12]*b[0]+ a[13]*b[4]+ a[14]*b[8] + a[15]*b[12];
	r[13]= a[12]*b[1]+ a[13]*b[5]+ a[14]*b[9] + a[15]*b[13];
	r[14]= a[12]*b[2]+ a[13]*b[6]+ a[14]*b[10]+ a[15]*b[14];
	r[15]= a[12]*b[3]+ a[13]*b[7]+ a[14]*b[11]+ a[15]*b[15];
}

const float ZPI    =  3.14159265358979323846f;

const float PI_MUL_2 =  6.28318530717958647692f;
const float PI_DIV_2 =  1.57079632679489655800f;
const float PI_DIV_4 =  0.78539816339744827900f;
const float INV_PI   =  0.31830988618379069122f;
const float DEGTORAD =  0.01745329251994329547f;
const float RADTODEG = 57.29577951308232286465f;
const float SQRT2    =  1.41421356237309504880f;
const float SQRT3    =  1.73205080756887729352f;
#define    FLOAT_EPSILON    float(1.192092896e-07)

#define LERP(x,y,z) (x+(y-x)*z)
#define zmax(x,y) ((x>y)?x:y)
#define zmin(x,y) ((x<y)?x:y)
#define Clamp(val,minimum,maximum) ( (val<minimum)?minimum:((val>maximum)?maximum:val) )
#define DegreeToRadian(fDegrees) ((3.14159265f/180.0f)*fDegrees)
#define FREQ30Hz (1.f/30.f)
#define FREQ60Hz (1.f/60.f)
#define FREQ100Hz (1.f/100.f)
#define FREQ120Hz (1.f/120.f)
#define FREQ240Hz (1.f/240.f)
#define FREQ480Hz (1.f/480.f)
#define UNUSED_PARAMETER(x)         ((void)(x))

#ifdef WIN32
typedef __int32 int32;
typedef unsigned __int32 uint32;

#else
    #ifdef LINUX
    #include <stdint.h>
    typedef int32_t int32;
    typedef uint32_t uint32;
    #else
    #ifdef NACL
    #include <stdint.h>
    typedef int32_t int32;
    typedef uint32_t uint32;
#else
        #ifdef MACOSX
        #include <sys/types.h>
        typedef int32_t int32;
        typedef u_int32_t uint32;
        #endif
#endif
    #endif
#endif

typedef unsigned char u8;
typedef unsigned char uint8;
typedef unsigned short u16;
typedef unsigned short uint16;
typedef uint32_t u32;
typedef unsigned int uint;

///////////////////////////////////////////////////////////////////////////////////////////////////

struct vec_t
{
public:
	vec_t(const vec_t& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
	vec_t() {}
	vec_t(float _x, float _y, float _z = 0.f, float _w = 0.f) : x(_x), y(_y), z(_z), w(_w)
	{
	}
	vec_t(int _x, int _y, int _z = 0, int _w = 0) : x((float)_x), y((float)_y), z((float)_z), w((float)_w)
	{
	}
	
	vec_t ( u32 col ) { fromUInt32(col); }
	vec_t (float v ) : x(v), y(v), z(v), w(v) {}

	float x,y,z,w;

	void Lerp(const vec_t& v, float t)
	{
		x += (v.x-x) * t;
		y += (v.y-y) * t;
		z += (v.z-z) * t;
		w += (v.w-w) * t;
	}
	void LerpColor(const vec_t& v, float t)
	{
		for (int i = 0; i < 4;i++)
			(*this)[i] = sqrtf(((*this)[i] * (*this)[i]) * (1.f - t) + (v[i] * v[i]) * (t));
	}
    void Lerp(const vec_t& v, const vec_t& v2,float t)
	{
        *this = v;
        Lerp(v2, t);
	}
    
	inline void set(float v) { x = y = z = w = v; }
	inline void set(float _x, float _y, float _z = 0.f, float _w = 0.f)	{ x = _x; y = _y; z = _z; w = _w; }

	inline vec_t& operator -= ( const vec_t& v ) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
	inline vec_t& operator += ( const vec_t& v ) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
	inline vec_t& operator *= ( const vec_t& v ) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
	inline vec_t& operator *= ( float v ) { x *= v;	y *= v;	z *= v;	w *= v;	return *this; }

	inline vec_t operator * ( float f ) const;
	inline vec_t operator - () const;
	inline vec_t operator - ( const vec_t& v ) const;
	inline vec_t operator + ( const vec_t& v ) const;
	inline vec_t operator * ( const vec_t& v ) const;
	
	inline const vec_t& operator + () const { return (*this); }
	inline float length() const { return sqrtf(x*x +y*y +z*z ); };
	inline float lengthSq() const { return (x*x +y*y +z*z ); };
	inline vec_t normalize() { (*this) *= (1.f/length()+FLT_EPSILON); return (*this); }
	inline vec_t normalize(const vec_t& v) { this->set(v.x, v.y, v.z, v.w); this->normalize(); return (*this); }
	inline int LongestAxis() const 
	{
		int res = 0; 
		res = (fabsf((*this)[1]) > fabsf((*this)[res])) ? 1 : res;
		res = (fabsf((*this)[2]) > fabsf((*this)[res])) ? 2 : res;
		return res;
	}
	inline void cross(const vec_t& v)
	{
		vec_t res;
		res.x = y * v.z - z * v.y;
		res.y = z * v.x - x * v.z;
		res.z = x * v.y - y * v.x;

		x = res.x;
		y = res.y;
		z = res.z;
		w = 0.f;
	}
	inline void cross(const vec_t& v1, const vec_t& v2)
	{
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
		w = 0.f;
	}
	inline float dot( const vec_t &v) const
	{
		return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
	}

	void isMaxOf(const vec_t& v)
	{
		x = (v.x>x)?v.x:x;
		y = (v.y>y)?v.y:y;
		z = (v.z>z)?v.z:z;
		w = (v.w>w)?v.z:w;
	}
	void isMinOf(const vec_t& v)
	{
		x = (v.x>x)?x:v.x;
		y = (v.y>y)?y:v.y;
		z = (v.z>z)?z:v.z;
		w = (v.w>w)?z:v.w;
	}

	bool isInside( const vec_t& min, const vec_t& max ) const
	{
		if ( min.x > x || max.x < x ||
			min.y > y || max.y < y ||
			min.z > z || max.z < z  )
			return false;
		return true;
	}
    
    vec_t symetrical(const vec_t& v) const 
    {
        vec_t res;
        float dist = signedDistanceTo(v);
        res = v;
        res -= (*this)*dist*2.f;
        
        return res;
    }
	void transform(const matrix_t& matrix );
	void transform(const vec_t & s, const matrix_t& matrix );

	void TransformVector(const matrix_t& matrix );
	void TransformPoint(const matrix_t& matrix );
	void TransformVector(const vec_t& v, const matrix_t& matrix ) { (*this) = v; this->TransformVector(matrix); }
	void TransformPoint(const vec_t& v, const matrix_t& matrix ) { (*this) = v; this->TransformPoint(matrix); }

    // quaternion slerp
    //void slerp(const vec_t &q1, const vec_t &q2, float t );

	inline float signedDistanceTo(const vec_t& point) const;
	vec_t interpolateHermite(const vec_t &nextKey, const vec_t &nextKeyP1, const vec_t &prevKey, float ratio) const;
    static float d(const vec_t& v1, const vec_t& v2) { return (v1-v2).length(); }
    static float d2(const vec_t& v1, const vec_t& v2) { return (v1-v2).lengthSq(); }
    
	static vec_t zero;

    uint16 toUInt5551() const { return (uint16_t)(((int)(w*1.f)<< 15) + ((int)(z*31.f)<< 10) + ((int)(y*31.f)<< 5) + ((int)(x*31.f))); }
    void fromUInt5551(unsigned short v) { w = (float)( (v&0x8000) >> 15) ; z = (float)( (v&0x7C00) >> 10) * (1.f/31.f); 
	y = (float)( (v&0x3E0) >> 5) * (1.f/31.f); x = (float)( (v&0x1F)) * (1.f/31.f); } 

	uint32_t toUInt32() const { return ((int)(w*255.f)<< 24) + ((int)(z*255.f)<< 16) + ((int)(y*255.f)<< 8) + ((int)(x*255.f)); }
	void fromUInt32(uint32_t v) { w = (float)( (v&0xFF000000) >> 24) * (1.f/255.f); z = (float)( (v&0xFF0000) >> 16) * (1.f/255.f);
	y = (float)( (v&0xFF00) >> 8) * (1.f/255.f); x = (float)( (v&0xFF)) * (1.f/255.f); } 

    vec_t swapedRB() const;
	float& operator [] (size_t index) { return ((float*)&x)[index]; }
	const float& operator [] (size_t index) const { return ((float*)&x)[index]; }
};

inline vec_t vec_t::operator * ( float f ) const { return vec_t(x * f, y * f, z * f, w *f); }
inline vec_t vec_t::operator - () const { return vec_t(-x, -y, -z, -w); }
inline vec_t vec_t::operator - ( const vec_t& v ) const { return vec_t(x - v.x, y - v.y, z - v.z, w - v.w); }
inline vec_t vec_t::operator + ( const vec_t& v ) const { return vec_t(x + v.x, y + v.y, z + v.z, w + v.w); }
inline vec_t vec_t::operator * ( const vec_t& v ) const { return vec_t(x * v.x, y * v.y, z * v.z, w * v.w); }
inline float vec_t::signedDistanceTo(const vec_t& point) const	{ return (point.dot(vec_t(x,y,z))) - w; }

inline vec_t normalized(const vec_t& v) { vec_t res; res = v; res.normalize(); return res; }
inline vec_t cross(const vec_t& v1, const vec_t& v2)
{
    vec_t res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.x - v1.x * v2.z;
    res.z = v1.x * v2.y - v1.y * v2.x;
    res.w = 0.f;
    return res;
}

inline float Dot( const vec_t &v1, const vec_t &v2) 
{
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

inline float Distance(const vec_t& v1, const vec_t& v2) { return vec_t::d(v1, v2); }
inline float DistanceXY(const vec_t& v1, const vec_t& v2) { return vec_t::d(vec_t(v1.x, v1.y), vec_t(v2.x, v2.y)); }
inline float DistanceSq(const vec_t& v1, const vec_t& v2) { return vec_t::d2(v1, v2); }

inline vec_t MakeNormal(const vec_t & point1, const vec_t & point2, const vec_t & point3)
{
	vec_t nrm;
	vec_t tmp1 = point1 - point3;
	vec_t tmp2 = point2 - point3;
	nrm.cross(tmp1, tmp2);
	return nrm;
}

inline float vecByIndex(const vec_t& v, int idx)
{
	switch( idx)
	{
	case 0: return v.x;
	case 1: return v.y;
	case 2: return v.z;
	default: return v.w;
	}
}

inline vec_t vecMul(const vec_t& v1, const vec_t& v2)
{
	return vec_t(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z , v1.w * v2.w);
}

inline vec_t vecMin(const vec_t& v1, const vec_t& v2)
{
	vec_t res = v1;
	res.isMinOf(v2);

	return res;
}
inline vec_t vecMax(const vec_t& v1, const vec_t& v2)
{
	vec_t res = v1;
	res.isMaxOf(v2);
	return res;
}

inline vec_t vecFloor(const vec_t& v)
{
	return vec_t(floorf(v.x), floorf(v.y), floorf(v.z), floorf(v.w) );
}

inline vec_t splatZ(const vec_t& v) { return vec_t(v.z); }
inline vec_t splatW(const vec_t& v) { return vec_t(v.w); }

inline vec_t vecReciprocal(const vec_t& v) { return vec_t(1.f/v.x, 1.f/v.y, 1.f/v.z, 1.f/v.w); }

inline vec_t buildPlan(const vec_t & p_point1, const vec_t & p_normal)
{
	vec_t normal, res;
	normal.normalize(p_normal);
	res.w = normal.dot(p_point1);
	res.x = normal.x;
	res.y = normal.y;
	res.z = normal.z;
	
	return res;
}
inline vec_t vec_t::swapedRB() const { return vec_t(z,y,x,w); }

inline float smootherstep(float edge0, float edge1, float x)
{
    // Scale, and clamp x to 0..1 range
    x = Clamp((x - edge0)/(edge1 - edge0), 0, 1);
    // Evaluate polynomial
    return x*x*x*(x*(x*6 - 15) + 10);
}

inline vec_t *slerp(vec_t *pout, const vec_t* pq1, const vec_t* pq2, float t)
{
    float dot, epsilon;

    epsilon = 1.0f;
    dot = pq1->dot( *pq2 );
    if ( dot < 0.0f ) epsilon = -1.0f;
    pout->x = (1.0f - t) * pq1->x + epsilon * t * pq2->x;
    pout->y = (1.0f - t) * pq1->y + epsilon * t * pq2->y;
    pout->z = (1.0f - t) * pq1->z + epsilon * t * pq2->z;
    pout->w = (1.0f - t) * pq1->w + epsilon * t * pq2->w;
    return pout;
}

inline vec_t TransformPoint(const vec_t& v, const matrix_t& matrix) { vec_t p(v); p.TransformPoint(matrix); return p; }
inline vec_t TransformVector(const vec_t& v, const matrix_t& matrix) { vec_t p(v); p.TransformVector(matrix); return p; }

///////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct vec3
{
    float x,y,z;
    void set( float v)
    {
        x = y = z = v;
    }
    float length() const 
    {
        return sqrtf( x * x + y * y + z * z );
    }
    void lerp( float v, float t)
    {
        x = LERP( x, v, t);
        y = LERP( y, v, t);
        z = LERP( z, v, t);        
    }
    vec3& operator = (const vec_t& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    vec3& operator * (const float v)
    {
        vec3 ret;
        ret.x = x * v;
        ret.y = y * v;
        ret.z = z * v;
        return *this;
    }
    
    vec3& operator += (const vec_t& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator += (const vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator -= (const vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    vec_t getVec() const
    {
        return vec_t( x, y, z, 0.f );
    }
} vec3;

///////////////////////////////////////////////////////////////////////////////////////////////////

struct matrix_t
{
public:
	union
	{
		float m[4][4];
		float m16[16];
		struct 
		{
			vec_t right, up, dir, position;
		} ;
	};

	matrix_t(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
    {
		m16[0] = v1;
		m16[1] = v2;
		m16[2] = v3;
		m16[3] = v4;
		m16[4] = v5;
		m16[5] = v6;
		m16[6] = v7;
		m16[7] = v8;
		m16[8] = v9;
		m16[9] = v10;
		m16[10] = v11;
		m16[11] = v12;
		m16[12] = v13;
		m16[13] = v14;
		m16[14] = v15;
		m16[15] = v16;
    }
	matrix_t(const matrix_t& other) { memcpy(&m16[0], &other.m16[0], sizeof(float) * 16); }
	matrix_t(const vec_t & r, const vec_t &u, const vec_t& d, const vec_t& p) { set(r, u, d, p); }
    matrix_t() {}
	void set(const vec_t & r, const vec_t &u, const vec_t& d, const vec_t& p) { right=r; up=u; dir=d; position=p; }
	void set(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9, float v10, float v11, float v12, float v13, float v14, float v15, float v16)
	{
		m16[0] = v1;
		m16[1] = v2;
		m16[2] = v3;
		m16[3] = v4;
		m16[4] = v5;
		m16[5] = v6;
		m16[6] = v7;
		m16[7] = v8;
		m16[8] = v9;
		m16[9] = v10;
		m16[10] = v11;
		m16[11] = v12;
		m16[12] = v13;
		m16[13] = v14;
		m16[14] = v15;
		m16[15] = v16;
	}
	static matrix_t GetIdentity() { return matrix_t(1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	}
	operator float * () { return m16; }
	operator const float* () const { return m16; }
	void translation(float _x, float _y, float _z) { this->translation( vec_t(_x, _y, _z) ); }
	
	void translation(const vec_t& vt)
	{ 
		right.set(1.f, 0.f, 0.f, 0.f); 
		up.set(0.f, 1.f, 0.f, 0.f); 
		dir.set(0.f, 0.f, 1.f, 0.f); 
		position.set(vt.x, vt.y, vt.z, 1.f); 
	}
	void translationScale(const vec_t& vt, const vec_t& scale)
	{
		right.set(scale.x, 0.f, 0.f, 0.f);
		up.set(0.f, scale.y, 0.f, 0.f);
		dir.set(0.f, 0.f, scale.z, 0.f);
		position.set(vt.x, vt.y, vt.z, 1.f);
	}

	inline void rotationY(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right.set(c, 0.f, -s, 0.f);
		up.set(0.f, 1.f, 0.f , 0.f);
		dir.set(s, 0.f, c , 0.f);
		position.set(0.f, 0.f, 0.f , 1.f);
	}

	inline void rotationX(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right.set(1.f, 0.f , 0.f, 0.f);
		up.set(0.f, c , s, 0.f);
		dir.set(0.f, -s, c, 0.f);
		position.set(0.f, 0.f , 0.f, 1.f);
	}

	inline void rotationZ(const float angle )
	{
		float c = cosf(angle);
		float s = sinf(angle);

		right.set(c , s, 0.f, 0.f);
		up.set(-s, c, 0.f, 0.f);
		dir.set(0.f , 0.f, 1.f, 0.f);
		position.set(0.f , 0.f, 0, 1.f);
	}
	inline void scale(float _s)
	{
		right.set(_s, 0.f, 0.f, 0.f); 
		up.set(0.f, _s, 0.f, 0.f); 
		dir.set(0.f, 0.f, _s, 0.f); 
		position.set(0.f, 0.f, 0.f, 1.f); 
	}
	inline void scale(float _x, float _y, float _z)
	{
		right.set(_x, 0.f, 0.f, 0.f); 
		up.set(0.f, _y, 0.f, 0.f); 
		dir.set(0.f, 0.f, _z, 0.f); 
		position.set(0.f, 0.f, 0.f, 1.f); 
	}
	inline void scale(const vec_t& s) { scale(s.x, s.y, s.z); }

	inline matrix_t& operator *= ( const matrix_t& mat )
	{
		matrix_t tmpMat;
		tmpMat = *this;
		tmpMat.Multiply(mat);
		*this = tmpMat;
		return *this;
	}
	inline matrix_t operator * ( const matrix_t& mat ) const
	{
		matrix_t matT;
		matT.Multiply(*this, mat);
		return matT;
	}

	inline void Multiply( const matrix_t &matrix)
	{
		matrix_t tmp;
		tmp = *this;

		FPU_MatrixF_x_MatrixF( (float*)&tmp, (float*)&matrix, (float*)this);
	}

	inline void Multiply( const matrix_t &m1, const matrix_t &m2 )
	{
		FPU_MatrixF_x_MatrixF( (float*)&m1, (float*)&m2, (float*)this);
	}

	void glhPerspectivef2(float fovyInDegrees, float aspectRatio, float znear, float zfar);
	void glhPerspectivef2Rad(float fovyRad, float aspectRatio, float znear, float zfar);
	
	void glhFrustumf2(float left, float right, float bottom, float top,	float znear, float zfar);
	void PerspectiveFovLH2(const float fovy, const float aspect, const float zn, const float zf );
	void OrthoOffCenterLH(const float l, float r, float b, const float t, float zn, const float zf );
	void lookAtRH(const vec_t &eye, const vec_t &at, const vec_t &up );
	void lookAtLH(const vec_t &eye, const vec_t &at, const vec_t &up );
	void LookAt(const vec_t &eye, const vec_t &at, const vec_t &up );
    void rotationQuaternion( const vec_t &q );

	inline float GetDeterminant() const
	{
		return m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] +    m[0][2] * m[1][0] * m[2][1] -
			m[0][2] * m[1][1] * m[2][0] - m[0][1] * m[1][0] * m[2][2] -    m[0][0] * m[1][2] * m[2][1];
	}

	float inverse(const matrix_t &srcMatrix, bool affine = false );
	float inverse(bool affine=false);
	void identity() { 		
		right.set(1.f, 0.f, 0.f, 0.f); 
		up.set(0.f, 1.f, 0.f, 0.f); 
		dir.set(0.f, 0.f, 1.f, 0.f); 
		position.set(0.f, 0.f, 0.f, 1.f); 
	}
	inline void transpose()
	{
		matrix_t tmpm;
		for (int l = 0; l < 4; l++)
		{
			for (int c = 0; c < 4; c++)
			{
				tmpm.m[l][c] = m[c][l];
			}
		}
		(*this) = tmpm;
	}
	void rotationAxis(const vec_t & axis, float angle );
	void lerp(const matrix_t& r, const matrix_t& t, float s) 
	{ 
		right = LERP(r.right, t.right, s);
		up = LERP(r.up, t.up, s);
		dir = LERP(r.dir, t.dir, s);
		position = LERP(r.position, t.position, s);
	}
	void rotationYawPitchRoll(const float yaw, const float pitch, const float roll );

	inline void orthoNormalize()
	{
		right.normalize();
		up.normalize();
		dir.normalize();
	}

    static matrix_t Identity;


};

#ifdef MACOSX
#include <xmmintrin.h> //declares _mm_* intrinsics
#endif
//#include <intrin.h>
/*
#ifdef WIN32
inline int FloatToInt_SSE(float x)
{
    return _mm_cvt_ss2si( _mm_load_ss(&x) );
}
#endif
*/
extern int g_seed;
inline int fastrand() 
{ 
	g_seed = (214013*g_seed+2531011); 
	return (g_seed>>16)&0x7FFF; 
} 

inline float r01()
{
	return ((float)fastrand())*(1.f/32767.f);
}



inline bool CollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2, vec_t& res )
{
    
    vec_t c = point - vertPos1;
    vec_t V;
    
    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);
    
    if (t < 0.f)
    {
        return false;//vertPos1;
    }
    
    if (t > d)
    {
        return false;//vertPos2;
    }
    
    res = vertPos1 + V * t;
    return true;
}

inline vec_t CollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2 )
{
    
    vec_t c = point - vertPos1;
    vec_t V;
    
    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);
    
    if (t < 0.f)
    {
        return vertPos1;
    }
    
    if (t > d)
    {
        return vertPos2;
    }
    
    return vertPos1 + V * t;
}



inline float DistanceCollisionClosestPointOnSegment( const vec_t & point, const vec_t & vertPos1, const vec_t & vertPos2 )
{
    vec_t c = point - vertPos1;
    vec_t V;
    
	if ((vertPos2 - vertPos1).lengthSq() < 0.001f)
		return Distance(point, vertPos1);

    V.normalize(vertPos2 - vertPos1);
    float d = (vertPos2 - vertPos1).length();
    float t = V.dot(c);
    
    if (t < 0.f)
    {
        return (vertPos1-point).length();
    }
    
    if (t > d)
    {
        return (vertPos2-point).length();
    }
    
    vec_t r = vertPos1 + V * t;
    return ( r - point ).length();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> struct PID
{
        inline PID()
        {
                Ki = 1;
                Kp = 1;
                Kd = 1;
                error = previouserror = I = 0.f;
        }
        inline PID(float _Ki, float _Kp, float _Kd)
        {
                Ki = _Ki;
                Kp = _Kp;
                Kd = _Kd;
                error = previouserror = I = 0.f;
        }
        inline void SetIPD(float _Ki, float _Kp, float _Kd)
        {
                Ki = _Ki;
                Kp = _Kp;
                Kd = _Kd;
        }
        /*
        start:
        previous_error = error or 0 if undefined
        error = setpoint - actual_position
        P = Kp * error
        I = I + Ki * error * dt
        D = Kd * (error - previous_error) / dt
        output = P + I + D
        wait(dt)
        goto start
        */
        inline T Compute(const T& desiredPos, const T& currentPos, float dt)
        {
                previouserror = error;
                error = desiredPos - currentPos;
                T P = Kp * error;
                I += Ki * error * dt;
                T D = Kd * (error - previouserror) / dt;
                T output = P + I + D;
                return output;
        }

        T error, previouserror, I;
        float Ki, Kp, Kd;
};

typedef PID<float> PIDf;

#pragma pack(push)
#pragma pack(1)
struct fixed816_t
{
    char intValue;
    short floatValue;

    float toFloat()
    {
        return static_cast<float>(intValue) + static_cast<float>(floatValue)/32767.f;
    }

    fixed816_t( float v)
    {
        intValue = static_cast<char>(v);
        floatValue = static_cast<unsigned short>(fmodf(v, 1.f) * 32767.f);
    }
};

#pragma pack(pop)



struct ZFrustum  
{
	ZFrustum()
	{
	}
	
    void    Update(const matrix_t &view, const matrix_t& projection);
	void	Update(const float* clip);
    bool    PointInFrustum( const vec_t & vt ) const
    {
        // If you remember the plane equation (A*x + B*y + C*z + D = 0), then the rest
        // of this code should be quite obvious and easy to figure out yourself.
        // In case don't know the plane equation, it might be a good idea to look
        // at our Plane Collision tutorial at www.GameTutorials.com in OpenGL Tutorials.
        // I will briefly go over it here.  (A,B,C) is the (X,Y,Z) of the normal to the plane.
        // They are the same thing... but just called ABC because you don't want to say:
        // (x*x + y*y + z*z + d = 0).  That would be wrong, so they substitute them.
        // the (x, y, z) in the equation is the point that you are testing.  The D is
        // The Distance the plane is from the origin.  The equation ends with "= 0" because
        // that is true when the point (x, y, z) is ON the plane.  When the point is NOT on
        // the plane, it is either a negative number (the point is behind the plane) or a
        // positive number (the point is in front of the plane).  We want to check if the point
        // is in front of the plane, so all we have to do is go through each point and make
        // sure the plane equation goes out to a positive number on each side of the frustum.
        // The result (be it positive or negative) is the Distance the point is front the plane.

        // Go through all the sides of the frustum
        for(int i = 0; i < 6; i++ )
        {
            // Calculate the plane equation and check if the point is behind a side of the frustum
            if(m_Frustum[i][A] * vt.x + m_Frustum[i][B] * vt.y + m_Frustum[i][C] * vt.z + m_Frustum[i][D] <= 0)
            {
                // The point was behind a side, so it ISN'T in the frustum
                return false;
            }
        }

        // The point was inside of the frustum (In front of ALL the sides of the frustum)
        return true;
    }
    
    bool    SphereInFrustum( const vec_t & vt) const
    {
        for(int i = 0; i < 6; i++ )    
        {
            // If the center of the sphere is farther away from the plane than the radius
            if( m_Frustum[i][A] * vt.x + m_Frustum[i][B] * vt.y + m_Frustum[i][C] * vt.z + m_Frustum[i][D] <= -vt.w )
            {
                // The Distance was greater than the radius so the sphere is outside of the frustum
                return false;
            }
        }
        
        // The sphere was inside of the frustum!
        return true;
    }

	int SphereInFrustumVis(const vec_t& v) const
	{

		float Distance;
		int result = 2;

		for(int i=0; i < 6; i++) {
			Distance = m_Frustum[i][A] * v.x + m_Frustum[i][B] * v.y + m_Frustum[i][C] * v.z + m_Frustum[i][D];//pl[i].Distance(p);
			if (Distance < -v.w)
				return 0;
			else if (Distance < v.w)
				result =  1;
		}
		return(result);
	}

    bool    BoxInFrustum( const vec_t & vt, const vec_t & size ) const
    {
        for(int i = 0; i < 6; i++ )
        {
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z - size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y - size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x - size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;
            if(m_Frustum[i][A] * (vt.x + size.x) + m_Frustum[i][B] * (vt.y + size.y) + m_Frustum[i][C] * (vt.z + size.z) + m_Frustum[i][D] > 0)
            continue;

            // If we get here, it isn't in the frustum
            return false;
        }

        return true;
    }

	// matrix is an orthonormalized matrix. only orientation is used.
	bool OBBInFrustum( const matrix_t &mt, const vec_t &pos, const vec_t& size) const;

public:

    float m_Frustum[6][4];
    void NormalizePlane(float frustum[6][4], int side);

    enum FrustumSide
    {
        RIGHT    = 0,        // The RIGHT side of the frustum
        LEFT    = 1,        // The LEFT     side of the frustum
        BOTTOM    = 2,        // The BOTTOM side of the frustum
        TOP        = 3,        // The TOP side of the frustum
        BACK    = 4,        // The BACK    side of the frustum
        FRONT    = 5            // The FRONT side of the frustum
    }; 

    // Like above, instead of saying a number for the ABC and D of the plane, we
    // want to be more descriptive.
    enum PlaneData
    {
        A = 0,                // The X value of the plane's normal
        B = 1,                // The Y value of the plane's normal
        C = 2,                // The Z value of the plane's normal
        D = 3                // The Distance the plane is from the origin
    };
    

};

///////////////////////////////////////////////////////////////////////////////////////////////////
/*
void CovarianceMatrix(Matrix33 &cov, Point pt[], int numPts)
{
float oon = 1.0f / (float)numPts;
Point c = Point(0.0f, 0.0f, 0.0f);
float e00, e11, e22, e01, e02, e12;
// Compute the center of mass (centroid) of the points
for (int i = 0; i < numPts; i++)
c += pt[i];
c *= oon;
// Compute covariance elements
e00 = e11 = e22 = e01 = e02 = e12 = 0.0f;
for (int i = 0; i < numPts; i++) {
// Translate points so center of mass is at origin
Point p = pt[i] - c;
// Compute covariance of translated points
e00 += p.x * p.x;
e11 += p.y * p.y;
e22 += p.z * p.z;
e01 += p.x * p.y;
e02 += p.x * p.z;
e12 += p.y * p.z;
}
// Fill in the covariance matrix elements
cov[0][0] = e00 * oon;
cov[1][1] = e11 * oon;
cov[2][2] = e22 * oon;
cov[0][1] = cov[1][0] = e01 * oon;
cov[0][2] = cov[2][0] = e02 * oon;
cov[1][2] = cov[2][1] = e12 * oon;
}
*/

inline int NumberOfSetBits(unsigned char i)
{
    i = i - ((i >> 1) & 0x55);
    i = (i & 0x33) + ((i >> 2) & 0x33);
    return (i + (i >> 4)) & 0x0F;
}




struct Segment
{
	Segment() {}
	Segment(vec_t p0, vec_t p1) : P0(p0), P1(p1)
	{
	}

	vec_t P0,P1;
};
struct CapsuleHitHandler;
struct Capsule : public Segment
{
	Capsule() :mask(0),mWeaponIndex(0),mHitStrength(0),mHandle(NULL) {}
	Capsule( vec_t p0, vec_t p1, float rad, CapsuleHitHandler *handle ) : Segment(p0, p1), radius( rad ), mask(0), mHandle(handle),mWeaponIndex(0),mHitStrength(0)
	{
		previousP0 = p0;
		previousP1 = p1;
	}
	float radius;
	vec_t previousP0;
	vec_t previousP1;
	CapsuleHitHandler *mHandle;
	unsigned char mWeaponIndex;
	unsigned char mHitStrength;
	union
	{
		uint32_t mask;
		struct
		{
			uint32_t mBody:1;
			uint32_t mSprawler:1;
			uint32_t mHit:1;
			uint32_t mDull:1;
		};
	};
};
//===================================================================


// dist3D_Segment_to_Segment(): get the 3D minimum Distance between 2 segments
//    Input:  two 3D line segments S1 and S2
//    Return: the shortest Distance between S1 and S2
inline float
dist3D_Segment_to_Segment( Segment S1, Segment S2)
{
    vec_t   u = S1.P1 - S1.P0;
    vec_t   v = S2.P1 - S2.P0;
    vec_t   w = S1.P0 - S2.P0;
    float    a = Dot(u,u);         // always >= 0
    float    b = Dot(u,v);
    float    c = Dot(v,v);         // always >= 0
    float    d = Dot(u,w);
    float    e = Dot(v,w);
    float    D = a*c - b*b;        // always >= 0
    float    sc, sN, sD = D;       // sc = sN / sD, default sD = D >= 0
    float    tc, tN, tD = D;       // tc = tN / tD, default tD = D >= 0

    // compute the line parameters of the two closest points
    if (D < FLOAT_EPSILON) { // the lines are almost parallel
        sN = 0.0;         // force using point P0 on segment S1
        sD = 1.0;         // to prevent possible division by 0.0 later
        tN = e;
        tD = c;
    }
    else {                 // get the closest points on the infinite lines
        sN = (b*e - c*d);
        tN = (a*e - b*d);
        if (sN < 0.0) {        // sc < 0 => the s=0 edge is visible
            sN = 0.0;
            tN = e;
            tD = c;
        }
        else if (sN > sD) {  // sc > 1  => the s=1 edge is visible
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    if (tN < 0.0) {            // tc < 0 => the t=0 edge is visible
        tN = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sN = 0.0;
        else if (-d > a)
            sN = sD;
        else {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD) {      // tc > 1  => the t=1 edge is visible
        tN = tD;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sN = 0;
        else if ((-d + b) > a)
            sN = sD;
        else {
            sN = (-d +  b);
            sD = a;
        }
    }
    // finally do the division to get sc and tc
    sc = (fabsf(sN) < FLOAT_EPSILON ? 0.0f : sN / sD);
    tc = (fabsf(tN) < FLOAT_EPSILON ? 0.0f : tN / tD);

    // get the difference of the two closest points
    vec_t   dP = w + (u * sc) - (v * tc);  // =  S1(sc) - S2(tc)

	return dP.length();   // return the closest Distance
}

static const float RopeDamping     = 0.01f;
/*
static const tvector2 GRAVITY(0, -9.81f);
float DENSITY_OFFSET                       = 100.f;
float GAS_K                                = 0.1f;
float VISC0SITY                            = 0.005f;
*/
inline void SolveVerletRope(vec_t& position, vec_t& positionOld, vec_t& velocity, vec_t acceleration, float timeStep)
{
	vec_t t;
	vec_t oldPos = position;
	acceleration *= timeStep*timeStep;
	t = position - positionOld;
	t *= 1.f-RopeDamping;
	t += acceleration;
	position += t;
	positionOld = oldPos;

	// calculate velocity
	// Velocity = (Position - PositionOld) / dt;
	t = position-positionOld;
	velocity = t*(1.f/timeStep);
}


inline void SolveVerletRope(vec_t& position, vec_t& positionOld, vec_t& velocity, vec_t acceleration, float damping, float timeStep)
{
	vec_t t;
	vec_t oldPos = position;
	acceleration *= timeStep*timeStep;
	t = position - positionOld;
	t *= 1.f-damping;
	t += acceleration;
	position += t;
	positionOld = oldPos;

	// calculate velocity
	// Velocity = (Position - PositionOld) / dt;
	t = position-positionOld;
	velocity = t*(1.f/timeStep);
}

inline int SIGNBIT(float v) { return (v<0.f)?1:0; }

inline vec_t perpStark(vec_t u)
{
    vec_t a = vec_t( fabsf(u.x), fabsf(u.y), fabsf(u.z), 0.f );
    uint uyx = SIGNBIT(a.x - a.y);
    uint uzx = SIGNBIT(a.x - a.z);
    uint uzy = SIGNBIT(a.y - a.z);

    uint xm = uyx & uzx;
    uint ym = (1^xm) & uzy;
    uint zm = 1^(xm & ym);

    vec_t v = cross( u, vec_t( (float)xm, (float)ym, (float)zm ) );
    return v;
}


inline vec_t computeVecFromPlanNormal( vec_t localDir, float angle, float len )
{
	vec_t newDirX = perpStark(localDir);
	vec_t newDirY;
	newDirY.cross( localDir, newDirX );
	newDirY.normalize();
	vec_t newDir = ( newDirX * cosf( angle ) + newDirY * sinf( angle ) ) * len;
	return newDir;
}

inline vec_t computeRandomVecFromPlanNormal( vec_t localDir )
{
	return computeVecFromPlanNormal( localDir, r01()*ZPI*2.f, r01() );
}

inline float roundf(float d)
{
  return floorf(d + 0.5f);
}

inline vec_t Reflect(const vec_t &incidentVec, const vec_t &normal)
{
	return incidentVec - normal * Dot(incidentVec, normal) * 2.f;
}


inline vec_t computePlansIntersections( vec_t n1, float d1, vec_t n2, float d2, vec_t n3, float d3 )
{
	float div = Dot(n1,cross(n2,n3));
	vec_t u = -cross(n2,n3)*d1 - cross(n3,n1)*d2 - cross(n1,n2)*d3;
	vec_t r = u * (1.f/div);
	return r;
}
//////////////////////////////////////////////////////////////////////

struct vec2i
{
	vec2i() {}
	vec2i(const vec2i& other) : x(other.x), y(other.y) {}
	vec2i(int _x, int _y) : x(_x), y(_y) {}

	bool operator == (const vec2i& other) { return (x == other.x && y == other.y); }
	bool operator != (const vec2i& other) { return (x != other.x || y != other.y); }
	void operator += (const vec2i& other) { x += other.x; y += other.y; }
	void operator -= (const vec2i& other) { x -= other.x; y -= other.y; }
	bool operator < (const vec2i& other) const
	{
		if (y < other.y)
			return true;
		if (y > other.y)
			return false;
		if (x < other.x)
			return true;
		if (x > other.x)
			return false;
		return false;
	}
	void ManNormalize()
	{
		x = (x>0) ? 1 : ((x<0) ? -1 : 0);
		y = (y>0) ? 1 : ((y<0) ? -1 : 0);
	}
	int ManLength() { return abs(x + y); }
	float length() const { return sqrtf((float)(x*x + y*y)); }

	vec2i Rotate(float angle) const
	{
		float cs = cosf(angle);
		float sn = sinf(angle);
		return vec2i(int(x*cs - y*sn), int(y*cs + x * sn));
	}
	int x, y;
};

inline vec2i operator + (const vec2i& a, const vec2i &b) { return vec2i(a.x + b.x, a.y + b.y); }
inline vec2i operator - (const vec2i& a, const vec2i &b) { return vec2i(a.x - b.x, a.y - b.y); }
inline vec2i operator * (const vec2i& a, int l) { return vec2i(a.x*l, a.y*l); }

inline vec2i min2i(const vec2i a, const vec2i b)
{
	return vec2i(a.x < b.x ? a.x : b.x, a.y < b.y ? a.y : b.y);
}

inline vec2i max2i(const vec2i a, const vec2i b)
{
	return vec2i(a.x > b.x ? a.x : b.x, a.y > b.y ? a.y : b.y);
}

inline vec_t Rotate(const vec_t & v, float a)
{
	float sn = sinf(a);
	float cs = cosf(a);
	return vec_t(v.x * cs - v.y * sn, v.x * sn + v.y * cs, 0.f);
}

inline float GetAngle(const vec_t& a, const vec_t& b)
{
	vec_t crossed = cross(a, b);
	float dt = Dot(a, b);
	return atan2f(crossed.length(), dt) * ((crossed.z >= 0.f) ? 1.f:-1.f);
}

inline float SignOf(float x)
{
	return (x < 0.f) ? -1.f : 1.f;
}
template<typename T> bool SegmentsIntersect(T x1, T x2, T y1, T y2)
{
	// Assumes x1 <= x2 and y1 <= y2; if this assumption is not safe, the code
	// can be changed to have x1 being min(x1, x2) and x2 being max(x1, x2) and
	// similarly for the ys.
	return x2 >= y1 && y2 >= x1;
}
//static const float ZPI = 3.14159265358979323846f;
static const float RAD2DEG = (180.f / ZPI);
static const float DEG2RAD = (ZPI / 180.f);

inline void DecomposeMatrixToComponents(const float* matrix, float* translation, float* rotation, float* scale)
{
	matrix_t mat = *(matrix_t*)matrix;

	scale[0] = mat.right.length();
	scale[1] = mat.up.length();
	scale[2] = mat.dir.length();

	mat.orthoNormalize();

	rotation[0] = RAD2DEG * atan2f(mat.m[1][2], mat.m[2][2]);
	rotation[1] = RAD2DEG * atan2f(-mat.m[0][2], sqrtf(mat.m[1][2] * mat.m[1][2] + mat.m[2][2] * mat.m[2][2]));
	rotation[2] = RAD2DEG * atan2f(mat.m[0][1], mat.m[0][0]);

	translation[0] = mat.position.x;
	translation[1] = mat.position.y;
	translation[2] = mat.position.z;
}

inline void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matrix)
{
	matrix_t& mat = *(matrix_t*)matrix;
	static const vec_t directionUnary[3] = { vec_t(1.f, 0.f, 0.f), vec_t(0.f, 1.f, 0.f), vec_t(0.f, 0.f, 1.f) };
	matrix_t rot[3];
	for (int i = 0; i < 3; i++)
	{
		rot[i].rotationAxis(directionUnary[i], rotation[i] * DEG2RAD);
	}

	mat = rot[0] * rot[1] * rot[2];

	float validScale[3];
	for (int i = 0; i < 3; i++)
	{
		if (fabsf(scale[i]) < FLT_EPSILON)
		{
			validScale[i] = 0.001f;
		}
		else
		{
			validScale[i] = scale[i];
		}
	}
	mat.right *= validScale[0];
	mat.up *= validScale[1];
	mat.dir *= validScale[2];
	mat.position = vec_t(translation[0], translation[1], translation[2], 1.f);
}