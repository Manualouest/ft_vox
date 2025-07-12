/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuadTree.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 17:46:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/12 18:30:41 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUADTREE_HPP
# define QUADTREE_HPP

#include "libs.hpp"
#include "Chunk.hpp"
#include "ChunkGenerator.hpp"

extern ChunkGeneratorManager	*CHUNK_GENERATOR;

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
		Quadtree(const glm::ivec2 &pos, const glm::ivec2 &size)
		{
			this->_pos = pos;
			this->_size = size;

			if (this->_size == glm::ivec2(32))
			{
				this->_leaf = new Chunk(glm::ivec3(_pos.x, 0, _pos.y), true);
				CHUNK_GENERATOR->deposit(this->_leaf);
			}
		}
		~Quadtree()
		{
			for (auto *branch : _branches)
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

			bool	isTop = targetPos.y >= _pos.y + _size.y / 2;
			bool	isRight = !(targetPos.x < _pos.x + _size.x / 2);

			childPos.x += isTop * (_size.x / 2);
			childPos.y += isRight * (_size.y / 2);

			if (quadrant != QTBranch::OUT_OF_BOUNDS)
			{
				if (_branches[quadrant] == NULL)
					_branches[quadrant] = new Quadtree(childPos, _size / 2);
				return (_branches[quadrant]->growBranch(targetPos));
			}

			consoleLog("WARNING could not find/create a leaf from the given branch", LogSeverity::WARNING);
			return (NULL);
		}
		/*
			Goes through branches until it can find a leaf on the given position without creating new branches.
			If it cant find a leaf, NULL will be returned.
		
			#param targetPos World position of a block inside the wanted leaf (chunk)
		*/
		Chunk	*getLeaf(const glm::ivec2 &targetPos)
		{
			if (isLeaf())
				return (_leaf);
			
			QTBranch	quadrant = _getQuadrant(targetPos);

			if (quadrant != QTBranch::OUT_OF_BOUNDS && _branches[quadrant] != NULL)
				return (_branches[quadrant]->getLeaf(targetPos));

			return (NULL);
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
				return (_branches[quadrant]->getBranch(targetPos, depth));

			return (NULL);
		}
		/*
			Frees memory by pruning branches that contain no used chunks
		*/
		void	pruneDeadLeaves() //shouldBranchDie
		{
			if (isLeaf() && !_leaf->rendered && _leaf->uploaded)
				_leaf->clear();

			if (_branches[QTBranch::TOP_LEFT] != NULL)
				_branches[QTBranch::TOP_LEFT]->pruneDeadLeaves();

			if (_branches[QTBranch::TOP_RIGHT] != NULL)
				_branches[QTBranch::TOP_RIGHT]->pruneDeadLeaves();

			if (_branches[QTBranch::BOTTOM_LEFT] != NULL)
				_branches[QTBranch::BOTTOM_LEFT]->pruneDeadLeaves();

			if (_branches[QTBranch::BOTTOM_RIGHT] != NULL)
				_branches[QTBranch::BOTTOM_RIGHT]->pruneDeadLeaves();
		}
		glm::vec2	getSize() const {return (this->_size);}
		glm::vec2	getPos() const {return (this->_pos);}
		bool	isLeaf() const {return (_leaf != NULL);}
	private:
		//Returns branch quadrant in wich pos is. (OUT_OF_BOUNDS can be returned but will never happen) @param pos target position of branch
		QTBranch	_getQuadrant(const glm::vec2 &pos) const
		{
			bool	isLeft = pos.x < _pos.x + _size.x / 2;
			bool	isTop = pos.y >= _pos.y + _size.y / 2;
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
};

#endif
