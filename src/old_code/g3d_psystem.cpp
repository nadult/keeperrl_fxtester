//------------------------------------------------------------------\
//GAME LIBRARY														|
//g3d_psystem.cpp													|
//Uses Direct3D														|
//																	|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2005					|
//------------------------------------------------------------------/




#include <xgraphics.h>


//	Instancja cz¹stki; zawiera minimaln¹ iloœæ danych z jakich mo¿na wygenerowaæ
//	cz¹stkê PARTICLE
class IPARTICLE
{
public:
	DWORD seed;
	float genTime;
};

class PARTICLE
{
public:
	Vec pos,tVec[4];
	Vec2 uv1,uv2;
	XCOLOR col;
};

struct PARTICLE_VERTEX: public Vec
{
	inline void set(Vec vec,XCOLOR Color,float U,float V)
		{ x=vec.x; y=vec.y; z=vec.z; color=Color; u=U; v=V; }

	DWORD color;
	float u,v;
};

#define PARTICLE_FVF		    (XFVF_XYZ|XFVF_DIFFUSE|XFVF_UV1)
typedef XDVERTEX(XDPOS,XDDIFFUSE,XDTEXCOORD) PARTICLE_VERTEX_;

#define MAX_PARTICLES			1024


static XVFORMAT				vFormat;
static XVBUFFER				*vertexBuffer=NULL;	//	Bufor na wierzcho³ki cz¹steczek
static XIBUFFER				*indexBuffer=NULL;	//	Statyczny bufor indeksów
static UINT					nParticles=0,nRenderParticles=0;//	Iloœæ cz¹stek w buforze;
															//	Iloœæ cz¹stek do zrenderowania
static PARTICLE_VERTEX		*buf=NULL,*bufP=NULL;		//	Tymczasowy bufor na wierzcho³ki
static int					activeSystems=0,countParts=0;

//	Kopiuje wierzcho³ki z bufora buf do vertexBuffera
static void flush()
{
	static bool discard=0;
	if(nRenderParticles==0) return;
	float *pOut=vertexBuffer->Lock(nParticles*4,nRenderParticles*4,discard?XL_DISCARD:XL_NOOVERWRITE);
	Memcpy(pOut,buf+(nParticles<<2),(nRenderParticles<<2)*sizeof(PARTICLE_VERTEX));
	vertexBuffer->Unlock();
	XRenderIndexed(XPT_TRIANGLE_LIST,nRenderParticles*2,nParticles*6,nParticles*4,nRenderParticles*4);

	discard=0; nParticles+=nRenderParticles; countParts+=nRenderParticles; nRenderParticles=0;
	if(nParticles+nRenderParticles>=MAX_PARTICLES-256) { discard=1; nParticles=0; bufP=buf; }
}

static bool initBuffers()
{
	XOnce vFormat=PARTICLE_VERTEX_::GetFormat();
	if(activeSystems>1) { activeSystems++; return 1; }

	vertexBuffer=new XVBUFFER;
	if(!vertexBuffer->Create(MAX_PARTICLES*4,PARTICLE_VERTEX_::GetFormat(),D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,0)) {
		XOnce XMsg("XPARTICLE_SYSTEM::"XERRMSG "Cannot create vertex buffer (%d vertices)",MAX_PARTICLES*4);
		return 0;
	}

	indexBuffer=new XIBUFFER;
	if(!indexBuffer->Create(MAX_PARTICLES*6,D3DUSAGE_WRITEONLY,0)) {
		delete vertexBuffer; vertexBuffer=NULL;
		XOnce XMsg("XPARTICLE_SYSTEM::"XERRMSG "Cannot create index buffer (%d indices)",MAX_PARTICLES*6);
		return 0;
	}
	USHORT *p=indexBuffer->Lock(0);
	for(int n=0;n<MAX_PARTICLES;n++) {
		int tn=n<<2;
		*p++=tn+1; *p++=tn;   *p++=tn+3;
		*p++=tn+1; *p++=tn+3; *p++=tn+2;
	}
	indexBuffer->Unlock();

	if(!(buf=new PARTICLE_VERTEX[MAX_PARTICLES*4])) {
		delete vertexBuffer; vertexBuffer=NULL;
		delete indexBuffer; indexBuffer=NULL;
		XOnce XMsg("XPARTICLE_SYSTEM::"XERRMSG "Cannot create buffer for %d vertices.",16384*4);
		return 0;
	}
	bufP=buf; nParticles=0; nRenderParticles=0;

	activeSystems++;
	return 1;
}

static void freeBuffers()
{
	activeSystems--;
	if(activeSystems) return;
	delete vertexBuffer; vertexBuffer=NULL;
	delete indexBuffer; indexBuffer=NULL;
	delete[](buf); buf=NULL;
}

static void renderParticles(const Matrix &mat,PARTICLE *parts,int numParts)
{
	for(int tNum=0;tNum<numParts;tNum++) {
		int toRender=numParts-tNum;
		int left=MAX_PARTICLES-(nParticles+nRenderParticles);
		bool doFlush=left<toRender?(toRender=left),1:0;

		for(int n=0;n<toRender;n++) {
			PARTICLE &part=parts[n];
			Vec pos=part.pos*mat;

			bufP[0].set(pos+part.tVec[0],part.col,part.uv1.x,part.uv1.y);
			bufP[1].set(pos+part.tVec[1],part.col,part.uv2.x,part.uv1.y);
			bufP[2].set(pos+part.tVec[2],part.col,part.uv2.x,part.uv2.y);
			bufP[3].set(pos+part.tVec[3],part.col,part.uv1.x,part.uv2.y);
			bufP+=4;
		}
		tNum+=toRender; nRenderParticles+=toRender; parts+=toRender;
		if(doFlush) flush();
	}
}

XPARTICLE_SYSTEM::XPARTICLE_SYSTEM() :buffersInitiated(0)
{
	randomSeed=0;
}

XPARTICLE_SYSTEM::~XPARTICLE_SYSTEM()
{
	if(buffersInitiated) { freeBuffers(); buffersInitiated=0; }
}

void XPARTICLE_SYSTEM::Free()
{
	if(buffersInitiated) { freeBuffers(); buffersInitiated=0; }
}

void XPARTICLE_SYSTEM::Serialize(XSERIALIZER &sr)
{
	if(sr.IsLoading()) Free();

	sr(particleTemplates,emiters,systems);
	sr(maxParticles,randomSeed,duration);
}

bool XPARTICLE_SYSTEM::Link(XOBJECT_MANAGER *mgr)
{
	bool ret=1;
	for(UINT n=0;n<emiters.Num();n++) if(emiters[n]) ret&=emiters[n]->Link(mgr);
	for(UINT n=0;n<particleTemplates.Num();n++) ret&=particleTemplates[n].texture.Link(mgr);

	return ret;
}

void XPARTICLE_SYSTEM::Paint(XFRUSTUM *frustum,float time,DWORD seed)
{
	pData.Add(XPSYSTEM_PAINTDATA(time/duration,seed));
}

static int timeSortFunc(const void *P1,const void *P2)
{
	XPSYSTEM_PAINTDATA &p1=**(XPSYSTEM_PAINTDATA**)P1,&p2=**(XPSYSTEM_PAINTDATA**)P2;
	return p1.time>p2.time?-1:p1.time<p2.time?1:p1.seed<p2.seed?-1:1;
}

//	---------------------------------------------------------------------------------------------
//	W³aœciwa metoda rysuj¹ca cz¹stki. Najpierw sortujemy wszystkie instancje wg. czasu. Nastêpnie
//	dla ka¿dego systemu:
//	- zaczynaj¹c od czasu zero przechodz¹c do koñca animacji, zatrzymuj¹c siê na czasie ka¿dej
//	  instancji animujemy cz¹stki, wrzucamy je do bufora i renderujemy.
//	---------------------------------------------------------------------------------------------
DWORD XPARTICLE_SYSTEM::Paint(float globalGravity)
{
	if(!buffersInitiated) {
		if(!initBuffers()) {
			XOnce XMsg(XERRMSG "Error while creating buffers.");
			return 0;
		}
		buffersInitiated=1;
	}

	XProfile("Paint"); DWORD tSaveRand=XGetSRand();
	//	Sortowanie instancji systemu cz¹stek wg. czasu
	XTAB<XPSYSTEM_PAINTDATA*> instTab; instTab.Create(pData.Num());
	for(UINT n=0;n<pData.Num();n++) instTab[n]=&pData[n];
	qsort(&instTab[0],instTab.Num(),sizeof(void*),timeSortFunc);

	if(maxParticles&(maxParticles-1)) maxParticles=128;
	if(maxParticles>MAX_PARTICLES) maxParticles=MAX_PARTICLES;
	int particlesMask=maxParticles-1;
	
	Matrix invViewMat;
	Vec tVecAll[4],tVecY[4],tVecXZ[4];	//	Wektory tw. cz¹stki o ró¿nych orientacjach
	{	//	Inicjacja zmiennych do kontroli orientacji partikla
		Matrix tMat=XGetViewMatrix(); Quat quat,q1,q2;
		D3DXQuaternionRotationMatrix((D3DXQUATERNION*)&quat,(D3DXMATRIX*)&tMat);
		invViewMat=Inv(XGetViewMatrix());
		tMat.SetupRotation(quat);
		tVecAll[0]=Vec(-0.5f,0.5f,0)*tMat; tVecAll[1]=Vec(0.5f,0.5f,0)*tMat;
		tVecAll[2]=Vec(0.5f,-0.5f,0)*tMat; tVecAll[3]=Vec(-0.5f,-0.5f,0)*tMat;
		q1=quat; q1.x=0; q1.z=0; q1=Normalize(q1); tMat=q1;
		tVecY[0]=Vec(-0.5f,0.5f,0)*tMat; tVecY[1]=Vec(0.5f,0.5f,0)*tMat;
		tVecY[2]=Vec(0.5f,-0.5f,0)*tMat; tVecY[3]=Vec(-0.5f,-0.5f,0)*tMat;
		q2=quat; q2.y=0; q2=Normalize(q2); tMat=q2;
		tVecXZ[0]=Vec(-0.5f,0.5f,0)*tMat; tVecXZ[1]=Vec(0.5f,0.5f,0)*tMat;
		tVecXZ[2]=Vec(0.5f,-0.5f,0)*tMat; tVecXZ[3]=Vec(-0.5f,-0.5f,0)*tMat;
	}
	
	countParts=0;
	indexBuffer->Use();
	XUseAutoDecl(&vFormat);
	XSetVertexStream(*vertexBuffer);
	XPushWM(); XWM->Identity();

	for(UINT nSystem=0;nSystem<systems.Num();nSystem++) {
		XPSYSTEM_TEMPLATE &pTempl=particleTemplates[(systems[nSystem]&0xff00)>>8];
		XPSYSTEM_EMITER_LINK emiter=emiters[systems[nSystem]&0xff];
		XPSYSTEM_EMITER_STD_LINK emiterStd=emiter;

		float emitStart=emiter->emissionInterval.x/duration,emitEnd=emiter->emissionInterval.y/duration;
		emitEnd=Max(emitStart+0.01f/duration,emitEnd);
		float emitMul=1.0f/(emitEnd-emitStart);//Mno¿nik do przejœcia z czasu systemu do czasu emitera

		float lifetime=Min(Max(pTempl.lifetime,0.01f)/duration,0.999f),invLifeTime=1.0f/lifetime;
		float gravity=globalGravity*pTempl.gravityMul;

		Vec tVecActive[4];
		Vec2 texUnit=Vec2(1.0f/pTempl.texFramesX,1.0f/pTempl.texFramesY);
		if(pTempl.flags&XPT_ALIGN_ALL) for(int t=0;t<4;t++) tVecActive[t]=tVecAll[t];
		else if(pTempl.flags&XPT_ALIGN_XZ) for(int t=0;t<4;t++) tVecActive[t]=tVecXZ[t];
		else if(pTempl.flags&XPT_ALIGN_Y) for(int t=0;t<4;t++) tVecActive[t]=tVecY[t];
		else if((pTempl.flags&30)==0)  {
			tVecActive[0]=Vec(-0.5f,0.5f,0); tVecActive[1]=Vec(0.5f,0.5f,0);
			tVecActive[2]=Vec(0.5f,-0.5f,0); tVecActive[3]=Vec(-0.5f,-0.5f,0);
		}
		Vec tRotAxisActive=(tVecActive[1]-tVecActive[0])^(tVecActive[3]-tVecActive[0]);

		XTEST
		/*{ //	Ustawienie materia³u
			static XMULTITEX_MATERIAL mat;
			XOnce mat.Set(XFM_SOLID,XCM_NONE,0,1,0,XBM_SRCALPHA,XBM_DSTALPHA);
			if(pTempl.texture) {
				mat.SetStage(0,0,XTO_MUL,XTO_MUL,XTA_TEXTURE,XTA_DIFFUSE,XTA_TEXTURE,XTA_DIFFUSE);
				mat.textures[0]=pTempl.texture;
			}
			else {
				mat.SetStage(0,0,XTO_ARG1,XTO_ARG1,XTA_DIFFUSE,XTA_DIFFUSE,XTA_DIFFUSE,XTA_DIFFUSE);
				mat.textures[0]=NULL;
			}
			mat.Use();
		}*/

		//	Bufory na cz¹stki
		IPARTICLE particles[MAX_PARTICLES];
		int activeParticles=0,startParticle=0;	//	Iloœæ aktywnych cz¹stek; Numer najstarszej cz¹stki
		PARTICLE outParticles[MAX_PARTICLES];
		int activeOutParticles=0;

		//	Czas ostatniej generacji cz¹stek
		float lastGenTime=-1;
		DWORD lastGenSeed=0;

		float time=emitStart;	//	Aktulany czas; wykorzystywany g³ównie do kontroli emisji
		UINT nInstance=0;

		//	Ominiêcie instancji bez cz¹stek
		for(;nInstance<instTab.Num();nInstance++) if(instTab[nInstance]->time>=time) break;
		
		for(;nInstance<instTab.Num();nInstance++) {
			float tTime=instTab[nInstance]->time;
			DWORD tSeed=instTab[nInstance]->seed;
			XProfile("emission");
			time-=emitStart; tTime-=emitStart;
			if(tTime-time>XEPSILION) {
				//	Pierwsze przejœcie przez czas cz¹stek których nie trzeba tworzyæ bo i tak
				//	nie bêd¹ widoczne
				for(float firstPart=tTime-lifetime;time<firstPart;) {	
					if(time>=emitEnd) break;
					float add=1.0f/(Min(emiter->emissionFrequency.Get(time*emitMul),1000)*duration);
					time+=add;
					if(time-add<1&&time>=1) XSRand(randomSeed);
					DWORD tRand=XRand();
				}
				//	Drugie przejœcie: w³aœciwe tworzenie instancji cz¹stek
				for(;time<tTime;) {
					if(time>=emitEnd) { time=tTime; break; }
					float add=1.0f/(Min(emiter->emissionFrequency.Get(time*emitMul),1000)*duration);
					time+=add;
					if(time-add<1&&time>=1) XSRand(randomSeed);
					DWORD tRand=XRand();
					if(activeParticles<maxParticles) {
						int nPart=(startParticle+activeParticles++)&particlesMask;
						particles[nPart].genTime=time+emitStart;
						particles[nPart].seed=tRand;
					}
					{	//	Kasowanie starych instancji cz¹stek
						float killTime=time-lifetime;
						while(particles[startParticle].genTime<=killTime&&activeParticles) {
							startParticle=(startParticle+1)&particlesMask;
							activeParticles--;
						}
					}
				}
			}
			time+=emitStart; tTime+=emitStart;
			XEndProfile();

			//	Generacja cz¹stek z instancji; Tylko wtedy kiedy trzeba; Jeœli poprzednia
			//	instancja ma niezbyt odleg³y czas, to korzystamy z cz¹stek dla niej wygenerowanych
			if(lastGenTime!=tTime||lastGenSeed!=tSeed) {
				XProfile("generate");
				DWORD tEmitSeed=XGetSRand();
				activeOutParticles=0;
				for(int n=0;n<activeParticles;n++) {
					float gTime,pTime,eTime; {
						IPARTICLE &tPart=particles[(n+startParticle)&particlesMask];
						gTime=tPart.genTime,eTime=(gTime-emitStart)*emitMul;
						pTime=(tTime-gTime)*invLifeTime; if(pTime<0) break;
						XSRand(tPart.seed*(tSeed+1));
					}
					PARTICLE &part=outParticles[n]; Vec dir; float rad,rot,stretchMul; {	//	Emisja
						XEMIT_PARTICLE ePart;
						emiter->Emit(ePart,gTime);
						part.pos=ePart.pos; dir=ePart.dir*duration;
						rad=ePart.rad*pTempl.size.Get(pTime);
						rot=ePart.rot+pTempl.rotation.Get(pTime);
						part.col=ePart.col;

						float t=(gTime-tTime)*duration; stretchMul=Length(Vec(0,-gravity*t,0)+dir);
						dir+=Vec(0,-1,0)*gravity*t*t;
						part.pos+=dir*(tTime-gTime)+pTempl.position.Get(pTime);
					}
					{	//	Wyliczanie koloru
						Vec col=pTempl.color.Get(pTime); float alpha=pTempl.alpha.Get(pTime);
						XUVARIABLE r(col.x+12582912.0f),g(col.y+12582912.0f),b(col.z+12582912.0f),
							a(alpha+12582912.0f);
						r.i=(XCut(r.i-1262485504,0,255)*XGetR(part.col))>>8;
						g.i=(XCut(g.i-1262485504,0,255)*XGetG(part.col))>>8;
						b.i=(XCut(b.i-1262485504,0,255)*XGetB(part.col))>>8;
						a.i=(XCut(a.i-1262485504,0,255)*XGetA(part.col))>>8;
						part.col=XCol(a.i,r.i,g.i,b.i);
					}
					{	//	Wyliczanie koordynat UV
						if(pTempl.texAnimMode==0) { part.uv1=Vec2(0,0); part.uv2=Vec2(1,1); }
						else {
							int nFrame;{XUVARIABLE v(pTime*pTempl.fps+12582912.0f);nFrame=v.i-1262485504;}
							int texCycle=XRand()%pTempl.texCycles;
							if(pTempl.texAnimMode==1)nFrame=Min(pTempl.texFramesC-1,nFrame);// single
							else if(pTempl.texAnimMode==2) nFrame%=pTempl.texFramesC; // looped
							else {	// switch
								texCycle=(texCycle+nFrame/pTempl.texFramesC)%pTempl.texCycles;
								nFrame%=pTempl.texFramesC;
							}

							int y=nFrame/pTempl.texFramesX,x=nFrame-y*pTempl.texFramesX;
							y+=texCycle*((pTempl.texFramesC+pTempl.texFramesX-1)/pTempl.texFramesX);
							part.uv1=Vec2(float(x),float(y))*texUnit; part.uv2=part.uv1+texUnit;
						}
					}
					{	//	Wyliczanie wektorów tworz¹cych cz¹stkê
						Vec rotAxis;

						if(pTempl.flags&XPT_ALIGN_DIR) {
							Vec tDir=Normalize(dir);
							Vec front=Vec(invViewMat.c1,invViewMat.c2,invViewMat.c3);
							Vec ttDir=Normalize(front^tDir)*0.5f; tDir*=0.5f;
							part.tVec[0]=-ttDir-tDir; part.tVec[1]=ttDir-tDir;
							part.tVec[2]=ttDir+tDir; part.tVec[3]=-ttDir+tDir;
							rotAxis=-front;
						}
						else {	//	XPT_ALIGN_ALL | XZ | Y | NONE
							part.tVec[0]=tVecActive[0]; part.tVec[1]=tVecActive[1];
							part.tVec[2]=tVecActive[2]; part.tVec[3]=tVecActive[3];
							rotAxis=tRotAxisActive;
						}
						{	//	Stretch (rozszerzanie)
							float stretch=pTempl.stretch.Get(pTime)*0.5f;
							if(pTempl.flags&XPT_SPEED_STRETCH) stretch*=stretchMul;
							Vec stretchVec1=(part.tVec[3]-part.tVec[0])*stretch;
							Vec stretchVec2=(part.tVec[2]-part.tVec[1])*stretch;
							part.tVec[0]-=stretchVec1; part.tVec[3]+=stretchVec1;
							part.tVec[1]-=stretchVec2; part.tVec[2]+=stretchVec2;
						}
						
						if(Abs(rot)>XEPSILION) {
							Matrix mat(Quat(rotAxis,rot)); mat.ScaleLocal(rad,rad,rad);
							part.tVec[0]*=mat; part.tVec[1]*=mat;
							part.tVec[2]*=mat; part.tVec[3]*=mat;
						}
						else {
							part.tVec[0]*=rad; part.tVec[1]*=rad;
							part.tVec[2]*=rad; part.tVec[3]*=rad;
						}
					}
					activeOutParticles++;
				}
				lastGenTime=tTime; lastGenSeed=tSeed;
				XSRand(tEmitSeed);
				XEndProfile();
			}

			XProfile("output");
			renderParticles(instTab[nInstance]->wm,outParticles,activeOutParticles);
			XEndProfile();
		}
		flush();
	}
	XPopWM();
	pData.Free();
	XEndProfile("Paint");

	XSRand(tSaveRand);
	return countParts;
}
