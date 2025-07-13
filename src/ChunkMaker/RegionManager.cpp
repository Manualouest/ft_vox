/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:44:51 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 18:52:12 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegionManager.hpp"
#include "ChunkGenerator.hpp"

RegionManager::RegionManager()
{
	RenderDist = 16;
	_QT = new Quadtree(glm::vec2(0, 0), glm::vec2(16384.0f, 16384.0f));
}

RegionManager::~RegionManager()
{
	delete _QT;
}

void	RegionManager::UpdateChunks()
{
	for (Chunk *chunk : _renderChunks)
		chunk->rendered = false;
	_renderChunks.clear();
	Chunk	*chunk = NULL;
	
	glm::ivec2	bottomLeftCorner = glm::ivec2(CAMERA->pos.x, CAMERA->pos.z) - glm::ivec2(RenderDist * 32) / 2;
	
	for (uint x = 0; x < RenderDist; x++)
	{
		for (uint y = 0; y < RenderDist; y++)
		{
			chunk = _QT->growBranch({bottomLeftCorner.x + x * 32, bottomLeftCorner.y + y * 32});
			if (chunk)
			{
				chunk->rendered = true;
				_renderChunks.push_back(chunk);
			}
		}
	}
	CHUNK_GENERATOR->deposit(_renderChunks);
}

#include "FrameBuffer.hpp"
#include "ShaderManager.hpp"

extern FrameBuffer	*MAIN_FRAME_BUFFER;
extern FrameBuffer	*DEPTH_FRAME_BUFFER;
extern FrameBuffer	*WATER_DEPTH_FRAME_BUFFER;

extern ShaderManager *SHADER_MANAGER;

void	RegionManager::Render(Shader &shader)
{
	UpdateChunks();
	sortChunks();
	MAIN_FRAME_BUFFER->use();
	for (auto *chunk : _renderChunks)
	{
		chunk->draw(shader);
	}
}

void	RegionManager::sortChunks()
{
	std::sort(_renderChunks.begin(), _renderChunks.end(),
		[](const Chunk *cp1, const Chunk *cp2)
		{
			return (cp1->getDistance() > cp2->getDistance());
		});
}
