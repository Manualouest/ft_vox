/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RegionManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 15:44:51 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 21:40:14 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RegionManager.hpp"
#include "ChunkGenerator.hpp"

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

	Frustum	camFrustum = createFrustumFromCamera(SCREEN_WIDTH / SCREEN_HEIGHT, glm::radians(FOV), 0.0001f, RenderDist * 32);
	VolumeAABB	boundingBox(glm::vec3(16.0f, 0.0f, 16.0f), glm::vec3(16.0f, 256.0f, 16.0f));


	_QT->getVisibleChunks(_renderChunks, camFrustum, boundingBox);


	sortChunks();
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
