/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FrameBuffer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 11:28:56 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 10:55:30 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "FrameBuffer.hpp"

float quadVertices[] = {
    // positions     // texCoords
    1.0f, -1.0f,      1.0f, 0.0f,
   -1.0f, -1.0f,      0.0f, 0.0f,
   -1.0f,  1.0f,      0.0f, 1.0f,

    1.0f,  1.0f,      1.0f, 1.0f,
    1.0f, -1.0f,      1.0f, 0.0f,
   -1.0f,  1.0f,      0.0f, 1.0f
};

unsigned int FBquadVAO = 0;
unsigned int FBquadVBO = 0;

FrameBuffer::FrameBuffer()
{
	if (DEBUG)
		consoleLog("Creating frame buffer", NORMAL);
	
	glGenFramebuffers(1, &frameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
	
	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

	glGenTextures(1, &depthTex);
	glBindTexture(GL_TEXTURE_2D, depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("FrameBuffer could not finish");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	FrameBuffer::loadQuadModel();
	this->resizeToWindow();
}

void	FrameBuffer::use()
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
	glEnable(GL_DEPTH_TEST);
}

void	FrameBuffer::clear()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void	FrameBuffer::resize(float width, float height)
{
	this->width = width;
	this->height = height;
	
	glBindTexture(GL_TEXTURE_2D, this->colorTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, this->depthTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void	FrameBuffer::resizeToWindow()
{
	int	width,height;
	glfwGetWindowSize(WINDOW->getWindowData(), &width, &height);
	this->resize(width, height);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &frameBufferID);
	glDeleteTextures(1, &colorTex);
	glDeleteTextures(1, &depthTex);
	glDeleteRenderbuffers(1, &RBO);
	if (FBquadVAO != 0)
		glDeleteVertexArrays(1, &FBquadVAO);
	if (FBquadVBO != 0)
		glDeleteBuffers(1, &FBquadVBO);

	FBquadVAO = 0;
	FBquadVBO = 0;
}

void	FrameBuffer::loadQuadModel()
{
	if (FBquadVAO != 0 || FBquadVBO != 0)
		return ;
		
	glGenVertexArrays(1, &FBquadVAO);
	glGenBuffers(1, &FBquadVBO);
	
	glBindVertexArray(FBquadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, FBquadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	glBindVertexArray(0);
}

void	FrameBuffer::reset()
{
	int	width,height;
	glfwGetWindowSize(WINDOW->getWindowData(), &width, &height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);
}

void	FrameBuffer::drawFrame(Shader *shader, uint texture)
{
	loadQuadModel();

	shader->use();
	glBindVertexArray(FBquadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
