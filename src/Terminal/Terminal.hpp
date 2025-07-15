/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Terminal.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 20:02:37 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 11:51:37 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TERMINAL_HPP
# define TERMINAL_HPP

# include "libs.hpp"
# include "Window.hpp"
# include "ShaderManager.hpp"
# include "Font.hpp"
# include "Commands.hpp"

extern bool		PAUSED;
extern Window	*WINDOW;

extern float	SCREEN_WIDTH;
extern float	SCREEN_HEIGHT;
extern uint		seed;

struct Command
{
	Command(std::string command, float time, std::string output)
	{
		this->command = command;
		this->time = time;
		this->output = output;
	}
	std::string	command;
	std::string	output;
	float		time;
};

class	Terminal
{
	public:
		Terminal() {}
		~Terminal() {}
		bool	specialInput(int key, int action)
		{
			(void)action;
			if (!(action == GLFW_PRESS || action == GLFW_REPEAT) || PAUSED)
				return (false);
			if (_active)
			{
				if (key == GLFW_KEY_ESCAPE)
				{
					_clear();
					return (true);
				}
				else if (key == GLFW_KEY_BACKSPACE)
					_deleteOne();
				else if (key == GLFW_KEY_ENTER)
					_execute();
				else if (key == GLFW_KEY_LEFT)
					_moveLeft();
				else if (key == GLFW_KEY_RIGHT)
					_moveRight();
				else if (key == GLFW_KEY_UP)
					_getHistoryUp();
				else if (key == GLFW_KEY_DOWN)
					_getHistoryDown();
				else if (key == GLFW_KEY_END)
					_cursor = _input.end();
				else if (key == GLFW_KEY_HOME)
					_cursor = _input.begin();
			}
			else if (key == GLFW_KEY_T || key == GLFW_KEY_SLASH)
			{
				_active = true;
				_ignoreNext = true;
				_input.clear();
				if (key == GLFW_KEY_SLASH)
					_input = "/";
				_cursor = _input.end();
				glfwSetInputMode(WINDOW->getWindowData(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			return (false);
		}
		void	input(uint key)
		{
			if (PAUSED || !_active)
				return ;

			if (!_ignoreNext)
			{
				if (key >= 32 && key <= 126)
				{
					_cursor = _input.insert(_cursor, 1, (char)key);
					_cursor++;
				}
			}
			_ignoreNext = false;
		}
		void	draw()
		{
			int	i = _history.size();
			
			SHADER_MANAGER->get("text")->setBool("drawBackground", true);
			if (_active)
			{
				std::string	tmp = _input;
				tmp.insert(_cursor - _input.begin(), 1, '_');
				drawLine(tmp, 0);
			}
			for (Command line : _history)
			{
				if (glfwGetTime() - line.time < 2)
				{
					drawLine(line.output, i);
				}
				i--;
			}
			SHADER_MANAGER->get("text")->setBool("drawBackground", false);
		}
		void	drawLine(std::string line, float offset)
		{			
			glm::vec2	size;
			size.x = line.size() * 15;
			size.y = 15;
			
			glm::vec2	pos;
			pos.x = 2;
			pos.y = SCREEN_HEIGHT - size.y - 2 - (offset * 15);

			FONT->putString(line, pos, size);
		}
		bool	isActive() {return (this->_active);}
	private:
		void	_deleteOne()
		{
			if (!_input.size())
				return ;
			if (_cursor - _input.begin() - 1 >= 0)
			{
				_input.erase(_cursor - _input.begin() - 1, 1);
				_cursor--;
			}
		}
		void	_clear()
		{
			_active = false;
			_historyCursor = _history.end();
			_input.clear();	
			WINDOW->setDefaultMousePos();
		}
		void	_execute()
		{
			_active = false;
			_addToHistory(_input, _commands.execute(_input));
			_historyCursor = _history.end();
			_input.clear();
			WINDOW->setDefaultMousePos();
		}
		void	_moveLeft()
		{
			if (_cursor == _input.begin())
				return ;
			_cursor--;
			if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			{
				while (_cursor != _input.begin() && std::isalnum(*_cursor))
					_cursor--;
			}
		}
		void	_moveRight()
		{
			if (_cursor == _input.end())
				return ;

			_cursor++;
			if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			{
				while (_cursor != _input.end() && std::isalnum(*_cursor))
					_cursor++;
			}
		}

		std::vector<Command>			_history;
		std::vector<Command>::iterator	_historyCursor;
		void	_addToHistory(std::string cmd, std::string output)
		{
			_history.push_back(Command(cmd, glfwGetTime(), output));
		}
		void	_getHistoryUp()
		{
			if (_historyCursor == _history.begin())
				return ;
			
			_historyCursor--;
			_input = (*_historyCursor).command;
			_cursor = _input.end();
		}
		void	_getHistoryDown()
		{
			if (_historyCursor != _history.end() && _historyCursor + 1 != _history.end())
			{
				_historyCursor++;
				_input = (*_historyCursor).command;
			}
			else
			{
				_input = "";
				_historyCursor = _history.end();
			}
			_cursor = _input.end();
		}

		bool	_active = false;
		bool	_ignoreNext = false;

		std::string				_input;
		std::string::iterator	_cursor;

		Commands				_commands;
};

#endif
