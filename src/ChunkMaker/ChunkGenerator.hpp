/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChunkGenerator.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 20:01:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 10:09:22 by mbatty           ###   ########.fr       */
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
		bool	deposit(std::vector<Chunk *> chunks);
		
		bool	isWorking() {return (this->_working);}
	private:
		void	_loop();
		void	_process();
		
		std::vector<Chunk *>	_deposit;
		std::mutex				_depositMutex;
		std::atomic_bool		_running;
		std::atomic_bool		_working;
		std::thread				_thread;
};

#endif
