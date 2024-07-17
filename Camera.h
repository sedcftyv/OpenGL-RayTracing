#pragma once
#ifndef __Camera_h__
#define __Camera_h__

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
public:
	Camera(int ScreenWidth, int ScreenHeight) {
		LoopNum = 0;
		Yaw = -90.0f;
		Pitch = 0.0f;
		fov = 22.5f;
		n = 1;
		f = 1000;
		firstMouse = true;
		cameraPos = glm::vec3(0.0f, 0.0f, 1.5f);
		worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		//ScreenRatio = (float)ScreenWidth / (float)ScreenHeight;
		updateScreenRatio(ScreenWidth, ScreenHeight);
		updateFov(0);
		//halfH = glm::tan(glm::radians(fov));
		//halfW = halfH * ScreenRatio;
		cameraSpeed = 10.0f;
		updateCameraVectors();
	}

	void updateCameraVectors() {
		cameraFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		cameraFront.y = sin(glm::radians(Pitch));
		cameraFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		cameraFront = glm::normalize(cameraFront);

		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));

		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		//LoopNum = 0;

		V = glm::mat4(1.0f);
		//V = glm::translate(V, -cameraPos);
		V[0] = glm::vec4(cameraRight,0);
		V[1] = glm::vec4(cameraUp, 0);
		V[2] = glm::vec4(cameraFront, 0);
		V[3] = glm::vec4(cameraPos, 1);
		WToS = PToS * P * glm::inverse(V);
		SToW = glm::inverse(WToS);
	}
	void updateFov(float offset) {
		fov -= (float)offset;
		if (fov < 1.0f)
			fov = 1.0f;
		if (fov > 45.0f)
			fov = 45.0f;

		halfH = glm::tan(glm::radians(fov));
		halfW = halfH * ScreenRatio;

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		//LoopNum = 0;

		P = glm::mat4(1.0f);
		P[0][0] = 1.f / halfH;
		P[1][1] = 1.f / halfH;
		P[2][2] = f / (f - n);
		P[3][3] = 0;
		P[2][3] = 1;
		P[3][2] = -f * n / (f - n);
		WToS = PToS * P * glm::inverse(V);
		SToW = glm::inverse(WToS);
	}
	void updateCameraFront(float xpos, float ypos) {
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.03f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// 保证pitch小于90度
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		updateCameraVectors();
	}
	void updateScreenRatio(int ScreenWidth, int ScreenHeight) {
		ScreenRatio = (float)ScreenWidth / (float)ScreenHeight;
		halfW = halfH * ScreenRatio;

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		//LoopNum = 0;

		glm::mat4 tmp1 = glm::mat4(1.0f);
		tmp1[3][0] = ScreenRatio;
		tmp1[3][1] = 1;
		glm::mat4 tmp2 = glm::mat4(1.0f);
		tmp2[0][0] = 0.5 / ScreenRatio;
		tmp2[1][1] = 0.5;
		PToS = tmp2 * tmp1;
		WToS = PToS * P * glm::inverse(V);
		SToW = glm::inverse(WToS);
	}
	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = cameraSpeed * deltaTime;
		if (direction == FORWARD)
			cameraPos += cameraFront * velocity;
		if (direction == BACKWARD)
			cameraPos -= cameraFront * velocity;
		if (direction == LEFT)
			cameraPos -= cameraRight * velocity;
		if (direction == RIGHT)
			cameraPos += cameraRight * velocity;
		if (direction == UP)
			cameraPos += worldUp * velocity;
		if (direction == DOWN)
			cameraPos -= worldUp * velocity;

		V[3] = glm::vec4(cameraPos, 1);
		WToS = PToS * P * glm::inverse(V);
		SToW = glm::inverse(WToS);

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		//LoopNum = 0;
	}
	void LoopIncrease() {
		LoopNum++;
	}

public:
	glm::vec3 cameraPos;
	// 相机方向
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	// 世界方向
	glm::vec3 worldUp;
	glm::mat4 V;
	glm::mat4 P;
	glm::mat4 PToS;
	glm::mat4 WToS;
	glm::mat4 SToW;
	float n,f;
	float fov;
	float Pitch;
	float Yaw;
	// 相机移动速度
	float cameraSpeed;
	// 鼠标交互相关
	bool firstMouse;
	float lastX;
	float lastY;
	// 屏幕长宽比
	float ScreenRatio;
	float halfH;
	float halfW;
	glm::vec3 LeftBottomCorner;
	// 渲染的轮数
	int LoopNum;
};


#include <GL/glew.h>
#include "GLFW/glfw3.h"

class timeRecord {
public:
	timeRecord() {
		lastFrameTime = 0.0f;
	}

	void updateTime() {
		currentFrameTime = static_cast<float>(glfwGetTime());
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
	}

	float currentFrameTime;
	float lastFrameTime;
	float deltaTime;

};






#endif



