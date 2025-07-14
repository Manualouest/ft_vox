/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuadTree.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 17:46:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/14 09:23:45 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUADTREE_HPP
# define QUADTREE_HPP

#include "libs.hpp"
#include "Chunk.hpp"
#include "ChunkGenerator.hpp"

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
		Quadtree(const glm::ivec2 &pos, QTBranch quadrant, const glm::ivec2 &size)
		{
			this->_pos = pos;
			this->_size = size;
			this->_quadrantInRoot = quadrant;

			if (this->_size == glm::ivec2(32))
				this->_leaf = new Chunk(glm::ivec3(_pos.x, 0, _pos.y));
		}
		~Quadtree()
		{
			for (auto *branch : _branches)
				if (branch)
					delete branch;
			if (_leaf)
				delete _leaf;
		}
		/*
			Grows branches until it finds a leaf on the given targetPos,
				if it cant find one a warning will be printed.
			
			@param targetPos World position of a block inside the wanted leaf (chunk)
		*/
		Chunk	*growBranch(const glm::ivec2 &targetPos)
		{
			if (isLeaf())
				return (_leaf);

			QTBranch	quadrant = _getQuadrant(targetPos);

			glm::ivec2	childPos(_pos);

			bool	isTop = targetPos.x >= _pos.x + _size.x / 2;
			bool	isRight = !(targetPos.y < _pos.y + _size.y / 2);

			childPos.x += isTop * (_size.x / 2);
			childPos.y += isRight * (_size.y / 2);

			if (quadrant != QTBranch::OUT_OF_BOUNDS)
			{
				if (_branches[quadrant] == NULL)
					_branches[quadrant] = new Quadtree(childPos, quadrant, _size / 2);
				if (!_branches[quadrant]->isInBounds(targetPos))
					return (NULL);
				return (_branches[quadrant]->growBranch(targetPos));
			}

			consoleLog("WARNING could not find/create a leaf from the given branch", LogSeverity::WARNING);
			return (NULL);
		}
		/*
			Goes through branches until it can find a leaf on the given position without creating new branches.
			If it cant find a leaf, NULL will be returned.
		
			@param targetPos World position of a block inside the wanted leaf (chunk)
		*/
		Chunk	*getLeaf(const glm::ivec2 &targetPos)
		{
			if (isLeaf())
				return (_leaf);
			
			QTBranch	quadrant = _getQuadrant(targetPos);

			if (quadrant != QTBranch::OUT_OF_BOUNDS && _branches[quadrant] != NULL)
			{
				if (!_branches[quadrant]->isInBounds(targetPos))
					return (NULL);
				return (_branches[quadrant]->getLeaf(targetPos));
			}

			return (NULL);
		}

		void	getVisibleChunks(std::vector<Chunk *> &chunks, const Frustum &camFrustum, VolumeAABB &AABB)
		{
			glm::vec2	npos = {0, 0};
			AABB.extents = glm::vec3(_size.x / 2, 256, _size.y / 2);
			AABB.center = glm::vec3(_size.x / 2, 0, _size.y / 2);
			if (isLeaf())
			{
				Chunk *chunk = getLeaf(glm::vec2(_pos.x, _pos.y));
				chunk->rendered = true;
				chunks.push_back(chunk);
			}
			else
			{
				for (int i = QTBranch::TOP_LEFT; i != QTBranch::OUT_OF_BOUNDS; ++i)
				{
					AABB.extents = glm::vec3(_size.x / 4, 256, _size.y / 4);
					AABB.center = glm::vec3(_size.x / 4, 0, _size.y / 4);
					if (i == QTBranch::TOP_LEFT)
						npos = {_pos.x + _size.x / 2, _pos.y};
					else if (i == QTBranch::TOP_RIGHT)
						npos = {_pos.x + _size.x / 2, _pos.y + _size.y / 2};
					else if (i == QTBranch::BOTTOM_LEFT)
						npos = {_pos.x, _pos.y};
					else if (i == QTBranch::BOTTOM_RIGHT)
						npos = {_pos.x, _pos.y + _size.y / 2};
					if (AABB.isOnFrustum(camFrustum, glm::vec3(npos.x, 0, npos.y)))
					{
						if (!_branches[i])
							_branches[i] = new Quadtree(npos, (QTBranch)i, _size / 2);
						_branches[i]->getVisibleChunks(chunks, camFrustum, AABB);
					}
				}
			}
		}

		/*
			Returns the branch that contains targetPos and stops at the given depth.
			(Can be useful to figure out in wich region the player is)

			@param targetPos World position of a block inside the wanted leaf (chunk)
			@param depth Depth at wich the search should stop
		*/
		Quadtree	*getBranch(const glm::ivec2 &targetPos, int depth)
		{
			if (depth-- <= 0)
				return (this);
		
			QTBranch	quadrant = _getQuadrant(targetPos);
				
			if (quadrant != QTBranch::OUT_OF_BOUNDS && _branches[quadrant] != NULL)
			{
				if (!_branches[quadrant]->isInBounds(targetPos))
					return (NULL);
				return (_branches[quadrant]->getBranch(targetPos, depth));
			}

			return (NULL);
		}
		/*
			Frees memory by pruning branches and leaves that contain no used chunks
		*/
		void	pruneDeadLeaves(Quadtree *root) //shouldBranchDie
		{
			pruneBranch(root, QTBranch::TOP_LEFT);
			pruneBranch(root, QTBranch::TOP_RIGHT);
			pruneBranch(root, QTBranch::BOTTOM_LEFT);
			pruneBranch(root, QTBranch::BOTTOM_RIGHT);

			if (!_branches[QTBranch::TOP_LEFT] && !_branches[QTBranch::TOP_RIGHT] && !_branches[QTBranch::BOTTOM_LEFT] && !_branches[QTBranch::BOTTOM_RIGHT])
			{
				if (this == root)
					return ;
				root->_branches[this->_quadrantInRoot] = NULL;
				delete this;
			}
		}
		void	pruneBranch(Quadtree *root, QTBranch quadrant)
		{
			(void)root;
			Quadtree	*branch = _branches[quadrant];
			if (branch != NULL)
			{
				if (branch->isLeaf())
					if (branch->_leaf->isUploaded() && branch->_leaf->isGenerated()
						&& !branch->_leaf->isGenerating()
						&& !branch->_leaf->rendered)
					{
						if (branch->_leaf->getDistance() > RENDER_DISTANCE)
						{
							delete branch;
							_branches[quadrant] = NULL;
						}
					}
					else
						return ;
				else
					branch->pruneDeadLeaves(this);
			}
		}
		glm::vec2	getSize() const {return (this->_size);}
		glm::vec2	getPos() const {return (this->_pos);}
		bool	isLeaf() const {return (_leaf != NULL);}
		bool	isInBounds(const glm::ivec2 &point)
		{
			bool	inBoundsLeft = point.y >= _pos.y;
			bool	inBoundsRight = point.y < _pos.y + _size.y;
			bool	inBoundsTop = point.x < _pos.x + _size.x;
			bool	inBoundsBottom = point.x >= _pos.x;

			return (inBoundsLeft && inBoundsRight && inBoundsTop && inBoundsBottom);
		}
	private:
		//Returns branch quadrant in wich pos is. (OUT_OF_BOUNDS can be returned but will never happen) @param pos target position of branch
		QTBranch	_getQuadrant(const glm::ivec2 &pos) const
		{
			bool	isLeft = pos.y < _pos.y + _size.y / 2;
			bool	isTop = pos.x >= _pos.x + _size.x / 2;
			bool	isRight = !isLeft;
			bool	isBottom = !isTop;

			if (isLeft && isTop)
				return (QTBranch::TOP_LEFT);
			else if (isRight && isTop)
				return (QTBranch::TOP_RIGHT);
			else if (isLeft && isBottom)
				return (QTBranch::BOTTOM_LEFT);
			else if (isRight && isBottom)
				return (QTBranch::BOTTOM_RIGHT);
			else
				return (QTBranch::OUT_OF_BOUNDS);
		}
		glm::ivec2				_size;
		glm::ivec2				_pos;
		std::vector<Quadtree*>	_branches = {NULL, NULL, NULL, NULL};
		Chunk					*_leaf = NULL;
		//Current branch's position in its parent's branches
		QTBranch				_quadrantInRoot;
};

#endif
