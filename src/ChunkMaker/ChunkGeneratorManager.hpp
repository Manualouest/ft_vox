/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGeneratorManager.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:06:39 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/13 13:28:46 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKGENERATORMANAGER_HPP
# define CHUNKGENERATORMANAGER_HPP

# include "libs.hpp"
# include "ChunkGenerator.hpp"

# define	GENERATION_THREAD_COUNT 16
# define	CHUNKS_PER_THREAD 4

class	ChunkGeneratorManager
{
	public:
		ChunkGeneratorManager();
		~ChunkGeneratorManager();

		void	start();
		void	stop();
		void	deposit(std::vector<Chunk *> &chunks);

		uint	workingThreads();
	private:
		void	_send();
		void	_loop();

		std::vector<Chunk *>			_deposit;
		std::vector<Chunk *>			_priority;
		std::mutex						_depositMutex;
		std::vector<ChunkGenerator *>	_generators;
		std::atomic_bool				_running;
		std::thread						_thread;
};

#endif
