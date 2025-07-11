/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:44:51 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/10 16:00:09 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegionManager.hpp"

RegionManager::RegionManager()
{

}

RegionManager::~RegionManager()
{

}

void	RegionManager::UpdateChunks()
{
	glm::vec3	center = floor(CAMERA->pos / glm::vec3(32.0f)) * glm::vec3(32.0f) + glm::vec3(16.0f, 0, 16.0f);

	
}

void	RegionManager::Render(Shader *shader)
{

}

void	RegionManager::sortChunks()
{
	std::sort(_RenderChunks.begin(), _RenderChunks.end(),
		[](const Chunk *cp1, const Chunk *cp2)
		{
			return (cp1->getDistance() > cp2->getDistance());
		});
}

	
		// std::vector<Chunk *>	_Chunks;
		// std::vector<Chunk *>	_RenderChunks;