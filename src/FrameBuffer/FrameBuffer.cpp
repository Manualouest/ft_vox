/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FrameBuffer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/05 11:28:56 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/05 11:31:18 by mbatty           ###   ########.fr       */
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

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

void	FrameBuffer::loadQuadModel()
{
	if (quadVAO != 0 || quadVBO != 0)
		return ;
		
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	glBindVertexArray(0);
}
