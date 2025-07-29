/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Terminal.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:35:00 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/29 18:22:10 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Terminal.hpp"

bool	Terminal::specialInput(int key, int action)
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

void	Terminal::input(uint key)
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

void	Terminal::draw()
{
	int	i = _history.size() + 1;

	if (_active)
	{
		std::string	tmp = _input;
		tmp.insert(_cursor - _input.begin(), 1, '_');
		_drawLine(tmp, 1);
	}
	for (Command line : _history)
	{
		if (glfwGetTime() - line.time < 2)
		{
			_drawLine(line.output, i);
		}
		i--;
	}
}

void	Terminal::_drawLine(std::string line, float offset)
{
	glm::vec2	pos;
	pos.x = 0;
	pos.y = SCREEN_HEIGHT - (offset * 16);

	FONT->putString(line, pos, {1, 1}, glm::vec3(1), true, false);
}

void	Terminal::_deleteOne()
{
	if (!_input.size())
		return ;
	if (_cursor - _input.begin() - 1 >= 0)
	{
		_input.erase(_cursor - _input.begin() - 1, 1);
		_cursor--;
	}
}

void	Terminal::_clear()
{
	_active = false;
	_historyCursor = _history.end();
	_input.clear();
	WINDOW->setDefaultMousePos();
}

void	Terminal::_execute()
{
	_active = false;
	_addToHistory(_input, _commands.execute(_input));
	_historyCursor = _history.end();
	_input.clear();
	WINDOW->setDefaultMousePos();
}

void	Terminal::_moveLeft()
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

void	Terminal::_moveRight()
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

void	Terminal::_addToHistory(std::string cmd, std::string output)
{
	_history.push_back(Command(cmd, glfwGetTime(), output));
}

void	Terminal::_getHistoryUp()
{
	if (_historyCursor == _history.begin())
		return ;

	_historyCursor--;
	_input = (*_historyCursor).command;
	_cursor = _input.end();
}

void	Terminal::_getHistoryDown()
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
