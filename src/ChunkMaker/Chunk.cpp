/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/16 14:51:14 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunk.hpp"
#include "RegionManager.hpp"
extern RegionManager	*CHUNKS;

extern uint	seed;

glm::vec2 randomGradient(int ix, int iy)
{
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443 + (seed + 1);

	b ^= a << s | a >> (w - s);
    b *= 1911520717;

	a ^= b << s | b >> (w - s);
    a *= 2048419325;
	float random = (a / (float)UINT_MAX) * 2.0f * M_PI;

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

float	calcNoise(const glm::vec2 &pos, float freq, float amp, int noisiness)
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

Chunk::Chunk(const glm::vec3 &nPos) : rendered(false), loaded(false),  _edited(false)
{
	_minHeight = 255;
	_maxHeight = 0;
	_indicesSize = 0;
	pos = nPos;
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	dist = 0;
}

void	Chunk::generate()
{
	if (Chunk::getState() >= ChunkState::CS_GENERATED)
		return ;
	_chunkTop.reserve(1024);
	genChunk();
}

void	Chunk::mesh()
{
	if (Chunk::getState() >= ChunkState::CS_MESHED && !getRemesh())
		return ;
	reGenMesh();
	_indicesSize = _indices.size();
}

void	Chunk::reGenMesh()
{
	_indicesSize = 0;
	genMesh();
	_indicesSize = _indices.size();
}

void	Chunk::clear()
{
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_EBO = 0;
	_VBO = 0;
	_VAO = 0;
	setState(ChunkState::CS_GENERATED);
}

void	Chunk::upload()
{
	makeBuffers();
	this->setState(ChunkState::CS_UPLOADED);
}

Chunk::~Chunk()
{
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Destroying the Chunk at " + sPos.str(), LogSeverity::NORMAL);
	}
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	ChunkMask.clear();
	ChunkMask.shrink_to_fit();
	RotChunkMask.clear();
	RotChunkMask.shrink_to_fit();
	WaterMask.clear();
	WaterMask.shrink_to_fit();
	Blocks.clear();
	Blocks.shrink_to_fit();
	_vertices.clear();
	_vertices.shrink_to_fit();
	_indices.clear();
	_indices.shrink_to_fit();
}

void	Chunk::initDist()
{
	dist = (glm::length(CAMERA->pos - (pos + glm::vec3(16.f, CAMERA->pos.y, 16.f))));
}

float	Chunk::getDist() const
{
	return (dist);
}

float	Chunk::getDistance() const
{
	return (glm::length(CAMERA->pos - (pos + glm::vec3(16.f, CAMERA->pos.y, 16.f))));
}

bool	Chunk::isInRange()
{
	return (glm::length(CAMERA->pos - (pos + glm::vec3(16.0f, 0.0f, 16.0f))));
}

void	Chunk::makeBuffers()
{
	if (_VAO <= 0)
		glGenVertexArrays(1, &_VAO);
	if (_VBO <= 0)
    	glGenBuffers(1, &_VBO);
	if (_EBO <= 0)
		glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(GLuint), _vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), (GLuint*)_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(GLuint), (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// we can clear them on the memory since they are now into the graphics card
	_vertices.clear();
	_vertices.shrink_to_fit();
	_indices.clear();
	_indices.shrink_to_fit();
}

/*
	fills a 32 x 32 portion of a vector with the rotated values of another
*/
void	Chunk::getRotSlice(std::vector<char32_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<char32_t>	&usedMask)
{
	char32_t slice;
	for (int i = 0; i < 32; ++i)
	{
		slice = usedMask[height + i];
		for (int ii = 0; ii < 32; ++ii)
			rotSlice[rotOffset + ii] = rotSlice[rotOffset + ii] << 1 | (((slice >> (31 - ii)) & 1));
	}
}

/*
	Fast culling of a whole line in the chunk, using the edge of the adjacent chunk
*/
char32_t	culling(const char32_t &slice, const bool &dir, const int &edge)
{
	if (dir)
		return (slice & ~((slice << 1) | edge)); // right faces
	else
		return (slice & ~((slice >> 1) | (edge << 31))); // left faces
}

/*
	generates a bit slice based on a height and a list of height (if under it gives a 1 for each of the list's elements)
*/
char32_t	getGenEdgeSlice(int edge[32], const int &height)
{
	char32_t	slice = 0;

	for (int i = 0; i < 32; ++i)
		slice |= ((height <= edge[i]) << (31 - i));

	return (slice);
}

// the offsets for the bitshift
const int OFFSET1 = 6;  // x
const int OFFSET2 = 15; // xy
const int OFFSET3 = 21; // xyz
const int OFFSET4 = 23; // xyz + texture coord
const int OFFSET5 = 26; // xyz + texture coord + blocktype

/*
	adds the vertices for a bocks's face to the "_vertices" vector and it's corresponding indices to the "_indices" vector
*/
void	Chunk::addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal)
{
	_vertices.push_back(BL.x | (BL.y << OFFSET1) | (BL.z << OFFSET2) | (0 << OFFSET3) | (0 << (OFFSET3 + 1)) | (type << OFFSET4) | (Normal << OFFSET5));
	_vertices.push_back(TL.x | (TL.y << OFFSET1) | (TL.z << OFFSET2) | (0 << OFFSET3) | (1 << (OFFSET3 + 1)) | (type << OFFSET4) | (Normal << OFFSET5));
	_vertices.push_back(TR.x | (TR.y << OFFSET1) | (TR.z << OFFSET2) | (1 << OFFSET3) | (1 << (OFFSET3 + 1)) | (type << OFFSET4) | (Normal << OFFSET5));
	_vertices.push_back(BR.x | (BR.y << OFFSET1) | (BR.z << OFFSET2) | (1 << OFFSET3) | (0 << (OFFSET3 + 1)) | (type << OFFSET4) | (Normal << OFFSET5));
	_indicesSize += 4;
	_indices.insert(_indices.end(), {_indicesSize - 4, _indicesSize - 2, _indicesSize - 3, _indicesSize - 4, _indicesSize - 1, _indicesSize - 2});
}

//      1_______2
//     /:      /│
//    4_:_____3 │                N
//    │ 5_ _ _│_6        top:  W U E
//    │.      │/                 S
//    8_______7

// offsets for the cube's edges: NO MORE MAGIC VECTORS
const glm::ivec3 V1 = glm::ivec3(0, 1, 1);
const glm::ivec3 V2 = glm::ivec3(1, 1, 1);
const glm::ivec3 V3 = glm::ivec3(1, 1, 0);
const glm::ivec3 V4 = glm::ivec3(0, 1, 0);
const glm::ivec3 V5 = glm::ivec3(0, 0, 1);
const glm::ivec3 V6 = glm::ivec3(1, 0, 1);
const glm::ivec3 V7 = glm::ivec3(1, 0, 0);
const glm::ivec3 V8 = glm::ivec3(0, 0, 0);

/*
This is the list of the Normals used in the shader:
	vec3 (-1, 0, 0),
	vec3 (1, 0, 0),
	vec3 (0, 1, 0),
	vec3 (0, 0, 1),
	vec3 (0, 0, -1),
	vec3 (0, -1, 0)
*/

/*
	Puts the blocks in the mesh using the culled slices, the given CHunkMask and the array of Blocks as reference.
*/
void	Chunk::placeBlock(glm::ivec3 &chunkPos, const std::vector<char32_t> &usedData, char32_t &slice, char32_t &westFaces, char32_t &eastFaces, char32_t &northFaces, char32_t &southFaces)
{
	// to add a face we add the offsets of each of it's vertexes to it's position the chunk and then give it's normal based on the face.
	if (westFaces & 1 || eastFaces & 1 || slice & 1 || slice & 1) // skipping full air
	{
		uint8_t	block = Blocks[(chunkPos.y << 10) + (chunkPos.z << 5) + 31 - chunkPos.x].type - 1;
		if (westFaces & 1)
			addVertices(block + (block == 3), V1 + chunkPos, V4 + chunkPos, V5 + chunkPos, V8 + chunkPos, 0);
		if (eastFaces & 1)
			addVertices(block + (block == 3), V3 + chunkPos, V2 + chunkPos, V7 + chunkPos, V6 + chunkPos, 1);

		if (slice & 1 // top face
			&& ((chunkPos.y + 1) * 32 + chunkPos.z >= (int)usedData.size()
				|| !((usedData[(chunkPos.y + 1) * 32 + chunkPos.z] >> chunkPos.x) & 1)))
			addVertices(block, V1 + chunkPos, V2 + chunkPos, V4 + chunkPos, V3 + chunkPos, 2);

		if (chunkPos.y != 0 && slice & 1 && block != 0 // bot face
			&& (chunkPos.y - 1) * 32 + chunkPos.z < (int)usedData.size()
				&& !((usedData[(chunkPos.y - 1) * 32 + chunkPos.z] >> chunkPos.x) & 1))
			addVertices(block, V7 + chunkPos, V6 + chunkPos, V8 + chunkPos, V5 + chunkPos, 5);
	}

	// things change for the north and south slices as the positions are rotated
	if (northFaces & 1 || southFaces & 1) // same as the other one
	{
		uint8_t	block = Blocks[(chunkPos.y << 10) + ((31 - chunkPos.x) << 5) + (chunkPos.z)].type - 1;
		glm::ivec3	rotChunkPos = glm::ivec3((31 - chunkPos.z), chunkPos.y, (31 - chunkPos.x));
		if (northFaces & 1)
			addVertices(block + (block == 3), V2 + rotChunkPos, V1 + rotChunkPos, V6 + rotChunkPos, V5 + rotChunkPos, 3);
		if (southFaces & 1)
			addVertices(block + (block == 3), V4 + rotChunkPos, V3 + rotChunkPos, V8 + rotChunkPos, V7 + rotChunkPos, 4);
	}

	westFaces >>= 1;
	eastFaces >>= 1;
	slice >>= 1;
	northFaces >>= 1;
	southFaces >>= 1;
}

/*
	generated the mesh of a chunk based on it's ChunkMask
*/
void	Chunk::genMesh()
{

	Slices	ground, water;

	_vertices.reserve(1572864); // 1572864 is 16*16*256*(6*4) because you can have max 16*16 VISIBLE blocks on a chunk's slice with ech having 6*4 vertices
	_indices.reserve(2359296); // 2359296 is (1572864/4) * 6 because for each 4 vertices 6 indices are added

	glm::ivec3	chunkPos(0, 0, 0);
	char32_t	edges[8]; // these are the slices of adjacent chunks (4-7 is for water only) 0 = west, 1 = east, 2 = north, 3 = south
	int			genEdges[4][32]; // same as above but takes the noise for ungenerated chunks 0 = west, 1 = east, 2 = north, 3 = south
	Chunk		*sideChunks[4] = {NULL, NULL, NULL, NULL}; // the adjacent chunks 0 = west, 1 = east, 2 = north, 3 = south

	// getting adjacent chunks
	sideChunks[0] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x - 1, pos.z));
	sideChunks[1] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x + 32, pos.z));
	sideChunks[2] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x, pos.z + 32));
	sideChunks[3] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x, pos.z - 1));

	// checking if they are edited
	sideChunks[0] = (sideChunks[0] && sideChunks[0]->_edited ? sideChunks[0] : NULL);
	sideChunks[1] = (sideChunks[1] && sideChunks[1]->_edited ? sideChunks[1] : NULL);
	sideChunks[2] = (sideChunks[2] && sideChunks[2]->_edited ? sideChunks[2] : NULL);
	sideChunks[3] = (sideChunks[3] && sideChunks[3]->_edited ? sideChunks[3] : NULL);

	// making the noise based adjacent slices
	for (int z = 0; z < 32; ++z)
	{
		genEdges[0][z] = (sideChunks[0] ? 0 : getGenerationHeight(glm::vec2(pos.x - 1, pos.z + z)));
		genEdges[1][z] = (sideChunks[1] ? 0 : getGenerationHeight(glm::vec2(pos.x + 32, pos.z + z)));
		genEdges[2][z] = (sideChunks[2] ? 0 : getGenerationHeight(glm::vec2(pos.x + (31 - z), pos.z + 32)));
		genEdges[3][z] = (sideChunks[3] ? 0 : getGenerationHeight(glm::vec2(pos.x + (31 - z), pos.z - 1)));
	}

	// Generating the mesh
	for (chunkPos.y = 0; chunkPos.y <= _maxHeight; ++chunkPos.y)
	{
		// getting the proper adjacent slice for the y level
		if (sideChunks[0])
			edges[0] = sideChunks[0]->RotChunkMask[chunkPos.y * 32];
		else
			edges[0] = getGenEdgeSlice(genEdges[0], chunkPos.y);

		if (sideChunks[1])
			edges[1] = sideChunks[1]->RotChunkMask[chunkPos.y * 32 + 31];
		else
			edges[1] = getGenEdgeSlice(genEdges[1], chunkPos.y);

		if (sideChunks[2])
			edges[2] = sideChunks[2]->ChunkMask[chunkPos.y * 32];
		else
			edges[2] = getGenEdgeSlice(genEdges[2], chunkPos.y);

		if (sideChunks[3])
			edges[3] = sideChunks[3]->ChunkMask[chunkPos.y * 32 + 31];
		else
			edges[3] = getGenEdgeSlice(genEdges[3], chunkPos.y);

		// // same but for water
		// edges[4] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[0]);
		// edges[5] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[1]);
		// edges[6] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[2]);
		// edges[7] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[3]);

		// // setting the rotslices for water since it doesn't have a rotated mask
		// if (chunkPos.y <= WATERLINE)
		// {
		// 	water.rotSlices.clear();
		// 	getRotSlice(water.rotSlices, 0, chunkPos.y * 32, WaterMask);
		// }

		for (chunkPos.z = 0; chunkPos.z < 32; ++chunkPos.z)
		{
			// culling the slices for the ground (setting which faces will be generated) using the adjacent chunk slices
			ground.slice = ChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.rotSlice = RotChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.westFaces = culling(ground.slice, true, ((edges[0] >> (31 - chunkPos.z)) & 1) != 0);
			ground.eastFaces = culling(ground.slice, false, ((edges[1] >> (31 - chunkPos.z)) & 1) != 0);
			ground.northSlices = culling(ground.rotSlice, true, ((edges[2] >> (31 - chunkPos.z)) & 1) != 0);
			ground.southSlices = culling(ground.rotSlice, false, ((edges[3] >> (31 - chunkPos.z)) & 1) != 0);

			// // same for water
			// if (chunkPos.y <= WATERLINE)
			// {
			// 	water.slice = WaterMask[chunkPos.y * 32 + chunkPos.z];
			// 	water.westFaces = culling(water.slice | ground.slice, true, ((edges[4] >> chunkPos.z) & 1) != 0);
			// 	water.eastFaces = culling(water.slice | ground.slice, false, ((edges[5] >> chunkPos.z) & 1) != 0);
			// 	water.northSlices = culling(water.rotSlices[chunkPos.z] | ground.rotSlice, true, ((edges[6] >> chunkPos.z) & 1) != 0);
			// 	water.southSlices = culling(water.rotSlices[chunkPos.z] | ground.rotSlice, false, ((edges[7] >> chunkPos.z) & 1) != 0);
			// }

			// creating the blocks for the slice
			for (chunkPos.x = 0; chunkPos.x < 32; ++chunkPos.x)
			{
				// if (chunkPos.y <= WATERLINE)
				// 	placeBlock(chunkPos, WaterMask, water.slice, water.westFaces, water.eastFaces, water.northSlices, water.southSlices);
				placeBlock(chunkPos, ChunkMask, ground.slice, ground.westFaces, ground.eastFaces, ground.northSlices, ground.southSlices);
			}
		}
	}
	for (chunkPos.y = _minHeight; chunkPos.y <= WATERLINE; ++chunkPos.y)
	{
		// getting the proper adjacent slice for the y level
		if (sideChunks[0])
			edges[0] = sideChunks[0]->RotChunkMask[chunkPos.y * 32];
		else
			edges[0] = getGenEdgeSlice(genEdges[0], chunkPos.y);

		if (sideChunks[1])
			edges[1] = sideChunks[1]->RotChunkMask[chunkPos.y * 32 + 31];
		else
			edges[1] = getGenEdgeSlice(genEdges[1], chunkPos.y);

		if (sideChunks[2])
			edges[2] = sideChunks[2]->ChunkMask[chunkPos.y * 32];
		else
			edges[2] = getGenEdgeSlice(genEdges[2], chunkPos.y);

		if (sideChunks[3])
			edges[3] = sideChunks[3]->ChunkMask[chunkPos.y * 32 + 31];
		else
			edges[3] = getGenEdgeSlice(genEdges[3], chunkPos.y);

		// same but for water
		edges[4] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[0]);
		edges[5] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[1]);
		edges[6] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[2]);
		edges[7] = (chunkPos.y <= WATERLINE ? char32_t(4294967295) : edges[3]);

		water.rotSlices.clear();
		getRotSlice(water.rotSlices, 0, chunkPos.y * 32, WaterMask);

		for (chunkPos.z = 0; chunkPos.z < 32; ++chunkPos.z)
		{
			ground.slice = ChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.rotSlice = RotChunkMask[chunkPos.y * 32 + chunkPos.z];

			// if (glm::ivec3(pos) == (glm::ivec3(CAMERA->pos) / glm::ivec3(32, 1, 32)) * glm::ivec3(32, 0, 32))
			// 	std::cout << "hey" <<std::endl;

			water.slice = WaterMask[chunkPos.y * 32 + chunkPos.z];
			water.westFaces = culling(water.slice | ground.slice, true, ((edges[4] >> chunkPos.z) & 1) != 0);
			water.eastFaces = culling(water.slice | ground.slice, false, ((edges[5] >> chunkPos.z) & 1) != 0);
			water.northSlices = culling(water.rotSlices[chunkPos.z] | ground.rotSlice, true, ((edges[6] >> chunkPos.z) & 1) != 0);
			water.southSlices = culling(water.rotSlices[chunkPos.z] | ground.rotSlice, false, ((edges[7] >> chunkPos.z) & 1) != 0);

			for (chunkPos.x = 0; chunkPos.x < 32; ++chunkPos.x)
			{
				placeBlock(chunkPos, WaterMask, water.slice, water.westFaces, water.eastFaces, water.northSlices, water.southSlices);
			}
		}
	}


	// freeing up unused space
	_vertices.shrink_to_fit();
	_indices.shrink_to_fit();
}

float	perlin(float x, float y, float z)
{
	float ab = perlin(x, y);
	float bc = perlin(y, z);
	float ac = perlin(x, z);

	float ba = perlin(y, x);
	float cb = perlin(z, y);
	float ca = perlin(z, x);

	return (ab + bc + ac + ba + cb + ca) / 6.0;
}

//Tweaked cave function to generate thinner caves

// float	getCaveValue(glm::vec3 pos, float minHeight, float maxHeight)
// {
// 	float	freq = 0.02;

// 	float minY = minHeight;
// 	float maxY = maxHeight;
// 	float mid  = (minY + maxY) * 0.5f;
// 	float range = (maxY - minY) * 0.5f;

// 	float dist = (pos.y - mid) / range;
// 	float heightFactor = 1.0f - (pos.y / 512.0f);
// 	//Pour pas depasser des limites
// 	float amp = exp(-dist * dist) * heightFactor;

// 	float	noise = 0;
// 	for (int i = 0; i < 2; i++)
// 	{
// 		noise += std::abs(perlin(pos.x * freq, pos.y * freq, pos.z * freq));

// 		freq *= 2;
// 		amp /= 2;
// 	}
// 	return (noise);
// }

#define CAVE_TRESHOLD 0.12

float	getCaveValue(const glm::vec3 &pos, float minHeight, float maxHeight)
{
	float	freq = 0.03;

	float minY = minHeight;
	float maxY = maxHeight;
	float mid  = (minY + maxY) * 0.5f;
	float range = (maxY - minY) * 0.5f;

	float dist = (pos.y - mid) / range;
	float heightFactor = 1.0f - (pos.y / 512.0f);
	//Pour pas depasser des limites
	float amp = exp(-dist * dist) * heightFactor;

	float	noise = 0;
	for (int i = 0; i < 2; i++)
	{
		noise += perlin(pos.x * freq, pos.y * freq, pos.z * freq) * amp;
		if (noise > CAVE_TRESHOLD)
			return (noise);

		freq *= 2;
		amp /= 2;
	}
	return (noise);
}

// See https://www.youtube.com/watch?v=CSa5O6knuwI&t=1029s

struct	SplinePoint
{
	float	x;
	float	y;
};

struct	Spline
{
	std::vector<SplinePoint>	points;
};

float	getValueInSpline(const Spline &spline, float value)
{
	if (value <= spline.points.front().x)
		return (spline.points.front().y);
	if (value >= spline.points.back().x)
		return (spline.points.back().y);

	for (size_t i = 0; i < spline.points.size() - 1; i++)
	{
		const SplinePoint& p1 = spline.points[i];
		const SplinePoint& p2 = spline.points[i + 1];

		if (value >= p1.x && value <= p2.x)
		{
			float t = (value - p1.x) / (p2.x - p1.x);
			return (glm::mix(p1.y, p2.y, t));
		}
	}
	return (0.0f);
}

/*
	Continentalness means the height of the world
*/
Spline continentalnessToHeight =
{
	{
		{ -1.0f, 20},  // deep ocean
		{ -0.2f, 63},  // shallow ocean
		{ -0.15f,  64},  // plains
		{ 0.1f,  72},  // plains
		{ 0.5f,  120},  // hills
		{ 1.0f,  255.0f}  // mountains
	}
};

/*
	Erosion is how much you squish down the terrain
*/
Spline erosionToHeight =
{
	{
		{-1.0f, 30},
		{0, 0},
		{ 0.3,  -5.0f},
		{ 1.0f,  -20.0f}
	}
};

Spline peaksValleysToHeight =
{
	{
		{-1, -40},
		{0, -20},
		{0.08, 0},
		{0.09, 0},
		{0.5, 25},
		{1, 100}
	}
};

float	Chunk::getErosion(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.001, 1, 3));
}

float	Chunk::getContinentalness(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.005, 1, 6));
}

float	Chunk::getPeaksValleys(const glm::vec2 &pos)
{
	return (std::abs(calcNoise(pos, 0.003, 1, 2)));
}

int	Chunk::getGenerationHeight(const glm::vec2 &pos)
{
	_currentContinentalness = getContinentalness(pos);
	_currentErosion = getErosion(pos);
	_currentPeaksValleys = getPeaksValleys(pos);

	//Shapes general terrain height
	float	res = getValueInSpline(continentalnessToHeight, _currentContinentalness);

	//Shapes rivers / peaks
	res += getValueInSpline(peaksValleysToHeight, _currentPeaksValleys);

	//Flattens some areas
	res -= getValueInSpline(erosionToHeight, _currentErosion);
	if (res < 0)
		res = 1;

	return (res);
}

GenInfo	Chunk::getGeneration(const glm::vec3 &pos)
{
	if (_currentMaxHeight == 0)
		getGenerationHeight(glm::vec2(pos.x, pos.z));

	GenInfo	res;

	//Gets world shape (caves and height is calculated outside)
	float noise = getCaveValue(pos, 5, _maxHeight);
	if (noise > CAVE_TRESHOLD)
	{
		res.type = 0;
		return (res);
	}

	//Gets world "paint" (dirt, sand and all)
	if (pos.y == _currentMaxHeight)
	{
		if (pos.y <= WATERLINE)
			res.type = 6;
		else
			res.type = 4;
	}
	else if (pos.y == _currentMaxHeight - 1)
	{
		if (pos.y <= WATERLINE)
			res.type = 6;
		else
			res.type = 3;
	}
	else
		res.type = 2;

	//Will be decoration?? (trees, structures)

	return (res);
}

/*
	Creates the array of blocks and the ChunkMasks using the noise
*/
void	Chunk::genChunk()
{
	GenInfo	newBlock = GenInfo();
	GenInfo	blockUp;
	uint8_t	height = 0;


	// we make the vectors the correct size for a potential fully filled chunk and set everything to 0
	ChunkMask.resize(8192, 0); // 32 * 256
	RotChunkMask.resize(8192, 0);
	Blocks.resize(262144, newBlock); // 32 * 32 * 256

	for (int z = 0; z < 32; ++z)
	{
		for (int x = 0; x < 32; ++x)
		{
			// height = initGeneration(glm::vec2{pos.x + (31 - x), pos.z + z}); // l'init de la gen
			height = getGenerationHeight(glm::vec2{(31 - x) + pos.x, pos.z + z});

			// adding the newBlock to the chunkmask / no touch pls
			if (height > _maxHeight)
				_maxHeight = height;
			if (height < _minHeight)
				_minHeight = height;
			_chunkTop.push_back(height); // this vector stores the y values of the top blocks

			_currentMaxHeight = height;
			for (int y = height; y >= 0; --y)
			{
				newBlock = getGeneration(glm::vec3((31 - x) + pos.x, y, pos.z + z));

				newBlock.height = y;
				Blocks[y * 1024 + z * 32 + x] = newBlock;

				if (newBlock.type == 0) // if it's air we skip it
					continue;
				ChunkMask[y * 32 + z] |= (char32_t)(((char32_t)1) << (31 - x)); // updating the chunkMask with the newly added block
			}
		}
	}
	for (int y = _maxHeight; y >= 0; --y)
		getRotSlice(RotChunkMask, y * 32, y * 32, ChunkMask);


	// Add water
	WaterMask.resize(32 * (WATERLINE + 1), 0);

	for (int z = 0; z < 32; ++z)
	{
		for (int x = 0; x < 32; ++x)
		{
			for (int y = WATERLINE; y > _chunkTop[z * 32 + x]; --y)
			{
				Blocks[y * 1024 + z * 32 + x].height = y;
				Blocks[y * 1024 + z * 32 + x].biome = 0;
				Blocks[y * 1024 + z * 32 + x].type = 1;
				WaterMask[y * 32 + z] |= (char32_t)(((char32_t)1) << (31 - x));
			}
		}
	}

	ChunkMask.shrink_to_fit();
	RotChunkMask.shrink_to_fit();
	Blocks.shrink_to_fit();
	WaterMask.shrink_to_fit();
}

#include "ChunkGeneratorManager.hpp"
extern ChunkGeneratorManager	*CHUNK_GENERATOR;

void	Chunk::draw(Shader &shader)
{
	if (getGenerating() || getState() <= CS_GENERATED)
		return ;
	if (getState() == CS_MESHED)
		upload();

	shader.setMat4("model", model);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, _indicesSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

bool	Chunk::removeBlock(const glm::ivec3 &targetPos)
{
	glm::ivec3 targetPosMod = targetPos % 32;
	int			pos = targetPos.y * 32 + targetPosMod.z;
	char32_t	slice = (pos >= ChunkMaskSize ? 0 : ChunkMask[pos]);
	if (!((slice >> targetPosMod.x) & 1))
		return (false);

	// this will make this chunk unable to be deleted by the QuadTree
	_edited.store(true);

	// editing the mask slice to remove the desired block
	char32_t rawSlice = slice;
	slice = ((rawSlice << (31 - targetPosMod.x)) >> (31 - targetPosMod.x)) ^ ((rawSlice >> targetPosMod.x) << targetPosMod.x);

	// updating the different vectors of the chunk
	ChunkMask[pos] = slice;
	getRotSlice(RotChunkMask, targetPos.y * 32, targetPos.y * 32, ChunkMask);

	// water filling - prototype, doesn't flood or connect through chunks: need more work
	// if (targetPos.y <= WATERLINE
	// 	&& ((targetPos.y < WATERLINE && (WaterMask[pos + 32] >> targetPosMod.x) & 1)
	// 		|| (WaterMask[pos + 1] >> targetPosMod.x) & 1
	// 		|| (WaterMask[pos - 1] >> targetPosMod.x) & 1
	// 		|| (WaterMask[pos] >> (targetPosMod.x + 1)) & 1
	// 		|| (WaterMask[pos] >> (targetPosMod.x - 1)) & 1))
	// {
	// 	Blocks[targetPos.y * 1024 + targetPosMod.z * 32 + (31 - targetPosMod.x)].type = 1;
	// 	WaterMask[pos] |= 1 << targetPosMod.x;
	// }
	// else
		Blocks[targetPos.y * 1024 + targetPosMod.z * 32 + (31 - targetPosMod.x)].type = 0;

	--_chunkTop[targetPosMod.z * 32 + (31 - targetPosMod.x)];

	reGenMesh();
	clear();

	// we detect if we are on a border
	glm::vec2	sideReload = {targetPos.x, targetPos.z};
	if (targetPosMod.x == 31)
		sideReload.x += 32;
	if (targetPosMod.x == 0)
		sideReload.x -= 32;
	if (targetPosMod.z == 31)
		sideReload.y += 32;
	if (targetPosMod.z == 0)
		sideReload.y -= 32;

	// if we aren't we do nothing
	if (sideReload == glm::vec2(targetPos.x, targetPos.z))
		return (true);

	// but if we are wi force the update of the adjacent chunk so it matches it's faces for the newly broken blocks
	if (targetPos.x != sideReload.x)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(sideReload.x, targetPos.z));
		if (chunk && chunk->getState() >= ChunkState::CS_GENERATED)
		{
			chunk->reGenMesh();
			chunk->clear();
		}
	}
	if (targetPos.z != sideReload.y)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(targetPos.x, sideReload.y));
		if (chunk && chunk->getState() >= ChunkState::CS_GENERATED)
		{
			chunk->reGenMesh();
			chunk->clear();
		}
	}

	// the return is important for the raycast so it knows to stop when a block is deleted.
	return (true);
}
