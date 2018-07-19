//------------------------------------------------------------------\
//X LIBRARY															|
//math_path.inl														|
//																	|
//Copyright (C)  Krzysztof Jakubowski 2002 - 2006					|
//------------------------------------------------------------------/




template <class T> void XPATH<T>::Create(UINT MaxKeys,XINTERPOLATION_TYPE IType,UINT Flags)
{
	keysTab.Create(MaxKeys); posTab.Create(MaxKeys);
	nKeys=0; iType=IType; flags=Flags;
}

template <class T> void XPATH<T>::Serialize(Serializer &sr)
{
	sr(nKeys,iType,flags);
	sr(keysTab,posTab);
}

template <class T> void XPATH<T>::Free()
{
	keysTab.Free();
	posTab.Free();
	nKeys=0;
}

template <class T> void XPATH<T>::Set(UINT idx,const T &key,float pos)
{
	assert(idx<keysTab.Num());
	keysTab[idx]=key;
	posTab[idx]=pos;
}

template <class T> void XPATH<T>::Add(const T &key,float pos)
{
	assert(nKeys<keysTab.Num());
	Set(nKeys++,key,pos);
}

template <class T> UINT XPATH<T>::GetStartKey(float pos) const
{
	assert(nKeys>0);
	UINT s=0,k=nKeys-1;

	for(;s+1<k;) {
		UINT m=(s+k)>>1;
		if(posTab[m]>pos) k=m;
		else s=m;
	}

	return pos>=posTab[k]?k:s;
}

template <class T>
T XPATH<T>::Get(float pos) const
{
	assert(nKeys>0);

	if(nKeys==1) return keysTab[0];
	if(flags&XP_LOOPED_ANIMATION) pos-=Round(pos-0.5f);
	else if(pos>=posTab[nKeys-1]) return keysTab[nKeys-1];

	UINT key1,key2;
	if(flags&XP_REGULAR_KEYS) key1=Round(pos*(nKeys-(flags&XP_LOOPED_ANIMATION?0:1))-0.5f);
	else key1=GetStartKey(pos);

	if(flags&XP_LOOPED_ANIMATION) {
		if(key1<nKeys-1)
			{ key2=key1+1; pos=(pos-posTab[key1])/(posTab[key2]-posTab[key1]); }
		else { key2=0; pos=(pos-posTab[key1])/(1.0f-posTab[key1]); }
	}
	else {
		if(key1>=nKeys-1) return keysTab[key1]; key2=key1+1;
		pos=(pos-posTab[key1])/(posTab[key2]-posTab[key1]);
	}
	
	if(iType<XI_QUADRATIC)	// linear or cosine
		return iType==XI_LINEAR?Lerp(keysTab[key1],keysTab[key2],pos):
								CLerp(keysTab[key1],keysTab[key2],pos);

	UINT key3=key1==0?flags&XP_LOOPED_ANIMATION?nKeys-1:key1:key1-1;
	UINT key4=key2==nKeys-1?flags&XP_LOOPED_ANIMATION?0:key2:key2+1;
	return QLerp(keysTab[key3],keysTab[key1],keysTab[key2],keysTab[key4],pos);
}
