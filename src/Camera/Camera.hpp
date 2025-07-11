/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 20:55:06 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/11 12:28:39 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAMERA_HPP
# define CAMERA_HPP

# include "libs.hpp"
# include "Shader.hpp"

extern float	FOV;
extern float	RENDER_DISTANCE;
extern float	SCREEN_WIDTH;
extern float	SCREEN_HEIGHT;

class Camera
{
	public:
		Camera();
		void		update();
		glm::mat4	getViewMatrix();
		void		setViewMatrix(Shader &shader);
		
		glm::vec3	direction;
		glm::vec3	pos;
		glm::vec3	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3	front;
		glm::vec3	up;
		glm::vec3	right;
		glm::vec2	flatFront;
		float	deltaTime;
		float	lastFrame;
		float	yaw;
		float	pitch;
};

#endif
