/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 20:01:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 12:08:51 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKGENERATOR_HPP
# define CHUNKGENERATOR_HPP

# include "libs.hpp"
# include "Chunk.hpp"
# include "Window.hpp"

extern Window	*WINDOW;

#define LOCK const std::lock_guard<std::mutex> lock

class	ChunkGenerator
{
	public:
		ChunkGenerator(): _running(false), _working(false) {}
		~ChunkGenerator() {stop();}
		void	stop();
		void	start();
		bool	isWorking() {return (this->_working);}
		bool	deposit(std::vector<Chunk *> chunks)
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
	private:
		void	_loop()
		{
			while (_running)
			{
				_process();
				usleep(100);
				_working = false;
			}
		}
		void	_process()
		{
			LOCK(_depositMutex);

			if (_deposit.size() <= 0)
				return ;

			_working = true;

			for (Chunk * chunk : _deposit)
			{
				if (chunk->rendered)
					chunk->generate();
				chunk->setGenerating(false);
			}
			
			_deposit.clear();
			_deposit.shrink_to_fit();
		}
		std::vector<Chunk *>	_deposit;
		std::mutex				_depositMutex;
		std::atomic_bool		_running;
		std::atomic_bool		_working;
		std::thread				_thread;
};

#define	GENERATION_THREAD_COUNT 12
#define CHUNKS_PER_THREAD 2

class	ChunkGeneratorManager
{
	public:
		ChunkGeneratorManager();
		~ChunkGeneratorManager();
		void	start();
		void	stop();	
		void	deposit(std::vector<Chunk *> chunks)
		{
			LOCK(_depositMutex);

			_deposit.reserve(chunks.size());

			for (Chunk * chunk : chunks)
				if (!chunk->isGenerated() && !chunk->isGenerating())
				{
					chunk->setGenerating(true);
					_deposit.push_back(chunk);
				}

			_deposit.shrink_to_fit();
		}
		void	depositVIP(std::vector<Chunk *> chunks)
		{
			LOCK(_depositMutex);

			_deposit.reserve(chunks.size());

			for (Chunk * chunk : chunks)
				if (!chunk->isGenerated() && !chunk->isGenerating())
				{
					chunk->setGenerating(true);
					_deposit.insert(_deposit.begin(), chunk);
				}

			_deposit.shrink_to_fit();
		}
		uint	availableWorkers()
		{
			uint	res = 0;
			for (ChunkGenerator *generator : _generators)
				res += generator->isWorking();
			return (res);
		}
	private:
		void	_send()
		{
			LOCK(_depositMutex);
			
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
		void	_loop()
		{
			while (_running)
			{
				_send();
				usleep(100);
			}
		}
		std::vector<Chunk *>			_deposit;
		std::mutex						_depositMutex;
		std::vector<ChunkGenerator *>	_generators;
		std::atomic_bool				_running;
		std::thread						_thread;
};

#endif
