/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:55:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/12 16:45:57 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGenerator.hpp"

bool	ChunkGenerator::deposit(std::vector<Chunk *> chunks)
{
	LOCK(_depositMutex);

	if (_deposit.size() > 0)
		return (false);

	_deposit.reserve(chunks.size());

	for (Chunk * chunk : chunks)
		_deposit.push_back(chunk);

	_deposit.shrink_to_fit();
	return (true);
}

void	ChunkGenerator::_process()
{
	LOCK(_depositMutex);

	if (_deposit.size() <= 0)
		return ;

	_working = true;

	for (Chunk * chunk : _deposit)
	{
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
