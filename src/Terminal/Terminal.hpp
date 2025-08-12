/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Terminal.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 20:02:37 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 10:41:24 by mbatty           ###   ########.fr       */
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
		/*
			Handles special keys like arrows, up down or opening the terminal
		*/
		bool	specialInput(int key, int action);
		
		/*
			Handles keyboard char input
		*/
		void	input(uint key);
		void	draw();
		
		/*
			Returns wether the terminal is active (opened)
		*/
		bool	isActive() {return (this->_active);}
	private:
		void	_execute();
		
		void	_deleteOne();
		void	_clear();
		
		void	_moveLeft();
		void	_moveRight();

		std::vector<Command>			_history;
		std::vector<Command>::iterator	_historyCursor;
		void	_addToHistory(std::string cmd, std::string output);
		void	_getHistoryUp();
		void	_getHistoryDown();
		
		void	_drawLine(std::string line, float offset);

		bool	_active = false;
		bool	_ignoreNext = false;

		std::string				_input;
		std::string::iterator	_cursor;

		Commands				_commands;
};

#endif
