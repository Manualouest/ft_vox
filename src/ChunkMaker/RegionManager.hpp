/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:32:56 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/11 09:05:50 by mbirou           ###   ########.fr       */
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
	
	private:
		bool	isInRange();
		void	sortChunks();
	
		std::vector<Chunk *>	_chunks;
		std::vector<Chunk *>	_renderChunks;
		Quadtree				*_QT;
};

#endif