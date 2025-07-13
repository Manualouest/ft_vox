/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:32:56 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 10:28:41 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REGIONMANAGER_HPP
# define REGIONMANAGER_HPP

# include "Chunk.hpp"
# include "QuadTree.hpp"

class RegionManager
{
	public:
		RegionManager();
		~RegionManager();

		void	UpdateChunks();
		void	Render(Shader &shader);

		uint	RenderDist;
	
		Quadtree	*getQuadTree() {return (this->_QT);}
	private:
		bool	isInRange();
		void	sortChunks();
	
		std::vector<Chunk *>	_renderChunks;
		Quadtree				*_QT;
};

#endif
