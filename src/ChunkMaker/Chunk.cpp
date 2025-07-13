/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 14:32:09 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunk.hpp"

glm::vec2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2; 
    unsigned a = ix, b = iy;
    a *= 3284157443;
 
    b ^= a << s | a >> (w - s);
    b *= 1911520717;
 
    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
    
    // Create the vector from the angle
    glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);
 
    return v;
}
 
// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    glm::vec2 gradient = randomGradient(ix, iy);
 
    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;
 
    // Compute the dot-product
    return (dx * gradient.x + dy * gradient.y);
}
 
float interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}
 
 
// Sample Perlin noise at coordinates x, y
float perlin(float x, float y) {
    
    // Determine grid cell corner coordinates
    int x0 = (int)x; 
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;
 
    // Compute Interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;
    
    // Compute and interpolate top two corners
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);
 
    // Compute and interpolate bottom two corners
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);
 
    // Final step: interpolate between the two previously interpolated values, now in y
    float value = interpolate(ix0, ix1, sy);
    
    return (value);
}

float	getFakeNoise(glm::vec2 pos) //! ////////////////////////////////////////////////////////////////////////// because no noise
{
	float	freq = 0.1 / 32;
	float	amp = 4;

	float	ret = 0;

	if (pos.x < 0 || pos.y < 0)
		return (256);
	
	for (int i = 0; i < 6; i++)
	{
		ret += perlin(pos.x * freq, pos.y * freq) * amp;

		freq *= 2;
		amp /= 2;
	}

	// ret = (ret + 1) / 2;
	ret = glm::clamp(ret, 0.0f, 1.0f);

	return (ret * 255);
}

Chunk::Chunk(const glm::vec3 &nPos) : _generated(false), _generating(false), _uploaded(false)
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
	_chunkTop.reserve(1024);
	gen();
	genMesh();
	_indicesSize = _indices.size();
	_generated.store(true);
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
	_indices.clear();
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

void	Chunk::clear()
{
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_uploaded = false;
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
					addVertices(GROUND, _vertices, _indices, {0 + iii, 1 + i, 1 + ii}, {0 + iii, 1 + i, 0 + ii}, {0 + iii, 0 + i, 1 + ii}, {0 + iii, 0 + i, 0 + ii}, {-1, 0, 0});
				if ((eastFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {1 + iii, 1 + i, 0 + ii}, {1 + iii, 1 + i, 1 + ii}, {1 + iii, 0 + i, 0 + ii}, {1 + iii, 0 + i, 1 + ii}, {1, 0, 0});
				if ((northFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {1 + (31 - ii), 1 + i, 1 + (31 - iii)}, {0 + (31 - ii), 1 + i, 1 + (31 - iii)}, {1 + (31 - ii), 0 + i, 1 + (31 - iii)}, {0 + (31 - ii), 0 + i, 1 + (31 - iii)}, {0, 0, 1});
				if ((southFaces >> iii) & 1)
					addVertices(GROUND, _vertices, _indices, {0 + (31 - ii), 1 + i, 0 + (31 - iii)}, {1 + (31 - ii), 1 + i, 0 + (31 - iii)}, {0 + (31 - ii), 0 + i, 0 + (31 - iii)}, {1 + (31 - ii), 0 + i, 0 + (31 - iii)}, {0, 0, -1});
				if ((chunkSlice >> iii) & 1 && 
					((groundData.find(((i + 1) * 32 + ii)) != groundData.end() && !((groundData.find(((i + 1) * 32 + ii))->second >> iii) & 1))
						|| groundData.find(((i + 1) * 32 + ii)) == groundData.end()))
					addVertices(GROUND, _vertices, _indices, {0 + iii, 1 + i, 1 + ii}, {1 + iii, 1 + i, 1 + ii}, {0 + iii, 1 + i, 0 + ii}, {1 + iii, 1 + i, 0 + ii}, {0, 1, 0});
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
	if (!_uploaded && !_generated && !_generating)
		CHUNK_GENERATOR->deposit(this);
	if (!_uploaded && _generated && !_generating)
		upload();
	if (!_generated || !_uploaded || _generating)
		return ;

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
