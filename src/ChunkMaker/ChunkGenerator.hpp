/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 20:01:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/13 14:47:47 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKGENERATOR_HPP
# define CHUNKGENERATOR_HPP

# include "libs.hpp"
# include "Chunk.hpp"
# include "Window.hpp"

class	ChunkGenerator
{
	public:
		ChunkGenerator(): _thread(&ChunkGenerator::_loop, this)
		{
			consoleLog("Starting generation thread", LogSeverity::NORMAL);
		}
		~ChunkGenerator()
		{
			stop();
			_thread.join();
			consoleLog("Successfully joined generation thread", LogSeverity::SUCCESS);
		}
		void	stop()
		{
			_running.store(false);
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
			return (this->_available.load());
		}
	private:
		void	_generateChunks()
		{
			for (Chunk *chunk : _chunksToGenerate)
			{
				if (!_running.load())
					break ;
				chunk->generate();
				chunk->setGenerating(false);
			}

			_chunksToGenerate.clear();
		}
		void	_retrieveDeposit()
		{
			_depositLock.lock();

			for (Chunk *chunk : _chunksDeposit)
			{
				if (!_running.load())
					break ;
				_chunksToGenerate.push_back(chunk);
			}
			_chunksDeposit.clear();

			_depositLock.unlock();
		}
		void	_waitDeposit()
		{
			_available.store(true);
			while (_available.load())
			{
				usleep(100);
				_depositLock.lock();
				if (_chunksDeposit.size() > 0 || !_running)
					_available.store(false);
				_depositLock.unlock();
			}
		}
		void	_loop()
		{
			_running.store(true);
			while (_running.load())
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

		//Processed chunks will be deposited here
		std::atomic_bool		_running;
		std::atomic_bool		_available;
		std::thread				_thread;
};

#define	GENERATION_THREAD_COUNT 16
#define CHUNKS_PER_THREAD 4

class	ChunkGeneratorManager
{
	public:
		ChunkGeneratorManager(): _running(false), _thread(&ChunkGeneratorManager::_loop, this)
		{
			for (int i = 0; i < GENERATION_THREAD_COUNT; i++)
				_generators.push_back(new ChunkGenerator);
			_running.store(true);
		}
		~ChunkGeneratorManager()
		{
			_running.store(false);
			_thread.join();
			for (ChunkGenerator *generator : _generators)
				delete generator;
		}
		void	deposit(Chunk *chunk)
		{
			if (chunk->isGenerating() || chunk->isGenerated() || chunk->isUploaded())
				return ;

			_depositLock.lock();

			chunk->setGenerating(true);
			_chunkDeposit.push_back(chunk);

			_depositLock.unlock();
		}
		void	dispatch()
		{
			_depositLock.lock();

			int	threads = getAvailableThreads();
			uint	chunks = _chunkDeposit.size();

			if (threads > 0 && chunks >= CHUNKS_PER_THREAD)
			{
				_deposit({_chunkDeposit.begin(), _chunkDeposit.begin() + CHUNKS_PER_THREAD});
				_chunkDeposit.erase(_chunkDeposit.begin(), _chunkDeposit.begin() + CHUNKS_PER_THREAD);
			}
			else if (threads >= GENERATION_THREAD_COUNT && chunks > 0)
			{
				_deposit(_chunkDeposit.back());
				_chunkDeposit.pop_back();
			}

			_depositLock.unlock();
		}
		uint	getWaitingChunks() {return (_chunkDeposit.size());}
		int	getAvailableThreads()
		{
			int	res = 0;

			for (ChunkGenerator *generator : _generators)
				if (generator->isAvailable())
					res++;
			return (res);
		}
	private:
		void	_loop()
		{
			while (!_running.load())
				;
			while (_running.load())
			{
				dispatch();
				usleep(100);
			}
		}
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
		void	_deposit(Chunk *chunk)
		{
			for (ChunkGenerator *generator : _generators)
				if (generator->isAvailable())
				{
					generator->deposit(chunk);
					return ;
				}
		}
		std::vector<ChunkGenerator *>	_generators;
		std::vector<Chunk *>			_chunkDeposit;
		std::mutex						_depositLock;
		std::atomic_bool				_running;
		std::thread						_thread;
};

#endif
