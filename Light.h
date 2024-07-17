#pragma once
#include "Shape.h"

struct LightsBuffer
{
	int lightNum;
	int index[64];
	float data[64 * 13];
};

class Light {
public:
	virtual vector<float> getConstant() const = 0;
	virtual int getSize() const = 0;
	int lightType;
	int lightIndex;
};
class AreaLight :public Light {
public:
	virtual vector<float> getConstant() const
	{
		vector<float> ret;
		ret.push_back(0);
		for (int i = 0; i < 3; ++i)
			ret.push_back(tri->v0[i]);
		for (int i = 0; i < 3; ++i)
			ret.push_back(tri->v1[i]);
		for (int i = 0; i < 3; ++i)
			ret.push_back(tri->v2[i]);
		for (int i = 0; i < 3; ++i)
			ret.push_back(albedo[i]);
		return ret;
	}
	virtual int getSize() const
	{
		return 13;
	}
	shared_ptr<Triangle> tri;
	glm::vec3 albedo;
};