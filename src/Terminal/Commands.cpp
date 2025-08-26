/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 09:51:17 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/26 20:35:22 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Commands.hpp"
#include "Camera.hpp"
#include "Skybox.hpp"

extern Skybox	*SKYBOX;
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
		return ("error: parsing command: bad args");
	CAMERA->pos = glm::vec3(x, y, z);
	return ("teleported to " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z));
}

std::string	command_quit(std::istringstream &)
{
	closeWindow();
	return ("quitting game");
}

#include "RegionManager.hpp"
extern RegionManager	*CHUNKS;

std::string	command_getblock(std::istringstream &args)
{
	int	x, y, z;
	if (!(args >> x >> y >> z))
		return ("error: parsing command: bad args");

	Chunk	*currentChunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(x, z));
	if (currentChunk && !currentChunk->getGenerating() && currentChunk->getState() >= ChunkState::CS_GENERATED)
	{
		GenInfo	currentBlock = currentChunk->getBlock(x, y, z);
		return ("block at " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " is: " + std::to_string((int)currentBlock.type));
	}
	else
		return ("error: chunk is not loaded");
}

std::string	command_settime(std::istringstream &args)
{
	float	time;
	if (!(args >> time))
		return ("error: parsing command: bad args");

	if (time < 0 || time > 1000)
		return ("error: invalid time: please put a value between 0 and 1000");
	
	float tpTime = time - 250;
	if (tpTime < 0)
		tpTime += 1000;

	SKYBOX->setTime(tpTime);

	return ("time changed to " + std::to_string(time));
}

Commands::Commands()
{
	_commands["/seed"] = command_seed;
	_commands["/tp"] = command_tp;
	_commands["/quit"] = command_quit;
	_commands["/q"] = command_quit;
	_commands["/getblock"] = command_getblock;
	_commands["/settime"] = command_settime;
}
