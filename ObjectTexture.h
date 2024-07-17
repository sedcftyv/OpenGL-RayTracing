#pragma once
#ifndef __ObjectTexture_h__
#define __ObjectTexture_h__


#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "Mesh.h"
#include "Shader.h"
#include "Shape.h"
#include "BVHTree.h"
#include "Light.h"

#include <cmath>
#include <vector>

class ObjectTexture {
public:
	GLuint ID_meshTex;
	GLuint ID_bvhNodeTex;
	//int meshNum, meshFaceNum;

	void setTex(Shader& shader) {

		//shader.setInt("meshNum", meshNum);
		//shader.setInt("bvhNodeNum", meshFaceNum);

		glActiveTexture(GL_TEXTURE0 + 1);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, ID_meshTex);

		// 激活并绑定纹理
		glActiveTexture(GL_TEXTURE0 + 2);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, ID_bvhNodeTex);

		shader.setInt("texMesh", 1);
		shader.setInt("texBvhNode", 2);
	}

};


void addCornellbox(std::vector<std::shared_ptr<Shape>>& primitives, std::vector<std::shared_ptr<Light>>& lights, Camera &cam)
{
	cam.LoopNum = 0;
	cam.Yaw = 90.0f;
	cam.Pitch = 0.0f;
	cam.fov = 20.f;
	cam.firstMouse = true;
	cam.cameraPos = glm::vec3(2.5f, 2.5f, -11.85f);
	cam.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	//cam.ScreenRatio = (float)1200 / (float)800;
	//cam.halfH = glm::tan(glm::radians(cam.fov));
	//cam.halfW = cam.halfH * cam.ScreenRatio;
	cam.cameraSpeed = 10.0f;
	cam.updateFov(0);
	cam.updateScreenRatio(1200, 800);
	//cam.updateFov(0);
	cam.updateCameraVectors();

	glm::mat4 mat = glm::mat4(1.0f);
	float length_Floor = 5;
	const float yPos_AreaLight = length_Floor - 0.00001f-0.1f;
	float len = 0.5;
	int nTrianglesAreaLight = 2;

	glm::vec3 P_AreaLight[6] = {
			glm::vec3(-len,yPos_AreaLight,len),glm::vec3(-len,yPos_AreaLight,-len),glm::vec3(len,yPos_AreaLight,len),
			glm::vec3(len,yPos_AreaLight,len),glm::vec3(-len,yPos_AreaLight,-len),glm::vec3(len,yPos_AreaLight,-len) };
	mat = glm::scale(mat, glm::vec3(1.f, 1.0f, -1.f));
	mat = glm::translate(mat, glm::vec3(2.5f, 0.0f, 2.5f));
	for (int i = 0; i < nTrianglesAreaLight; ++i) {
		std::shared_ptr<Triangle> tri = std::make_shared<Triangle>();
		std::shared_ptr<AreaLight> arealight = std::make_shared<AreaLight>();
		tri->v0 = mat * glm::vec4(P_AreaLight[i * 3], 1.f);
		tri->v1 = mat * glm::vec4(P_AreaLight[i * 3 + 1], 1.f);
		tri->v2 = mat * glm::vec4(P_AreaLight[i * 3 + 2], 1.f);
		tri->mat = make_shared<DiffuseAreaLight>(glm::vec3(20.f, 20.f, 20.f));
		arealight->tri = tri;
		arealight->albedo = glm::vec3(20.f, 20.f, 20.f);
		primitives.push_back(tri);
		lights.push_back(arealight);		
	}

	const int nTrianglesFloor = 10;
	const int nVerticesFloor = nTrianglesFloor * 3;
	glm::vec3 P_Floor[nVerticesFloor] = {
				glm::vec3(0.f,0.f,length_Floor),glm::vec3(length_Floor,0.f,length_Floor),glm::vec3(0.f,0.f,0.f),
				glm::vec3(length_Floor,0.f,length_Floor),glm::vec3(length_Floor,0.f,0.f),glm::vec3(0.f,0.f,0.f),
				glm::vec3(0.f,length_Floor,length_Floor),glm::vec3(0.f,length_Floor,0.f),glm::vec3(length_Floor,length_Floor,length_Floor),
				glm::vec3(length_Floor,length_Floor,length_Floor),glm::vec3(0.f,length_Floor,0.f),glm::vec3(length_Floor,length_Floor,0.f),
				glm::vec3(0.f,0.f,0.f),glm::vec3(length_Floor,0.f,0.f),glm::vec3(length_Floor,length_Floor,0.f),
				glm::vec3(0.f,0.f,0.f),glm::vec3(length_Floor,length_Floor,0.f),glm::vec3(0.f,length_Floor,0.f),
				glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,length_Floor,length_Floor),glm::vec3(0.f,0.f,length_Floor),
				glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,length_Floor,0.f),glm::vec3(0.f,length_Floor,length_Floor),
				glm::vec3(length_Floor,0.f,0.f),glm::vec3(length_Floor,length_Floor,length_Floor),glm::vec3(length_Floor,0.f,length_Floor),
				glm::vec3(length_Floor,0.f,0.f),glm::vec3(length_Floor,length_Floor,0.f),glm::vec3(length_Floor,length_Floor,length_Floor) };
	mat = glm::mat4(1.0f);
	mat = glm::scale(mat, glm::vec3(1.f, 1.0f, -1.f));
	for (int i = 0; i < nTrianglesFloor; ++i)
	{
		std::shared_ptr<Triangle> tri = std::make_shared<Triangle>();
		tri->v0 = mat * glm::vec4(P_Floor[i * 3], 1.f);
		tri->v1 = mat * glm::vec4(P_Floor[i * 3 + 1], 1.f);
		tri->v2 = mat * glm::vec4(P_Floor[i * 3 + 2], 1.f);
		glm::vec3 green(.12f, .45f, .15f);
		glm::vec3 red(.65f, .05f, .05f);
		glm::vec3 white(.73f, .73f, .73f);
		if (i < 2)
			tri->mat = make_shared<lambertian>(white);
		else if (i < 4)
			tri->mat = make_shared<lambertian>(white);
		else if (i < 6)
			tri->mat = make_shared<lambertian>(white);
		else if (i < 8)
			tri->mat = make_shared<lambertian>(red);
		else if (i < 10)
			tri->mat = make_shared<lambertian>(green);
		primitives.push_back(tri);
	}

	const int nTrianglesCube = 12;
	const int nVerticesCube = nTrianglesCube * 3;
	int vertexIndicesCube[nVerticesCube];
	for (int i = 0; i < nVerticesCube; ++i)
		vertexIndicesCube[i] = i;
	float height = 1.2f;
	height = 1.486f;
	glm::vec3 P_Cube[nVerticesCube] = {
			glm::vec3(0.f,0.f,height),glm::vec3(height,0.f,height),glm::vec3(0.f,0.f,0.f),
			glm::vec3(height,0.f,height),glm::vec3(height,0.f,0.f),glm::vec3(0.f,0.f,0.f),
			glm::vec3(0.f,height,height),glm::vec3(0.f,height,0.f),glm::vec3(height,height,height),
			glm::vec3(height,height,height),glm::vec3(0.f,height,0.f),glm::vec3(height,height,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(height,0.f,0.f),glm::vec3(height,height,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(height,height,0.f),glm::vec3(0.f,height,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,height,height),glm::vec3(0.f,0.f,height),
			glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,height,0.f),glm::vec3(0.f,height,height),
			glm::vec3(height,0.f,0.f),glm::vec3(height,height,height),glm::vec3(height,0.f,height),
			glm::vec3(height,0.f,0.f),glm::vec3(height,height,0.f),glm::vec3(height,height,height),
			glm::vec3(0.f,0.f,height),glm::vec3(height,0.f,height),glm::vec3(height,height,height),
			glm::vec3(0.f,0.f,height),glm::vec3(height,height,height),glm::vec3(0.f,height,height),
	};

	mat = glm::mat4(1.0f);
	mat = glm::scale(mat, glm::vec3(1.f, 1.0f, -1.f));
	mat = glm::translate(mat, glm::vec3(0.7f, 0.f, 2.8f));
	mat = glm::rotate(mat, glm::radians(18.0f), glm::vec3(0.0, 1.0, 0.0));
	for (int i = 0; i < nTrianglesCube; ++i)
	{
		std::shared_ptr<Triangle> tri = std::make_shared<Triangle>();
		tri->v0 = mat * glm::vec4(P_Cube[i * 3], 1.f);
		tri->v1 = mat * glm::vec4(P_Cube[i * 3 + 1], 1.f);
		tri->v2 = mat * glm::vec4(P_Cube[i * 3 + 2], 1.f);
		glm::vec3 white(.73f, .73f, .73f);
		tri->mat = make_shared<lambertian>(white);
		primitives.push_back(tri);
	}

	float radius = 2.25;
	std::shared_ptr<Sphere> sph = std::make_shared<Sphere>();
	sph->pos = glm::vec3(radius / 2.f, radius / 2.f, radius / 2.f) + glm::vec3(2.6f, 0.f, 1.0f);
	sph->pos.z *= -1;
	sph->r = radius / 2.f;
	sph->mat = make_shared<metal>(glm::vec3(1.f, 1.f, 1.f));
	//primitives.push_back(sph);



	const int nTrianglesRect = 12;
	const int nVerticesRect = nTrianglesRect * 3;
	float width = height * 2;
	glm::vec3 P_Rect[nVerticesRect] = {
			glm::vec3(0.f,0.f,height),glm::vec3(height,0.f,height),glm::vec3(0.f,0.f,0.f),
			glm::vec3(height,0.f,height),glm::vec3(height,0.f,0.f),glm::vec3(0.f,0.f,0.f),//底
			glm::vec3(0.f,width,height),glm::vec3(0.f,width,0.f),glm::vec3(height,width,height),
			glm::vec3(height,width,height),glm::vec3(0.f,width,0.f),glm::vec3(height,width,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(height,0.f,0.f),glm::vec3(height,width,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(height,width,0.f),glm::vec3(0.f,width,0.f),
			glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,width,height),glm::vec3(0.f,0.f,height),
			glm::vec3(0.f,0.f,0.f),glm::vec3(0.f,width,0.f),glm::vec3(0.f,width,height),
			glm::vec3(height,0.f,0.f),glm::vec3(height,width,height),glm::vec3(height,0.f,height),
			glm::vec3(height,0.f,0.f),glm::vec3(height,width,0.f),glm::vec3(height,width,height),
			glm::vec3(0.f,0.f,height),glm::vec3(height,0.f,height),glm::vec3(height,width,height),
			glm::vec3(0.f,0.f,height),glm::vec3(height,width,height),glm::vec3(0.f,width,height),
	};

	mat = glm::mat4(1.0f);
	mat = glm::scale(mat, glm::vec3(1.f, 1.0f, -1.f));
	//mat = glm::translate(mat, glm::vec3(2.7f, 0.f, 0.8f));
	mat = glm::translate(mat, glm::vec3(2.7f, 0.f, 1.f));
	mat = glm::rotate(mat, glm::radians(-15.f), glm::vec3(0.0, 1.0, 0.0));
	for (int i = 0; i < nTrianglesRect; ++i) 
	{
		std::shared_ptr<Triangle> tri = std::make_shared<Triangle>();
		tri->v0 = mat * glm::vec4(P_Rect[i * 3], 1.f);
		tri->v1 = mat * glm::vec4(P_Rect[i * 3 + 1], 1.f);
		tri->v2 = mat * glm::vec4(P_Rect[i * 3 + 2], 1.f);
		glm::vec3 white(.73f, .73f, .73f);
		if(i<12)
			tri->mat = make_shared<lambertian>(white);
		else
			tri->mat = make_shared<metal>(glm::vec3(1.f, 1.f, 1.f));
		primitives.push_back(tri);
	}
}

void addPrimitives(std::vector<Mesh>& data, std::vector<std::shared_ptr<Shape>> &primitives, float Scale = 1.0f, glm::vec3 bias = glm::vec3(0.0f))
{
	int dataSize_v = 0, dataSize_f = 0;
	for (int i = 0; i < data.size(); i++) {
		// 累加每个Mesh的size
		dataSize_v += data[i].vertices.size();
		dataSize_f += data[i].indices.size();
	}
	std::cout << "dataSize_t = " << dataSize_v << std::endl;
	std::cout << "dataSize_f = " << dataSize_f << std::endl;

	// 生成每个三角形
	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].indices.size() / 3; j++) {
			std::shared_ptr<Triangle> tri = std::make_shared<Triangle>();
			tri->v0 = Scale * data[i].vertices[data[i].indices[j * 3 + 0]].Position + bias;
			tri->v1 = Scale * data[i].vertices[data[i].indices[j * 3 + 1]].Position + bias;
			tri->v2 = Scale * data[i].vertices[data[i].indices[j * 3 + 2]].Position + bias;
			tri->mat = make_shared<lambertian>(glm::vec3(0.83, 0.73, 0.1));
			primitives.push_back(tri);
		}
	}

	float floorHfW = 1.0, upBias = -0.22;
	std::shared_ptr<Triangle> triFloor0 = std::make_shared<Triangle>();
	triFloor0->v0 = glm::vec3(-floorHfW, upBias, floorHfW);
	triFloor0->v1 = glm::vec3(-floorHfW, upBias, -floorHfW);
	triFloor0->v2 = glm::vec3(floorHfW, upBias, floorHfW);
	//triFloor0->mat = make_shared<lambertian>(glm::vec3(0.87, 0.87, 0.87));
	triFloor0->mat = make_shared<metal>(glm::vec3(0.87, 0.87, 0.87));
	primitives.push_back(triFloor0);
	std::shared_ptr<Triangle> triFloor1 = std::make_shared<Triangle>();
	triFloor1->v0 = glm::vec3(floorHfW, upBias, floorHfW);
	triFloor1->v1 = glm::vec3(-floorHfW, upBias, -floorHfW);
	triFloor1->v2 = glm::vec3(floorHfW, upBias, -floorHfW);
	//triFloor1->mat = make_shared<lambertian>(glm::vec3(0.87, 0.87, 0.87));
	triFloor1->mat = make_shared<metal>(glm::vec3(0.87, 0.87, 0.87));
	primitives.push_back(triFloor1);

	std::shared_ptr<Sphere> sph = std::make_shared<Sphere>();
	sph->pos = glm::vec3(0, 0, -1);
	sph->r = 0.25;
	sph->mat = make_shared<lambertian>(glm::vec3(0.83, 0.73, 0.1));
	primitives.push_back(sph);

	std::cout << "primitives.size():" << primitives.size() << std::endl;

}

void getTexture(std::vector<std::shared_ptr<Shape>>& primitives, std::vector<std::shared_ptr<Light>>& lights, Shader& shader, ObjectTexture& objTex, BVHTree& bvhTree) {

	// 构建BVH树
	bvhTree.BVHBuildTree(primitives);

	// 绑定到纹理中
	shader.use();

	glGenTextures(1, &objTex.ID_meshTex);
	glBindTexture(GL_TEXTURE_2D, objTex.ID_meshTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, bvhTree.meshNumX, bvhTree.meshNumY, 0, GL_RED, GL_FLOAT, bvhTree.MeshArray);
	// 最近邻插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 绑定纹理
	shader.setInt("texMesh", 1);

	glGenTextures(1, &objTex.ID_bvhNodeTex);
	glBindTexture(GL_TEXTURE_2D, objTex.ID_bvhNodeTex); //dataSize_f
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, bvhTree.nodeNumX, bvhTree.nodeNumY, 0, GL_RED, GL_FLOAT, bvhTree.NodeArray);
	// 最近邻插值
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// 绑定纹理
	shader.setInt("texBvhNode", 2);


	LightsBuffer* lightsbuffer = new LightsBuffer;
	int cnt = 0;
	lightsbuffer->lightNum = lights.size();
	for (int i = 0; i < lights.size(); ++i)
	{
		lightsbuffer->index[i] = cnt;
		vector<float> arealightData = lights[i]->getConstant();
		for (int j = 0; j < arealightData.size(); ++j)
		{
			lightsbuffer->data[cnt] = arealightData[j];
			cnt++;
		}
	}

	//std::cout << sizeof(LightsBuffer) << std::endl;
	GLuint ssboBlock;
	glGenBuffers(1, &ssboBlock);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboBlock);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightsBuffer), lightsbuffer, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboBlock);
}





#endif






