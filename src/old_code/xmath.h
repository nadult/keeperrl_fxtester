//------------------------------------------------------------------\
//X LIBRARY															|
//xmath.h															|
//																	|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2006					|
//------------------------------------------------------------------/




#ifndef XMATH_H
#define XMATH_H



#include <xsystem.h>

class Vec2;
class Vec3;
class Vec4;
class Matrix;
class AMatrix;
class Quat;
class Plane;
class Ray;

XTAddPod(Vec2)		XTAddPod(Vec3)
XTAddPod(Vec4)		XTAddPod(Quat)
XTAddPod(Plane)		XTAddPod(Ray)
XTAddPod(Matrix)	XTAddPod(AMatrix)

class Scale;
class Translate;
class AxisAngle;
class Matrix;
class AMatrix;

#if SSE_ENABLED

class FFloat;
class FVec2;
class FVec3;
class FVec4;

#include <xmmintrin.h>
#if SSE2_ENABLED
#include <emmintrin.h>
#endif

__m128 _mm_nrrcp_ss(const __m128 &x);
__m128 _mm_nrrcp_ps(const __m128 &x);
__m128 _mm_nrrsqrt_ss(const __m128 &x);
__m128 _mm_nrrsqrt_ps(const __m128 &x);
__m128 _mm_nrsqrt_ss(const __m128 &x);
__m128 _mm_nrsqrt_ps(const __m128 &x);

template <int t> __m128 __mm_shuffle(__m128 v) { return _mm_shuffle_ps(v,v,t); }
#define _mm_shuffle(val,...) __mm_shuffle<val>(__VA_ARGS__)

// Loads matrix m into xmm registers m1,m2,m3,m4
#define _mm_loadmat(m,m1,m2,m3,m4) \
	if(u32(m)&15){ m1=_mm_loadu_ps((m)+0);m2=_mm_loadu_ps((m)+4);m3=_mm_loadu_ps((m)+8);m4=_mm_loadu_ps((m)+12); } \
	else{ m1=_mm_load_ps((m)+0); m2=_mm_load_ps((m)+4); m3=_mm_load_ps((m)+8); m4=_mm_load_ps((m)+12); }

/// Similar to set, supports every data type supported by UValue
#define _mm_set(a,b,c,d) _mm_set_ps(UValue(d).f,UValue(c).f,UValue(b).f,UValue(a).f)


#else

typedef Vec2 FVec2;
typedef Vec3 FVec3;
typedef Vec4 FVec4;
typedef Ray FRay;
typedef Plane FPlane;
typedef float FFloat;

#endif


#define DECLARE_VEC_FUNCS(vec,scl,mat,amat) \
	template<> scl Length<vec>(const vec&); \
	template<> vec Normalize<vec>(const vec&); \
	template<> vec Abs<vec>(const vec&); \
	template<> vec Min<vec>(const vec&,const vec&); \
	template<> vec Max<vec>(const vec&,const vec&); \
	scl Sum(const vec&); \
	vec Round(const vec&); \
	vec Inv(const vec&); \
	vec Sqrt(const vec&); \
	vec RSqrt(const vec&); \
	vec And(const vec&,const vec&); \
	vec AndNot(const vec&,const vec&); \
	vec Xor(const vec&,const vec&); \
	vec Or(const vec&,const vec&); \
	vec CmpG(const vec&,const vec&); \
	vec CmpL(const vec&,const vec&); \
	vec CmpGE(const vec&,const vec&); \
	vec CmpLE(const vec&,const vec&); \
	int GetSign(const vec&); \
	inline vec operator +(const vec &a,const vec &b) { vec t(a); t+=b; return t; } \
	inline vec operator -(const vec &a,const vec &b) { vec t(a); t-=b; return t; } \
	inline vec operator /(const vec &a,const vec &b) { vec t(a); t/=b; return t; } \
	inline vec operator ^(const vec &a,const vec &b) { vec t(a); t^=b; return t; } \
	inline vec operator *(const vec &a,const vec &b) { vec t(a); t*=b; return t; } \
	inline vec operator *(const vec &a,const scl &s) { vec t(a); t*=s; return t; } \
	inline vec operator *(const scl &s,const vec &a) { vec t(a); t*=s; return t; } \
	inline vec operator /(const vec &a,const scl &s) { vec t(a); t/=s; return t; }

#define DECLARE_FLOAT_OPS(vec) \
	inline vec operator *(const vec &a,const float &f) { vec t(a); t*=FFloat(f); return t; } \
	inline vec operator /(const vec &a,const float &f) { vec t(a); t/=FFloat(f); return t; } \
	inline void operator*=(vec &a,const float &f) { a*=FFloat(f); } \
	inline void operator/=(vec &a,const float &f) { a/=FFloat(f); }


typedef Vec3 Vec;
typedef FVec3 FVec;



#define PI					3.141592654f
#define PI2					6.283185308f
#define INVPI				0.318309886f
#define INVPI2				0.159154943f
#define PIDIV360			0.008726646f
#define PI2DIV360			0.017453293f
#define INVPIDIV360			114.5915624f
#define INVPI2DIV360		57.29578121f

#define XEPSILION			0.000001f


template <class T> T Abs(const T &a) { return a<0?-a:a; }
template <class T> T Lerp(const T &a,const T &b,float x) { return a+(b-a)*x; }
template <class T> T CLerp(const T &a,const T &b,float x) { return Lerp(a,b,(1.0f-Cos(x*PI))*0.5f); }
template <class T> T QLerp(const T &v0,const T &v1,const T &v2,const T &v3,float x)
{ XTEST; return v1*(1-x)+v2*x+(v2-v3+v1-v0)*(1-x)*x; }


float Sqrt(float);
float RSqrt(float);
float Inv(float);
int Round(float);

float Cos(float rad);
float Sin(float rad);
Vec2 SinCos(float rad);
float CosD(float deg);
float SinD(float deg);
Vec2 SinCosD(float rad);

template <class VEC> typename VEC::TScalar Length(const VEC &t) { return Sqrt(t|t); }
template <class VEC> VEC Normalize(const VEC &t) { VEC out; out=t/Length(t); return out; }
template <class VEC> float Dist(const VEC &a,const VEC &b) { return Length(b-a); }


/**
XCOLOR - procedurki
*/

///	16-bitowa (R5G6B5) wartoœæ koloru (0,R,G,B)
u16 inline XCol16(u8 r,u8 g,u8 b)
{ return (u16)(((r<<8)&63488)|((g<<3)&2016)|(b>>3)); }
///	32-bitowa (A8R8G8B8) wartoœæ koloru (0,R,G,B)
XCOLOR inline XCol(u8 r,u8 g,u8 b)
{ return ((XCOLOR)r<<16)|((XCOLOR)g<<8)|(XCOLOR)b; }
///	32-bitowa (A8R8G8B8) wartoœæ koloru (A,R,G,B)
XCOLOR inline XCol(u8 a,u8 r,u8 g,u8 b)
{ return ((XCOLOR)a<<24)|((XCOLOR)r<<16)|((XCOLOR)g<<8)|(XCOLOR)b; }
///	Konwertuje kolor 32-bitowy na 16-bitowy
u16 inline XCol32To16(XCOLOR col)
{ return (u16)((((col&16711681)>>8)&63488)|(((col&65280)>>5)&2016)|((col&255)>>3)); }
///	Konwertuje kolor 16-bitowy na 32-bitowy
XCOLOR inline XCol16To32(u16 tCol)
{ u32 col=tCol; return ((col&63488)<<8)|((col&2016)<<5)|((col&31)<<3); }
///	Konwertuje format R5G6B5 na R5G5B5
u16 inline XR5G6B5TOR5G5B5(u16 col)
{ return ((((col&0x7E0)>>1)&0x3e0)+(col&31)+((col&0xF800)>>1))&0x7fff; }
///	Konwertuje format R5G5B5 na R5G6B5
u16 inline XR5G5B5TOR5G6B5(u16 col)
{ return (((col&0x3e0)<<1)&0x7e0)+(col&31)+((col&0xFC00)<<1); }
///	Zwraca sk³adow¹ R 16-bitowego koloru
u8 inline XGetR16(u16 col) { return (u8)((col&63488)>>8); }
///	Zwraca sk³adow¹ G 16-bitowego koloru
u8 inline XGetG16(u16 col) { return (u8)((col&2016)>>3); }
///	Zwraca sk³adow¹ B 16-bitowego koloru
u8 inline XGetB16(u16 col) { return (u8)((col&31)<<3); }
///	Zwraca sk³adow¹ A 32-bitowego koloru
u8 inline XGetA(XCOLOR col) { return (u8)(col>>24); }
///	Zwraca sk³adow¹ R 32-bitowego koloru
u8 inline XGetR(XCOLOR col) { return (u8)((col&16711681)>>16); }
///	Zwraca sk³adow¹ G 32-bitowego koloru
u8 inline XGetG(XCOLOR col) { return (u8)((col&65280)>>8); }
///	Zwraca sk³adow¹ B 32-bitowego koloru
u8 inline XGetB(XCOLOR col) { return (u8)(col&255); }
///	Interpoluje liniowo 2 kolory
template <> inline XCOLOR Lerp(const XCOLOR &A,const XCOLOR &B,float X)
{
	UValue x(X*255.0f+12582912.0f); x.i-=1262485504;
	u8 r=((XGetR(A)<<8)+(XGetR(B)-XGetR(A))*x.ui)>>8;
	u8 g=((XGetG(A)<<8)+(XGetG(B)-XGetG(A))*x.ui)>>8;
	u8 b=((XGetB(A)<<8)+(XGetB(B)-XGetB(A))*x.ui)>>8;
	u8 a=((XGetA(A)<<8)+(XGetA(B)-XGetA(A))*x.ui)>>8;
	return XCol(a,r,g,b);
}
///	Interpoluje liniowo 2 kolory
inline XCOLOR XBlend(XCOLOR a,XCOLOR b,float x) { return Lerp(a,b,x); }

/**
2 component 3D vector class; (x,y,0,1)
*/
class Vec2
{
public:
	typedef float TScalar;

	INLINE Vec2() { }
	INLINE Vec2(const Vec2 &v) :x(v.x),y(v.y) { }
	INLINE void operator=(const Vec2 &v) { x=v.x; y=v.y; }
	INLINE explicit Vec2(float X) :x(X),y(X) { }
	INLINE Vec2(float X,float Y) :x(X), y(Y) { }
	Vec2(const Vec4&);
	operator IVec();
#if SSE_ENABLED
	Vec2(const FVec2 &v);
#endif

	void operator+=(const Vec2&);
	void operator-=(const Vec2&);
	void operator*=(const Vec2&);
	void operator/=(const Vec2&);
	void operator^=(const Vec2&);
	void operator/=(float);
	void operator*=(float);
	Vec2 operator-() const;
	float operator|(const Vec2&) const;
	bool operator==(const Vec2&) const;
	bool operator!=(const Vec2&) const;
	void Zero();

	float x,y;
};

DECLARE_VEC_FUNCS(Vec2,float,Matrix,AMatrix)
float XAngle(const Vec2&);
float XAngleDeg(const Vec2&);


/**
3 component 3D vector class; (x,y,z,1)
*/
class Vec3
{
public:
	typedef float TScalar;

	INLINE Vec3() { }
	INLINE Vec3(const Vec3 &v) :x(v.x),y(v.y),z(v.z) { }
	INLINE void operator=(const Vec3 &v) { x=v.x; y=v.y; z=v.z; }
	INLINE explicit Vec3(float X) :x(X),y(X),z(X) { }
	INLINE Vec3(float X,float Y,float Z) :x(X), y(Y), z(Z) { }
	INLINE Vec3(const Vec2 &v) :x(v.x), y(v.y), z(0) { }
	Vec3(const Vec4 &v);
#if SSE_ENABLED
	Vec3(const FVec3 &v);
#endif

	void operator+=(const Vec3&);
	void operator-=(const Vec3&);
	void operator*=(const Vec3&);
	void operator/=(const Vec3&);
	void operator^=(const Vec3&);
	void operator/=(float);
	void operator*=(float);
	Vec3 operator-() const;
	float operator|(const Vec3&) const;
	bool operator==(const Vec3&) const;
	bool operator!=(const Vec3&) const;
	void Zero();

	float x,y,z;
};

DECLARE_VEC_FUNCS(Vec3,float,Matrix,AMatrix)


/**
4D vector class; (x,y,z,w)
*/
class Vec4
{
public:
	typedef float TScalar;

	INLINE Vec4() { }
	INLINE void operator=(const Vec4 &v) { x=v.x; y=v.y; z=v.z; w=v.w; }
	INLINE Vec4(const Vec4 &v):x(v.x),y(v.y),z(v.z),w(v.w) { }
	INLINE Vec4(const Vec2 &v) :x(v.x),y(v.y),z(0),w(0) { }
	INLINE Vec4(const Vec3 &v) :x(v.x),y(v.y),z(v.z),w(0) { }
	INLINE explicit Vec4(float X) :x(X),y(X),z(X),w(X) { }
	INLINE Vec4(float X,float Y,float Z,float W) :x(X),y(Y),z(Z),w(W) { }
#if SSE_ENABLED
	Vec4(const FVec4 &v);
#endif

	void operator+=(const Vec4&);
	void operator-=(const Vec4&);
	void operator*=(const Vec4&);
	void operator/=(const Vec4&);
	void operator^=(const Vec4&);
	void operator/=(float);
	void operator*=(float);
	Vec4 operator-() const;
	float operator|(const Vec4&) const;
	bool operator==(const Vec4&) const;
	bool operator!=(const Vec4&) const;
	void Zero();

	float x,y,z,w;
};

DECLARE_VEC_FUNCS(Vec4,float,Matrix,AMatrix)



/**
* 2D rectangle class
*/

class Rect
{
public:
	typedef float TScalar;

	INLINE Rect() { }
	INLINE Rect(Vec2 a,Vec2 b) : x1(a.x), y1(a.y), x2(b.x), y2(b.y) { }
	INLINE Rect(float X1,float Y1,float X2,float Y2) : x1(X1), y1(Y1), x2(X2), y2(Y2) { }
	INLINE Rect(IRect &rect) :x1((float)rect.x1),y1((float)rect.y1),x2((float)rect.x2), y2((float)rect.y2) { }

	Vec2 Center() const { return Vec2(x1+(x2-x1)*0.5f,y1+(y2-y1)*0.5f); }
	float CenterX() const { return x1+(x2-x1)*0.5f; }
	float CenterY() const { return y1+(y2-y1)*0.5f; }

	inline float Width() const { return x2-x1; }
	inline float Height() const { return y2-y1; }

	bool PointIsIn(float x,float y) const { return (x>=x1&&x<=x2&&y>=y1&&y<=y2); }
	inline bool PointIsIn(Vec2 p) const { return PointIsIn(p.x,p.y); }

	inline void	SetPointTL(const Vec2 P1) { x1=P1.x; y1=P1.y; }
	inline void	SetPointTR(const Vec2 P1) { x2=P1.x; y1=P1.y; }
	inline void	SetPointBL(const Vec2 P2) { x1=P2.x; y2=P2.y; }
	inline void	SetPointBR(const Vec2 P2) { x2=P2.x; y2=P2.y; }

	inline Vec2 PointTL() const { return Vec2(x1,y1); }
	inline Vec2 PointTR() const { return Vec2(x2,y1); }
	inline Vec2 PointBL() const { return Vec2(x1,y2); }
	inline Vec2 PointBR() const { return Vec2(x2,y2); }

	Rect operator+(const Vec2 &move) const
	{ return Rect(x1+move.x,y1+move.y,x2+move.x,y2+move.y); }
	Rect operator-(const Vec2 &move) const
	{ return Rect(x1-move.x,y1-move.y,x2-move.x,y2-move.y); }
	Rect operator*(float v) const { return Rect(x1*v,y1*v,x2*v,y2*v); }
	Rect operator*(const Vec2 v) const { return Rect(x1*v.x,y1*v.y,x2*v.x,y2*v.y); }
	/// Rectangles intersection; returns (1,1,0,0) when the intersection is empty
	Rect operator&(const Rect rect) const;

	inline operator float*() { return &x1; }
	inline operator const float*() const { return &x1; }

	float x1,y1;
	float x2,y2;
};



#if SSE_ENABLED


class FFloat
{
public:
	INLINE FFloat() { }
	INLINE FFloat(const FFloat &s) :_m(s._m) { }
	INLINE void operator=(const FFloat &s) { _m=s._m; }
	INLINE FFloat(const float &f) :_m(_mm_shuffle(0,_mm_set_ss(f))) { }
	INLINE operator float() const { float out; _mm_store_ss(&out,_m); return out; }
	INLINE FFloat(const __m128 &v) :_m(_mm_shuffle(0,v)) { }
	INLINE operator __m128() const { return _m; }
	void Serialize(Serializer &sr);

	void operator+=(const FFloat&);
	void operator-=(const FFloat&);
	void operator*=(const FFloat&);
	void operator/=(const FFloat&);

	__m128 _m;
};

inline FFloat operator +(const FFloat &a,const FFloat &b) { FFloat t(a); t+=b; return t; }
inline FFloat operator -(const FFloat &a,const FFloat &b) { FFloat t(a); t-=b; return t; }
inline FFloat operator *(const FFloat &a,const FFloat &b) { FFloat t(a); t*=b; return t; }
inline FFloat operator /(const FFloat &a,const FFloat &b) { FFloat t(a); t/=b; return t; }
inline FFloat operator +(const FFloat &a,float b) { FFloat t(a); t+=FFloat(b); return t; }
inline FFloat operator -(const FFloat &a,float b) { FFloat t(a); t-=FFloat(b); return t; }
inline FFloat operator *(const FFloat &a,float b) { FFloat t(a); t*=FFloat(b); return t; }
inline FFloat operator /(const FFloat &a,float b) { FFloat t(a); t/=FFloat(b); return t; }
inline FFloat operator +(float a,const FFloat &b) { FFloat t(a); t+=b; return t; }
inline FFloat operator -(float a,const FFloat &b) { FFloat t(a); t-=b; return t; }
inline FFloat operator *(float a,const FFloat &b) { FFloat t(a); t*=b; return t; }
inline FFloat operator /(float a,const FFloat &b) { FFloat t(a); t*=b; return t; }

FFloat Sqrt(const FFloat&);
FFloat RSqrt(const FFloat&);
FFloat Inv(const FFloat&);
FFloat Round(const FFloat&);


/**
	2 component 3D vector stored as (x,y,?,?)
	Converted to FVec4 becomes (x,y,0,0)
*/
class FVec2
{
public:
	typedef FFloat TScalar;

	INLINE FVec2() { }
	INLINE FVec2(const FVec2 &v) { _m=v._m; }
	INLINE void operator=(const FVec2 &v) { _m=v._m; }
	INLINE FVec2(const __m128 &v) :_m(v) { }
	INLINE operator __m128() const { return _m; }
	INLINE explicit FVec2(float f) { _m=_mm_shuffle(0,_mm_set_ss(f)); }
	INLINE explicit FVec2(const FFloat &s) { _m=s._m; }
	INLINE FVec2(const FFloat &x,const FFloat &y)
		{ _m=_mm_shuffle_ps(_mm_unpacklo_ps(x._m,y._m),_mm_setzero_ps(),68); }
	INLINE FVec2(float X,float Y) { ALIGN16 Vec2 v(X,Y); _m=_mm_loadl_pi(_mm_setzero_ps(),(__m64*)&v.x); }
	INLINE FVec2(const Vec2 &v) { _m=_mm_loadl_pi(_mm_setzero_ps(),(__m64*)&v.x); }
	FVec2(const FVec4&);
	void Serialize(Serializer &sr);

	void operator+=(const FVec2&);
	void operator-=(const FVec2&);
	void operator*=(const FVec2&);
	void operator/=(const FVec2&);
	void operator^=(const FVec2&);
	void operator/=(const FFloat&);
	void operator*=(const FFloat&);
	FVec2 operator-() const;
	FFloat operator|(const FVec2&) const;
	void Zero();

	FFloat _GetX() const; FFloat _SetX(const FFloat&);
	FFloat _GetY() const; FFloat _SetY(const FFloat&);
	__declspec(property(get=_GetX,put=_SetX)) FFloat x;
	__declspec(property(get=_GetY,put=_SetY)) FFloat y;
	__m128 _m;
};

DECLARE_VEC_FUNCS(FVec2,FFloat,Matrix,AMatrix)
DECLARE_FLOAT_OPS(FVec2)


/**
	3 component 3D vector stored as (x,y,z,?)
	Converted to FVec4 becomes (x,y,z,0)
*/
class FVec3
{
public:
	typedef FFloat TScalar;

	INLINE FVec3() { }
	INLINE FVec3(const FVec3 &v) { _m=v._m; }
	INLINE void operator=(const FVec3 &v) { _m=v._m; }
	INLINE FVec3(const __m128 &v) :_m(v) { }
	INLINE operator __m128() const { return _m; }
	INLINE explicit FVec3(float f) { _m=_mm_shuffle(0,_mm_set_ss(f)); }
	INLINE explicit FVec3(const FFloat &s) { _m=s._m; }
	INLINE FVec3(const FFloat &X,const FFloat &Y,const FFloat &Z)
		{ _m=_mm_shuffle_ps(_mm_unpacklo_ps(X._m,Y._m),_mm_unpackhi_ps(Z._m,_mm_setzero_ps()),68); }
	INLINE FVec3(float X,float Y,float Z) { ALIGN16 Vec4 tv(X,Y,Z,0); _m=_mm_load_ps(&tv.x); }
	INLINE FVec3(const Vec3 &v){ _m=_mm_loadl_pi(_mm_shuffle(0,_mm_load_ss(&v.z)),(__m64*)&v.x); }
	FVec3(const FVec4&);
	void Serialize(Serializer &sr);

	void operator+=(const FVec3&);
	void operator-=(const FVec3&);
	void operator*=(const FVec3&);
	void operator/=(const FVec3&);
	void operator^=(const FVec3&);
	void operator/=(const FFloat&);
	void operator*=(const FFloat&);
	FVec3 operator-() const;
	FFloat operator|(const FVec3&) const;
	void Zero();

	FFloat _GetX() const; FFloat _SetX(const FFloat&);
	FFloat _GetY() const; FFloat _SetY(const FFloat&);
	FFloat _GetZ() const; FFloat _SetZ(const FFloat&);
	__declspec(property(get=_GetX,put=_SetX)) FFloat x;
	__declspec(property(get=_GetY,put=_SetY)) FFloat y;
	__declspec(property(get=_GetZ,put=_SetZ)) FFloat z;
	__m128 _m;
};

DECLARE_VEC_FUNCS(FVec3,FFloat,Matrix,AMatrix)
DECLARE_FLOAT_OPS(FVec3)

/**
	4D vector class stored as (x,y,z,w)
	its better to use this class instead of FVec3, FVec2 and especially FFloat
	because in the same time it can process much more (1.33x, 2x, 4x) data than other classes 
	you should also avoid these instructions: operator|, Length, Normalize, Sum
	you can avoid them by processing 4 vectors at once
*/
class FVec4
{
public:
	typedef FFloat TScalar;

	INLINE FVec4() { }
	INLINE FVec4(const FVec4 &v) :_m(v._m) { }
	INLINE void operator=(const FVec4 &v) { _m=v._m; }
	INLINE FVec4(const __m128 &v) :_m(v) { }
	INLINE operator __m128() const { return _m; }
	INLINE explicit FVec4(float f) { _m=_mm_shuffle(0,_mm_set_ss(f)); }
	INLINE explicit FVec4(const FFloat &s) { _m=s._m; }
	INLINE FVec4(const FFloat &X,const FFloat &Y,const FFloat &Z,const FFloat &W)
		{ _m=_mm_shuffle_ps(_mm_unpacklo_ps(X._m,Y._m),_mm_unpackhi_ps(Z._m,W._m),68); }
	INLINE FVec4(float X,float Y,float Z,float W) { ALIGN16 Vec4 tv(X,Y,Z,W); _m=_mm_load_ps(&tv.x); }
	INLINE FVec4(const Vec4 &v) { _m=_mm_loadu_ps(&v.x); }
	FVec4(const FVec2 &v);
	FVec4(const FVec3 &v);
	void Serialize(Serializer &sr);
	
	void operator+=(const FVec4&);
	void operator-=(const FVec4&);
	void operator*=(const FVec4&);
	void operator/=(const FVec4&);
	void operator^=(const FVec4&);
	void operator/=(const FFloat&);
	void operator*=(const FFloat&);
	FVec4 operator-() const;
	FFloat operator|(const FVec4&) const;
	void Zero();

	FFloat _GetX() const; FFloat _SetX(const FFloat&);
	FFloat _GetY() const; FFloat _SetY(const FFloat&);
	FFloat _GetZ() const; FFloat _SetZ(const FFloat&);
	FFloat _GetW() const; FFloat _SetW(const FFloat&);
	__declspec(property(get=_GetX,put=_SetX)) FFloat x;
	__declspec(property(get=_GetY,put=_SetY)) FFloat y;
	__declspec(property(get=_GetZ,put=_SetZ)) FFloat z;
	__declspec(property(get=_GetW,put=_SetW)) FFloat w;
	__m128 _m;
};

DECLARE_VEC_FUNCS(FVec4,FFloat,Matrix,AMatrix)
DECLARE_FLOAT_OPS(FVec4)

#endif

class AxisAngle: public FVec4
{
public:
	inline AxisAngle() { }
	AxisAngle(const FVec4&);
	operator FVec4() const;
	AxisAngle(const FVec3 &axis,const TScalar &angle);
};

class Quat: public FVec4
{
public:
	INLINE Quat() { }
	INLINE Quat(const Quat &q) :FVec4(q) { }
	INLINE void operator=(const Quat &q) { FVec4::operator=(FVec4(q)); }
	INLINE explicit Quat(const FVec4 &v) { FVec4::operator=(v); }

	Quat(const Matrix&);
	operator AMatrix() const;
	Quat(const AxisAngle&);
	operator AxisAngle() const;

	void operator*=(const Quat&);
	void operator+=(const Quat&);
	void operator-=(const Quat&);
	void operator*=(const FFloat&);
	void operator/=(const FFloat&);

	FFloat operator|(const Quat&) const;
	Quat operator-() const;
};

inline Quat operator +(const Quat &a,const Quat &b) { Quat t(a); t+=b; return t; }
inline Quat operator -(const Quat &a,const Quat &b) { Quat t(a); t-=b; return t; }
inline Quat operator *(const Quat &a,const Quat &b) { Quat t(a); t*=b; return t; }
inline Quat operator *(const Quat &a,const FFloat &f) { Quat t(a); t*=f; return t; }
inline Quat operator /(const Quat &a,const FFloat &f) { Quat t(a); t/=f; return t; }
#if SSE_ENABLED
DECLARE_FLOAT_OPS(Quat)
#endif

template<> FFloat Length<Quat>(const Quat &v);
template<> Quat Normalize(const Quat &v);
Quat Inv(const Quat &q);
Quat Lerp(const Quat &quat1,const Quat &quat2,float x);

class Matrix
{
public:
	typedef FFloat TScalar;

	INLINE Matrix() { }
	INLINE Matrix(const Matrix &m) :x(m.x),y(m.y),z(m.z),w(m.w) { }
	INLINE void operator=(const Matrix &m) { x=m.x; y=m.y; z=m.z; w=m.w; }
	INLINE Matrix(const FVec4 &X,const FVec4 &Y,const FVec4 &Z,const FVec4 &W) :x(X),y(Y),z(Z),w(W) { }

	void operator*=(const Matrix &m);
	void operator+=(const Matrix &m);
	void operator-=(const Matrix &m);
	void operator*=(const FFloat &f);

	INLINE operator float*() { return &a1; }
	INLINE operator const float*() const { return &a1; }

	union {
		struct { FVec4 x,y,z,w; };
		struct {
			float a1,a2,a3,a4;
			float b1,b2,b3,b4;
			float c1,c2,c3,c4;
			float d1,d2,d3,d4; };
	};
};

inline Matrix operator+(const Matrix &a,const Matrix &b) { Matrix t(a); t*=b; return t; }
inline Matrix operator*(const Matrix &a,const Matrix &b) { Matrix t(a); t*=b; return t; }
inline Matrix operator*(const Matrix &a,float b) { Matrix t(a); t*=b; return t; }

const FVec2 &operator*=(FVec2&,const Matrix&);
const FVec3 &operator*=(FVec3&,const Matrix&);
const FVec4 &operator*=(FVec4&,const Matrix&);
const FVec2 &operator&=(FVec2&,const Matrix&);
const FVec3 &operator&=(FVec3&,const Matrix&);
inline const FVec4 &operator&=(FVec4 &v,const Matrix &m) { v*=m; return v; }

inline FVec2 operator*(const FVec2 &v,const Matrix &m) { FVec2 out(v); out*=m; return out; }
inline FVec3 operator*(const FVec3 &v,const Matrix &m) { FVec3 out(v); out*=m; return out; }
inline FVec4 operator*(const FVec4 &v,const Matrix &m) { FVec4 out(v); out*=m; return out; }
inline FVec2 operator&(const FVec2 &v,const Matrix &m) { FVec2 out(v); out&=m; return out; }
inline FVec3 operator&(const FVec3 &v,const Matrix &m) { FVec3 out(v); out&=m; return out; }
inline FVec4 operator&(const FVec4 &v,const Matrix &m) { FVec4 out(v); out*=m; return out; }


class AMatrix: public Matrix
{
public:
	typedef FFloat TScalar;

	INLINE AMatrix() { }
	INLINE AMatrix(const AMatrix &m) :Matrix(m) { }
	INLINE AMatrix(const FVec4 &X,const FVec4 &Y,const FVec4 &Z,const FVec4 &W) :Matrix(X,Y,Z,W) { }

	void operator*=(const AMatrix &m);
};

inline AMatrix operator*(const AMatrix &a,const AMatrix &b) { AMatrix t(a); t*=b; return t; }

const FVec2 &operator*=(FVec2&,const AMatrix&);
const FVec3 &operator*=(FVec3&,const AMatrix&);
const FVec4 &operator*=(FVec4&,const AMatrix&);

inline FVec2 operator*(const FVec2 &v,const AMatrix &m) { FVec2 out(v); out*=m; return out; }
inline FVec3 operator*(const FVec3 &v,const AMatrix &m) { FVec3 out(v); out*=m; return out; }
inline FVec4 operator*(const FVec4 &v,const AMatrix &m) { FVec4 out(v); out*=m; return out; }

class Translate
{
public:
	Translate(const FVec3 &trans);
	Translate(float x,float y,float z);
	Translate operator*(const Translate &m) const;
	Matrix operator*(const Matrix &m) const;
	AMatrix operator*(const AMatrix &m) const;
	operator AMatrix() const;

protected:
	inline Translate() { }
	FVec4 val;

	friend Matrix operator*(const Matrix&,const Translate&);
	friend AMatrix operator*(const AMatrix&,const Translate&);
	friend AMatrix operator*(const Translate &t,const Scale &s);
	friend AMatrix operator*(const Scale &t,const Translate &s);
};

Matrix operator*(const Matrix&,const Translate&);
AMatrix operator*(const AMatrix&,const Translate&);

class Scale
{
public:
	Scale(const FVec3 &scale);
	Scale(float x,float y,float z);
	Scale operator*(const Scale &m) const;
	Matrix operator*(const Matrix &m) const;
	AMatrix operator*(const AMatrix &m) const;
	operator AMatrix() const;
protected:
	inline Scale() { }
	FVec4 val;

	friend Matrix operator*(const Matrix&,const Scale&);
	friend AMatrix operator*(const AMatrix&,const Scale&);
	friend AMatrix operator*(const Translate &t,const Scale &s);
	friend AMatrix operator*(const Scale &t,const Translate &s);
};

Matrix operator*(const Matrix&,const Scale&);
AMatrix operator*(const AMatrix&,const Scale&);

AMatrix operator*(const Translate&,const Scale&);
AMatrix operator*(const Scale&,const Translate&);

FVec3 RotateVecX(const FVec3 &point,float angle);
FVec3 RotateVecY(const FVec3 &point,float angle);
FVec3 RotateVecZ(const FVec3 &point,float angle);
FVec2 RotateVec(const FVec2 &point,float angle);

///	Zwraca normaln¹ trójk¹ta
inline Vec TriangleNormal(const Vec a,const Vec b,const Vec c) { return Normalize((c-a)^(b-a)); }

AMatrix Rotate(float yaw,float pitch,float roll);
AMatrix RotateX(float angle);
AMatrix RotateY(float angle);
AMatrix RotateZ(float angle);

Matrix Inv(const Matrix&);
Matrix Transpose(const Matrix&);

AMatrix Identity();

FVec4 VecMask(u32 x,u32 z,u32 y,u32 w);

extern const FVec4 XMHalf,XMOne,XMTwo,XMThree,XMFour;
extern const FVec4 XMPi,XMPi2,XMInvPi,XMInvPi2;
extern const FVec4 XMSqrt2,XMSqrt3;
extern const FVec4 XMMaskMinus,XMMaskNotMinus;
extern const FVec4 XMMaskX,XMMaskY,XMMaskZ,XMMaskW;
extern const AMatrix XMIdentity;

FVec4 Sin(const FVec4&);
FVec4 Cos(const FVec4&);
FVec4 SinD(const FVec4&);
FVec4 CosD(const FVec4&);
FVec4 SinCos(const FVec4&);
FVec4 SinCosD(const FVec4&);

/**
* Standard plane class
*/
class Plane: public FVec4
{
public:
	INLINE Plane() { }
	INLINE Plane(const FVec &dir,const FVec &point) { Set(dir,point); }
	INLINE Plane(const FVec &v1,const FVec &v2,const FVec &v3) { Set(v1,v2,v3); }
	INLINE explicit Plane(const FVec4 &v) :FVec4(v) { }

	void Set(const FVec &dir,const FVec &point);
	void Set(const FVec &v1,const FVec &v2,const FVec &v3);

	FVec3 _GetN() const; FVec3 _SetN(const FVec3&);
	__declspec(property(get=_GetN,put=_SetN)) FVec3 normal;
	__declspec(property(get=_GetW,put=_SetW)) FFloat dist;
};

Plane Normalize(const Plane&);

/// 0	point lies on the plane
///	>0	point is at the front side of then plane
///	<0	point is at the back side of the plane
FFloat operator*(const Plane&,const FVec&);
inline FFloat operator*(const FVec &v,const Plane &p) { return p*v; }


/**
* Standard ray class
*/
class Ray
{
public:
	typedef float TScalar;

	INLINE Ray() { }
	INLINE Ray(const FVec3 &Origin,const FVec3 &Dir) : origin(Origin), dir(Dir) { }
	INLINE Ray(const FVec2 &pos,const Matrix &proj,const Matrix &invView) { Set(pos,proj,invView); }

	///	Calculates ray from:
	///	pos: (0,0) top left screen corner   (1,1) bottom right screen corner
	/// proj: projection matrix
	/// invView: inverse of view matrix
	void Set(const FVec2 &pos,const Matrix &proj,const Matrix &invView);

	FVec3 origin,dir;
};

bool Intersect(const Plane&,const Ray&,FVec3 *intersection);




// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//W œcie¿ce key[0] musi mieæ pozycjê 0
//wartoœæ step podawana w funkcji getValue musi byæ nieujemna
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

enum XINTERPOLATION_TYPE
{
	XI_LINEAR				= 1,
	XI_COSINE				= 2,
	XI_QUADRATIC			= 3,
};

#define XP_LOOPED_ANIMATION	8
#define XP_REGULAR_KEYS		16

/**
 * This object represents path of interpolated objects.
 * Specified objects must have implemented Lerp methods
 */
template <class T>
class XPATH
{
public:
	XPATH() :nKeys(0) { }
	XPATH(UINT maxKeys,XINTERPOLATION_TYPE iType,UINT flags=0) :nKeys(0)
		{ Create(maxKeys,iType,flags); }

	void Create(UINT maxKeys,XINTERPOLATION_TYPE iType,UINT flags=0);
	void Set(UINT idx,const T &Key,float pos);
	void Add(const T &key,float pos);
	void Serialize(Serializer &sr);
	void Free();

	T Get(float pos) const;
	UINT GetStartKey(float pos) const;

	T &operator[](UINT num) { return keysTab[num]; }
	const T &operator[](UINT num) const { return keysTab[num]; }

	XINTERPOLATION_TYPE iType;
	UINT flags,nKeys;
	Tab<T> keysTab;
	Tab<float> posTab;
};

#include <math/math_path.inl>



typedef XPATH<float>	XFLOAT_PATH;
typedef XPATH<Vec>		XVEC_PATH;
typedef XPATH<Quat>		XQUAT_PATH;



/**
 * Funkcje do kolizji
 */


///	Kolizja p³aszczyzny i promienia
bool XCollide(const Plane &p,const Ray &r,float *dist=0,Vec *intersection=0);
///	Kolizja p³aszczyzny i promienia
inline bool XCollide(const Ray &r,const Plane &p,float *dist=0,Vec *intersection=0)
{ return XCollide(p,r,dist,intersection); }

///	Sprawdza czy zasz³a kolizja pomiêdzy trójk¹tem, a promieniem
///	v1, v2, v3 - wierzcho³ki trójk¹ta
///	intersetion - punkt w jakim zasz³a kolizja
///	u, v - koordynaty trójk¹ta (barycentryczne)
bool XCollide(const Ray &ray,const FVec &v1,const FVec &v2,const FVec &v3,
			  float *dist=0,FVec *intersection=0,FVec2 *uv=0);

///	Odleg³oœæ od promienia do punktu
float XDistance(const Ray &ray,Vec point);

/// Najbli¿szy punkt na odcinku
Vec XClosestPointOnRay(Vec point,Vec rayStart,Vec rayEnd);

/// vertex - dowolny punkt nale¿¹cy do p³aszczyzny
/// normal - wektor normalny p³aszczyzny
/// lineStart, lineEnd - dwa punkty (pocz¹tkowy i koñcowy) odcinka
bool XRayPlaneCollision(Vec vertex,Vec normal,Vec rayStart,Vec rayEnd);

/// aParamSize oznacza wielkoœæ (w bajtach) podawanego parametru, np. :
/// Jeœli vertex to zmienna typu VECTOR to nale¿y podaæ 12 ( sizeof(VECTOR) )
/// Jeœli vertex to zmienna typu VERTEX to nale¿y podaæ 20 ( sizeof(VERTEX) )
bool XPointInsidePolygon(Vec point,Vec *vertex,int verticesNumber,int aParamSize);

///	Kolizja kuli z poligonem
bool XSpherePolygonCollision(Vec spherePosition,float sphereRadius,Vec *vertex,int nVerts,
							 int aParamSize,Vec polygonNormal);

///	Kolizja kuli z kul¹
bool XSphereSphereCollision(Vec sphere1Position,float sphere1Radius,
							Vec sphere2Position,float sphere2Radius);



/**
 * Funkcje losuj¹ce
 */

/// Noise function
float XNoise(int seed);

/// Softened noise function
float XSmoothNoise(int seed);

/// 1-dimensional perlin noise
float XPerlin(float x,int octaves,float persistence);

/// 2-dimensional noise function
float XNoise2D(int x,int y);

/// Softened, 2-dimensional noise function
float XSmoothNoise2D(int x,int y);

/// Softened, interpolated, 2-dimensional noise function
float XInterpolatedNoise2D(int x,int y);

/// Perlin noise 2D
float XPerlin2D(float x,float y,int octaves,float persistence);



#endif
