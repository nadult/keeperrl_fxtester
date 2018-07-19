//----------------------------------------------------------------------\
//X LIBRARY																|
//xgraphics_low.h														|
//Uses Direct3D9														|
//																		|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2006						|
//----------------------------------------------------------------------/



#ifndef XGRAPHICS_LOW_H
#define XGRAPHICS_LOW_H


#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")



#include <d3d9.h>
#include <d3dx9.h>
#include <xsystem.h>
#include <xmath.h>
#include <xscript.h>


#pragma region ENUMS

enum XVELEMENT_USAGE {
	XVEU_TEXCOORD=0x00,
	XVEU_POS=0x10,
	XVEU_NORMAL=0x20,
	XVEU_BINORMAL=0x30,
	XVEU_TANGENT=0x40,
	XVEU_DIFFUSE=0x50,
	XVEU_SPECULAR=0x60,
	XVEU_POST=0x70,
	XVEU_UNKNOWN=0x80,
};

enum XVELEMENT_TYPE {
	XVET_FLOAT=0x0,  XVET_FLOAT1=XVET_FLOAT, XVET_VEC1=XVET_FLOAT,
	XVET_COLOR=0x1,
	XVET_FLOAT2=0x2, XVET_VEC2=XVET_FLOAT2,
	XVET_FLOAT3=0x3, XVET_VEC3=XVET_FLOAT3, XVET_VEC=XVET_FLOAT3,
	XVET_FLOAT4=0x4, XVET_VEC4=XVET_FLOAT4,
	XVET_UNKNOWN=0x05,
};

enum XRENDER_STATE {
	XRS_FILL_MODE			= D3DRS_FILLMODE,
	XRS_CULL_MODE			= D3DRS_CULLMODE,
	XRS_BLENDING			= D3DRS_ALPHABLENDENABLE,
	XRS_SRCBLEND			= D3DRS_SRCBLEND,
	XRS_DSTBLEND			= D3DRS_DESTBLEND,
	XRS_Z_WRITE				= D3DRS_ZWRITEENABLE,
	XRS_ALPHA_TEST			= D3DRS_ALPHATESTENABLE,

	XRS_Z_TEST				= D3DRS_ZENABLE,
	XRS_Z_FUNC				= D3DRS_ZFUNC,
	XRS_ALPHA_REF			= D3DRS_ALPHAREF,
	XRS_ALPHA_FUNC			= D3DRS_ALPHAFUNC,
	XRS_SHADE_MODE			= D3DRS_SHADEMODE,
	XRS_SCISSOR_TEST		= D3DRS_SCISSORTESTENABLE,
	XRS_LINE_ANTIALIAS		= D3DRS_ANTIALIASEDLINEENABLE,

	XRS_FOG					= D3DRS_FOGENABLE,
	XRS_FOG_TYPE			= 0xff000001,
	XRS_FOG_COLOR			= D3DRS_FOGCOLOR,
	XRS_FOG_START			= D3DRS_FOGSTART,
	XRS_FOG_END				= D3DRS_FOGEND,
	XRS_FOG_DENSITY			= D3DRS_FOGDENSITY,

	// Fixed vertex processing states:
	XRS_AMBIENT				= D3DRS_AMBIENT,
	XRS_SPECULAR			= D3DRS_SPECULARENABLE,
	XRS_LIGHTING			= D3DRS_LIGHTING,
};

enum XFILL_MODE {
	XFM_SOLID				= D3DFILL_SOLID,
	XFM_LINES				= D3DFILL_WIREFRAME,
	XFM_POINTS				= D3DFILL_POINT,
};

enum XSHADE_MODE {
	XSM_FLAT				= D3DSHADE_FLAT,
	XSM_GOURAUD				= D3DSHADE_GOURAUD,
};

enum XCULL_MODE {
	XCM_NONE				= D3DCULL_NONE,
	XCM_CW					= D3DCULL_CW,
	XCM_CCW					= D3DCULL_CCW,
};

enum XBLEND_MODE {
	XBM_ZERO				= D3DBLEND_ZERO,
	XBM_ONE					= D3DBLEND_ONE,
	XBM_SRCCOLOR			= D3DBLEND_SRCCOLOR,
	XBM_INVSRCCOLOR			= D3DBLEND_INVSRCCOLOR,
	XBM_SRCALPHA			= D3DBLEND_SRCALPHA,
	XBM_INVSRCALPHA			= D3DBLEND_INVSRCALPHA,
	XBM_DSTALPHA			= D3DBLEND_DESTALPHA,
	XBM_INVDSTALPHA			= D3DBLEND_INVDESTALPHA,
	XBM_DSTCOLOR			= D3DBLEND_DESTCOLOR,
	XBM_INVDSTCOLOR			= D3DBLEND_INVDESTCOLOR,
	XBM_SRCALPHASAT			= D3DBLEND_SRCALPHASAT,
	XBM_BOTHINVSRCALPHA		= D3DBLEND_BOTHINVSRCALPHA,
	XBM_BLENDFACTOR			= D3DBLEND_BLENDFACTOR,
	XBM_INVBLENDFACTOR		= D3DBLEND_INVBLENDFACTOR,
};

enum XCUBE_FACE {
	XCF_POSITIVEX			= D3DCUBEMAP_FACE_POSITIVE_X,
	XCF_NEGATIVEX			= D3DCUBEMAP_FACE_NEGATIVE_X,
	XCF_POSITIVEY			= D3DCUBEMAP_FACE_POSITIVE_Y,
	XCF_NEGATIVEY			= D3DCUBEMAP_FACE_NEGATIVE_Y,
	XCF_POSITIVEZ			= D3DCUBEMAP_FACE_POSITIVE_Z,
	XCF_NEGATIVEZ			= D3DCUBEMAP_FACE_NEGATIVE_Z,
};

XFlags(XLOCK) {
	XL_DEFAULT				=0,
	XL_DISCARD				=D3DLOCK_DISCARD,
	XL_READONLY				=D3DLOCK_READONLY,
	XL_NOOVERWRITE			=D3DLOCK_NOOVERWRITE,
};

enum XPOOL {
	XP_DEFAULT				=D3DPOOL_DEFAULT,
	XP_MANAGED				=D3DPOOL_MANAGED,
	XP_SYSMEM				=D3DPOOL_SYSTEMMEM,
};

XFlags(XUSAGE) {
	XU_DEFAULT				=0,
	XU_DYNAMIC				=D3DUSAGE_DYNAMIC,
	XU_WRITEONLY			=D3DUSAGE_WRITEONLY,
	XU_AUTOGENMIPMAP		=D3DUSAGE_AUTOGENMIPMAP,
	XU_SOFTWAREPROCESSING	=D3DUSAGE_SOFTWAREPROCESSING,
};


enum SURFACE_TYPE {
	//	Surfaces with these types can be created by calling XSURFACE::Create
	XST_DEPTH_BUFFER		= 1,	//	Depth, stencil buffer
	XST_RENDER_TARGET		= 2,	//	Render target
	XST_LRENDER_TARGET		= 3,	//	Lockable render target
	XST_SYSMEM_BUFFER		= 4,	//	System memory color buffer

	//	You cant create surfaces with these types directly, only by calling XTEXTURE::getSurface or
	//	XGetSurface( ... )

	XST_DEFAULT_TEXTURE		= 5,	//	The surface is a part of texture
	XST_MANAGED_TEXTURE		= 6,	//	The surface is a part of managed texture

	XST_UNKNOWN				= 255
};

enum SURFACE_FORMAT {
	FMT_UNKNOWN			=0,

	FMT_R3G3B2			=1,
	FMT_A8				=2,
	FMT_L8				=3,
	FMT_A4L4			=4,

	FMT_R5G6B5			=5,
	FMT_X1R5G5B5		=6,
	FMT_A1R5G5B5		=7,
	FMT_A4R4G4B4		=8,
	FMT_A8R3G3B2		=9,
	FMT_X4R4G4B4		=10,
	FMT_L16				=11,	
	FMT_A8L8			=12,

	FMT_R8G8B8			=13,

	FMT_A8R8G8B8		=14,
	FMT_X8R8G8B8		=15,
	FMT_A2B10G10R10		=16,
	FMT_A8B8G8R8		=17,
	FMT_X8B8G8R8		=18,
	FMT_G16R16			=19,	
	FMT_A2R10G10B10		=20,

	FMT_L6V5U5			=21,	
	FMT_X8L8V8U8		=22,
	FMT_A2W10V10U10		=23,

	FMT_DXT1			=24,
	FMT_DXT2			=25,
	FMT_DXT3			=26,
	FMT_DXT4			=27,
	FMT_DXT5			=28,

	FMT_UYVY			=29,
	FMT_YUY2			=30,

	FMT_R16F			=31,
	FMT_R32F			=32,
	FMT_G16R16F			=33,	
	FMT_G32R32F			=34,
	FMT_A16B16G16R16F	=35,
	FMT_A32B32G32R32F	=36,

	FMT_V8U8			=37,
	FMT_Q8W8V8U8		=38,
	FMT_V16U16			=39,
	FMT_Q16W16V16U16	=40,

	FMT_D24S8			=41,
	FMT_D24X8			=42,
	FMT_D16				=43,

	FMT_LAST			=43,
};

D3DFORMAT SurfaceFormat2D3DFormat(SURFACE_FORMAT fmt);
SURFACE_FORMAT D3DFormat2SurfaceFormat(D3DFORMAT fmt);

enum XTEXTURE_FILTER {
	XTF_NONE				= D3DTEXF_NONE,
	XTF_POINT				= D3DTEXF_POINT,
	XTF_LINEAR				= D3DTEXF_LINEAR,
	XTF_GAUSSIAN			= D3DTEXF_GAUSSIANQUAD,
	XTF_PYRAMIDAL			= D3DTEXF_PYRAMIDALQUAD,
	XTF_ANISOTROPIC			= D3DTEXF_ANISOTROPIC,
};

enum XTEXTURE_ADDRESS {
	XTA_WRAP				= D3DTADDRESS_WRAP,
	XTA_MIRROR				= D3DTADDRESS_MIRROR,
	XTA_CLAMP				= D3DTADDRESS_CLAMP,
	XTA_BORDER				= D3DTADDRESS_BORDER,
	XTA_MIRROR_ONCE			= D3DTADDRESS_MIRRORONCE,
};

enum XSAMPLER_STATE {
	XSS_MIP_FILTER			= D3DSAMP_MIPFILTER,
	XSS_MAG_FILTER			= D3DSAMP_MAGFILTER,
	XSS_MIN_FILTER			= D3DSAMP_MINFILTER,

	//	Inne
	XSS_ADDRESSU			= D3DSAMP_ADDRESSU,
	XSS_ADDRESSV			= D3DSAMP_ADDRESSV,
	XSS_ADDRESSUV			= 0xff000001,	//	Ustawia naraz U i V, odczytuje U
	XSS_ADDRESSW			= D3DSAMP_ADDRESSW,
	XSS_BORDER_COLOR		= D3DSAMP_BORDERCOLOR,
	XSS_MAXANISOTROPY		= D3DSAMP_MAXANISOTROPY,
	XSS_MAXMIPLEVEL			= D3DSAMP_MAXMIPLEVEL,
	XSS_SRGBTEXTURE			= D3DSAMP_SRGBTEXTURE,
};

enum XPRIMITIVE_TYPE {
	XPT_TRIANGLE_LIST		= D3DPT_TRIANGLELIST,
	XPT_TRIANGLE_STRIP		= D3DPT_TRIANGLESTRIP,
	XPT_TRIANGLE_FAN		= D3DPT_TRIANGLEFAN,
	XPT_LINE_LIST			= D3DPT_LINELIST,
	XPT_LINE_STRIP			= D3DPT_LINESTRIP,
	XPT_POINT_LIST			= D3DPT_POINTLIST,
};

enum XFOG_TYPE {
	XFT_NONE,
	XFT_LINEAR_PIXEL,
	XFT_EXP_PIXEL,
	XFT_EXP2_PIXEL,
	XFT_LINEAR_VERTEX,
	XFT_EXP_VERTEX,
	XFT_EXP2_VERTEX,
};

enum {
	XCID_SURFACE				=0x01000000,
	XCID_BASE_TEXTURE			=0x02000000,
		XCID_TEXTURE			=0x02010000,
		XCID_CUBE_TEXTURE		=0x02020000,
		XCID_VOLUME_TEXTURE		=0x02030000,
	XCID_VBUFFER				=0x03000000,
	XCID_IBUFFER				=0x04000000,
	XCID_VSHADER				=0x05000000,
	XCID_PSHADER				=0x06000000,
	XCID_FPSHADER				=0x07000000,
	XCID_LIGHT					=0x08000000,
	XCID_GEOMETRIC_OBJECT		=0x09000000,
};

XDEFINE_LINK(XSURFACE)
XDEFINE_LINK(XTEXTURE)
XDEFINE_LINK(XVBUFFER)
XDEFINE_LINK(XIBUFFER)
XDEFINE_LINK(XVSHADER)
XDEFINE_LINK(XPSHADER)
XDEFINE_LINK(XLIGHT)
XDEFINE_LINK(XGEOMETRIC_OBJECT)


#pragma endregion
#pragma region D3D_INTERFACES


class XID3D_IBUFFER: public XIUNKNOWN<LPDIRECT3DINDEXBUFFER9>
	{ public: HRESULT Create(UINT length,DWORD usage,D3DFORMAT format,D3DPOOL pool); };

class XID3D_VBUFFER: public XIUNKNOWN<LPDIRECT3DVERTEXBUFFER9>
	{ public: HRESULT Create(UINT length,DWORD usage,DWORD fvf,D3DPOOL pool); };

class XID3D_VDECL: public XIUNKNOWN<LPDIRECT3DVERTEXDECLARATION9>
	{ public: HRESULT Create(const D3DVERTEXELEMENT9 *elements); };

class XID3D_PSHADER: public XIUNKNOWN<LPDIRECT3DPIXELSHADER9>
	{ public: HRESULT Create(const DWORD *function); };

class XID3D_VSHADER: public XIUNKNOWN<LPDIRECT3DVERTEXSHADER9>
	{ public: HRESULT Create(const DWORD *function); };

class XID3D_SURFACE: public XIUNKNOWN<LPDIRECT3DSURFACE9> { public:
HRESULT Create(const DWORD *function);
HRESULT CreateDepthStencil(UINT width,UINT height,D3DFORMAT format,D3DMULTISAMPLE_TYPE multiSample,
						   DWORD multisampleQuality,BOOL discard);
HRESULT CreateOffscreenPlain(UINT width,UINT height,D3DFORMAT format,D3DPOOL pool);
HRESULT CreateRenderTarget(UINT width,UINT height,D3DFORMAT format,D3DMULTISAMPLE_TYPE multiSample,
						   DWORD multisampleQuality,BOOL lockable);
};

class XID3D_BASE_TEXTURE: public XIUNKNOWN<LPDIRECT3DBASETEXTURE9> { };

class XID3D_TEXTURE: public XIUNKNOWN<LPDIRECT3DTEXTURE9> { public:
HRESULT Create(UINT width,UINT height,UINT levels,DWORD usage,D3DFORMAT format,D3DPOOL pool);
HRESULT Create(LPCVOID pSrcData,UINT srcDataSize,UINT width,UINT height,UINT mipLevels,DWORD usage,
  D3DFORMAT format,D3DPOOL pool,DWORD filter,DWORD mipFilter,D3DCOLOR colorKey,D3DXIMAGE_INFO *pSrcInfo,
  PALETTEENTRY *pPalette);
};

class XID3D_CUBE_TEXTURE: public XIUNKNOWN<LPDIRECT3DCUBETEXTURE9> { public:
HRESULT Create(UINT edgeLength,UINT levels,DWORD usage,D3DFORMAT format,D3DPOOL pool);
};

class XID3D_VOLUME_TEXTURE: public XIUNKNOWN<LPDIRECT3DVOLUMETEXTURE9> { public:
HRESULT Create(UINT w,UINT h,UINT depth,UINT levels,DWORD usage,D3DFORMAT format,D3DPOOL pool);
};


#pragma endregion

/**
	Enclosing box aligned to axis.
*/
class XBOUNDARY
{
public:
	///	Extends to boundary enclosing given boundary
	void operator+=(const XBOUNDARY &b);
	///	Returns boundary transformed by given matrix
	void operator*=(const AMatrix &mat);

	///	Returns box enclosing sphere enclosing the box
	XBOUNDARY Sphere() const;
	///	Extending to box enclosing sphere enclosing the box
	void Spherize();

	///	Returns true if given ray colliedes the box
	/// sDist: Distance from ray origin to collision point
	/// cPoint: collision point
	bool Collision(const Ray &ray,const FVec &pivot=FVec(0,0,0),float *sDist=0,FVec *cPoint=0) const;

	FVec p1,p2;
};
XTAddPod(XBOUNDARY)

inline XBOUNDARY operator+(const XBOUNDARY &b1,const XBOUNDARY &b2) { XBOUNDARY t(b1); t+=b2; return t; }
inline XBOUNDARY operator*(const XBOUNDARY &b1,const AMatrix &m) { XBOUNDARY t(b1); t*=m; return t; }

//	Zamiast generowaæ obiekt otaczaj¹cy dla siatki wygenerowanej z interpolacji dwóch innych siatek
//	lepiej jest odpowiednio zinterpolowaæ poszczególne bboxy. Tak zinterpolowany szeœcian mo¿e byæ
//	troche wiêkszy, ale generuje sie znacznie szybciej; Ta metoda jest odporna na interpolowane translacje,
//	obroty, skalowanie i standardow¹ (liniow¹ lub kwadratow¹) interpolacjê wierzcho³ków siatek

template<> inline XBOUNDARY Lerp(const XBOUNDARY &b1,const XBOUNDARY &b2,float x)
	{ return b1.Sphere()+b2.Sphere(); }
template<> inline XBOUNDARY CLerp(const XBOUNDARY &b1,const XBOUNDARY &b2,float x)
	{ return b1.Sphere()+b2.Sphere(); }



class XFRUSTUM
{
public:
	///	Calculates frustum from given view and projection matrices
	void Calc(const Matrix &view,const Matrix &proj);

	///	Calculates frustum from active view and projection matrices
	void Calc();

	/// Returns true if the point is visible
	bool PointIn(const FVec &point) const;

	/// Returns true if the sphere is visible
	bool SphereIn(const FVec &position,float radius) const;

	/// Returns true if the box is visible
	bool BoxIn(const FVec &p1,const FVec &p2) const;

	//	Sprawdza czy obiekt XBOUNDARY jest w polu widzenia
	//	Koordynaty punktu przed testem s¹ dodatkowo mno¿one przez aktualn¹ macierz œwiata
	//	subPivot jest odejmowany od koordynatów szeœcianu
	inline bool BoundaryIn(const XBOUNDARY &boundary,const FVec &subPivot=FVec(0,0,0)) const
	{ return BoxIn(boundary.p1-subPivot,boundary.p2-subPivot); }

	//	Sprawdza, czy dany punkt jest w polu widzenia, jeœli tak, to zwraca 1
	//	Koordynaty punktu przed testem s¹ dodatkowo mno¿one przez aktualn¹ macierz œwiata
	bool PointInNT(const FVec &point) const;

	//	Sprawdza, czy kula jest w polu widzenia, jeœli tak, to zwraca 1
	//	Koordynaty punktu przed testem s¹ dodatkowo mno¿one przez aktualn¹ macierz œwiata
	bool SphereInNT(const FVec &position,float radius) const;

	//	Sprawdza, czy dany szeœcian jest w polu widzenia, jeœli tak, to zwraca 1
	//	Koordynaty punktu przed testem s¹ dodatkowo mno¿one przez aktualn¹ macierz œwiata
	bool BoxInNT(const FVec &p1,const FVec &p2) const;

	//	Sprawdza czy obiekt XBOUNDARY jest w polu widzenia
	//	Koordynaty punktu przed testem s¹ dodatkowo mno¿one przez aktualn¹ macierz œwiata
	//	subPivot jest odejmowany od koordynatów szeœcianu
	inline bool BoundaryInNT(const XBOUNDARY &boundary,const FVec &subPivot=FVec(0,0,0)) const
	{ return BoxInNT(boundary.p1-subPivot,boundary.p2-subPivot); }

	Plane plane[6];
	FVec eyePos,eyeDir;
};
XTAddPod(XFRUSTUM)

class XVELEMENT
{
public:
	XVELEMENT() { }
	XVELEMENT(XVELEMENT_USAGE u,XVELEMENT_TYPE t) :flags(u|t) { }

	void SetEnd() { flags=0xff; }
	inline bool End() const { return flags==0xff; }
	void SetType(XVELEMENT_TYPE type) { flags=Usage()|type; }
	void SetUsage(XVELEMENT_USAGE usage) { flags=Type()|usage; }
	inline XVELEMENT_USAGE Usage() const { return (XVELEMENT_USAGE)(flags&0xf0); }
	inline XVELEMENT_TYPE Type() const { return (XVELEMENT_TYPE)(flags&0x0f); }
	/// 4,8,12 or 16 bytes; depends on type
	inline u32 Size() const { return u32(Max(Type(),XVET_COLOR))<<2; }

	u8 flags;
};
XTAddPod(XVELEMENT)



class XVFORMAT
{
public:
	enum { MaxElements=16 };

	bool operator==(const XVFORMAT &);

	/// Computes correct vertex size
	u32 VertexSize() const;
	u32 NumElements() const;
	inline operator XVELEMENT *() { return tab; }
	inline operator const XVELEMENT *() const { return tab; }
	u32 Hash() const;

	/// Returns type of n-th element of given type
	XVELEMENT_TYPE ElementType(XVELEMENT_USAGE elemUsage,u32 n=0);
	/// Returns offset of n-th element of given type
	u32 ElementOffset(XVELEMENT_USAGE elemUsage,u32 n=0);

	XVELEMENT tab[MaxElements];
};
XTAddPod(XVFORMAT)



class XVDECL
{
public:
	bool Create(const Tab<XVFORMAT>&);
	bool Create(const XVFORMAT&);
	bool Create(const XVFORMAT&,const XVFORMAT&);
	bool Create(const XVFORMAT&,const XVFORMAT&,const XVFORMAT&);
	bool Create(const XVFORMAT&,const XVFORMAT&,const XVFORMAT&,const XVFORMAT&);
	void Serialize(Serializer &sr);
	void Use();
	void Free();

private:
	bool _Create();
	Tab<XVFORMAT> formats;
	XID3D_VDECL buffer;
};



class XVSHADER: public XDC<XOBJECT,XCID_VSHADER>
{
public:
	XVSHADER(Str code);
	XVSHADER() { }

	void Free();
	void Serialize(Serializer &sr);
	virtual bool Compile();

	DStr code;
	Tab<DWORD> cCode;
	XID3D_VSHADER buffer;
};



class XPSHADER: public XDC<XOBJECT,XCID_PSHADER>
{
public:
	XPSHADER(Str code);
	XPSHADER() { }

	void Free();
	void Serialize(Serializer &sr);

	virtual bool Compile();
	virtual void Use();

	DStr code;
	Tab<DWORD> cCode;
	XID3D_PSHADER buffer;
};



/**
	Surface class. It can be: texture (2d or 1d) or zbuffer
*/
class XSURFACE: public XDC<XOBJECT,XCID_SURFACE>
{
public:
	void Free();
	void Serialize(Serializer &sr);

	//	Tworzy now¹ powierzchniê
	virtual bool Create(UINT width,UINT height,SURFACE_TYPE type,SURFACE_FORMAT format);

	//	Bezpoœredni dostep do danych
	//	OUT: p, pitch   IN: rect
	virtual bool Lock(void **p,int *pitch,IRect *rect=0,DWORD flags=0);

	//	Zamyka dostêp
	virtual void Unlock();

	//	Wype³nia powierzchniê danym kolorem
	virtual bool Fill(XCOLOR color,IRect *rect=0);

	//	Kopiuje powierzchniê
	//	rect/point jest u¿ywane tylko, gdy powierzchnia Ÿród³owa jest w pamiêci systemowej
	virtual bool Blit(XSURFACE *dest,IRect *srcRect=0,IVec *dstPoint=0);


	//read-only:
	UINT width,height;
	SURFACE_TYPE type;
	SURFACE_FORMAT format;
	XID3D_SURFACE buffer;
};
typedef XLINK<XSURFACE> XSURFACE_LINK;



/**
	Vertex Buffer
*/
class XVBUFFER: public XDC<XOBJECT,XCID_VBUFFER>
{
public:
	XVBUFFER();
	XVBUFFER(const XVBUFFER &tBuf);
	~XVBUFFER();
	const XVBUFFER &operator=(const XVBUFFER &tBuf);

	void Free();
	void Serialize(Serializer &sr);
	bool Create(u32 size,XUSAGE usage,XPOOL pool);

	void Use(u32 nStream,u32 offset,u32 stride);
	float *Lock(u32 offset,u32 size,XLOCK flags);
	void Unlock();

	u32 Size() const;
	XPOOL GetPool() const;
	XUSAGE GetUsage() const;

protected:
	XID3D_VBUFFER buffer;
};



/**
	Index buffer.\n
	Supports only 16-bit indices.
*/
class XIBUFFER: public XDC<XOBJECT,XCID_IBUFFER>
{
public:
	XIBUFFER();
	XIBUFFER(const XIBUFFER &tBuf);
	~XIBUFFER();
	void operator=(const XIBUFFER &tBuf);

	void Free();
	void Serialize(Serializer &sr);

	bool Create(u32 nIndices,XUSAGE usage,XPOOL pool);
	u16 *Lock(u32 startIndex,u32 indices,XLOCK flags);
	void Unlock();
	void Use();

	u32 Num() const;
	XPOOL GetPool() const;
	XUSAGE GetUsage() const;

protected:
	XID3D_IBUFFER buffer;
};



class XTEXTURE: public XDC<XOBJECT,XCID_TEXTURE>
{
public:
	XID3D_BASE_TEXTURE GetInterface();
	void Free();
	void Serialize(Serializer &sr);

	void *Lock(u32 *pitch,IRect *rect,u32 level,XLOCK flags);
	void *Lock(u32 *pitch,XCUBE_FACE face,u32 level,XLOCK flags);
	void Unlock(u32 level=0);
	void Unlock(u32 level,XCUBE_FACE face);
	void Use(u32 nSampler);

	bool Create(u32 width,u32 height,SURFACE_FORMAT format,XUSAGE usage,XPOOL pool);
	bool CreateCube(u32 edge,SURFACE_FORMAT format,XUSAGE usage,XPOOL pool);
	u32 Width(u32 mip=0) const;
	u32 Height(u32 mip=0) const;
	u32 Depth(u32 mip=0) const;
	SURFACE_FORMAT Format() const;
	u32 MipsNum() const;
	XPOOL GetPool() const;
	XUSAGE GetUsage() const;

	XID3D_BASE_TEXTURE buffer;

	enum TYPE {
		T_NORMAL,
		T_CUBE,
		T_VOLUME,
	} type;

	XID3D_TEXTURE &_getTex() const { return *((XID3D_TEXTURE*)&buffer); }
	XID3D_CUBE_TEXTURE &_getCTex() const { return *((XID3D_CUBE_TEXTURE*)&buffer); }
	XID3D_VOLUME_TEXTURE &_getVTex() const { return *((XID3D_VOLUME_TEXTURE*)&buffer); }

	__declspec(property(get= _getTex)) XID3D_TEXTURE _tex;
	__declspec(property(get=_getCTex)) XID3D_CUBE_TEXTURE _cTex;
	__declspec(property(get=_getVTex)) XID3D_VOLUME_TEXTURE _vTex;
};


enum XLIGHT_TYPE {
	XLT_POINT				= D3DLIGHT_POINT,
	XLT_SPOT				= D3DLIGHT_SPOT,
	XLT_DIRECTIONAL			= D3DLIGHT_DIRECTIONAL,
};

class XLIGHT: public XDC<XOBJECT,XCID_LIGHT>
{
public:
	void Serialize(Serializer &sr);

	void Set(XLIGHT_TYPE type,XCOLOR ambient,XCOLOR diffuse,XCOLOR specular,Vec position,
		Vec direction,Vec attenuation,float range,float falloff,float theta,float phi);
	void SetPoint(XCOLOR ambient,XCOLOR diffuse,XCOLOR specular,Vec position,
		Vec attenuation,float range);
	void SetDirectional(XCOLOR ambient,XCOLOR diffuse,XCOLOR specular,Vec direction);

	//	Computes optimal range value for given attenuation and color precision
	static float OptimalRange(Vec att,UINT bits=8);


	XLIGHT_TYPE	type;
	Vec position,direction;
	Vec attenuation;
	float range,falloff;
	float theta,phi;
	XCOLOR ambient,diffuse,specular;
};



/**
	Geometric object abstract class
*/
class XGEOMETRIC_OBJECT: public XDC<XOBJECT,XCID_GEOMETRIC_OBJECT>
{
public:
	void Serialize(Serializer &sr);

	virtual void Paint(XFRUSTUM *frustum) { }

	//	Returns true if there was a collision
	//	dist - distance from ray origin to closest collision point
	//	collision point = ray.origin+ray.distance*dist
	//	This method should work faster when dist=0 and cPoint=0
	virtual bool Collision(const Ray &ray,float *dist=0) { return 0; }

	/// Transforms object by given matrix
	virtual	void Transform(Matrix &matrix) { }

	virtual void Animate(UINT nAnim,float animPos) { }


	///	Object central point
	Vec pivot;
	///	Object boundary
	XBOUNDARY boundary;
};



enum XGET_SURFACE {
	XGS_BACK_BUFFER,			/// Returns back buffer
	XGS_FRONT_BUFFER,			///	Creates new surface and fills it with
								///	front buffer data
	XGS_DEFAULT_DEPTH_BUFFER,	///	Default depth buffer
	XGS_DEPTH_BUFFER,			///	Active depth buffer
	XGS_RENDER_TARGET,			///	Active Render Targer
};
XSURFACE *XGetSurface(XGET_SURFACE surfaceType);

enum XSET_SURFACE {
	XSS_RENDER_TARGET,			///	Sets active render target
	XSS_DEPTH_BUFFER,			///	Sets active depth buffer; Possible input:
								///	- Surface of type: XST_DEPTH_BUFFER
								/// - 0
};
bool XSetSurface(XSET_SURFACE surfaceType,XSURFACE *surface);


XFlags(XCLEAR) {
	XC_COLOR			=D3DCLEAR_TARGET,
	XC_DEPTH			=D3DCLEAR_ZBUFFER,
	XC_STENCIL			=D3DCLEAR_STENCIL,
};
void XClear(XCLEAR flags,XCOLOR color,float depth=1.0f,int stencil=0);

void XSetViewport(Rect viewport,float minZ=0,float maxZ=1);
Rect XGetViewport();

void XSetScissorRect(Rect &rect);
Rect XGetScissorRect();

void XSetSamplerState(u32 stage,XSAMPLER_STATE state,u32 value);
u32  XGetSamplerState(u32 stage,XSAMPLER_STATE state);
void XPushSamplerState(u32 stage,XSAMPLER_STATE state);
void XPopSamplerState(u32 stage,XSAMPLER_STATE state);
void XSetTexture(u32 stage,XTEXTURE_LINK tex);

#define XSetSS XSetSamplerState
#define XGetSS XGetSamplerState
#define XPushSS XPushSamplerState
#define XPopSS XPopSamplerState

void XSetRenderState(XRENDER_STATE state,u32 value);
u32  XGetRenderState(XRENDER_STATE state);
void XPushRenderState(XRENDER_STATE state);
void XPopRenderState(XRENDER_STATE state);

//	Nie bêdzie mo¿na zmieniæ danej zmiennej, a¿ do uruchomienia metody unForce
//	Mo¿na u¿ywaæ: XRS_FILL_MODE XRS_CULL_MODE XRS_Z_TEST XRS_Z_WRITE XRS_BLENDING XRS_ALPHA_TEST
void XForceRenderState(XRENDER_STATE state,u32 value);
void XUnForceRenderState(XRENDER_STATE state);


#define XSetRS		XSetRenderState
#define XGetRS		XGetRenderState
#define XPushRS		XPushRenderState
#define XPopRS		XPopRenderState


#pragma region VERTEX_PROCESSING

void XSetVShader(XVSHADER_LINK shader);
void XSetVShaderConstant(UINT startReg,bool	*data,UINT num);
void XSetVShaderConstant(UINT startReg,int	*data,UINT num);
void XSetVShaderConstant(UINT startReg,float *data,UINT num);
void XSetVShaderConstant(UINT startReg,const Matrix &m);
void XSetVShaderConstant(UINT startReg,const FVec4 &v);
void XSetVShaderConstant(UINT startReg,const Vec4 &v);

enum XAUTO_VSHADER_CONSTANT {
	/// Transposed model*view*proj matrix
	XAVC_TMVP_MATRIX =1,
	/// Transposed inversed model*view*proj matrix
	XAVC_TIMVP_MATRIX =2,
	/// Eye (camera) position
	XAVC_EYE_POS=3,
};

/// if targetReg==-1 then specified matrix will not be automaticly sended
/// to vshader constant register
void XSetAutoVShaderConstant(XAUTO_VSHADER_CONSTANT type,UINT targetReg);

///	Active world matrix
/// Use to represent affine transformations
extern AMatrix *XWM;

///	Pushes world matrix onto the stack
void XPushWM();
/// Pops world matrix from the stack
void XPopWM();

void XSetViewMatrix(const Matrix &mat);
void XSetProjMatrix(const Matrix &mat);

const Matrix &XGetProjMatrix();
const Matrix &XGetViewMatrix();

void XSetProjMatrix(float fovAngle,float nearZ,float farZ);
void XSetOrthoProjMatrix(float width,float height,float nearZ,float farZ);
void XSetOrthoProjMatrix(Rect screen,float nearZ,float farZ);
void XSetViewMatrix(Vec position,Vec target,Vec up);

/// Sets view, projection and world matrix so: the view is orthogonal, and
/// top left corner=Vec(0,0,z), bootom right corner=Vec(resX,resY,z)
void XSet2DView(float resX,float resY);

void XSetPShaderConstant(UINT startReg,bool *data,UINT num);
void XSetPShaderConstant(UINT startReg,int *data,UINT num);
void XSetPShaderConstant(UINT startReg,float *data,UINT num);
void XSetPShader(XPSHADER_LINK pshader);



///	Uses (and automaticly creates is necessary) declaration with given format
/// creates unique declaration for every different pointer
bool XUseAutoDecl(const XVFORMAT* format);


void XDraw(XPRIMITIVE_TYPE type,u32 nPrims,u32 startVert);
void XDrawIndexed(XPRIMITIVE_TYPE type,u32 nPrims,u32 startInd,u32 baseVert,u32 minVert,u32 usedVerts);

void XDrawUP(XPRIMITIVE_TYPE type,u32 nPrims,void *vertData,u32 vertSize);
void XDrawIndexedUP(XPRIMITIVE_TYPE type,u32 nPrims,void *vertData,u32 vertSize,void *indexData,
					  u32 minVert,u32 usedVerts);

void XBeginRender();
void XFinishRender();

struct DisplayMode
{
	u32 width,height;
	u32 refreshRate;
	SURFACE_FORMAT format;
};

DisplayMode GetActiveDisplayMode();

XFlags(DEVICE_FLAGS)
{
	DF_DEFAULT				=0,
	DF_FULLSCREEN			=1,
	DF_COLOR_DEPTH_16BIT	=2, /// Useable only in fullscreen mode
	DF_TRIPLE_BUFFERING		=4,	/// Triple buffering (instead of double)
};

class DeviceParams
{
public:
	DeviceParams() { }
	DeviceParams(u32 w,u32 h,u32 rr,DEVICE_FLAGS tFlags=DF_DEFAULT)
		:width(w),height(h),refreshRate(rr),flags(tFlags) { }

	u32 width,height;
	u32 refreshRate;
	DEVICE_FLAGS flags;
};

XFlags(SURFACE_FORMAT_AVALIABLE)
{
	SFA_BACK_BUFFER				= 0x1,
	SFA_RENDER_TARGET			= 0x2,
	SFA_DEPTH_STENCIL			= 0x4,
	SFA_PLAIN					= 0x8,

	SFA_TEXTURE					= 0x10,
	/// Texture used as render target
	SFA_TEXTURE_RT				= 0x20,

	SFA_CUBE_TEXTURE			= 0x100,
	/// Cube texture used as render target
	SFA_CUBE_TEXTURE_RT			= 0x200,

	SFA_VOLUME_TEXTURE			= 0x1000,
};
SURFACE_FORMAT_AVALIABLE SurfaceFormatAvaliable(SURFACE_FORMAT fmt);


XFlags(CREATE_GFX_DEVICE) {
	GCD_DEFAULT				=0,
	CGD_MULTIPLE_TARGETS	=1,	/// Multiple render targets
	CGD_SOFTWARE_VP			=2,	///	Software vertex processing
	CGD_NVPERFHUD			=4,	///	Support for NVPerfHud
};

class GfxDevice;

GfxDevice *CreateGfxDevice(HWND window,DeviceParams params,CREATE_GFX_DEVICE flags=GCD_DEFAULT);

class GfxDevice
{
private:
	friend GfxDevice *CreateGfxDevice(HWND,DeviceParams,CREATE_GFX_DEVICE);
	GfxDevice();

public:
	~GfxDevice();

	void SetParams(DeviceParams &p);
	void OnLost();
	SURFACE_FORMAT_AVALIABLE FormatAvaliable(SURFACE_FORMAT fmt) const;
	void RefreshFormatCaps();
	void SetFormatCaps(D3DDEVTYPE devType,u32 adapter,D3DFORMAT adapterFmt,bool windowed);

private:
	D3DPRESENT_PARAMETERS GetPP(DeviceParams params,HWND window);

	DeviceParams params;
	HWND window;
	DisplayMode activeDisplayMode;
	SURFACE_FORMAT_AVALIABLE formatCaps[FMT_LAST+1];
	
public: /// D3D stuff
	LPDIRECT3DDEVICE9 d3dDevice;
	LPDIRECT3D9 d3d;
	D3DCAPS9 caps;

public: /// stats
	///	number of triangles rendered since last XBeginRender call
	u32 RenderedPrimitives;
};

extern GfxDevice *GGfxDevice;
extern LPDIRECT3DDEVICE9 GD3dDevice;

//	Wyœwietla zrenderowane dane na ekranie
//	Jeœli opcja XCGD_MULTIPLE_TARGETS jest w³¹czona, to jako parametr mo¿na podaæ wskaŸnik
//	do okna, w którym dane maj¹ byæ wyœwietlone
void XPresent(HWND target=0,IRect *srcRect=0);

//	Ustawia mg³ê; setState
void XSetFog(XFOG_TYPE type,XCOLOR color,float start,float end,float density);



namespace XFIXED_PROCESSING
{
	//	Number of active lights
	extern UINT	XActiveLights;

	void XUseLight(UINT number,const XLIGHT &light);
	void XEnableLight(UINT number,bool enable=1);

	// XUseLight(XActiveLights,light)
	// XEnableLight(XActiveLights++,1)
	void XAddLight(const XLIGHT &light);
	// Wy³¹cza wszystkie aktywne œwiat³a
	void XResetLights();

	void XSetTextureTransform(UINT nTex,Matrix &matrix);
	void XGetTextureTransform(UINT nTex,Matrix *output);

	enum XFPSHADER_ARG {
		XFPSA_TEXTURE	= D3DTA_TEXTURE,
		XFPSA_DIFFUSE	= D3DTA_DIFFUSE,
		XFPSA_SPECULAR	= D3DTA_SPECULAR,
		XFPSA_CURRENT	= D3DTA_CURRENT,
		XFPSA_CONSTANT	= D3DTA_CONSTANT,
		XFPSA_TEMP		= D3DTA_TEMP,
	};

	enum XFPSHADER_UV {
		XFPSU_TEXTURE	= D3DTSS_TCI_CAMERASPACENORMAL,
		XFPSU_DIFFUSE	= D3DTSS_TCI_CAMERASPACEPOSITION,
		XFPSU_SPECULAR	= D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR,
		XFPSU_CURRENT	= D3DTSS_TCI_SPHEREMAP,
	};

	enum XFPSHADER_OP{
		XFPSO_DISABLE				= D3DTOP_DISABLE,
		XFPSO_ARG1					= D3DTOP_SELECTARG1,
		XFPSO_ARG2					= D3DTOP_SELECTARG2,
		XFPSO_MUL					= D3DTOP_MODULATE,
		XFPSO_MUL2X					= D3DTOP_MODULATE2X,
		XFPSO_MUL4X					= D3DTOP_MODULATE4X,
		XFPSO_ADD					= D3DTOP_ADD,
		XFPSO_ADDSIGNED				= D3DTOP_ADDSIGNED,
		XFPSO_ADDSIGNED2X			= D3DTOP_ADDSIGNED2X,
		XFPSO_SUB					= D3DTOP_SUBTRACT,
		XFPSO_ADD_SMOOTH			= D3DTOP_ADDSMOOTH,
		XFPSO_BLENDDIFFUSEALPHA		= D3DTOP_BLENDDIFFUSEALPHA,
		XFPSO_BLENDTEXTUREALPHA		= D3DTOP_BLENDTEXTUREALPHA,
		XFPSO_BLENDFACTORALPHA		= D3DTOP_BLENDFACTORALPHA,
		XFPSO_BLENDTEXTUREALPHAPM	= D3DTOP_BLENDTEXTUREALPHAPM,
		XFPSO_BLENDCURRENTALPHA		= D3DTOP_BLENDCURRENTALPHA,
		XFPSO_PREMUL				= D3DTOP_PREMODULATE,
		XFPSO_MULALPHA_ADDCOLOR		= D3DTOP_MODULATEALPHA_ADDCOLOR,
		XFPSO_MULCOLOR_ADDALPHA		= D3DTOP_MODULATECOLOR_ADDALPHA,
		XFPSO_MULINVALPHA_ADDCOLOR	= D3DTOP_MODULATEINVALPHA_ADDCOLOR,
		XFPSO_MULINVCOLOR_ADDALPHA	= D3DTOP_MODULATEINVCOLOR_ADDALPHA,
		XFPSO_BUMPENVMAP			= D3DTOP_BUMPENVMAP,
		XFPSO_BUMPENVMAPLUMINANCE	= D3DTOP_BUMPENVMAPLUMINANCE,
		XFPSO_DOT					= D3DTOP_DOTPRODUCT3,
		XFPSO_MAD					= D3DTOP_MULTIPLYADD,
		XFPSO_LERP					= D3DTOP_LERP,
	};

	enum XFPSHADER_STATE {
		XFPSS_COLOROP				= 0,
		XFPSS_ALPHAOP				= 1,
		XFPSS_RESULTARG				= 2,
		XFPSS_COLORARG1				= 3,
		XFPSS_COLORARG2				= 4,
		XFPSS_COLORARG3				= 5,
		XFPSS_ALPHAARG1				= 6,
		XFPSS_ALPHAARG2				= 7,
		XFPSS_ALPHAARG3				= 8,
		XFPSS_TEXCOORDINDEX			= 9,
		XFPSS_BUMPENVMAT00			= 10,
		XFPSS_BUMPENVMAT01			= 11,
		XFPSS_BUMPENVMAT10			= 12,
		XFPSS_BUMPENVMAT11			= 13,
		XFPSS_BUMPENVLSCALE			= 14,
		XFPSS_BUMPENVLOFFSET		= 15,
		XFPSS_CONSTANT				= 16,
	};

	/**
	 * Class used in generating XFPSHADER states.
	 */
	class XFPSHADER_GEN
	{
	public:
		XFPSHADER_GEN() :nLevels(0) { arg[0]=XFPSA_CURRENT; }

		XFPSHADER_GEN operator&(const XFPSHADER_GEN &state);	// multiply
		XFPSHADER_GEN	operator+(const XFPSHADER_GEN &state);	// add
		XFPSHADER_GEN	operator-(const XFPSHADER_GEN &state);	// sub
		XFPSHADER_GEN	operator|(const XFPSHADER_GEN &state);	// dot (only for rgb)
		XFPSHADER_GEN	operator&(DWORD col);				// multiply constant
		XFPSHADER_GEN	operator+(DWORD col);				// add constant
		XFPSHADER_GEN	operator-(DWORD col);				// sub constant
		XFPSHADER_GEN	operator|(DWORD col);				// dot constnat

		XFPSHADER_GEN	operator-(float subValue);			// modifier for + (0.5f supported)
		XFPSHADER_GEN	operator<<(int shrMod);				// modifier for + (1) and & (1,2)

		DWORD constant[9];
		char texUV[9];
		XFPSHADER_ARG arg[9];
		XFPSHADER_OP op[8];
		int nLevels;
	};

	class XMG_DIF: public XFPSHADER_GEN { public: XMG_DIF() { arg[0]=XFPSA_DIFFUSE; } };
	class XMG_SPEC:public XFPSHADER_GEN { public: XMG_SPEC(){ arg[0]=XFPSA_SPECULAR; } };
	class XMG_TEX: public XFPSHADER_GEN { public: XMG_TEX(int uv){texUV[0]=uv;arg[0]=XFPSA_TEXTURE;}};
	class XMG_NULL:public XFPSHADER_GEN { public: XMG_NULL(){ arg[0]=XFPSA_CURRENT; } };

	XDEFINE_LINK(XFPSHADER)
	class XFPSHADER: public XDC<XOBJECT,XCID_FPSHADER>
	{
	public:
		typedef XFPSHADER_OP OP;
		typedef XFPSHADER_ARG ARG;
		typedef XFPSHADER_UV UV;

		struct STAGE
		{
			XCOLOR constant;
			u32 uvIndex;
			OP colorOp,alphaOp;
			ARG result,colorArg[3],alphaArg[3];
			bool refresh;

			float bumpMat[4],bumpL[2];
		};

		void Serialize(Serializer &sr);

		///	You can configure fixed shader states by using this method.
		///	Example color config: (XMT_TEX(0)&XMT_DIF())+XMT_DIF-0.5f
		void Gen(const XFPSHADER_GEN &color,const XFPSHADER_GEN &alpha);
		void Create(u32 nStages);
		void SetStage(u32 nStage,u32 uvIndex,OP colorOp,OP alphaOp,ARG c1,ARG c2,ARG a1,ARG a2,
						XCOLOR constant=0,ARG c3=XFPSA_CURRENT,ARG a3=XFPSA_CURRENT,ARG result=XFPSA_CURRENT);
		/// colorOp: XFPSO_BUMPENVMAP or XFPSO_BUMPENVMAPLUMINANCE
		void SetStageBump(u32 nStage,u32 uvIndex,OP colorOp,OP alphaOp,ARG c1,ARG c2,ARG a1,ARG a2,
						XCOLOR constant=0,ARG result=XFPSA_CURRENT,float bmat00=1.0f,float bmat01=0.0f,
						float bmat10=0.0f,float bmat11=1.0f,float lscale=0.0f,float loffset=0.0f);
		void Use();

	protected:
		Tab<STAGE> stages;
	};

	void XSetFixedPShader(XFPSHADER_LINK pshader);
	void SetFixedPShaderState(u32 stage,XFPSHADER_STATE state,DWORD value);
	inline void SetFixedPShaderState(u32 stage,XFPSHADER_STATE state,float value)
		{ SetFixedPShaderState(stage,state,*(DWORD*)&value); }
	DWORD GetFixedPShaderState(u32 stage,XFPSHADER_STATE state);
};


XTAddPod(XFIXED_PROCESSING::XFPSHADER::STAGE)


#endif
