/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 17:55:09 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/13 18:40:56 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ChunkGenerator.hpp"

//ChunkGenerator

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

//ChunkGeneratorManager

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
