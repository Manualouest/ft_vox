/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:32:56 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/12 23:52:12 by mbatty           ###   ########.fr       */
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
			static double	lastCheck = 0;
			static uint		lastCount = 0;

			if (glfwGetTime() - lastCheck > 0.2)
			{
				lastCheck = glfwGetTime();
				_generatedChunks = 0;

				for (Chunk *chunk : _loadedChunks)
					if (chunk->isGenerating())
						_generatedChunks++;

				lastCount = _generatedChunks;
				return (_generatedChunks);
			}
			else
				return (lastCount);
		}
		uint	getLoadedChunkCount()
		{
			return (this->_loadedChunks.size());
		}
	private:
		uint	_generatedChunks = 0;
		uint	RenderDist;
		bool	isInRange();
		void	sortChunks(std::vector<Chunk *> &chunks);

		std::vector<Chunk *>	_renderChunks;
		std::vector<Chunk *>	_loadedChunks;
		Quadtree				*_QT;
};

#endif
