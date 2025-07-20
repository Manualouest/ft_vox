/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Interface.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 22:01:22 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/20 19:11:20 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERFACE_HPP
# define INTERFACE_HPP

# include "libs.hpp"
# include "UIElement.hpp"

class	Interface
{
	public:
		Interface() {}
		~Interface()
		{
			for (auto &element : _elements)
				delete element.second;
		}

		void	clear()
		{
			for (auto &element : _elements)
				delete element.second;
			_elements.clear();
		}

		void	draw()
		{
			if (_onDraw)
				_onDraw(this);

			for (auto &element : _elements)
				element.second->draw();
		}
		void	update()
		{
			if (_onUpdate)
				_onUpdate(this);

			double mouseX, mouseY;
			bool mousePressed = glfwGetMouseButton(WINDOW->getWindowData(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
			glfwGetCursorPos(WINDOW->getWindowData(), &mouseX, &mouseY);
			for (auto &element : _elements)
				element.second->update(glm::vec2(mouseX, mouseY), mousePressed);
		}

		void	setUpdateFunc(std::function<void(Interface*)> func)
		{
			this->_onUpdate = func;
		}
		void	setDrawFunc(std::function<void(Interface*)> func)
		{
			this->_onDraw = func;
		}

		UIElement	*addElement(const std::string &key, UIElement *elem)
		{
			std::map<std::string, UIElement *>::iterator	finder;

			finder = _elements.find(key);
			if (finder != _elements.end())
			{
				consoleLog("ERROR UIElement already exists in this interface", LogSeverity::ERROR);
				return (finder->second);
			}
			elem->setID(key);
			_elements.insert({key, elem});
			return (elem);
		}
		UIElement	*getElement(const std::string &key)
		{
			std::map<std::string, UIElement *>::iterator	finder;

			finder = _elements.find(key);
			if (finder == _elements.end())
			{
				consoleLog("WARNING UIElement does not exist in this interface", LogSeverity::WARNING);
				return (NULL);
			}
			return (finder->second);
		}
		void	charInput(unsigned int key)
		{
			UIElement	*elementPTR = NULL;

			for (auto &element : _elements)
			{
				elementPTR = element.second;
				if (elementPTR->type == UIElementType::UITYPE_TEXTBOX)
					static_cast<TextBox*>(elementPTR)->charInput(key);
			}
		}
		void	specialInput(int key, int action)
		{
			UIElement	*elementPTR = NULL;

			for (auto &element : _elements)
			{
				elementPTR = element.second;
				if (elementPTR->type == UIElementType::UITYPE_TEXTBOX)
					static_cast<TextBox*>(elementPTR)->specialInput(key, action);
			}
		}
	private:
		std::map<std::string, UIElement *>		_elements;
		std::function<void(Interface*)>			_onUpdate = NULL;
		std::function<void(Interface*)>			_onDraw = NULL;
};

#endif
