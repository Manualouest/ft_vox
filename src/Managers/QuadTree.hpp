/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuadTree.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 17:46:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 10:20:42 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUADTREE_HPP
# define QUADTREE_HPP

#include "libs.hpp"
#include "Chunk.hpp"
#include "ChunkGeneratorManager.hpp"

extern ChunkGeneratorManager	*CHUNK_GENERATOR;

struct Plane
{
	glm::vec3	normal = { 0.f, 1.f, 0.f };	// unit vector
	float		distance = 0.f;        		// Distance with origin

	Plane() = default;

	Plane(const glm::vec3 &p1, const glm::vec3 &norm) : normal(glm::normalize(norm)), distance(glm::dot(normal, p1))
	{}

	float getSignedDistanceToPlane(const glm::vec3 &point) const
	{
		return glm::dot(normal, point) - distance;
	}
};

struct Frustum
{
	Plane	topFace;
	Plane	bottomFace;

	Plane	rightFace;
	Plane	leftFace;

	Plane	farFace;
	Plane	nearFace;
};

class VolumeAABB
{
	public:
		glm::vec3	center{ 16.f, 0.f, 16.f };
		glm::vec3	extents{ 16.f, 256.f, 16.f };

		VolumeAABB(const glm::vec3 &inCenter, const glm::vec3 &inExtents) : center(inCenter), extents(inExtents)
		{}

		~VolumeAABB()
		{}


		bool isOnOrForwardPlane(const Plane &plane)
		{
			// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
			const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
			extents.z * std::abs(plane.normal.z);
			return -r <= plane.getSignedDistanceToPlane(center);
		}

		bool isOnFrustum(const Frustum &camFrustum, const glm::vec3 &pos)
		{
			//Get global scale thanks to our transform
			const glm::vec3	globalCenter = pos + center;

			VolumeAABB	globalAABB(globalCenter, extents);

			return (globalAABB.isOnOrForwardPlane(camFrustum.leftFace) &&
				globalAABB.isOnOrForwardPlane(camFrustum.rightFace) &&
				globalAABB.isOnOrForwardPlane(camFrustum.topFace) &&
				globalAABB.isOnOrForwardPlane(camFrustum.bottomFace) &&
				globalAABB.isOnOrForwardPlane(camFrustum.nearFace) &&
				globalAABB.isOnOrForwardPlane(camFrustum.farFace));
		}
};

/*
	Quad tree branch enum
*/
enum QTBranch
{
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	OUT_OF_BOUNDS
};

/*
	Quad tree used to store chunks, each branch has 4 branches under it,
		if a branch reaches a size of 32 it will transform into a Leaf (Chunk).
	
	Each time a branch grows under another one, its size will be:
	- Size of branch above / 2

	Only branches and leaves that are used at some point will be allocated but wont be freed automatically,
	pleaes call pruneDeadLeaves to free some memory (Please do it)

	Please use powers of 2 when working with this quadtree as its built for it
*/
class	Quadtree
{
	public:
		/*
			@param pos Bottom left position
			@param size Size from bottom left position
		*/
		Quadtree(const glm::ivec2 &pos, QTBranch quadrant, const glm::ivec2 &size);
		~Quadtree();
		
		/*
			Grows branches until it finds a leaf on the given targetPos,
				if it cant find one a warning will be printed.
			
			@param targetPos World position of a block inside the wanted leaf (chunk)
		*/
		Chunk	*growBranch(const glm::ivec2 &targetPos);
		
		/*
			Goes through branches until it can find a leaf on the given position without creating new branches.
			If it cant find a leaf, NULL will be returned.
		
			@param targetPos World position of a block inside the wanted leaf (chunk)
		*/
		Chunk	*getLeaf(const glm::ivec2 &targetPos);

		void	getVisibleChunks(std::vector<Chunk *> &chunks, const Frustum &camFrustum, VolumeAABB &AABB);

		/*
			Returns the branch that contains targetPos and stops at the given depth.
			(Can be useful to figure out in wich region the player is)

			@param targetPos World position of a block inside the wanted leaf (chunk)
			@param depth Depth at wich the search should stop
		*/
		Quadtree	*getBranch(const glm::ivec2 &targetPos, int depth);
		
		/*
			Frees memory by pruning branches and leaves that contain no used chunks
		*/
		void	pruneDeadLeaves(Quadtree *root);
		void	pruneBranch(Quadtree *root, QTBranch quadrant);
		
		glm::vec2	getSize() const {return (this->_size);}
		glm::vec2	getPos() const {return (this->_pos);}
		
		bool	isLeaf() const {return (_leaf != NULL);}
		bool	isInBounds(const glm::ivec2 &point);
	private:
		//Returns branch quadrant in wich pos is. (OUT_OF_BOUNDS can be returned but will never happen) @param pos target position of branch
		QTBranch	_getQuadrant(const glm::ivec2 &pos) const;
		
		glm::ivec2				_size;
		glm::ivec2				_pos;
		
		std::vector<Quadtree*>	_branches = {NULL, NULL, NULL, NULL};
		Chunk					*_leaf = NULL;
		
		//Current branch's position in its parent's branches
		QTBranch				_quadrantInRoot;
};

#endif
