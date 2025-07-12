/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FrameBuffer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 11:27:41 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/11 19:13:17 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRAMEBUFFER_HPP
# define FRAMEBUFFER_HPP

#include "libs.hpp"
#include "Window.hpp"
#include "Shader.hpp"
extern float SCREEN_WIDTH;
extern float SCREEN_HEIGHT;
extern Window *WINDOW;

extern uint quadVAO;
extern uint quadVBO;

class	FrameBuffer
{
	public:
		FrameBuffer()
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
		~FrameBuffer()
		{
			glDeleteFramebuffers(1, &frameBufferID);
			glDeleteTextures(1, &colorTex);
			glDeleteTextures(1, &depthTex);
			glDeleteRenderbuffers(1, &RBO);
			if (quadVAO != 0)
				glDeleteVertexArrays(1, &quadVAO);
			if (quadVBO != 0)
				glDeleteBuffers(1, &quadVBO);

			quadVAO = 0;
			quadVBO = 0;
		}
		/*
			resizes the frame buffer, will also resize viewport when calling use() so dont forget to use reset
		*/
		void	resize(float width, float height)
		{
			this->width = width;
			this->height = height;
			
			glBindTexture(GL_TEXTURE_2D, this->colorTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, this->depthTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
			
			glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		void	resizeToWindow()
		{
			int	width,height;
			glfwGetWindowSize(WINDOW->getWindowData(), &width, &height);
			this->width = width;
			this->height = height;
		
			glBindTexture(GL_TEXTURE_2D, this->colorTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, this->depthTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		
			glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}
		void	use()
		{
			SCREEN_WIDTH = width;
			SCREEN_HEIGHT = height;
			glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
			glEnable(GL_DEPTH_TEST);
		}
		void	clear()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, this->frameBufferID);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		//Resets back to the main frame (The one drawn on screen)
		static void	reset()
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
		static void	loadQuadModel();
		//Will draw the texture using the shader on the currently bound frame.
		static void	drawFrame(Shader *shader, uint texture)
		{
			loadQuadModel();

			shader->use();
			glBindVertexArray(quadVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		uint	getColorexture()
		{
			return (colorTex);
		}
		uint	getDepthTexture()
		{
			return (depthTex);
		}
	private:
		float	width = 0;
		float	height = 0;
		uint frameBufferID = 0;
		uint colorTex = 0;
		uint depthTex = 0;
		uint RBO = 0;
};

#endif
