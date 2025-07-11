/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/10 15:51:32 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunk.hpp"

float	getFakeNoise(glm::vec2 pos) //! ////////////////////////////////////////////////////////////////////////// because no noise
{
	return ((sin(pos.x * 0.04f) + sin(pos.y * 0.04f) + 2.f) / 4.f * 30.f);
}

Chunk::Chunk(const glm::vec3 &nPos)
{
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Creating the Chunk at " + sPos.str(), NORMAL);
	}
	_EBO = 0;
	_VAO = 0;
	_VBO = 0;
	_model = glm::mat4(1);
	_minHeight = 255;
	_maxHeight = 0;
	_indicesSize = 0;
	pos = nPos;
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	_chunkTop.reserve(1024);
	gen();
	genMesh();
	_indicesSize = _indices.size();
	makeBuffers();
}

Chunk::~Chunk()
{
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Destroying the Chunk at " + sPos.str(), NORMAL);
	}
	glDeleteBuffers(1, &_EBO);
	glDeleteBuffers(1, &_VBO);
	glDeleteVertexArrays(1, &_VAO);
}

float	Chunk::getDistance() const
{
	return (glm::length(CAMERA->pos - (pos + glm::vec3(16.f, 0.f, 16.f))));
}

bool	Chunk::isInRange()
{
	return (glm::length(CAMERA->pos - (pos + glm::vec3(16.0f, 0.0f, 16.0f))));
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
		slice = groundData.find((height + i))->second;
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
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);
    
	glGenBuffers(1, &_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), (GLuint*)_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)(5 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void	addVertices(float type, std::vector<float> &vertices, std::vector<int> &_indices, const glm::vec3 &TL, const glm::vec3 &TR, const glm::vec3 &BL, const glm::vec3 &BR)
{
	vertices.insert(vertices.end(), {BL.x, BL.y, BL.z, 0, 0, type, TL.x, TL.y, TL.z, 0, 1, type, TR.x, TR.y, TR.z, 1, 1, type, BR.x, BR.y, BR.z, 1, 0, type});
	int vertLen = vertices.size() / LINELEN - 1;
	_indices.insert(_indices.end(), {vertLen - 3, vertLen - 1, vertLen - 2, vertLen - 3, vertLen - 0, vertLen - 1});
}

void	Chunk::genMesh()
{
	char32_t				chunkSlice;
	std::vector<char32_t>	rotSlices;
	rotSlices.reserve(32);

	// Gen mesh for ground
	for (int i = 0; i <= _maxHeight; ++i)
	{
		rotSlices.clear();
		getRotSlice(rotSlices, i * 32);
		for (int ii = 0; ii < 32; ++ii)
		{
			chunkSlice = groundData.find((i * 32 + ii))->second;
			if (!chunkSlice && !rotSlices[ii])
				continue;
			char32_t westFaces = culling(chunkSlice, true, int(getFakeNoise(glm::vec2{pos.x - 1, pos.z + ii})) >= i);
			char32_t eastFaces = culling(chunkSlice, false, int(getFakeNoise(glm::vec2{pos.x + 32, pos.z + ii})) >= i);
			char32_t northFaces = culling(rotSlices[ii], true, int(getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.z + 32})) >= i);
			char32_t southFaces = culling(rotSlices[ii], false, int(getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.z - 1})) >= i);

			for (int iii = 0; iii < 32; ++iii)
			{
				if ((westFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {0 + iii, 1 + i, 1 + ii}, {0 + iii, 1 + i, 0 + ii}, {0 + iii, 0 + i, 1 + ii}, {0 + iii, 0 + i, 0 + ii});
				if ((eastFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {1 + iii, 1 + i, 0 + ii}, {1 + iii, 1 + i, 1 + ii}, {1 + iii, 0 + i, 0 + ii}, {1 + iii, 0 + i, 1 + ii});
				if ((northFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {1 + (31 - ii), 1 + i, 1 + (31 - iii)}, {0 + (31 - ii), 1 + i, 1 + (31 - iii)}, {1 + (31 - ii), 0 + i, 1 + (31 - iii)}, {0 + (31 - ii), 0 + i, 1 + (31 - iii)});
				if ((southFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {0 + (31 - ii), 1 + i, 0 + (31 - iii)}, {1 + (31 - ii), 1 + i, 0 + (31 - iii)}, {0 + (31 - ii), 0 + i, 0 + (31 - iii)}, {1 + (31 - ii), 0 + i, 0 + (31 - iii)});
				if ((chunkSlice >> iii) & 1 && 
					((groundData.find(((i + 1) * 32 + ii)) != groundData.end() && !((groundData.find(((i + 1) * 32 + ii))->second >> iii) & 1))
						|| groundData.find(((i + 1) * 32 + ii)) == groundData.end()))
					addVertices(GROUND, _vertices, _indices, {0 + iii, 1 + i, 1 + ii}, {1 + iii, 1 + i, 1 + ii}, {0 + iii, 1 + i, 0 + ii}, {1 + iii, 1 + i, 0 + ii});
			}
		}
	}

	// Gen mesh for water
	for (int i = 0; i < 32; ++i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			std::unordered_map<int, char32_t>::iterator chunkSlice = waterData.find(WATERLINE * 32 + i);
			if (chunkSlice != waterData.end() && (chunkSlice->second >> ii) & 1)
				addVertices(WATER, _vertices, _indices, {0 + ii, 1 + WATERLINE, 1 + i}, {1 + ii, 1 + WATERLINE, 1 + i}, {0 + ii, 1 + WATERLINE, 0 + i}, {1 + ii, 1 + WATERLINE, 0 + i});
		}
	}
}

void	Chunk::gen()
{
	int	height;

	// gen highest of each point in chunk
	for (int i = 0; i < 32; ++i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			height = getFakeNoise(glm::vec2{pos.x + (31 - ii), pos.z + i}); // since no gen yet;
			std::unordered_map<int, char32_t>::iterator chunkSlice = groundData.find(height * 32 + i);

			if (chunkSlice != groundData.end())
				chunkSlice->second = chunkSlice->second | (char32_t)(((char32_t)1) << (31 - ii));
			else
				groundData.insert(std::pair<int, char32_t>(height * 32 + i, (char32_t)(((char32_t)1) << (31 - ii))));

			if (height > _maxHeight)
				_maxHeight = height;
			if (height < _minHeight)
				_minHeight = height;
			_chunkTop.push_back(height);
		}
	}

	// fill the bottom
	for (int i = _maxHeight; i > 0; --i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			std::unordered_map<int, char32_t>::iterator chunkSlice = groundData.find((i * 32 + ii));

			if (chunkSlice != groundData.end())
			{
				std::unordered_map<int, char32_t>::iterator underSlice = groundData.find(((i - 1) * 32 + ii));
				if (underSlice != groundData.end())
					underSlice->second = underSlice->second | chunkSlice->second;
				else
					groundData.insert(std::pair<int, char32_t>(((i - 1) * 32 + ii), chunkSlice->second));
			}
			else
			{
				if (i > _minHeight)
					groundData.insert(std::pair<int, char32_t>((i * 32 + ii), 0));
				else
					groundData.insert(std::pair<int, char32_t>((i * 32 + ii), char32_t(2147483648)));
			}
		}
	}

	// Add water
	for (int i = 0; i < 32; ++i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			for (int iii = WATERLINE; iii > _chunkTop[i * 32 + ii]; --iii)
			{
				std::unordered_map<int, char32_t>::iterator chunkSlice = waterData.find(iii * 32 + i);

				if (chunkSlice != waterData.end())
					chunkSlice->second = chunkSlice->second | (char32_t)(((char32_t)1) << (31 - ii));
				else
					waterData.insert(std::pair<int, char32_t>(iii * 32 + i, (char32_t)(((char32_t)1) << (31 - ii))));
			}
		}
	}
}

void	Chunk::draw(Shader &shader)
{
    glEnable(GL_DEPTH_TEST);
	shader.use();
	shader.setMat4("model", model);
	CAMERA->setViewMatrix(shader);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, _indicesSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);
}
