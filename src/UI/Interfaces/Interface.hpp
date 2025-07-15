/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Interface.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 22:01:22 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/14 19:32:30 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERFACE_HPP
# define INTERFACE_HPP

# include "libs.hpp"
# include "UIElement.hpp"

class	Interface
{
	public:
		Interface(){}
		~Interface()
		{
			for (UIElement *element : elements)
				delete element;
		}

		void	draw()
		{
			for (UIElement *element : elements)
				element->draw();
		}
		void	update()
		{
			double mouseX, mouseY;
			bool mousePressed = glfwGetMouseButton(WINDOW->getWindowData(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			glfwGetCursorPos(WINDOW->getWindowData(), &mouseX, &mouseY);
			for (UIElement *element : elements)
				element->update(glm::vec2(mouseX, mouseY), mousePressed);
		}
		std::vector<UIElement *>	elements;
		void						*updateData = NULL;
		void						*drawData = NULL;
};

#endif
