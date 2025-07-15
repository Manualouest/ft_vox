/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:32:56 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/15 13:22:43 by mbatty           ###   ########.fr       */
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
	private:
		uint	RenderDist;
		bool	isInRange();
		void	sortChunks();
	
		std::vector<Chunk *>	_renderChunks;
		Quadtree				*_QT;
};

#endif
