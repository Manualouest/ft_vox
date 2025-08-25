/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGeneratorManager.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:07:42 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/25 16:25:05 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGeneratorManager.hpp"
#include "RegionManager.hpp"

void	ChunkGeneratorManager::deposit(std::vector<Chunk *> &chunks)
{
	if (isWorking())
		return ;

	LOCK(_depositMutex);

	for (Chunk * chunk : chunks)
	{
		if (!chunk->getGenerating() && (chunk->getState() < CS_MESHED || chunk->getRemesh()))
		{
			chunk->setGenerating(true);
			_deposit.push_back(chunk);
		}
	}

	RegionManager::sortChunks(_deposit);
	std::reverse(_deposit.begin(), _deposit.end());
}

void	ChunkGeneratorManager::_send()
{
	setWorking(true);
	LOCK(_depositMutex);

	if (workingThreads() >= GENERATION_THREAD_COUNT)
		return ;

	for (ChunkGenerator *generator : _generators)
	{
		uint	sizeToAdd = std::min(_deposit.size(), (size_t)CHUNKS_PER_THREAD);
		if (sizeToAdd <= CHUNKS_PER_THREAD && sizeToAdd > 0 && !generator->isWorking())
		{
			if (generator->deposit({_deposit.begin(), _deposit.begin() + sizeToAdd}))
				_deposit.erase(_deposit.begin(), _deposit.begin() + sizeToAdd);
		}
	}
}

void	ChunkGeneratorManager::_loop()
{
	_running = true;
	while (_running)
	{
		_send();
		setWorking(false);
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
	for (ChunkGenerator *generator : _generators)
		generator->start();
	while (!_allStarted())
		usleep(200);
	_thread = std::thread(&ChunkGeneratorManager::_loop, this);
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

