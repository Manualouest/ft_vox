/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:44:51 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/13 18:31:58 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegionManager.hpp"
#include "ChunkGeneratorManager.hpp"

extern	float SPEEDBOOST;


Frustum createFrustumFromCamera(float aspect, float fovY, float zNear, float zFar)
{
	Frustum			frustum;
	const float		halfVSide = zFar * tanf(fovY * .5f);
	const float		halfHSide = halfVSide * aspect;
	const glm::vec3	frontMultFar = zFar * CAMERA->front;

	frustum.nearFace = { CAMERA->pos + zNear * CAMERA->front, CAMERA->front };
	frustum.farFace = { CAMERA->pos + frontMultFar, -CAMERA->front };
	frustum.rightFace = { CAMERA->pos, glm::cross(frontMultFar - CAMERA->right * halfHSide, CAMERA->up) };
	frustum.leftFace = { CAMERA->pos, glm::cross(CAMERA->up, frontMultFar + CAMERA->right * halfHSide) };
	frustum.topFace = { CAMERA->pos, glm::cross(CAMERA->right, frontMultFar - CAMERA->up * halfVSide) };
	frustum.bottomFace = { CAMERA->pos, glm::cross(frontMultFar + CAMERA->up * halfVSide, CAMERA->right) };
	return frustum;
}

RegionManager::RegionManager()
{
	RenderDist = 10;
	_QT = new Quadtree(glm::vec2(0, 0), QTBranch::BOTTOM_LEFT, glm::vec2(WORLD_SIZE, WORLD_SIZE));
}

RegionManager::~RegionManager()
{
	delete _QT;
}

void	RegionManager::UpdateChunks()
{
	for (Chunk *chunk : _renderChunks)
		chunk->rendered = false;
	for (Chunk *chunk : _loadedChunks)
	{
		chunk->loadedThisFrame = false;
		chunk->rendered = false;
	}

	_renderChunks.clear();
	// _loadedChunks.clear();

	std::vector<Chunk*>	loadedChunks;

	Frustum	camFrustum = createFrustumFromCamera(SCREEN_WIDTH / SCREEN_HEIGHT, glm::radians(FOV), 0.0001f, RenderDist * 32);
	VolumeAABB	boundingBox(glm::vec3(16.0f, 0.0f, 16.0f), glm::vec3(16.0f, 256.0f, 16.0f));

	int	startX = (CAMERA->pos.x / 32) - RenderDist;
	int	startZ = (CAMERA->pos.z / 32) - RenderDist;

	int	endX = (CAMERA->pos.x / 32) + RenderDist;
	int	endZ = (CAMERA->pos.z / 32) + RenderDist;

	for (int x = startX; x < endX; x++)
	{
		for (int z = startZ; z < endZ; z++)
		{
			if (x < 0 || z < 0)
				continue ;

			Chunk *tmp = _QT->growBranch(glm::vec2(x * 32, z * 32));
			if (tmp)
			{
				tmp->loaded = true;
				tmp->loadedThisFrame = true;
				loadedChunks.push_back(tmp);
			}
		}
	}

	for (Chunk *chunk : _loadedChunks)
	{
		if (!chunk->loadedThisFrame)
			chunk->loaded = false;
	}

	_loadedChunks = loadedChunks;

	_QT->pruneDeadLeaves(_QT);

	_QT->getVisibleChunks(_renderChunks, camFrustum, boundingBox);


	sortChunks(_renderChunks);
	CHUNK_GENERATOR->deposit(_loadedChunks);
}

bool	RegionManager::isFullyGenerated()
{
	if (getGeneratingChunksCount())
		return (false);
	return (true);
}

#include "FrameBuffer.hpp"
#include "ShaderManager.hpp"

extern FrameBuffer	*MAIN_FRAME_BUFFER;
extern FrameBuffer	*DEPTH_FRAME_BUFFER;
extern FrameBuffer	*WATER_DEPTH_FRAME_BUFFER;

extern ShaderManager *SHADER_MANAGER;

void	RegionManager::Render(Shader &shader)
{
    glEnable(GL_DEPTH_TEST);
	shader.use();
	CAMERA->setViewMatrix(shader);
	for (auto *chunk : _renderChunks)
		chunk->draw(shader);
}

void	RegionManager::sortChunks(std::vector<Chunk *> &chunks)
{
	for (std::vector<Chunk *>::iterator it = chunks.begin(); it != chunks.end(); ++it)
		(*it)->initDist();

	std::sort(chunks.begin(), chunks.end(),
		[](const Chunk *cp1, const Chunk *cp2)
		{
			return (cp1->getDist() > cp2->getDist());
		});
}
