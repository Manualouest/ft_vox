/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:44:51 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 10:28:54 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegionManager.hpp"
#include "ChunkGenerator.hpp"

RegionManager::RegionManager()
{
	RenderDist = 14;
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
	glm::vec2	dirPos = CAMERA->flatFront;
	glm::vec2	pos = glm::vec2(CAMERA->pos.x, CAMERA->pos.z) - dirPos * 32.0f;
	glm::vec2	leftDir = glm::vec2(0);
	glm::vec2	rightDir = glm::vec2(0);
	glm::vec2	tmpDirPos = glm::vec2(0);
	Chunk		*chunk = NULL;
	int			ChunkAmount = 10;

	for (float i = 1; i <= RenderDist * 2; ++i)
	{
		tmpDirPos = dirPos * 16.0f *i;
		leftDir = glm::normalize(glm::vec2(-tmpDirPos.y, tmpDirPos.x));
		rightDir = glm::normalize(glm::vec2(tmpDirPos.y, -tmpDirPos.x));
		for (float ii = i * 1.25; ii > 0; --ii)
		{
			chunk = _QT->getLeaf(pos + tmpDirPos + (leftDir * 16.0f * ii));
			if (chunk != NULL)
			{
				chunk->rendered = true;
				_renderChunks.push_back(chunk);
			}
			else if (ChunkAmount-- > 0)
			{
				chunk = _QT->growBranch(pos + tmpDirPos + (leftDir * 16.0f * ii));
				chunk->rendered = true;
				_renderChunks.push_back(chunk);
			}
			chunk = _QT->getLeaf(pos + tmpDirPos + (rightDir * 16.0f * ii));
			if (chunk != NULL)
			{
				chunk->rendered = true;
				_renderChunks.push_back(chunk);
			}
			else if (ChunkAmount-- > 0)
			{
				chunk = _QT->growBranch(pos + tmpDirPos + (rightDir * 16.0f * ii));
				chunk->rendered = true;
				_renderChunks.push_back(chunk);
			}
		}
		chunk = _QT->getLeaf(pos + tmpDirPos);
		if (chunk != NULL)
		{
			chunk->rendered = true;
			_renderChunks.push_back(chunk);
		}
		else if (ChunkAmount-- > 0)
		{
			chunk = _QT->growBranch(pos + tmpDirPos);
			chunk->rendered = true;
			_renderChunks.push_back(chunk);
		}
	}
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
