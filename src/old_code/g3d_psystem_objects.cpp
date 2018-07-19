//------------------------------------------------------------------\
//GAME LIBRARY														|
//g3d_psystem.cpp													|
//Uses Direct3D														|
//																	|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2005					|
//------------------------------------------------------------------/




#include <xgraphics.h>



void XPSYSTEM_EMITER::Serialize(XSERIALIZER &sr)
{
//	sr(emissionFrequency,emissionInterval);
}

void XPSYSTEM_EMITER::Free()
{
	emissionFrequency.Free();
}

void XPSYSTEM_EMITER_STD::Serialize(XSERIALIZER &sr)
{
	XPSYSTEM_EMITER::Serialize(sr);
/*	sr(type,size);
	sr(emissionStrength,emissionDirection,emissionDirScatter);
	sr(emitAtRandomAngle,rotation,sizeMultiplier,pos);
*/}

void XPSYSTEM_EMITER_STD::Free()
{
/*	emissionFrequency.Free();
	emissionStrength.Free();
	emissionDirection.Free();
	emissionDirScatter.Free();
	rotation.Free();
	sizeMultiplier.Free();
*/}

void XPSYSTEM_EMITER_STD::Emit(XEMIT_PARTICLE &part,float eTime)
{
	switch(type) {
	case XPSYSTEM_EMITER_STD::XPET_BOX:
		part.pos=Vec(XFRand()-0.5f,XFRand()-0.5f,XFRand()-0.5f)*size;
		break;
	case XPSYSTEM_EMITER_STD::XPET_SPHERE: {
		float sx2=size.x*size.x,sy2=size.y*size.y;
		float tRad2=XFRand()*sy2,rad;
		if(tRad2>sx2+XEPSILION) rad=tRad2/size.y;
		else rad=sqrt(sx2-tRad2);
		float angle1=(XFRand())*360,angle2;
		switch(XRand()&3) {
		case 0: angle2=sqrt(XFRand())*90; break;
		case 1: angle2=180.0f-sqrt(XFRand())*90; break;
		case 2: angle2=sqrt(XFRand())*90+180; break;
		case 3: angle2=360.0f-sqrt(XFRand())*90; break;
		}
		Vec2 sc1=SinCosD(angle1),sc2=SinCosD(angle2);
		part.pos=Vec(sc1.y*sc2.x,sc2.y,sc1.x*sc2.x)*rad;
		break; }
	}
	part.pos+=pos;
	part.col=0xffffffff;
	
	//	Wyliczanie kierunku lotu:
	Vec rDir(2.0f*XFRand()-1.0f,2.0f*XFRand()-1.0f,2.0f*XFRand()-1.0f);
	Vec tDir=emissionDirection.Get(eTime)+rDir*emissionDirScatter.Get(eTime);
	Vec scx=SinCosD(tDir.x),scy=SinCosD(tDir.y);
	part.dir=Vec(scx.y*scy.x,scy.y,scx.x*scy.x)*emissionStrength.Get(eTime);

	part.rad=sizeMultiplier.Get(eTime);
	part.rot=rotation.Get(eTime).x;
	if(emitAtRandomAngle) part.rot+=rotation.Get(eTime).y*(2*XFRand()-1);
	
}

bool XPSYSTEM_EMITER_STD::Link(XOBJECT_MANAGER *mgr)
{
	return 1;
}

void XPSYSTEM_TEMPLATE::Serialize(XSERIALIZER &sr)
{
	if(sr.IsLoading()) Free();
/*	sr(name,texture,lifetime,gravityMul);
	sr(texAnimMode,texFramesX,texFramesY,texFramesC);
	sr(texCycles,fps,size,color);
	sr(alpha,rotation,stretch,position);
	sr(flags);
*/}

void XPSYSTEM_TEMPLATE::Free()
{
	texture.Free();
	size.Free();
	color.Free();
	alpha.Free();
	rotation.Free();
	position.Free();
}

bool XPSYSTEM_INSTANCE::Collision(const Ray &ray,float *smallestDistance)
{
	return boundary.Collision(ray,pivot,smallestDistance);
}

void XPSYSTEM_INSTANCE::Free()
{

}
