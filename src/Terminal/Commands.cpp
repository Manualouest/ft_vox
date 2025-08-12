/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 09:51:17 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 11:30:49 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Camera.hpp"

extern uint		seed;
extern Camera	*CAMERA;

void	closeWindow();

std::string	command_seed(std::istringstream &)
{
	return ("world seed is " + std::to_string(seed));
}

std::string	command_tp(std::istringstream &args)
{
	float	x, y, z;
	if (!(args >> x >> y >> z))
		return ("error parsing command: bad args");
	CAMERA->pos = glm::vec3(x, y, z);
	return ("teleported to " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z));
}

std::string	command_quit(std::istringstream &)
{
	closeWindow();
	return ("quitting game");
}

Commands::Commands()
{
	_commands["/seed"] = command_seed;
	_commands["/tp"] = command_tp;
	_commands["/quit"] = command_quit;
	_commands["/q"] = command_quit;
}
