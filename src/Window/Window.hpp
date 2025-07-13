/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:11:18 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/13 11:41:56 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WINDOW_HPP
# define WINDOW_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Font.hpp"

extern float	SCREEN_WIDTH;
extern float	SCREEN_HEIGHT;

class	Window
{
	public:
		Window();
		~Window();
		bool	up(void);
		void		loopStart(void);
		void		loopEnd();
		void		setIcon(const char *path);
		void		center();
		
		GLFWwindow	*getWindowData(void);
		float		getDeltaTime(void);
		void		setLastMouseX(float val){this->lastMouseX = val;}
		void		setLastMouseY(float val){this->lastMouseY = val;}
		float		getLastMouseX(){return (this->lastMouseX);}
		float		getLastMouseY(){return (this->lastMouseY);}
		void		setDefaultMousePos();
	private:
		float		_deltaTime;
		float		_lastFrame;
		float		_currentFrame;
		GLFWwindow	*_windowData;
		float		lastMouseX = 0;
		float		lastMouseY = 0;
};

#endif
