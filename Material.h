#pragma once

class Material {
public:
	virtual vector<float> getConstant() const = 0;
	virtual int getSize() const = 0;
	int matType;
};

class lambertian: public Material {
public:
	lambertian(glm::vec3 color)
	{
		matType = 0;
		albedo = color;
	}
	virtual vector<float> getConstant() const
	{
		vector<float> ret;
		ret.push_back(matType);
		for (int i = 0; i < 3; ++i)
			ret.push_back(albedo[i]);
		return ret;
	}
	virtual int getSize() const
	{
		return 4;
	}
	static int lambertianSize()
	{
		return 4;
	}
	glm::vec3 albedo;
};
class DiffuseAreaLight : public Material {
public:
	DiffuseAreaLight(glm::vec3 color)
	{
		matType = 1;
		albedo = color;
	}
	virtual vector<float> getConstant() const
	{
		vector<float> ret;
		ret.push_back(matType);
		for (int i = 0; i < 3; ++i)
			ret.push_back(albedo[i]);
		return ret;
	}
	virtual int getSize() const
	{
		return 4;
	}
	static int DiffuseAreaLightSize()
	{
		return 4;
	}
	glm::vec3 albedo;
};
class metal : public Material {
public:
	metal(glm::vec3 color)
	{
		matType = 2;
		albedo = color;
	}
	virtual vector<float> getConstant() const
	{
		vector<float> ret;
		ret.push_back(matType);
		for (int i = 0; i < 3; ++i)
			ret.push_back(albedo[i]);
		return ret;
	}
	virtual int getSize() const
	{
		return 4;
	}
	static int metalSize()
	{
		return 4;
	}
	glm::vec3 albedo;
};