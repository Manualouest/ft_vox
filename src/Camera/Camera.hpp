/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Camera.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 20:55:06 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/28 23:25:10 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CAMERA_HPP
# define CAMERA_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Window.hpp"

extern float	FOV;
extern float	RENDER_DISTANCE;
extern float	SCREEN_WIDTH;
extern float	SCREEN_HEIGHT;
extern Window	*WINDOW;

class Camera
{
	public:
		Camera();
		void		update();
		glm::mat4	getViewMatrix();
		void		setViewMatrix(Shader &shader);

		float	getSpeed()
		{
			return (speed);
		}

		glm::vec3	direction;
		glm::vec3	pos;
		glm::vec3	lastPos = glm::vec3(0);
		glm::vec3	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3	front;
		glm::vec3	up;
		glm::vec3	right;
		float	deltaTime;
		float	lastFrame;
		float	yaw;
		float	pitch;
		float	speed;
};

#endif
