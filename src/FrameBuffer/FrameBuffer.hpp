/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FrameBuffer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 11:27:41 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 10:52:29 by mbatty           ###   ########.fr       */
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
		FrameBuffer();
		~FrameBuffer();
		void	resize(float width, float height);
		void	resizeToWindow();
		void	use();
		void	clear();

		/*
			Resets to the default frame buffer to render on screen
		*/
		static void	reset();
		static void	drawFrame(Shader *shader, uint texture);
		
		/*
			Loads a simple quad model to display de buffers on screen
		*/
		static void	loadQuadModel();
		
		uint	getColorexture() {return (colorTex);}
		uint	getDepthTexture() {return (depthTex);}
	private:
		float	width = 0;
		float	height = 0;
		
		uint frameBufferID = 0;
		uint colorTex = 0;
		uint depthTex = 0;
		uint RBO = 0;
};

#endif
