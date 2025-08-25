/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:55:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/25 10:43:10 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGenerator.hpp"

bool	ChunkGenerator::deposit(std::vector<Chunk *> chunks)
{
	LOCK(_depositMutex);

	if (isWorking() || _deposit.size() > 0)
		return (false);

	for (Chunk * chunk : chunks)
		_deposit.push_back(chunk);

	return (true);
}

void	ChunkGenerator::_process()
{
	setWorking(true);
	LOCK(_depositMutex);

	for (Chunk * chunk : _deposit)
	{
		if (!chunk->loaded)
			continue ;
			
		if (chunk->loaded && chunk->getState() == ChunkState::CS_EMPTY)
		{
			chunk->generate();
			chunk->setState(ChunkState::CS_GENERATED);
		}
		if (chunk->getState() == ChunkState::CS_GENERATED || chunk->getRemesh())
		{
			chunk->mesh();
			if (!chunk->getRemesh())
				chunk->setState(ChunkState::CS_MESHED);
		}
	}

	for (Chunk * chunk : _deposit)
		chunk->setGenerating(false);

	_deposit.clear();
	_deposit.shrink_to_fit();
}

void	ChunkGenerator::_loop()
{
	_running = true;
	while (_running)
	{
		_process();
		setWorking(false);
		usleep(200);
	}
}

void	ChunkGenerator::start()
{
	if (_running)
		return ;

	consoleLog("Starting generation thread", LogSeverity::NORMAL);
	_thread = std::thread(&ChunkGenerator::_loop, this);
}

void	ChunkGenerator::stop()
{
	if (!_running)
		return ;

	_running = false;
	_thread.join();
	consoleLog("Successfully joined generation thread", LogSeverity::SUCCESS);
}
