/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/15 12:09:45 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunk.hpp"
#include "RegionManager.hpp"
#include "ChunkGenerator.hpp"
extern RegionManager	*CHUNKS;
extern ChunkGeneratorManager	*CHUNK_GENERATOR;

#include <bitset>


glm::vec2 randomGradient(int ix, int iy)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; 
    unsigned a = ix, b = iy;
    a *= 3284157443;

	b ^= a << s | a >> (w - s);
    b *= 1911520717;

	a ^= b << s | b >> (w - s);
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);

	return v;
}

float dotGridGradient(int ix, int iy, float x, float y)
{
    glm::vec2 gradient = randomGradient(ix, iy);

	float dx = x - (float)ix;
    float dy = y - (float)iy;

	return (dx * gradient.x + dy * gradient.y);
}

float interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}

float perlin(float x, float y)
{
    int x0 = (int)x; 
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float sx = x - (float)x0;
    float sy = y - (float)y0;

	float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    float value = interpolate(ix0, ix1, sy);
    
    return (value);
}

float	calcNoise(const glm::vec2 &pos, float freq, float amp, int noisiness, float heightScale)
{
	float	res = 0;
	for (int i = 0; i < noisiness; i++)
	{
		res += perlin(pos.x * freq, pos.y * freq) * amp;
	
		freq *= 2;
		amp /= 2;	
	}

	if (res > 1.0f)
		res = 1.0f;
	else if (res < -1.0f)
		res = -1.0f;
		
	res = ((res + 1.0f) * 0.5f) * heightScale;

	return (res);
}

#define MOUNTAINS_FREQ 0.25 / 32
#define MOUNTAINS_AMP 1.0
#define MOUNTAINS_NOISE 5

#define PLAINS_FREQ 0.3 / 32
#define PLAINS_AMP 0.2
#define PLAINS_NOISE 3

#define OCEAN_FREQ 0.3 / 32
#define OCEAN_AMP 0.1
#define OCEAN_NOISE 3

struct	Biome
{
	float	frequency;
	float	amplitude;
	float	noisiness;
	float	heightScale;
	float	bias;
	float	center;
	float	range;
};

float getFakeNoise(glm::vec2 pos)
{
	float biomeNoise = calcNoise(pos * 0.001f, 1.0f, 1.0f, 1, 1.0f);
	float biomeSelector = biomeNoise;

	float plains = calcNoise(pos, PLAINS_FREQ, PLAINS_AMP, PLAINS_NOISE, 90.0f);
	float hills = calcNoise(pos, MOUNTAINS_FREQ, MOUNTAINS_AMP, MOUNTAINS_NOISE, 240.0f);
	float ocean = calcNoise(pos, OCEAN_FREQ, OCEAN_AMP, OCEAN_NOISE, 50.0f);

	float plainsWeight = glm::smoothstep(0.3f, 0.5f, biomeSelector);
	float hillsWeight = glm::smoothstep(0.5f, 0.7f, biomeSelector);
	float oceanWeight = 1.5 - (plainsWeight + hillsWeight) / 2;

	int res = ocean * oceanWeight + plains * plainsWeight + hills * hillsWeight;
	return (glm::clamp(res, 0, 255));
}

Chunk::Chunk(const glm::vec3 &nPos) : rendered(false), _edited(false), _generated(false), _needGen(true), _generating(false), _uploaded(false)
{
	_model = glm::mat4(1);
	_minHeight = 255;
	_maxHeight = 0;
	_indicesSize = 0;
	pos = nPos;
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
}

void	Chunk::generate()
{
	if (_generated)
		return ;
	if (_needGen)
	{
		_chunkTop.reserve(1024);
		gen();
	}
	genMesh();
	_indicesSize = _indices.size();
	_generated.store(true);
	_needGen.store(false);
}

void	Chunk::upload()
{
	makeBuffers();
	_uploaded = true;
}

Chunk::~Chunk()
{
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Destroying the Chunk at " + sPos.str(), NORMAL);
	}
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	groundData.clear();
	waterData.clear();
	_vertices.clear();
	_vertices.shrink_to_fit();
	_indices.clear();
	_indices.shrink_to_fit();
}

float	Chunk::getDistance() const
{
	return (glm::length(CAMERA->pos - (pos + glm::vec3(16.f, CAMERA->pos.y, 16.f))));
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

void	Chunk::reset()
{
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_uploaded.store(false);
	_vertices.clear();
	_vertices.shrink_to_fit();
	_indices.clear();
	_indices.shrink_to_fit();
	_generated.store(false);
}

void	Chunk::clear()
{
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_uploaded.store(false);
}

void	Chunk::makeBuffers()
{
	glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
	glGenBuffers(1, &_EBO);
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);
    
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), (GLuint*)_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, LINELEN * sizeof(float), (void*)(6 * sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void	addVertices(float type, std::vector<float> &vertices, std::vector<int> &_indices, const glm::vec3 &TL, const glm::vec3 &TR, const glm::vec3 &BL, const glm::vec3 &BR, const glm::vec3 &Normal)
{
	vertices.insert(vertices.end(), {BL.x, BL.y, BL.z, 0, 0, type, Normal.x, Normal.y, Normal.z, TL.x, TL.y, TL.z, 0, 1, type, Normal.x, Normal.y, Normal.z, TR.x, TR.y, TR.z, 1, 1, type, Normal.x, Normal.y, Normal.z, BR.x, BR.y, BR.z, 1, 0, type, Normal.x, Normal.y, Normal.z});
	int vertLen = vertices.size() / LINELEN - 1;
	_indices.insert(_indices.end(), {vertLen - 3, vertLen - 1, vertLen - 2, vertLen - 3, vertLen - 0, vertLen - 1});
}

void	Chunk::checkSurround(const glm::ivec3 &chunkPos, const Block &block, const char32_t &slice, const char32_t &rotSlice, const char32_t &up, const char32_t &down)
{
	Chunk	*chunk;

	if ((slice >> (chunkPos.x)) & 1)
	{
		if (chunkPos.x != 31 && !((slice >> (chunkPos.x + 1)) & 1))
			addVertices(block.eastFace, _vertices, _indices, {1 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {1, 0, 0});
		if (chunkPos.x != 0 && !((slice >> (chunkPos.x - 1)) & 1))
			addVertices(block.westFace, _vertices, _indices, {0 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {-1, 0, 0});
	}

	if (chunkPos.x == 31 && ((slice >> (chunkPos.x)) & 1))
	{
		chunk = CHUNKS->getQuadTree()->getLeaf({pos.x + 32, pos.z});
		if (chunk && chunk->_generated && !chunk->isOnBlock(chunkPos - glm::ivec3(31, 0, 0)))
			addVertices(block.eastFace, _vertices, _indices, {1 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {1, 0, 0});
		else if (int(getFakeNoise(glm::vec2{pos.x + 32, pos.z + chunkPos.z})) < chunkPos.y)
			addVertices(block.eastFace, _vertices, _indices, {1 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {1, 0, 0});
	}
	if (chunkPos.x == 0 && ((slice >> (chunkPos.x)) & 1))
	{
		chunk = CHUNKS->getQuadTree()->getLeaf({pos.x - 32, pos.z});
		if (chunk && chunk->_generated && !chunk->isOnBlock(chunkPos + glm::ivec3(31, 0, 0)))
			addVertices(block.westFace, _vertices, _indices, {0 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {-1, 0, 0});
		else if (int(getFakeNoise(glm::vec2{pos.x - 1, pos.z + chunkPos.z})) < chunkPos.y)
			addVertices(block.westFace, _vertices, _indices, {0 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 0 + chunkPos.y, 0 + chunkPos.z}, {-1, 0, 0});
	}


	if ((rotSlice >> 1) & 1)
	{
		if (chunkPos.x != 0 && !((rotSlice >> 2) & 1))
			addVertices(block.southFace, _vertices, _indices, {0 + (chunkPos.x), 1 + chunkPos.y, 0 + (chunkPos.z)}, {1 + (chunkPos.x), 1 + chunkPos.y, 0 + (chunkPos.z)}, {0 + (chunkPos.x), 0 + chunkPos.y, 0 + (chunkPos.z)}, {1 + (chunkPos.x), 0 + chunkPos.y, 0 + (chunkPos.z)}, {0, 0, -1});
		if (chunkPos.x != 31 && !((rotSlice) & 1))
			addVertices(block.northFace, _vertices, _indices, {1 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {1 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0, 0, 1});
	}
	if (chunkPos.x == 31 && ((rotSlice >> 1) & 1))//
	{
		chunk = CHUNKS->getQuadTree()->getLeaf({pos.x, pos.z + 32});
		if (chunk&& chunk->_generated  && !chunk->isOnBlock(glm::ivec3(chunkPos.x, chunkPos.y, chunkPos.z) - glm::ivec3(0, 0, 31))) //&& chunk->_generated
			addVertices(block.northFace, _vertices, _indices, {1 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {1 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0, 0, 1});
		if (int(getFakeNoise(glm::vec2{pos.x + (chunkPos.z), pos.z + 32})) < chunkPos.y)
			addVertices(block.northFace, _vertices, _indices, {1 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 1 + chunkPos.y, 1 + (chunkPos.z)}, {1 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0 + (chunkPos.x), 0 + chunkPos.y, 1 + (chunkPos.z)}, {0, 0, 1});
	}

	if ((slice >> (chunkPos.x)) & 1)
	{
		if (!((up >> (chunkPos.x)) & 1))
			addVertices(block.topFace, _vertices, _indices, {0 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {1 + chunkPos.x, 1 + chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, 1 + chunkPos.y, 0 + chunkPos.z}, {0, 1, 0});
	}
	if (chunkPos.y > 0 && (slice >> (chunkPos.x)) & 1)
	{
		if (!((down >> (chunkPos.x)) & 1))
		{
			std::cout << "oi" << std::endl; //																						3, 2, 1, 3, 4, 2
			addVertices(block.bottomFace, _vertices, _indices, {0 + chunkPos.x, chunkPos.y, 1 + chunkPos.z}, {0 + chunkPos.x, chunkPos.y, 0 + chunkPos.z}, {1 + chunkPos.x, chunkPos.y, 1 + chunkPos.z}, {1 + chunkPos.x, chunkPos.y, 0 + chunkPos.z}, {0, 1, 0});
		}
	}
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

void	Chunk::genMesh()
{
	char32_t				slice;
	std::unordered_map<int, char32_t>::iterator chunkSlice;
	std::vector<char32_t>	rotSlices;
	rotSlices.reserve(32);

	// if (pos.x != 0 || pos.y != 0)
	// 	return ;

	// Gen mesh for ground
	for (int y = 0; y <= _maxHeight; ++y)
	{
		rotSlices.clear();
		getRotSlice(rotSlices, y * 32);


		// std::bitset<32>	b(0);
		// std::cout << "normal:" << std::endl;
		// for (int i = 0; i < 32; ++i)
		// {
		// 	std::unordered_map<int, char32_t>::iterator slice = groundData.find(y * 32 + i);
		// 	b = 0;
		// 	if (slice != groundData.end())
		// 		b = slice->second;
		// 	std::cout << "	" << b << std::endl;
		// }
		// std::cout << "rot:" << std::endl;
		// for (int i = 0; i < 32; ++i)
		// {
		// 	b = rotSlices[i];
		// 	std::cout << "	" << b << std::endl;
		// }


		for (int z = 0; z < 32; ++z)
		{
			Block	block(1);
			if (1 + y < 86 && 1 + y > 64)
				block = Block(4, 4, 4, 4, 3, 2);
			else if (1 + y < 65)
				block = Block(5);
			else
				block = Block(1);

			slice = groundData.find((y * 32 + z))->second;
			if (!slice && !rotSlices[z])
				continue;
			// char32_t westFaces = culling(slice, true, int(getFakeNoise(glm::vec2{pos.x - 1, pos.z + z})) >= y);
			// char32_t eastFaces = culling(slice, false, int(getFakeNoise(glm::vec2{pos.x + 32, pos.z + z})) >= y);
			// char32_t northFaces = culling(rotSlices[z], true, int(getFakeNoise(glm::vec2{pos.x + (31 - z), pos.z + 32})) >= y);
			// char32_t southFaces = culling(rotSlices[z], false, int(getFakeNoise(glm::vec2{pos.x + (31 - z), pos.z - 1})) >= y);

			for (int x = 0; x < 32; ++x)
			{
				chunkSlice = groundData.find(((y + 1) * 32 + z));
				char32_t	up = 0;
				if (chunkSlice != groundData.end())
					up = chunkSlice->second;

				chunkSlice = groundData.find(((y - 1) * 32 + z));
				char32_t	down = 0;
				if (chunkSlice != groundData.end())
					down = chunkSlice->second;
				
				char32_t	rotSlice = (groundData.find((y * 32 + z - 1)) != groundData.end() ? (groundData.find((y * 32 + z - 1))->second >> x & 1) : 0) << 2 | (slice >> x & 1) << 1 | (groundData.find((y * 32 + z + 1)) != groundData.end() ? (groundData.find((y * 32 + z + 1))->second >> x & 1) >> x & 1 : 0);
				checkSurround({x, y, z}, block, slice, rotSlice, up, down);
				// if ((westFaces >> x) & 1)
				// 	addVertices(block.westFace, _vertices, _indices, {0 + x, 1 + y, 1 + z}, {0 + x, 1 + y, 0 + z}, {0 + x, 0 + y, 1 + z}, {0 + x, 0 + y, 0 + z}, {-1, 0, 0});
				// if ((eastFaces >> x) & 1)
				// 	addVertices(block.eastFace, _vertices, _indices, {1 + x, 1 + y, 0 + z}, {1 + x, 1 + y, 1 + z}, {1 + x, 0 + y, 0 + z}, {1 + x, 0 + y, 1 + z}, {1, 0, 0});
				// if ((northFaces >> x) & 1)
				// 	addVertices(block.northFace, _vertices, _indices, {1 + (31 - z), 1 + y, 1 + (31 - x)}, {0 + (31 - z), 1 + y, 1 + (31 - x)}, {1 + (31 - z), 0 + y, 1 + (31 - x)}, {0 + (31 - z), 0 + y, 1 + (31 - x)}, {0, 0, 1});
				// if ((southFaces >> x) & 1)
				// 	addVertices(block.southFace, _vertices, _indices, {0 + (31 - z), 1 + y, 0 + (31 - x)}, {1 + (31 - z), 1 + y, 0 + (31 - x)}, {0 + (31 - z), 0 + y, 0 + (31 - x)}, {1 + (31 - z), 0 + y, 0 + (31 - x)}, {0, 0, -1});
				// if ((slice >> x) & 1 && 
				// 	((groundData.find(((y + 1) * 32 + z)) != groundData.end() && !((groundData.find(((y + 1) * 32 + z))->second >> x) & 1))
				// 		|| groundData.find(((y + 1) * 32 + z)) == groundData.end()))
				// 	addVertices(block.topFace, _vertices, _indices, {0 + x, 1 + y, 1 + z}, {1 + x, 1 + y, 1 + z}, {0 + x, 1 + y, 0 + z}, {1 + x, 1 + y, 0 + z}, {0, 1, 0});
			}
		}
	}
	// exit(1);

	// Gen mesh for water
	for (int i = 0; i < 32; ++i)
	{
		for (int ii = 0; ii < 32; ++ii)
		{
			std::unordered_map<int, char32_t>::iterator chunkSlice = waterData.find(WATERLINE * 32 + i);
			if (chunkSlice != waterData.end() && (chunkSlice->second >> ii) & 1)
				addVertices(WATER, _vertices, _indices, {0 + ii, 1 + WATERLINE, 1 + i}, {1 + ii, 1 + WATERLINE, 1 + i}, {0 + ii, 1 + WATERLINE, 0 + i}, {1 + ii, 1 + WATERLINE, 0 + i}, {0, 1, 0});
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

#include "ChunkGenerator.hpp"
extern ChunkGeneratorManager	*CHUNK_GENERATOR;

void	Chunk::draw(Shader &shader)
{
	if (_generating)
		return ;
	if (!_uploaded && _generated)
		upload();

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


bool	Chunk::isOnBlock(const glm::vec3 &targetPos)
{
	if (targetPos.y > 256 || targetPos.y < 0)
		return (false);
	if (pos.z != 32 || pos.x != 0 || targetPos.y < 50)
		return true;
	std::bitset<32>y(0);
	std::unordered_map<int, char32_t>::iterator	slice = groundData.find(int(targetPos.y) * 32 + (int(targetPos.z) % 32));
	if (slice == groundData.end())
	{
		std::cout << "no slice data" << std::endl;
		return (false);
	}
	std::cout << "pos: " << pos.x << "; " << pos.y << "; " << pos.z << " | " << targetPos.x << "; " << targetPos.y << "; " << targetPos.z << " : ";
	y = (slice->second);
	std::cout << y << std::endl;
	return ((slice->second >> (int(targetPos.x) % 32)) & 1);
}

float	Chunk::distToBlock(const glm::vec3 &targetPos)
{
	if (targetPos.y < 0)
		return (-1);
	int	i = 0;
	int	y = targetPos.y;
	std::unordered_map<int, char32_t>::iterator	slice = groundData.find(y * 32 + (int(targetPos.z) % 32));
	while (y > 0 && (slice == groundData.end() || !((slice->second >> (int(targetPos.x) % 32)) & 1)))
	{
		slice = groundData.find(--y * 32 + (int(targetPos.z) % 32));
		++i;
	}
	return (i + (targetPos.y - std::floor(targetPos.y)));
}


bool	Chunk::removeBlock(const glm::vec3 &targetPos)
{
	std::unordered_map<int, char32_t>::iterator	slice = groundData.find(int(targetPos.y) * 32 + (int(targetPos.z) % 32));
	if (slice == groundData.end() || !((slice->second >> (int(targetPos.x) % 32)) & 1))
	{
		slice = waterData.find(int(targetPos.y) * 32 + (int(targetPos.z) % 32));
		if (slice == waterData.end() || !((slice->second >> (int(targetPos.x) % 32)) & 1))
			return (false);
	}

	_edited.store(true);

	char32_t rawSlice = slice->second;
	slice->second = ((rawSlice << int(31 - targetPos.x)) >> int(31 - targetPos.x)) ^ ((rawSlice >> int(targetPos.x)) << int(targetPos.x));

	reset();
	generate();

	glm::vec2	sideReload = {targetPos.x, targetPos.z};
	if (int(targetPos.x) % 32 == 31)
		sideReload.x += 32;
	if (int(targetPos.x) % 32 == 0)
		sideReload.x -= 32;
	if (int(targetPos.z) % 32 == 31)
		sideReload.y += 32;
	if (int(targetPos.z) % 32 == 0)
		sideReload.y -= 32;

	if (sideReload == glm::vec2(targetPos.x, targetPos.z))
		return (true);

	Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(sideReload);
	if (chunk && chunk->_generated)
	{
		// std::cout << "poses:" << chunk->pos.x << "; " << chunk->pos.y << "; " << chunk->pos.z << " | " << targetPos.x << "; " << targetPos.y << "; " << targetPos.z << std::endl;
		chunk->reset();
		chunk->generate();
	}
	return (true);
}
