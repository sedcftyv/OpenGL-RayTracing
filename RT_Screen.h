#pragma once
#ifndef __RT_Screen_h__
#define __RT_Screen_h__

#include <GL/glew.h>
#include "GLFW/glfw3.h"

const float ScreenVertices[] = {
	//位置坐标(x,y)     //纹理坐标

	//三角形1
	-1.0f, 1.0f,   0.0f, 1.0f, //左上角 
	-1.0f, -1.0f,  0.0f, 0.0f, //左下角
	1.0f, -1.0f,   1.0f, 0.0f, //右下角

	//三角形2
	-1.0f,  1.0f,  0.0f, 1.0f, //左上角
	1.0f, -1.0f,   1.0f, 0.0f, //右下角
	1.0f,  1.0f,   1.0f, 1.0f  //右上角
};

class RT_Screen {
public:
	void InitScreenBind() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ScreenVertices), ScreenVertices, GL_STATIC_DRAW);
		// 位置
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// 纹理
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Unbind VAO
		glBindVertexArray(0);
	}
	void DrawScreen() {
		// 绑定VAO并开始绘制
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	void Delete() {
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}
private:
	unsigned int VBO, VAO;
};

class ScreenFBO {
public:
	ScreenFBO() { }
	void configuration(int SCR_WIDTH, int SCR_HEIGHT) {
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		// 绑定颜色纹理
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
		// 绑定法线纹理
		glGenTextures(1, &textureNormalbuffer);
		glBindTexture(GL_TEXTURE_2D, textureNormalbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureNormalbuffer, 0);
		// 绑定位置纹理
		glGenTextures(1, &texturePositionbuffer);
		glBindTexture(GL_TEXTURE_2D, texturePositionbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texturePositionbuffer, 0);

		glGenTextures(1, &textureIDbuffer);
		glBindTexture(GL_TEXTURE_2D, textureIDbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED_INTEGER, GL_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, textureIDbuffer, 0);

		glGenTextures(1, &texturehistIDbuffer);
		glBindTexture(GL_TEXTURE_2D, texturehistIDbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED_INTEGER, GL_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, texturehistIDbuffer, 0);

		glGenTextures(1, &texturehisIDbuffer);
		glBindTexture(GL_TEXTURE_2D, texturehisIDbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED_INTEGER, GL_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, texturehisIDbuffer, 0);

		GLuint attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5 };
		glDrawBuffers(6, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER)== GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
				std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT " << std::endl;
			else if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
				std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT " << std::endl;
			else if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED)
				std::cout << "GL_FRAMEBUFFER_UNSUPPORTED  " << std::endl;
			else
				std::cout << "error  " << std::endl;
			// 没有正确实现，则报错
		}
		// 绑定到默认FrameBuffer
		unBind();
	}

	void Bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glDisable(GL_DEPTH_TEST);
	}

	void unBind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void BindAsTexture() {
		// 作为第0个纹理
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureNormalbuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texturePositionbuffer);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureIDbuffer);
	}

	void Delete() {
		// 删除
		unBind();
		glDeleteFramebuffers(1, &framebuffer);
		glDeleteTextures(1, &textureColorbuffer);
	}
public:
	// framebuffer配置
	unsigned int framebuffer;
	// 颜色附件纹理
	unsigned int textureColorbuffer;
	// 法线附件纹理
	unsigned int textureNormalbuffer;
	// 位置附件纹理
	unsigned int texturePositionbuffer;
	unsigned int textureIDbuffer;
	unsigned int texturehistIDbuffer;
	unsigned int texturehisIDbuffer;
	// 深度和模板附件的renderbuffer object
	unsigned int rbo;
};


class RenderBuffer {
public:
	void Init(int SCR_WIDTH, int SCR_HEIGHT) {
		fbo[0].configuration(SCR_WIDTH, SCR_HEIGHT);
		fbo[1].configuration(SCR_WIDTH, SCR_HEIGHT);
		fbo[2].configuration(SCR_WIDTH, SCR_HEIGHT);
		fbo[3].configuration(SCR_WIDTH, SCR_HEIGHT);
		fbo[4].configuration(SCR_WIDTH, SCR_HEIGHT);
		currentIndex = 0;
	}
	// fbo[0]为渲染到当前帧的内容
	void setCurrentBuffer(int LoopNum) {
		int histIndex = (LoopNum % 2) ;
		int curIndex = (histIndex == 0 ? 1 : 0);
		//std::cout << LoopNum << std::endl;

		//fbo[curIndex].Bind();
		//fbo[histIndex].BindAsTexture();

		fbo[curIndex + 2].Bind();
		//fbo[0].BindAsTexture();
	}
	void setCurrentAsTexture(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);
		//fbo[curIndex].BindAsTexture();
		fbo[curIndex + 2].BindAsTexture();
		//glActiveTexture(GL_TEXTURE4);
		//glBindTexture(GL_TEXTURE_2D, fbo[histIndex].textureColorbuffer);
		//glActiveTexture(GL_TEXTURE5);
		//glBindTexture(GL_TEXTURE_2D, fbo[histIndex + 2].textureIDbuffer);
	}

	void setDenoiserBuffer(int LoopNum) {
		int histIndex = (LoopNum % 2);
		int curIndex = (histIndex == 0 ? 1 : 0);
		fbo[4].Bind();
	}

	void setDenoiserAsTexture(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);
		fbo[4].BindAsTexture();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fbo[curIndex + 2].texturePositionbuffer);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, fbo[curIndex + 2].textureIDbuffer);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, fbo[histIndex].textureColorbuffer);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, fbo[histIndex + 2].textureIDbuffer);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, fbo[histIndex + 2].texturePositionbuffer);
	}

	void setReprojectionBuffer(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);
		fbo[curIndex].Bind();
	}

	void setReprojectionAsTexture(int LoopNum) {
		int histIndex = LoopNum % 2;
		int curIndex = (histIndex == 0 ? 1 : 0);
		fbo[curIndex].BindAsTexture();
	}

	void Delete() {
		fbo[0].Delete();
		fbo[1].Delete();
	}
private:
	// 用于渲染当前帧的索引
	int currentIndex;
	ScreenFBO fbo[5];
};

#endif
