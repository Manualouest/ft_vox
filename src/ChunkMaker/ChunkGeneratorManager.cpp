/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGeneratorManager.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:07:42 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/12 16:32:03 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGeneratorManager.hpp"

void	ChunkGeneratorManager::deposit(std::vector<Chunk *> &chunks)
{
	if (!_depositMutex.try_lock())
		return ;

	std::vector<Chunk*>	cpy = chunks;
	std::reverse(cpy.begin(), cpy.end());

	_deposit.reserve(cpy.size());

	for (Chunk * chunk : cpy)
		if (!chunk->isGenerated() && !chunk->isGenerating())
		{
			chunk->setGenerating(true);
			if (chunk->rendered)
				_priority.push_back(chunk);
			else
				_deposit.push_back(chunk);
		}

	_deposit.shrink_to_fit();

	_depositMutex.unlock();
}

void	ChunkGeneratorManager::_send()
{
	if (workingThreads() >= GENERATION_THREAD_COUNT)
		return ;

	LOCK(_depositMutex);

	for (ChunkGenerator *generator : _generators)
	{
		uint	sizeToAdd = std::min(_priority.size(), (size_t)CHUNKS_PER_THREAD);
		if (sizeToAdd <= CHUNKS_PER_THREAD && sizeToAdd > 0 && !generator->isWorking())
		{
			if (generator->deposit({_priority.begin(), _priority.begin() + sizeToAdd}))
				_priority.erase(_priority.begin(), _priority.begin() + sizeToAdd);
		}
	}
	if (_priority.empty())
		_priority.shrink_to_fit();

	for (ChunkGenerator *generator : _generators)
	{
		uint	sizeToAdd = std::min(_deposit.size(), (size_t)CHUNKS_PER_THREAD);
		if (sizeToAdd <= CHUNKS_PER_THREAD && sizeToAdd > 0 && !generator->isWorking())
		{
			if (generator->deposit({_deposit.begin(), _deposit.begin() + sizeToAdd}))
				_deposit.erase(_deposit.begin(), _deposit.begin() + sizeToAdd);
		}
	}
	if (_deposit.empty())
		_deposit.shrink_to_fit();
}

void	ChunkGeneratorManager::_loop()
{
	while (_running)
	{
		_send();
		usleep(200);
	}
}

uint	ChunkGeneratorManager::workingThreads()
{
	uint	res = 0;
	for (ChunkGenerator *generator : _generators)
		res += generator->isWorking();
	return (res);
}

ChunkGeneratorManager::ChunkGeneratorManager(): _running(false)
{
	for (int i = 0; i < GENERATION_THREAD_COUNT; i++)
		_generators.push_back(new ChunkGenerator);
	start();
}

ChunkGeneratorManager::~ChunkGeneratorManager()
{
	stop();
	for (ChunkGenerator *generator : _generators)
		delete generator;
}

void	ChunkGeneratorManager::start()
{
	if (_running)
		return ;

	consoleLog("Starting chunk generator manager thread", LogSeverity::NORMAL);
	_thread = std::thread(&ChunkGeneratorManager::_loop, this);
	_running = true;
	for (ChunkGenerator *generator : _generators)
		generator->start();
}

void	ChunkGeneratorManager::stop()
{
	if (!_running)
		return ;

	for (ChunkGenerator *generator : _generators)
		generator->stop();
	_running = false;
	_thread.join();
	consoleLog("Successfully joined generator manager thread", LogSeverity::SUCCESS);
}

