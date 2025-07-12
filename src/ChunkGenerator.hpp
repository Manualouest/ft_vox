/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 20:01:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/12 09:27:06 by mbatty           ###   ########.fr       */
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
		//ONLY USE ON MAIN THREAD OR ILL DO THINGS TO YOU!
		void	upload()
		{
			consoleLog("caca in", LogSeverity::NORMAL);
			_processLock.lock();

			if (_generatedChunks.size() < 4)
			{
				_processLock.unlock();
				return ;
			}

			for (Chunk *chunk : _generatedChunks)
				chunk->upload();
			_generatedChunks.clear();

			_processLock.unlock();
			consoleLog("caca out", LogSeverity::NORMAL);
		}
	private:
		void	_generateChunks()
		{
			_processLock.lock();

			for (uint i = 0; i < _chunksToGenerate.size(); i++)
			{
				_chunksToGenerate.back()->generate();
				_generatedChunks.push_back(_chunksToGenerate.back());
				_chunksToGenerate.pop_back();
			}

			_processLock.unlock();
		}
		void	_retrieveDeposit()
		{
			_depositLock.lock();

			for (uint i = 0; i < _chunksDeposit.size(); i++)
			{
				_chunksToGenerate.push_back(_chunksDeposit.back());
				_chunksDeposit.pop_back();
			}

			_depositLock.unlock();
		}
		void	_loop()
		{
			_running = true;
			while (_running)
			{
				_retrieveDeposit();
				_generateChunks();
				usleep(1);
			}
		}
		//Main thread will deposit chunks in here
		std::vector<Chunk *>	_chunksDeposit;
		std::mutex				_depositLock;
		
		//Generator thread will process chunks here
		std::vector<Chunk *>	_chunksToGenerate;
		std::mutex				_processLock;

		//Processed chunks will be deposited here
		std::vector<Chunk *>	_generatedChunks;
		std::mutex				_chunksLock;
		
		std::atomic_bool		_running;
		std::thread				_thread;
};

#endif
