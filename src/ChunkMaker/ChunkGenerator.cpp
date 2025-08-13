/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:55:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/13 14:18:25 by mbatty           ###   ########.fr       */
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
	{
		chunk->waiting = true;
		_deposit.push_back(chunk);
	}

	return (true);
}

void	ChunkGenerator::_process()
{
	LOCK(_depositMutex);

	for (Chunk * chunk : _deposit)
	{
		_working = true;
		chunk->waiting = false;
		if (chunk->loaded)
			chunk->generate();
		chunk->setGenerating(false);
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
