/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:55:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/24 18:38:45 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGenerator.hpp"

bool	ChunkGenerator::deposit(std::vector<Chunk *> chunks)
{
	LOCK(_depositMutex);

	if (_deposit.size() > 0 || _working)
		return (false);

	_deposit.reserve(chunks.size());

	for (Chunk * chunk : chunks)
		_deposit.push_back(chunk);

	return (true);
}

void	ChunkGenerator::_process()
{
	LOCK(_depositMutex);

	for (Chunk * chunk : _deposit)
	{
		// consoleLog("0", NORMAL);
		_working = true;
		if (!chunk->loaded)
		{
			chunk->setGenerating(false);
			continue ;
		}
		// consoleLog("1", NORMAL);
		if (chunk->loaded && !chunk->getRemesh())
		{
			// consoleLog("1.1", NORMAL);
			chunk->generate();
			// consoleLog("1.2", NORMAL);
			chunk->setState(ChunkState::CS_GENERATED);
		}
		// consoleLog("2", NORMAL);
		if (chunk->getState() == ChunkState::CS_GENERATED || chunk->getRemesh())
		{
			// consoleLog("2.1", NORMAL);
			chunk->mesh();
			// consoleLog("2.2", NORMAL);
			if (!chunk->getRemesh())
				chunk->setState(ChunkState::CS_MESHED);
			// consoleLog("2.3", NORMAL);
		}
		// consoleLog("3", NORMAL);

		chunk->setGenerating(false);
		// consoleLog("4", NORMAL);
	}

	_deposit.clear();
	_deposit.shrink_to_fit();
}

void	ChunkGenerator::_loop()
{
	while (_running)
	{
		_process();
		usleep(200);
		_working = false;
		usleep(200);
	}
}

void	ChunkGenerator::start()
{
	if (_running)
		return ;

	consoleLog("Starting generation thread", LogSeverity::NORMAL);
	_thread = std::thread(&ChunkGenerator::_loop, this);
	_running = true;
}

void	ChunkGenerator::stop()
{
	if (!_running)
		return ;

	_running = false;
	_thread.join();
	consoleLog("Successfully joined generation thread", LogSeverity::SUCCESS);
}
