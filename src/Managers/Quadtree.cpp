/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Quadtree.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:15:07 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/28 16:55:03 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "QuadTree.hpp"

uint	_QT_SizeBranches;
uint	_QT_SizeLeaves;

Quadtree::Quadtree(const glm::ivec2 &pos, QTBranch quadrant, const glm::ivec2 &size)
{
	this->_pos = pos;
	this->_size = size;
	this->_quadrantInRoot = quadrant;

	if (this->_size == glm::ivec2(32))
		this->_leaf = new Chunk(glm::ivec3(_pos.x, 0, _pos.y));
}

Quadtree::~Quadtree()
{
	for (auto *branch : _branches)
		if (branch)
			delete branch;
	if (_leaf)
		delete _leaf;
}

void	Quadtree::getVisibleChunks(std::vector<Chunk *> &chunks, const Frustum &camFrustum, VolumeAABB &AABB)
{
	glm::vec2	npos = {0, 0};
	
	if (isLeaf())
	{
		Chunk *chunk = getLeaf(glm::vec2(_pos.x, _pos.y));
		
		AABB.extents = glm::vec3(16, (std::max(chunk->_minHeight, chunk->_maxHeight) + 16) / 2, 16);
		AABB.center = glm::vec3(16, AABB.extents.y, 16);

		if (AABB.isOnFrustum(camFrustum, glm::vec3(_pos.x, 0, _pos.y)))
		{
			chunk->rendered = true;
			chunks.push_back(chunk);
		}
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

			AABB.extents = glm::vec3(_size.x / 4, 128, _size.y / 4);
			AABB.center = glm::vec3(_size.x / 4, 128, _size.y / 4);

			if (AABB.isOnFrustum(camFrustum, glm::vec3(npos.x, 0, npos.y)))
			{
				if (!_branches[i])
					_branches[i] = new Quadtree(npos, (QTBranch)i, _size / 2);
				_branches[i]->getVisibleChunks(chunks, camFrustum, AABB);
			}
		}
	}
}

Chunk	*Quadtree::growBranch(const glm::ivec2 &targetPos)
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

Quadtree	*Quadtree::getBranch(const glm::ivec2 &targetPos, int depth)
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

Chunk	*Quadtree::getLeaf(const glm::ivec2 &targetPos)
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

QTBranch	Quadtree::_getQuadrant(const glm::ivec2 &pos) const
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

void	Quadtree::pruneDeadLeaves(Quadtree *root) //shouldBranchDie
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

void	Quadtree::pruneBranch(Quadtree *root, QTBranch quadrant)
{
	(void)root;
	Quadtree	*branch = _branches[quadrant];
	if (branch != NULL)
	{
		if (branch->isLeaf())
			if (!branch->_leaf->isGenerating()
				&& !branch->_leaf->rendered
				&& !branch->_leaf->_edited)
			{
				if (branch->_leaf->getDistance() > RENDER_DISTANCE) // !FIX HERE, WRONG RENDER DISTANCE BEING USED OR SMTH (TOO MANY CHUNKS ARE STAYING)
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

bool	Quadtree::isInBounds(const glm::ivec2 &point)
{
	bool	inBoundsLeft = point.y >= _pos.y;
	bool	inBoundsRight = point.y < _pos.y + _size.y;
	bool	inBoundsTop = point.x < _pos.x + _size.x;
	bool	inBoundsBottom = point.x >= _pos.x;

	return (inBoundsLeft && inBoundsRight && inBoundsTop && inBoundsBottom);
}
