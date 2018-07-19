//----------------------------------------------------------------------\
//X LIBRARY																|
//xgraphics.h															|
//Uses Direct3D															|
//																		|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2006						|
//----------------------------------------------------------------------/



//	!!! UWAGA !!! ---------------------------------------------------
//	Na obiekty rysowane w 2D, np. XFONT albo XSPRITE nie dzia³aj¹
//	macierze œwiat³a, patrzenia i projekcji
// -----------------------------------------------------------------



#ifndef XGRAPHICS_H
#define XGRAPHICS_H


#include <xgraphics_t.h>

class XMULTITEX_MATERIAL { int a; }; XTAddPod(XMULTITEX_MATERIAL);


class XCAMERA;
class XFRUSTUM;
class XTEXTURE_MANAGER;
class XSIMPLE_MESH;
class XLOD_OBJECT;
class XMESH;
class XSKINNED_MESH;
class XPSYSTEM_INSTANCE;
class XPARTICLE_SYSTEM;
class XANIMATED_OBJECT;
class XANIMATOR;
class XFONT;



/**
 * XCAMERA
 */

enum XCAMERA_MOVE_TYPE
{
	XCMT_FORWARD			= 1,
	XCMT_BACKWARD			= 2,
	XCMT_LEFT				= 3,
	XCMT_RIGHT				= 4,
	XCMT_UP					= 5,
	XCMT_DOWN				= 6,
	XCMT_FORWARD_FREE		= 7,
	XCMT_BACKWARD_FREE		= 8,
	XCMT_UP_FREE			= 9,
	XCMT_DOWN_FREE			= 10,
};

#define XCID_CAMERA			(XCID_UNKNOWN|0x0400)
class XCAMERA: public XDC<XOBJECT,XCID_CAMERA>
{
public:
	//	Aktualizuje macierz patrzenia
	void Use();
	void Serialize(Serializer &sr);

	//	Ustawia parametry
	void Set(const FVec &pos,float rotLR,float rotUD,float minUD,float maxUD,bool lockUDView);

	//	Przesuwa kamerê w danym kierunku o podan¹ d³ugoœæ
	void Move(XCAMERA_MOVE_TYPE type,float length);

	//	Ustawia kamerê tak, aby patrzy³a na dany obiekt
	void LookAt(const FVec &target);

	//	Obraca kamerê o podany k¹t do góry / do³u
	void RotateUD(float angle);

	//	Obraca kamerê o podany k¹t w lewo / prawo
	void RotateLR(float angle);

	FVec pos,target,up;
	float rotLR,rotUD;
	float minUD,maxUD;
	bool lockUDView;

private:
	void Calc();
	bool preCalc;
};
XDEFINE_LINK(XCAMERA)



/**
 * XSIMPLE_MESH
 * Prosta siatka. Bez animacji.
 */


#define XCID_SIMPLE_MESH		(XCID_GEOMETRIC_OBJECT|0x0100)

class XSIMPLE_MESH_PART
{
public:
	void Serialize(Serializer &sr);

	XMULTITEX_MATERIAL material;
	int	faces,usedVerts;
};

class XSIMPLE_MESH: public XDC<XGEOMETRIC_OBJECT,XCID_SIMPLE_MESH>
{
public:
	void Free();
	void Serialize(Serializer &sr);
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager);

	virtual void Paint(XFRUSTUM *frustum);
	virtual bool Collision(const Ray &ray,float *dist=0);

	XVDECL decl;
	XVBUFFER vertexBuffer;
	XIBUFFER indexBuffer;
	Tab<XSIMPLE_MESH_PART>parts;	//	Jeœli puste, to wykorzystywane s¹ ca³e bufory wierzch./ind.
};
XDEFINE_LINK(XSIMPLE_MESH)



class XLOD_OBJECT_LEVEL
{
public:
	void Serialize(Serializer &sr);

	XGEOMETRIC_OBJECT_LINK	object;
	float					length;
};


#define XCID_LOD_OBJECT		(XCID_GEOMETRIC_OBJECT|0x0200)
/**
 * Obiekt ze zmiennym poziomem szczegó³owoœci.
 */
class XLOD_OBJECT: public XDC<XGEOMETRIC_OBJECT,XCID_LOD_OBJECT>
{
public:
	void Free();
	void Serialize(Serializer &sr);
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager);

	virtual void Paint(XFRUSTUM *frustum);
	virtual bool Collision(const Ray &ray,float *dist=0);
	virtual	void Transform(Matrix &matrix);
	virtual void GenBoundingBox();

	Tab<XLOD_OBJECT_LEVEL> levels;
	//	Maksymalna ró¿nica zmiany poziomu
	float maxDifference;
};
XDEFINE_LINK(XLOD_OBJECT)




class XMESH_NODE
{
public:
	XMESH_NODE() { posAnim=0; rotAnim=0; }
	~XMESH_NODE() { delete(posAnim); delete(rotAnim); }

	void Serialize(Serializer &sr);

	AMatrix matrix;
	String name;

	Vec pivot;
	XVEC_PATH *posAnim;
	XQUAT_PATH *rotAnim;

	Tab<UINT> subObjects;

	//	Materia³ jest ustawiony tylko w pierwszym elemencie w tablicy nodes w grupie elementów
	//	o takim samym materiale
	//	-1: brak materia³u
	UINT material;

	//	matrix nie brane pod uwagê
	XBOUNDARY boundary;

	//	Jak faces==0 to nie rysowane
	int	faces,startVert,usedVerts,startIndex,temp;
};

class XMESH_ANIM
{
public:
	void Serialize(Serializer &sr);

	float start,end;
	float length;
	String name;
};




#define XCID_MESH				(XCID_GEOMETRIC_OBJECT|0x0300)
/**
 * Z³o¿ona siatka; Obs³uguje animacje.
 * Jeœli root==0, to siatka nie jest animowana;
 */
class XMESH: public XDC<XGEOMETRIC_OBJECT,XCID_MESH>
{
public:
	void Free();
	void Serialize(Serializer &sr);
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager);

	virtual void Animate(UINT nAnim,float animPos);
	virtual void Paint(XFRUSTUM *frustum);
	virtual bool Collision(const Ray &ray,float *dist=0);

	UINT root;

	//	Tablica czêœci modelu do zrenderowania / sprawdzenia kolizji;
	//	Na koñcu znajduj¹ siê elementy bez œcian
	Tab<XMESH_NODE> nodes;

	XVBUFFER vertexBuffer;
	XIBUFFER indexBuffer;

	Tab<XMULTITEX_MATERIAL> materials;//	Mo¿e byæ pusta
	Tab<XMESH_ANIM> anims; //	Mo¿e byæ pusta
};
XDEFINE_LINK(XMESH)



/**
 * XBONE_ANIMATION, XBONE, XSKINNED_MESH
 */

struct XBONE_ANIMATION
{
	Vec *position;
	Quat *rotation;
};

struct XBONE
{
	AMatrix inverseOrientation;
	String name;
	XBONE_ANIMATION *animation;

	UINT mvertsNumber;
	USHORT *mvertIndex;
	float *mvertWeight;
};

#define XCID_SKINNED_MESH		(XCID_SIMPLE_MESH|0x01)
class XSKINNED_MESH: public XDC<XSIMPLE_MESH,XCID_SKINNED_MESH>
{
public:
	XSKINNED_MESH();

	void Free();
	void Serialize(Serializer &sr);

	//	Animuje obiekt
	//	Jeœli pod parametr anim wstawi siê liczbê 0xffffffff, to obiekt przyjmie pozycjê
	//	pocz¹tkow¹
	virtual void Animate(UINT anim,float time);
	virtual void Paint(XFRUSTUM *frustum);
	virtual void Transform(Matrix &matrix);

	//	Maluje szkielet obiektu
	virtual void PaintSkeleton();

	//	Generuje szeœciany otaczaj¹ce dla wszystkich klatek
	virtual void GenBoundingBoxes();


	XBONE *bone;

	UINT nBones;
	UINT nAnimations;

	//	Tablica iloœci kluczy animacji
	UINT *nKeys;

	//	Tablica powtarzalnoœci animacji
	bool *looped;

	//	Pocz¹tkowe pozycje wierzcho³ków siatki
	Vec *tempPosition;

	//	Szeœciany otaczaj¹ce dla ka¿dej klatki
	Vec **tempBoundingBoxP1;
	Vec **tempBoundingBoxP2;
};
XDEFINE_LINK(XSKINNED_MESH)



/**
 * Obiekt ten s³u¿y do rysowania mapy wysokoœci ze zmiennym poziomem
 * szczegó³owoœci. Stosowane techniki: LOD (Quadtree), Frustum Culling, mapa
 * maksimów. Do karty graficznej wysy³ana jest minimalna iloœæ danych w 2 fazach
 * (chyba ¿e siatka jest bardzo szczegó³owa i nie mieœci siê w buforze) i tylko w
 * razie potrzeby (jeœli np. mapa jest rysowana powtórnie i kamera nie zmieni³a
 * swojej pozycji to nic nie jest wysy³ane do karty).
 * Mapa jest 16-bitowa i mo¿e byc wielkoœci: ((2^n)+1)x((2^n)+1); (4<=n<=11)
 * W³aœciwoœci:
 * - quality: jakoœæ siatki (od 0 do 3)
 * - qualityDecreaseSpeed - prêdkoœæ z jak¹ zmnienia siê poziom dok³adnoœci
 *   wraz ze zmian¹ wysokoœci kamery.
 */


//	Powierzchniê nale¿y tworzyæ za pomoc¹ metody Create; lub ³adowaæ za pomoc¹
//	metody Load. Nie nale¿y allokowaæ pamiêci samemu - mo¿e to spowodowaæ b³êdne
//	dzia³anie obiektu

#define XCID_LOD_SURFACE		(XCID_GEOMETRIC_OBJECT|0x0400)
class XLOD_SURFACE: public XDC<XGEOMETRIC_OBJECT,XCID_LOD_SURFACE>
{
public:
	XLOD_SURFACE();
	~XLOD_SURFACE();

	void Free();
	void Serialize(Serializer &sr);

	//	Tworzy mapê; (alokuje heightMap, size; Resztê wartoœci trzeba ustawiæ samemu )
	virtual bool Create(UINT size);
	
	virtual void Paint(XFRUSTUM *frustum);

	//	Zwraca wysokoœæ w danym punkcie z przedzia³u <(0;0);(1;1)>;
	float getHeight(float x,float y);

	//	Zwraca wektor normalny w danym punkcie z przedzia³u <(0;0);(1;1)>;
	Vec getNormal(float x,float y);
	

	//	Przelicza mapy maksimów wysokoœci
	virtual bool CountMaximum();
	virtual bool Collision(const Ray &ray,float *dist=0);


	//	Mozna to ustawiæ; Jest wykorzystywane podczas generowania cieni (w XLANDSCAPE)
	//	i do wyliczania poprawnych normalnych
	//	Tereny s¹siednie musz¹ mieæ tak¹ sam¹ wielkoœæ co aktualny
	//	Uwaga! obiekt XLANDSCAPE nie mo¿e mieæ ustawionych s¹siadów XLOD_SURFACE (musi byæ XLANDSCAPE)
	//	Inaczej generator cieni siê wykrzaczy
	//	0 - lewy   1 - prawy   2 - górny    3 - dolny
	XLOD_SURFACE *neighbour[4];

	//	Dodatkowy modyfikator wysokoœci
	int	heightMod;

	UINT quality;
	float qualityDecreaseSpeed;
	USHORT *heightMap;

// read-only:
	int	size;
	USHORT **maximumMap;
	UINT nMaximumMaps;
};
XDEFINE_LINK(XLOD_SURFACE)



/**
 * Obiekt ten jest oparty na XLOD_SURFACE i s³u¿y do rysowania szczegó³owego
 * terenu. Obs³uguje texture splatting, posiada szybki generator mapy oœwietlenia
 * z cieniami (generacja w czasie rzeczywistym).
 * Teren rysuje na kilka sposobów:
 * - texture splatting (jeœli jest alphaMapa i co najmniej 2 texMapy);
 *   wykorzystuje pixel shadery w wersji 1.4;
 * - rozci¹gniêta tekstura + mapa detali (jeœli nie ma alphaMapy i s¹ 2 texMapy);
 *   wymaga obs³ugi 3 tekstur w 1 przebiegu.
 * - rozci¹gniêta tekstura (jeœli nie ma alphaMapy i jest 1 texMapa)
 *
 * Mo¿na tez rysowaæ kilka takich obiektów ko³o siebie, nie spowoduje to zadnych
 * nieci¹g³oœci ani w siatce ani w mapie cieni (trzeba tylko odpowiednio
 * skonfigurowaæ po³¹czenia).
 */

#define XCID_LANDSCAPE		(XCID_LOD_SURFACE|0x01)
class XLANDSCAPE: public XDC<XLOD_SURFACE,XCID_LANDSCAPE>
{
public:
	XLANDSCAPE();
	~XLANDSCAPE();
	
	void Free();
	void Serialize(Serializer &sr);

	virtual bool Create(UINT size);
	virtual bool Link(XOBJECT_MANAGER *objSource=XObjectManager);
	virtual bool Link(XTEXTURE *alphaMap,XTEXTURE *tex0,XTEXTURE *tex1=0,XTEXTURE *tex2=0,XTEXTURE *tex3=0);

	//	Maluje teren; frustum nie mo¿e byæ równe 0
	virtual void Paint(XFRUSTUM *frustum);
	

	//	Tworzy powierzchnie do cieni
	virtual bool CreateShadowMaps();

	//	Generuje cienie dla podanego œwiat³a (u¿ywango jako kierunkowe)
	virtual void GenShadow(XLIGHT &light,Vec landScale);

	//	Poszczególne kana³y (R,G,B) oznaczaj¹ wp³yw odpowiednich tekstur (1,2,3)
	XTEXTURE_LINK alphaMap;

	XTEXTURE_LINK texMap[4];
	Vec2 texScale[4];

// read-only:
	XTEXTURE *shadowMap;
	UCHAR *shadow;

private:
	int border[4][1024],shadowPart;
	int	tH[2048],sMode;
	int	shadowHeightMod;
	Vec tDir;
};
XDEFINE_LINK(XLANDSCAPE)



/**
 * Procedura emit w Emiterze powinny korzystaæ z funkcji XRand w celu pobrania
 * wartoœci losowej, aby zachowaæ stabilnoœæ systemu.
 */

#define XCID_PARTICLE_SYSTEM	(XCID_GEOMETRIC_OBJECT|0x0500)

struct XEMIT_PARTICLE
{
	Vec					pos,dir;		//pozycja, kierunek lotu (wektor jedn. przesuniêcia)
	float					rot,rad;		//obrót, wielkoœæ
	XCOLOR					col;			//mno¿nik koloru
};

#define XCID_PSYSTEM_EMITER	(XCID_PARTICLE_SYSTEM|0x01)
class XPSYSTEM_EMITER: public XDC<XOBJECT,XCID_PSYSTEM_EMITER>
{
public:
	void Free();
	void Serialize(Serializer &sr);
	bool Link(XOBJECT_MANAGER *mgr=XObjectManager) { return 1; }

	//	Procedura ta powinna wype³niæ strukturê part odpowiednimi wartoœciami
	virtual void Emit(XEMIT_PARTICLE &part,float emitTime) { }

	Vec2 emissionInterval;
	XFLOAT_PATH emissionFrequency;
};
XDEFINE_LINK(XPSYSTEM_EMITER)

#define XCID_PSYSTEM_EMITER_STD	(XCID_PARTICLE_SYSTEM|0x02)
class XPSYSTEM_EMITER_STD: public XDC<XPSYSTEM_EMITER,XCID_PSYSTEM_EMITER_STD>
{
public:
	void Free();
	void Serialize(Serializer &sr);
	bool Link(XOBJECT_MANAGER *mgr=XObjectManager);

	virtual void Emit(XEMIT_PARTICLE &part,float emitTime);

	enum XPARTICLE_EMITER_TYPE {
		XPET_SPHERE	=1,
		XPET_BOX	=2,
	} type;
	Vec size;

	XFLOAT_PATH emissionStrength;
	XVEC_PATH emissionDirection;
	XVEC_PATH emissionDirScatter;
	bool emitAtRandomAngle;

	XVEC_PATH rotation;
	XFLOAT_PATH sizeMultiplier;
	Vec pos;
};
XDEFINE_LINK(XPSYSTEM_EMITER_STD)


class XPSYSTEM_TEMPLATE
{
public:
	void Serialize(Serializer &sr);
	void Free();

	String name;
	XTEXTURE_LINK texture;
	float lifetime,gravityMul;
	int	texFramesX,texFramesY;	//	Iloœæ klatek w rzêdzie / kolumnie
	int texFramesC;				//	Iloœc klatek w cyklu
	int	fps,texCycles;			//	FPS, iloœæ cykli w teksturze
	int	texAnimMode;			//	0-brak; 1-single; 2-looped; 3-switch cycles;
	XFLOAT_PATH	size;
	XVEC_PATH color;
	XFLOAT_PATH alpha;
	XFLOAT_PATH rotation,stretch;
	XVEC_PATH position;

#define XPT_SPEED_STRETCH	1
#define XPT_ALIGN_NONE		0
#define XPT_ALIGN_ALL		2
#define XPT_ALIGN_XZ		4
#define XPT_ALIGN_Y			8
#define XPT_ALIGN_DIR		16
	BYTE					flags;
};


class XPSYSTEM_PAINTDATA
{
public:
	XPSYSTEM_PAINTDATA() { }
	XPSYSTEM_PAINTDATA(float Time,u32 Seed):wm(*XWM),seed(Seed)
		{ time=Time>1?Time-((int)Time)+1:Time; }

	float time;
	u32 seed;
	Matrix wm;
};

class XPARTICLE_SYSTEM: public XDC<XOBJECT,XCID_PARTICLE_SYSTEM>
{
public:
	XPARTICLE_SYSTEM();
	~XPARTICLE_SYSTEM();

	void Free();
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager);
	void Serialize(Serializer &sr);

	virtual void Paint(XFRUSTUM *frustum,float Time,u32 seed);

	//	Rysuje wszystkie instancje
	//	Wymaga ok. 100KB miejsca na stosie
	u32 Paint(float gravity);

	float duration;
	u32 randomSeed;
//private: read-only:
	DTab<XPSYSTEM_PAINTDATA>pData;
	int	maxParticles;		//	Musi byæ potêg¹ dwójki
	bool looped;
	
	Tab<XPSYSTEM_EMITER_LINK> emiters;			//	max 256
	Tab<XPSYSTEM_TEMPLATE>	particleTemplates;	//	max 256 
	Tab<int> systems;
	bool buffersInitiated;
};
XDEFINE_LINK(XPARTICLE_SYSTEM)

#define XCID_PSYSTEM_INSTANCE	(XCID_GEOMETRIC_OBJECT|0x0600)
class XPSYSTEM_INSTANCE: public XDC<XGEOMETRIC_OBJECT,XCID_PSYSTEM_INSTANCE>
{
public:
	void Free();
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager) { return 1; }
	void Serialize(Serializer &sr);
	virtual bool Collision(const Ray &ray,float *dist=0);

	virtual void Animate(XPSYSTEM_INSTANCE *inst) { }
	virtual void Paint(XPSYSTEM_INSTANCE *inst) { }


	XPARTICLE_SYSTEM_LINK pSystem;
};
XDEFINE_LINK(XPSYSTEM_INSTANCE)



/**
 * XFONT - s³u¿y do rysowania tekstu w 2D
 */

enum XFONT_ALIGN_TYPE
{
	XFAT_TOP				= 0,
	XFAT_DOWN				= 2,
	XFAT_LEFT				= 0,
	XFAT_RIGHT				= 2,
	XFAT_CENTER				= 1,
};


XFlags(XFONT_ALIGN) {
	XFA_TOP				=0,
	XFA_LEFT			=0,
	XFA_RIGHT			=2,
	XFA_CENTERX			=3,
	XFA_BOTTOM			=8,
	XFA_CENTERY			=16,
	XFA_CENTER			=(XFA_CENTERX|XFA_CENTERY),
};

#define XCID_FONT				0x01030400
/*
	Standard font rendering class
	fonts are rendered on the plane (0,0,1,1)
*/
class XFONT: public XDC<XOBJECT,XCID_FONT>
{
public:
	void Free();

	//	Inicjuje obiekt
	bool Create(XTEXTURE_LINK texture);

	//	Maluje tekst o podanych parametrach
	//	Maksymalna d³ugoœæ tekstu to 2048
	void Paint(float x,float y,XCOLOR color,float sx,float sy,const char *text,...);
	
	//	Maluje tekst o podanych parametrach
	//	Jeœli fit != 0, to tekst zostaje dopasowany do podanej szerokoœci; jeœli wystaje poza
	//	ni¹, to zostaje zwê¿ony
	//	Maksymalna d³ugoœæ tekstu to 2048
	//	align	- kombinacja flag XFONT_ALIGN; np. XFA_TOP|XFA_RIGHT
	//	rot		- obrót dooko³a punktu x,y
	//	jump	- odstêp pomiêdzy literami; domyœlinie 0.85 (zale¿ne od wielkoœci liter)
	void PaintEx(float x,float y,XCOLOR color,float scaleX,float scaleY,XFONT_ALIGN align,
					float jump,float fit,float rotation,const char *text, ...);


	XTEXTURE_LINK texture;
};
//XDEFINE_LINK(XFONT)



/**
 * XSPRITE - s³u¿y do rysowania prostok¹tów w 2D
 */

#define XCID_SPRITE			0x01030500
class XSPRITE: public XDC<XOBJECT,XCID_SPRITE>
{
public:
	//	£aduje ze skryptu
	bool Load(XCONFIG_SCRIPT *script);

	//	Maluje sprite w pozycji p1 obracaj¹c go o k¹t roatation (podawaæ w stopniach)
	//	dodatkowo mo¿na go te¿ przeskalowaæ (najpierw skaluje, potem obraca)
	void Paint(Vec2 pos,float rotation=0.0f,Vec2 scale=Vec2(1,1));

	//	Zwalnia pamiêæ
	void Free();

	//	£¹czy z innymi obiektami
	bool Link(XOBJECT_MANAGER *objectsSource=XObjectManager);

	Rect uvRect;
	XCOLOR color;
	Vec2 size;
	//XMATERIAL_LINK material;
};
//XDEFINE_LINK(XSPRITE)



/**
 * Different generation procedures.
 * Every generated object is centered in point (0,0,0)
 */

/// Generates box with given size
XSIMPLE_MESH *XGenerateBox(Vec size);
/// Generates cylinder
XSIMPLE_MESH *XGenerateCylinder(Vec size,int slices,int stacks);
/// Generates teapot
XSIMPLE_MESH *XGenerateTeapot(Vec size);
/// Generates sphere
XSIMPLE_MESH *XGenerateSphere(float radius,int slices,int stacks);
/// Generates torus
XSIMPLE_MESH *XGenerateTorus(float height,float innerRadius,float outerRadius,int sides,int rings);



//	Procedures for drawing triangle lists
//	those procedures are slow; dont use them in retail programs
//	Default values: diffuse=0xffffffff specular=0xffffffff uv=(0,0) normal=(0,0,1)

void XPBegin(XPRIMITIVE_TYPE type=XPT_TRIANGLE_LIST);
void XPDiff(XCOLOR col);
void XPUV(Vec2 uv);
void XPNrm(Vec nrm);
void XPVert(Vec pos);
void XPEnd();

inline void XPUV(float u,float v) { XPUV(Vec2(u,v)); }
inline void XPVert(Vec2 pos) { XPVert(Vec(pos.x,pos.y,0)); }
inline void XPVert(float x,float y,float z=0.0f) { XPVert(Vec(x,y,z)); }
inline void XPNrm(float x,float y,float z) { XPNrm(Vec(x,y,z)); }
inline void XPVert(Vec pos,Vec nrm) { XPNrm(nrm);XPVert(pos); }
inline void XPVert(Vec pos,XCOLOR diff,Vec nrm) { XPNrm(nrm);XPDiff(diff);XPVert(pos); }
inline void XPVert(Vec pos,Vec nrm,Vec2 uv) { XPNrm(nrm);XPUV(uv);XPVert(pos); }
inline void XPVert(Vec pos,Vec2 uv) { XPUV(uv);XPVert(pos); }
inline void XPVert(Vec pos,XCOLOR diff,Vec nrm,Vec2 uv) {XPDiff(diff);XPUV(uv);XPNrm(nrm);XPVert(pos);}
inline void XPVert(Vec pos,XCOLOR diff,Vec2 uv) { XPDiff(diff);XPUV(uv);XPVert(pos); }


void XPaintLine(Vec p1,Vec p2,XCOLOR a,XCOLOR b);
inline void XPaintLine(float x1,float y1,float x2,float y2,XCOLOR a,XCOLOR b)
	{ XPaintLine(Vec(x1,y1,1),Vec(x2,y2,1),a,b); }
/// Rotation around Z axis
void XPaintRect(Vec2 p1,Vec2 p2,float z,Vec2 uv1,Vec2 uv2,XCOLOR a,XCOLOR b,XCOLOR c,XCOLOR d,
				float rot=0.0f);
inline void XPaintRect(Rect rect,float z,Vec2 uv1,Vec2 uv2,XCOLOR a,XCOLOR b,XCOLOR c,XCOLOR d,float rot=0.0f)
	{ XPaintRect(Vec2(rect.x1,rect.y1),Vec2(rect.x2,rect.y2),z,uv1,uv2,a,b,c,d,rot); }
inline void XPaintRect(Rect rect,float z,Rect uvRect,XCOLOR a,XCOLOR b,XCOLOR c,XCOLOR d,float rot=0.0f)
{ XPaintRect(Vec2(rect.x1,rect.y1),Vec2(rect.x2,rect.y2),z,Vec2(uvRect.x1,uvRect.y1),
			 Vec2(uvRect.x2,uvRect.y2),a,b,c,d,rot); }




#endif
