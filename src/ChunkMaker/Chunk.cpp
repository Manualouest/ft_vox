/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/10 11:31:31 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunk.hpp"

float	getFakeNoise(glm::vec2 pos) //! ////////////////////////////////////////////////////////////////////////// because no noise
{
	return (5);
	return ((sin(pos.x * 0.04f) + sin(pos.y * 0.04f) + 2.f) / 4.f * 30.f);
}

Chunk::Chunk(const glm::vec3 &nPos)
{
	_EBO = 0;
	_VAO = 0;
	_VBO = 0;
	_model = glm::mat4(1);
	_minHeight = 255;
	_maxHeight = 0;
	_indicesSize = 0;
	pos = nPos;
	gen();
	_indicesSize = _indices.size();
	genMesh();

	// int i = 0;
	// std::vector<int>::iterator indice = _indices.begin();
	// for (std::vector<float>::iterator it = _vertices.begin(); it != _vertices.end(); it++)
	// {
	// 	if (i == 5)
	// 	{
	// 		if (indice != _indices.end())
	// 		{
	// 			std::cout << *(indice);
	// 			indice ++;
	// 		}
	// 		std::cout << std::endl;
	// 		i = 0;
	// 	}
	// 	std::cout << *(it) << "; ";
	// 	++i;
	// }
	// while (indice != _indices.end())
	// {
	// 	std::cout << *(indice) << "; ";
	// 	indice ++;
	// }

	makeBuffers();
}

Chunk::~Chunk()
{
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Destroying Chunk at " + sPos.str(), NORMAL);
	}
	glDeleteBuffers(1, &_EBO);
	glDeleteBuffers(1, &_VBO);
	glDeleteVertexArrays(1, &_VAO);
}

char32_t	culling(const char32_t &slice, const bool &dir, const int &edge)
{
	if (dir)
		return (slice & ~((slice << 1) | edge)); // right faces
	else
		return (slice & ~((slice >> 1) | (edge << 31))); // left faces
}

void	Chunk::getRotSlice(std::vector<char32_t> &rotSlice, const int &height)
{
	char32_t slice;
	for (int i = 0; i < 32; ++i)
	{
		slice = chunkData.find((height + i))->second;
		for (int ii = 0; ii < 32; ++ii)
			rotSlice[ii] = rotSlice[ii] << 1 | (((slice >> (31 - ii)) & 1));
	}
}

void	Chunk::makeBuffers()
{
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices.data(), GL_STATIC_DRAW);
    
	glGenBuffers(1, &_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), (GLuint*)_indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);
}

void	addVertices(std::vector<float> &vertices, std::vector<int> &indices, const glm::vec3 &TL, const glm::vec3 &TR, const glm::vec3 &BL, const glm::vec3 &BR)
{
	vertices.insert(vertices.end(), {BL.x, BL.y, BL.z, 0, 0, TL.x, TL.y, TL.z, 0, 1, TR.x, TR.y, TR.z, 1, 1, BR.x, BR.y, BR.z, 1, 0});
	int vertLen = vertices.size() / 5 - 1;
	indices.insert(indices.end(), {vertLen - 3, vertLen - 1, vertLen - 2, vertLen - 3, vertLen - 0, vertLen - 1});
}

void	Chunk::genMesh()
{
	char32_t				chunkSlice;
	std::vector<char32_t>	rotSlices;
	rotSlices.reserve(32);

	for (int i = 0; i <= _maxHeight; ++i)
	{
		rotSlices.clear();
		getRotSlice(rotSlices, i * 32);
		for (int ii = 0; ii < 32; ++ii)
		{
			chunkSlice = chunkData.find((i * 32 + ii))->second;
			if (!chunkSlice && !rotSlices[ii])
				continue;
			char32_t westFaces = culling(chunkSlice, true, int(getFakeNoise(glm::vec2{pos.x - 1, pos.y + ii})) >= i);
			char32_t eastFaces = culling(chunkSlice, false, int(getFakeNoise(glm::vec2{pos.x + 32, pos.y + ii})) >= i);
			char32_t northFaces = culling(rotSlices[ii], true, int(getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.y + 32})) >= i);
			char32_t southFaces = culling(rotSlices[ii], false, int(getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.y - 1})) >= i);

			for (int iii = 0; iii < 32; ++iii)
			{
				if ((westFaces >> iii) & 1)
					addVertices(_vertices, _indices, {0 + pos.x + iii, 1 + i, 1 + pos.y + ii}, {0 + pos.x + iii, 1 + i, 0 + pos.y + ii}, {0 + pos.x + iii, 0 + i, 1 + pos.y + ii}, {0 + pos.x + iii, 0 + i, 0 + pos.y + ii});
				if ((eastFaces >> iii) & 1)
					addVertices(_vertices, _indices, {1 + pos.x + iii, 1 + i, 0 + pos.y + ii}, {1 + pos.x + iii, 1 + i, 1 + pos.y + ii}, {1 + pos.x + iii, 0 + i, 0 + pos.y + ii}, {1 + pos.x + iii, 0 + i, 1 + pos.y + ii});
				if ((northFaces >> iii) & 1)
					addVertices(_vertices, _indices, {1 + pos.x + (31 - ii), 1 + i, 1 + pos.y + (31 - iii)}, {0 + pos.x + (31 - ii), 1 + i, 1 + pos.y + (31 - iii)}, {1 + pos.x + (31 - ii), 0 + i, 1 + pos.y + (31 - iii)}, {0 + pos.x + (31 - ii), 0 + i, 1 + pos.y + (31 - iii)});
				if ((southFaces >> iii) & 1)
					addVertices(_vertices, _indices, {0 + pos.x + (31 - ii), 1 + i, 0 + pos.y + (31 - iii)}, {1 + pos.x + (31 - ii), 1 + i, 0 + pos.y + (31 - iii)}, {0 + pos.x + (31 - ii), 0 + i, 0 + pos.y + (31 - iii)}, {1 + pos.x + (31 - ii), 0 + i, 0 + pos.y + (31 - iii)});
				if ((chunkSlice >> iii) & 1 && 
					((chunkData.find(((i + 1) * 32 + ii)) != chunkData.end() && !((chunkData.find(((i + 1) * 32 + ii))->second >> iii) & 1))
						|| chunkData.find(((i + 1) * 32 + ii)) == chunkData.end()))
					addVertices(_vertices, _indices, {0 + pos.x + iii, 1 + i, 1 + pos.y + ii}, {1 + pos.x + iii, 1 + i, 1 + pos.y + ii}, {0 + pos.x + iii, 1 + i, 0 + pos.y + ii}, {1 + pos.x + iii, 1 + i, 0 + pos.y + ii});
			}
		}
	}
}

void	Chunk::gen()
{
	int	height;

	for (int i = 0; i < 32; ++i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			height = getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.y + i}); // since no gen yet;
			std::unordered_map<int, char32_t>::iterator chunkSlice = chunkData.find(height * 32 + i);

			if (chunkSlice != chunkData.end())
				chunkSlice->second = chunkSlice->second | (char32_t)(((char32_t)1) << (31 - ii));
			else
				chunkData.insert(std::pair<int, char32_t>(height * 32 + i, (char32_t)(((char32_t)1) << (31 - ii))));

			if (height > _maxHeight)
				_maxHeight = height;
			if (height < _minHeight)
				_minHeight = height;
		}
	}

	for (int i = _maxHeight; i > 0; --i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			std::unordered_map<int, char32_t>::iterator chunkSlice = chunkData.find((i * 32 + ii));

			if (chunkSlice != chunkData.end())
			{
				std::unordered_map<int, char32_t>::iterator underSlice = chunkData.find(((i - 1) * 32 + ii));
				if (underSlice != chunkData.end())
					underSlice->second = underSlice->second | chunkSlice->second;
				else
					chunkData.insert(std::pair<int, char32_t>(((i - 1) * 32 + ii), chunkSlice->second));
			}
			else
			{
				if (i > _minHeight)
					chunkData.insert(std::pair<int, char32_t>((i * 32 + ii), 0));
				else
					chunkData.insert(std::pair<int, char32_t>((i * 32 + ii), char32_t(2147483648)));
			}
		}
	}
	for (int i = 0; i < 32; ++i)
	{
		std::unordered_map<int, char32_t>::iterator chunkSlice = chunkData.find((i));
		if (chunkSlice == chunkData.end())
			chunkData.insert(std::pair<int, char32_t>((i), char32_t(2147483648)));
	}
}

void	Chunk::draw(Camera &camera, Shader &shader)
{
    glEnable(GL_DEPTH_TEST);
	shader.use();
	camera.setViewMatrix(shader);

	glBindVertexArray(_VAO);
	glDrawElements(GL_TRIANGLES, _indicesSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}




// glm::vec3	pos;
// priv:
// unsigned int		_EBO;
// unsigned int		_VAO;
// unsigned int		_VBO;
// glm::mat4			_model;
// unsigned int		_minHeight:8;
// unsigned int		_maxHeight:8;
// std::vector<float>	_vertices;
// std::vector<int>	_indices;
// int					_indicesSize;