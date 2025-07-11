/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   QuadTree.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 17:46:24 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/11 06:45:11 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef QUADTREE_HPP
# define QUADTREE_HPP

#include "libs.hpp"
#include "Chunk.hpp"

/*
	Quad tree branch enum
*/
enum QTBranch
{
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};
/*
	Tree to store chunks, each branch has 4 more branches.

	When a branch will try to grow a branch of size 32, it will be a leaf (Chunk)
*/
class	Quadtree
{
	public:
		Quadtree(const glm::vec2 &pos, const glm::vec2 &size)
		{
			this->_pos = pos;
			this->_size = size;

			if (this->_size == glm::vec2(32))
				this->_leaf = new Chunk(glm::vec3(_pos.x, _pos.y, 0));
		}
		~Quadtree()
		{
			for (auto *branch : _branches)
				delete branch;
			if (_leaf)
				delete _leaf;
		}
		void	print()
		{
			std::cout << "Position " << _pos.x << ", " << _pos.y << " | Size " << _size.x << ", " << _size.y << std::endl;
			if (isLeaf())
				return ;
				
			if (_branches[TOP_LEFT])
			{
				_branches[TOP_LEFT]->print();
			}
			if (_branches[TOP_RIGHT])
			{
				_branches[TOP_RIGHT]->print();
			}
			if (_branches[BOTTOM_LEFT])
			{
				_branches[BOTTOM_LEFT]->print();
			}
			if (_branches[BOTTOM_RIGHT])
			{
				_branches[BOTTOM_RIGHT]->print();
			}
		}
		bool	isLeaf() {return (_leaf != NULL);}
		Chunk	*growBranch(const glm::vec2 &targetPos)
		{
			if (isLeaf())
				return (_leaf);
			
			if (targetPos.x >= _pos.x + _size.x / 2.f && targetPos.y <= _pos.y + _size.y / 2.f) //Top left
			{
				if (_branches[QTBranch::TOP_LEFT] == NULL)
					_branches[QTBranch::TOP_LEFT] = new Quadtree(glm::vec2(_pos.x + _size.x / 2, _pos.y), _size / 2.0f);
				return (_branches[QTBranch::TOP_LEFT]->growBranch(targetPos));
			}
				
			else if (targetPos.x >= _pos.x + _size.x / 2.f && targetPos.y >= _pos.y + _size.y / 2.f) //Top right
			{
				if (_branches[QTBranch::TOP_RIGHT] == NULL)
					_branches[QTBranch::TOP_RIGHT] = new Quadtree(glm::vec2(_pos.x + _size.x / 2, _pos.y + _size.y / 2), _size / 2.0f);
				return (_branches[QTBranch::TOP_RIGHT]->growBranch(targetPos));
			}
				
			else if (targetPos.x <= _pos.x + _size.x / 2.f && targetPos.y <= _pos.y + _size.y / 2.f) //Bottom left
			{
				if (_branches[QTBranch::BOTTOM_LEFT] == NULL)
					_branches[QTBranch::BOTTOM_LEFT] = new Quadtree(glm::vec2(_pos.x, _pos.y), _size / 2.0f);
				return (_branches[QTBranch::BOTTOM_LEFT]->growBranch(targetPos));
			}

			else if (targetPos.x <= _pos.x + _size.x / 2.f && targetPos.y >= _pos.y + _size.y / 2.f) //Bottom right
			{
				if (_branches[QTBranch::BOTTOM_RIGHT] == NULL)
					_branches[QTBranch::BOTTOM_RIGHT] = new Quadtree(glm::vec2(_pos.x, _pos.y + _size.y / 2), _size / 2.0f);
				return (_branches[QTBranch::BOTTOM_RIGHT]->growBranch(targetPos));
			}
			consoleLog("WARNING could not find/create a leaf from the given branch", LogSeverity::WARNING);
			return (NULL);
		}

		Chunk	*TryGrowBranch(const glm::vec2 &targetPos)
		{
			if (isLeaf())
				return (_leaf);
			
			if (targetPos.x >= _pos.x + _size.x / 2.f && targetPos.y <= _pos.y + _size.y / 2.f) //Top left
			{
				if (_branches[QTBranch::TOP_LEFT] != NULL)
					return (_branches[QTBranch::TOP_LEFT]->growBranch(targetPos));
			}
				
			else if (targetPos.x >= _pos.x + _size.x / 2.f && targetPos.y >= _pos.y + _size.y / 2.f) //Top right
			{
				if (_branches[QTBranch::TOP_RIGHT] != NULL)
					return (_branches[QTBranch::TOP_RIGHT]->growBranch(targetPos));
			}
				
			else if (targetPos.x <= _pos.x + _size.x / 2.f && targetPos.y <= _pos.y + _size.y / 2.f) //Bottom left
			{
				if (_branches[QTBranch::BOTTOM_LEFT] != NULL)
					return (_branches[QTBranch::BOTTOM_LEFT]->growBranch(targetPos));
			}

			else if (targetPos.x <= _pos.x + _size.x / 2.f && targetPos.y >= _pos.y + _size.y / 2.f) //Bottom right
			{
				if (_branches[QTBranch::BOTTOM_RIGHT] != NULL)
					return (_branches[QTBranch::BOTTOM_RIGHT]->growBranch(targetPos));
			}
			return (NULL);
		}
	private:
		glm::vec2				_size;
		glm::vec2				_pos;
		std::vector<Quadtree*>	_branches = {NULL, NULL, NULL, NULL};
		Chunk					*_leaf = NULL;
};

#endif
