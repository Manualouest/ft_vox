/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UIElement.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 23:23:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/06/07 23:23:29 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "UIElement.hpp"

unsigned int	UIquadVAO = 0;
unsigned int	UIquadVBO = 0;

bool	UIElement::isInside(glm::vec2 buttonPos, glm::vec2 size, glm::vec2 mousePos)
{
	return mousePos.x >= buttonPos.x && mousePos.x <= buttonPos.x + size.x && mousePos.y >= buttonPos.y && mousePos.y <= buttonPos.y + size.y;
}

void	UIElement::initButtonQuad()
{
	if (UIquadVAO != 0)
		return;

	if (DEBUG)
		std::cout << "Loading simple quad" << std::endl;

	float vertices[] =
	{
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	glGenVertexArrays(1, &UIquadVAO);
	glGenBuffers(1, &UIquadVBO);

	glBindVertexArray(UIquadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, UIquadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

void	UIElement::anchorPos()
{
	if (this->anchor == UIAnchor::UI_TOP_CENTER)
	{
		this->pos.x = SCREEN_WIDTH / 2 - this->size.x / 2 + this->offset.x;
		this->pos.y = 0 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_TOP_LEFT)
	{
		this->pos.x = 0 + this->offset.x;
		this->pos.y = 0 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_TOP_RIGHT)
	{
		this->pos.x = SCREEN_WIDTH - this->size.x + this->offset.x;
		this->pos.y = 0 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_CENTER)
	{
		this->pos.x = SCREEN_WIDTH / 2 - this->size.x / 2 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT / 2 - this->size.y / 2 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_CENTER_LEFT)
	{
		this->pos.x = 0 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT / 2 - this->size.y / 2 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_CENTER_RIGHT)
	{
		this->pos.x = SCREEN_WIDTH - this->size.x + this->offset.x;
		this->pos.y = SCREEN_HEIGHT / 2 - this->size.y / 2 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_BOTTOM_CENTER)
	{
		this->pos.x = SCREEN_WIDTH / 2 - this->size.x / 2 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT - this->size.y + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_BOTTOM_LEFT)
	{
		this->pos.x = 0 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT - this->size.y + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_BOTTOM_RIGHT)
	{
		this->pos.x = SCREEN_WIDTH - this->size.x + this->offset.x;
		this->pos.y = SCREEN_HEIGHT - this->size.y + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_CENTER_HALF_LEFT)
	{
		this->pos.x = (SCREEN_WIDTH * 0.25) - this->size.x / 2 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT / 2 - this->size.y / 2 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_CENTER_HALF_RIGHT)
	{
		this->pos.x = (SCREEN_WIDTH * 0.75) - this->size.x / 2 + this->offset.x;
		this->pos.y = SCREEN_HEIGHT / 2 - this->size.y / 2 + this->offset.y;
	}
	else if (this->anchor == UIAnchor::UI_TOP_CENTER_HALF)
	{
		this->pos.x = (SCREEN_WIDTH / 2) - this->size.x / 2 + this->offset.x;
		this->pos.y = (SCREEN_HEIGHT * 0.25) - this->size.y + this->offset.y;
	}
		else if (this->anchor == UIAnchor::UI_BOTTOM_CENTER_HALF)
	{
		this->pos.x = (SCREEN_WIDTH / 2) - this->size.x / 2 + this->offset.x;
		this->pos.y = (SCREEN_HEIGHT * 0.75) - this->size.y + this->offset.y;
	}
}
