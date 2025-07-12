/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 20:01:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/12 18:38:47 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKGENERATOR_HPP
# define CHUNKGENERATOR_HPP

# include "libs.hpp"
# include "Chunk.hpp"

class	ChunkGenerator
{
	public:
		ChunkGenerator(): _thread(&ChunkGenerator::_loop, this)
		{
			consoleLog("Starting generation thread", LogSeverity::SUCCESS);
		}
		~ChunkGenerator()
		{
			stop();
			_thread.join();
			consoleLog("Successfully joined generation thread", LogSeverity::SUCCESS);
		}
		void	stop()
		{
			_running = false;
		}
		void	deposit(Chunk *chunk)
		{
			_depositLock.lock();
			_chunksDeposit.push_back(chunk);
			_depositLock.unlock();
		}
		void	deposit(std::vector<Chunk *> chunks)
		{
			_depositLock.lock();
			for (Chunk* chunk : chunks)
				_chunksDeposit.push_back(chunk);
			_depositLock.unlock();
		}
		bool	isAvailable()
		{
			return (this->_available);
		}
	private:
		void	_generateChunks()
		{
			_processLock.lock();

			for (Chunk *chunk : _chunksToGenerate)
			{
				if (!_running)
					break ;
				chunk->generate();
			}

			_chunksToGenerate.clear();

			_processLock.unlock();
		}
		void	_retrieveDeposit()
		{
			_depositLock.lock();

			for (uint i = 0; i < _chunksDeposit.size(); i++)
			{
				if (!_running)
					break ;
				_chunksToGenerate.push_back(_chunksDeposit.back());
				_chunksDeposit.pop_back();
			}

			_depositLock.unlock();
		}
		void	_waitDeposit()
		{
			_available = true;
			while (_available)
			{
				_depositLock.lock();
				if (_chunksDeposit.size() > 0 || !_running)
					_available = false;
				_depositLock.unlock();
				usleep(100);
			}
		}
		void	_loop()
		{
			_running = true;
			while (_running)
			{
				_waitDeposit();
				_retrieveDeposit();
				_generateChunks();
				usleep(100);
			}
		}
		//Main thread will deposit chunks in here
		std::vector<Chunk *>	_chunksDeposit;
		std::mutex				_depositLock;
		
		//Generator thread will process chunks here
		std::vector<Chunk *>	_chunksToGenerate;
		std::mutex				_processLock;

		//Processed chunks will be deposited here
		std::atomic_bool		_running;
		std::atomic_bool		_available;
		std::thread				_thread;
};

#define	GENERATION_THREAD_COUNT 8

class	ChunkGeneratorManager
{
	public:
		ChunkGeneratorManager()
		{
			for (int i = 0; i < GENERATION_THREAD_COUNT; i++)
				_generators.push_back(new ChunkGenerator);
		}
		~ChunkGeneratorManager()
		{
			for (ChunkGenerator *generator : _generators)
				delete generator;
		}
		void	deposit(Chunk *chunk)
		{
			_chunkDeposit.push_back(chunk);

			if (_chunkDeposit.size() >= 4 && getAvailableThreads() > 0)
			{
				_deposit(_chunkDeposit);
				_chunkDeposit.clear();
			}
		}
		int	getAvailableThreads()
		{
			int	res = 0;

			for (ChunkGenerator *generator : _generators)
				if (generator->isAvailable())
					res++;
			return (res);
		}
	private:
		//Add a whole vector to the generation queue
		void	_deposit(std::vector<Chunk *> chunks)
		{
			for (ChunkGenerator *generator : _generators)
				if (generator->isAvailable())
				{
					generator->deposit(chunks);
					return ;
				}
		}
		std::vector<ChunkGenerator *>	_generators;
		std::vector<Chunk *>			_chunkDeposit;

};

#endif
