/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:32:56 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/16 14:40:51 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REGIONMANAGER_HPP
# define REGIONMANAGER_HPP

# include "Chunk.hpp"
# include "QuadTree.hpp"

#define WORLD_SIZE 65536.0

class RegionManager
{
	public:
		RegionManager();
		~RegionManager();

		void	UpdateChunks();
		void	Render(Shader &shader);


		Quadtree	*getQuadTree() {return (this->_QT);}
		void	setRenderDist(uint renderDist) {this->RenderDist = renderDist;}
		uint	getRenderDist() {return (this->RenderDist);}
		uint	renderCount() {return (_renderChunks.size());}
		bool	isFullyGenerated();
		uint	getGeneratingChunksCount()
		{
			uint	res = 0;

			for (Chunk *chunk : _loadedChunks)
				if (chunk->getGenerating())
					res++;

			return (res);
		}
		uint	getLoadedChunkCount()
		{
			return (this->_loadedChunks.size());
		}
		const std::vector<Chunk *>	&getLoadedChunks()
		{
			return (this->_loadedChunks);
		}
		static void	sortChunks(std::vector<Chunk *> &chunks);
	private:
		uint	RenderDist;
		bool	isInRange();

		std::vector<Chunk *>	_renderChunks;
		std::vector<Chunk *>	_loadedChunks;
		Quadtree				*_QT;
};

#endif
