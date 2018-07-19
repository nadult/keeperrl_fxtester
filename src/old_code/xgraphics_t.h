//----------------------------------------------------------------------\
//X LIBRARY																|
//xgraphics_t.h															|
//Templated classes improving XGRAPHICS interfece.						|
//																		|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2006						|
//----------------------------------------------------------------------/



#ifndef XGRAPHICS_T_H
#define XGRAPHICS_T_H


#include <xgraphics_low.h>


#pragma region XVERTEX

/// Vertex position element of given type
template<class T> class XVPOS;
/// Transformed vertex position element of given type
template<class T> class XVPOST;
/// Vertex normal vector element of given type
template<class T> class XVNORMAL;
/// Vertex binormal element of given type
template<class T> class XVBINORMAL;
/// Vertex tangent element of given type
template<class T> class XVTANGENT;
/// Vertex diffuse element of given type
template<class T> class XVDIFFUSE;
/// Vertex specular element of given type
template<class T> class XVSPECULAR;
/// Vertex texture coordinate element of given type
template<class T> class XVTEXCOORD;



/// DOXYS_OFF
#include <graphics/graphics_vertex.inl>
/// DOXYS_ON

/**
	Vertex class template
	\example
	//Use example:
	...
	XDVERTEX( XDPOS, XDNORMAL, XDDIFFUSE ) vert;
	vert.pos=Vec(10,20,30);
	vert.normal=Normalize(Vec(1,2,0));
	vert.diffuse=0xff00ff00;
	\endexample
*/
template <class ELEMENTS_TUPLE>
class XVERTEX: public _XVERTEX<ELEMENTS_TUPLE,0>
{
	XSASSERT(ELEMENT_USED_MORE_THAN_ONCE, !_error )
public:
//	operator float*() { return (float*)this; }
	static inline XVFORMAT GetFormat() { XVFORMAT f; _fillVertexFormat(f.tab,0); return f; }

	/// Number of vertex elements
	static const UINT nElements=_nElements;
};


/// Default vertex position element ( Vec )
#define XDPOS			XVPOS<Vec3>
/// Default transformed vertex position element ( Vec4 )
#define XDPOST			XVPOST<Vec4>
/// Default vertex normal vector element ( Vec )
#define XDNORMAL		XVNORMAL<Vec3>
/// Default vertex binormal element ( Vec )
#define XDBINORMAL		XVBINORMAL<Vec3>
/// Default vertex tangent element ( Vec )
#define XDTANGENT		XVTANGENT<Vec3>
/// Default vertex diffuse element ( XCOLOR )
#define XDDIFFUSE		XVDIFFUSE<XCOLOR>
/// Default vertex specular element ( XCOLOR )
#define XDSPECULAR		XVSPECULAR<XCOLOR>
/// Default vertex texture coordinate element ( Vec2 )
#define XDTEXCOORD		XVTEXCOORD<Vec2>

/**
	Use this macro to define vertex class
	\example
	// Definition examples:
	XDVERTEX( XDPOS, XDNORMAL )
	XDVERTEX( XDPOS, XDDIFFUSE, XDNORMAL, XVTEXCOORD<Vec4> )
	XDVERTEX( XDTEXCOORD, XVTEXCOORD<Vec4>, XDTEXCOORD )
	\endexample
*/
#define XDVERTEX(...) XVERTEX<XTTUPLE<__VA_ARGS__>::Result >

#pragma endregion


#pragma region XSVBUFFER

/*
	Specialized vertex buffer. Has its own vertex declaration.
*/
/*
template<class _VERTEX>
class XSVBUFFER: public XVBUFFER
{
public:
	typedef _VERTEX VERTEX;

	XSVBUFFER() { format=VERTEX::GetFormat(); vertexSize=sizeof(VERTEX); }
	XSVBUFFER(const XSVBUFFER<VERTEX> &tBuf) { (*this)=(tBuf); }

	bool Create(UINT numVerts,XUSAGE usage,XPOOL pool)
	{
		if(!XVBUFFER::Create(numVerts,format,usage,pool)) return 0;
		return decl.Create(format);
	}
	void Free()
	{
		decl.Free();
		XVBUFFER::Free();
	}
	VERTEX *Lock(UINT startVertex=0,UINT verticesToLock=0,XLOCK flags=XL_DEFAULT)
	{
		return (VERTEX*)XVBUFFER::Lock(startVertex,verticesToLock,flags);
	}
	void Unlock() { XVBUFFER::Unlock(); }

	operator VERTEX*() { return (VERTEX*)dataPointer; }
	operator const VERTEX*() const { return (VERTEX*)dataPointer; }

	void Use() { decl.Use(); XVBUFFER::Use(0); }
	UINT Num() const { return XVBUFFER::Num(); }

	//	Generuje szeœcian otaczaj¹cy wierzcho³ki z podanego zakresu
	//	Jeœli nVertices==0 to uwzgledniane s¹ wszystkie wierzcho³ki
	//XBOUNDARY GenBoundary(int startVertex=0,int nVertices=0);

	//	Przekszta³ca pozycje i normalne wierzcho³ków
	//	Jeœli nVerts<0 to jest ustawiany na verticesNumber
	//void Transform(const Matrix &mat,int startIndex=0,int nVerts=-1);

	XVDECL decl;
};
*/
#pragma endregion




/**
Vertex stream class; Contains pointers to vertex buffers and vertex declaration describing
the buffers. You can change vertex buffer pointers in buffers tab, but dont forget to
change vertex declaration (if new vertex buffer has different format).
*/
class XVSTREAM
{
public:
	XVSTREAM() { }
	bool Create(XVBUFFER_LINK *buffers,UINT num);
	bool Create(XVBUFFER&);
	bool Create(XVBUFFER&,XVBUFFER&);
	bool Create(XVBUFFER&,XVBUFFER&,XVBUFFER&);
	bool Create(XVBUFFER&,XVBUFFER&,XVBUFFER&,XVBUFFER&);
	void Use();

	Tab<XVBUFFER_LINK> buffers;
	XVDECL decl;
};


#endif