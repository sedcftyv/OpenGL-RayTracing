#include <GL/glew.h>

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "RT_Screen.h"
#include "Tool.h"
#include "ObjectTexture.h"

#include <windows.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 800;

timeRecord tRecord;

Camera cam(SCR_WIDTH, SCR_HEIGHT);

RenderBuffer screenBuffer;

BVHTree bvhTree;

ObjectTexture ObjTex;

// RayTracerShader 纹理序号：
// 纹理0：Framebuffer
// 纹理1：MeshVertex
// 纹理2：MeshFaceIndex

// ScreenShader 纹理序号：
// 纹理0：Framebuffer

int main()
{
	// GLFW初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 创建GLFW窗口
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// 交互事件
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 窗口捕获鼠标，不显示鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 加载所有的OpenGL函数指针
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// CPU随机数初始化
	CPURandomInit();

	// 加载着色器
	Shader RayTracerShader("RayTracerVertexShader.glsl", "RayTracerFragmentShader.glsl");
	Shader DenoiseShader("DenoiserVertexShader.glsl", "DenoiserFragmentShader.glsl");
	Shader ScreenShader("ScreenVertexShader.glsl", "ScreenFragmentShader.glsl");
	Shader ReprojectionShader("ReprojectionVertexShader.glsl", "ReprojectionFragmentShader.glsl");

	// 绑定屏幕的坐标位置
	RT_Screen screen;
	screen.InitScreenBind();

	// 生成屏幕FrameBuffer
	screenBuffer.Init(SCR_WIDTH, SCR_HEIGHT);

	// 加载数据纹理
	//Model bunny("bunny.obj");
	Model dragon("dragon.obj");
	std::vector<std::shared_ptr<Shape>> primitives;
	std::vector<std::shared_ptr<Light>> lights;
	addCornellbox(primitives, lights, cam);
	//addPrimitives(dragon.meshes, primitives, 0.04, glm::vec3(0.0, -0.2, 0.0));
	getTexture(primitives, lights, RayTracerShader, ObjTex, bvhTree);
	
	//Model box("box.obj");
	//getTexture(box.meshes, RayTracerShader, ObjTex, bvhTree, 0.2, glm::vec3(0.0, 0.0, 0.0));

	//测试BVH树
	//BVHTest(bvhTree, cam);
	bvhTree.releaseAll();

	RayTracerShader.setInt("primitiveDataSize[0]", Triangle::TriangleSize());
	RayTracerShader.setInt("primitiveDataSize[1]", Sphere::SphereSize());
	RayTracerShader.setInt("materialDataSize[0]", lambertian::lambertianSize());
	RayTracerShader.setInt("materialDataSize[1]", DiffuseAreaLight::DiffuseAreaLightSize());
	RayTracerShader.setInt("materialDataSize[2]", metal::metalSize());

	glm::mat4 historySToW = glm::mat4(1.f);
	glm::mat4 historyWToS = glm::mat4(1.f);
	glm::mat4 nowSToW = glm::mat4(1.f);
	glm::mat4 nowWTos = glm::mat4(1.f);
	bool flag = false;
	// 渲染大循环
	while (!glfwWindowShouldClose(window))
	{
		// 计算时间
		tRecord.updateTime();

		// 输入
		processInput(window);


		historyWToS = nowWTos;
		historySToW = nowSToW;

		nowSToW = cam.SToW;
		nowWTos = cam.WToS;

		// 渲染循环加1
		cam.LoopIncrease();

		// 光线追踪渲染当前帧
		{
			// 绑定到当前帧缓冲区
			screenBuffer.setCurrentBuffer(cam.LoopNum);

			// MeshTex赋值
			ObjTex.setTex(RayTracerShader);

			// 激活着色器
			RayTracerShader.use();

			// 相机参数赋值
			RayTracerShader.setVec3("camera.camPos", cam.cameraPos);
			RayTracerShader.setVec3("camera.front", cam.cameraFront);
			RayTracerShader.setVec3("camera.right", cam.cameraRight);
			RayTracerShader.setVec3("camera.up", cam.cameraUp);
			RayTracerShader.setFloat("camera.halfH", cam.halfH);
			RayTracerShader.setFloat("camera.halfW", cam.halfW);
			RayTracerShader.setVec3("camera.leftbottom", cam.LeftBottomCorner);
			RayTracerShader.setInt("camera.LoopNum", cam.LoopNum);
			RayTracerShader.setMat4("camera.SToW", nowSToW);
			RayTracerShader.setMat4("camera.WToS", nowWTos);
			RayTracerShader.setInt("screenWidth", SCR_WIDTH);
			RayTracerShader.setInt("screenHeight", SCR_HEIGHT);

			// 随机数初值赋值
			RayTracerShader.setFloat("randOrigin", 874264.0f * (GetCPURandom() + 1.0f));
			// 渲染FrameBuffer
			screen.DrawScreen();
		}

		{
			DenoiseShader.use();
			screenBuffer.setCurrentAsTexture(cam.LoopNum);
			screenBuffer.setDenoiserBuffer(cam.LoopNum);
			DenoiseShader.setInt("screenTexture", 0);
			DenoiseShader.setInt("screenNormal", 1);
			DenoiseShader.setInt("screenPosition", 2);
			DenoiseShader.setInt("screenWidth", SCR_WIDTH);
			DenoiseShader.setInt("screenHeight", SCR_HEIGHT);
			DenoiseShader.setInt("camera.LoopNum", cam.LoopNum);
			// 降噪
			screen.DrawScreen();
			
		}

		{

			ReprojectionShader.use();
			screenBuffer.setDenoiserAsTexture(cam.LoopNum);
			screenBuffer.setReprojectionBuffer(cam.LoopNum);
			ReprojectionShader.setInt("screenTexture", 0);
			ReprojectionShader.setInt("screenPosition", 2);
			ReprojectionShader.setInt("screenID", 3);
			ReprojectionShader.setInt("historyTexture", 4);
			ReprojectionShader.setInt("historyID", 5);
			ReprojectionShader.setInt("historyPosition", 6);
			ReprojectionShader.setInt("screenWidth", SCR_WIDTH);
			ReprojectionShader.setInt("screenHeight", SCR_HEIGHT);
			ReprojectionShader.setInt("camera.LoopNum", cam.LoopNum);
			ReprojectionShader.setBool("flag", flag);
			ReprojectionShader.setMat4("nowWToS", nowWTos);
			ReprojectionShader.setMat4("historyWToS", historyWToS);
			ReprojectionShader.setMat4("nowSToW", nowSToW);
			ReprojectionShader.setMat4("historySToW", historySToW);
			// 降噪
			screen.DrawScreen();
		}
		// 渲染到默认Buffer上
		{
			// 绑定到默认缓冲区
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// 清屏
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ScreenShader.use();
			screenBuffer.setReprojectionAsTexture(cam.LoopNum);
			ScreenShader.setInt("screenTexture", 0);
			ScreenShader.setInt("screenWidth", SCR_WIDTH);
			ScreenShader.setInt("screenHeight", SCR_HEIGHT);
			// 绘制屏幕
			screen.DrawScreen();
		}

		// 交换Buffer
		glfwSwapBuffers(window);
		glfwPollEvents();

		flag = true;

	}

	// 条件终止
	glfwTerminate();

	// 释放资源
	screenBuffer.Delete();
	screen.Delete();

	return 0;
}

// 按键处理
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.ProcessKeyboard(FORWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.ProcessKeyboard(BACKWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.ProcessKeyboard(LEFT, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.ProcessKeyboard(RIGHT, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cam.ProcessKeyboard(UP, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cam.ProcessKeyboard(DOWN, tRecord.deltaTime);
}

// 处理窗口尺寸变化
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	cam.updateScreenRatio(SCR_WIDTH, SCR_HEIGHT);
	glViewport(0, 0, width, height);
}

// 鼠标事件响应
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	cam.updateCameraFront(xpos, ypos);
}

// 设置fov
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cam.updateFov(static_cast<float>(yoffset));
}





