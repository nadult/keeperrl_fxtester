#include "curve.h"

template <class T>
Curve<T>::Curve(vector<float> keys, vector<T> values, InterpolationType interp)
	: m_keys(move(keys)), m_values(move(values)), m_interp(interp) {
	DASSERT(m_keys.size() == m_values.size());
	for(int n = 0; n < m_keys.size(); n++) {
		PASSERT(m_keys[n] >= 0.0f && m_keys[n] <= 1.0f);
		if(n > 0)
			PASSERT(m_keys[n] >= m_keys[n - 1]);
	}
}

// TODO: add sampleLooped
template <class T> T Curve<T>::sample(float position) const {
	PASSERT(position >= 0.0f && position <= 1.0f);
	if(m_values.size() <= 1)
		return m_values.empty() ? T() : m_values.front();

	int id = 0;
	while(id < m_keys.size() && m_keys[id] < position)
		id++;

	int ids[4] = {id == 0 ? 0 : id - 1, id, id + 1, id + 2};

	if(ids[2] == m_keys.size())
		ids[2] = ids[3] = id;
	else if(ids[3] == m_keys.size())
		ids[3] = id + 1;

	float key1 = m_keys[ids[1]], key2 = m_keys[ids[2]];
	float t = key1 == key2 ? 0.0 : position - key1 / (key2 - key1);

	switch(m_interp) {
	case InterpolationType::linear:
		return lerp(m_values[ids[1]], m_values[ids[2]], t);
	case InterpolationType::cosine:
		return interpCosine(m_values[ids[1]], m_values[ids[2]], t);
	case InterpolationType::quadratic:
		return interpQuadratic(m_values[ids[0]], m_values[ids[1]], m_values[ids[2]],
							   m_values[ids[3]], t);
	case InterpolationType::cubic:
		return interpCubic(m_values[ids[0]], m_values[ids[1]], m_values[ids[2]], m_values[ids[3]],
						   t);
	}

	return T();
}

template struct Curve<float>;
template struct Curve<float2>;
template struct Curve<float3>;
