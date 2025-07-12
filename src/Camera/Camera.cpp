/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 20:54:38 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/11 09:34:47 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libs.hpp"
#include "Camera.hpp"

Camera::Camera()
{
	this->yaw = -90.0f;
	this->pitch = 0.0f;
	this->deltaTime = 0.0f;
	this->lastFrame = 0.0f;
	pos = glm::vec3(0.0f, 0.0f, 0.0f);
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void	Camera::update()
{
	this->direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->direction.y = sin(glm::radians(pitch));
	this->direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(direction);
	flatFront = glm::normalize(glm::vec2(cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
}

void	Camera::setViewMatrix(Shader &shader)
{
	glm::mat4	view = this->getViewMatrix();
	glm::mat4	projection = glm::perspective(glm::radians(FOV), SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, RENDER_DISTANCE);

	shader.use();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
}

glm::mat4	Camera::getViewMatrix()
{
	return (glm::lookAt(pos, pos + front, up));
}
