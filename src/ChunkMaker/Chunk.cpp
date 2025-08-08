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
	dist = 0;
}

void	Chunk::generate()
{
	if (_generated)
		return ;
	if (_needGen)
	{
		_chunkTop.reserve(1024);
		genChunk();
	}
	_indicesSize = 0;
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
void	Chunk::placeBlock(glm::ivec3 chunkPos, const std::vector<char32_t> &usedData, char32_t slice, char32_t westFaces, char32_t eastFaces, char32_t northFaces, char32_t southFaces)
{
	uint8_t	block = Blocks[chunkPos.y * 1024 + chunkPos.z * 32 + 31 - chunkPos.x] - 1;

	// to add a face we add the offsets of each of it's vertexes to it's position the chunk and then give it's normal based on the face.
	if (block != UINT8_MAX) //UINT8_MAX because 0 - 1 overflows with unsigned
	{
		if ((westFaces >> chunkPos.x) & 1)
			addVertices(block + (block == 3), V1 + chunkPos, V4 + chunkPos, V5 + chunkPos, V8 + chunkPos, 0);
		if ((eastFaces >> chunkPos.x) & 1)
			addVertices(block + (block == 3), V3 + chunkPos, V2 + chunkPos, V7 + chunkPos, V6 + chunkPos, 1);

		if ((slice >> chunkPos.x) & 1 // top face
			&& ((chunkPos.y + 1) * 32 + chunkPos.z >= (int)usedData.size()
				|| !((usedData[(chunkPos.y + 1) * 32 + chunkPos.z] >> chunkPos.x) & 1)))
			addVertices(block, V1 + chunkPos, V2 + chunkPos, V4 + chunkPos, V3 + chunkPos, 2);

		if (chunkPos.y != 0 && (slice >> chunkPos.x) & 1 // bot face
			&& (chunkPos.y - 1) * 32 + chunkPos.z < (int)usedData.size()
				&& !((usedData[(chunkPos.y - 1) * 32 + chunkPos.z] >> chunkPos.x) & 1))
			addVertices(block, V7 + chunkPos, V6 + chunkPos, V8 + chunkPos, V5 + chunkPos, 5);
	}

	// things change for the north and south slices as the positions are rotated
	block = Blocks[chunkPos.y * 1024 + (31 - chunkPos.x) * 32 + (chunkPos.z)] - 1;
	glm::ivec3	rotChunkPos = glm::ivec3((31 - chunkPos.z), chunkPos.y, (31 - chunkPos.x));

	if (block != UINT8_MAX)
	{
		if ((northFaces >> chunkPos.x) & 1)
			addVertices(block + (block == 3), V2 + rotChunkPos, V1 + rotChunkPos, V6 + rotChunkPos, V5 + rotChunkPos, 3);
		if ((southFaces >> chunkPos.x) & 1)
			addVertices(block + (block == 3), V4 + rotChunkPos, V3 + rotChunkPos, V8 + rotChunkPos, V7 + rotChunkPos, 4);
	}
}

/*
	slice structure, used for generating the mesh of a chunk, stores the slices of a y level in the chunk
*/
struct Slices
{
	char32_t				slice, westFaces, eastFaces = 0;
	std::vector<char32_t>	rotSlices, northSlices, southSlices;

	Slices()
	{
		rotSlices.reserve(32);
		northSlices.reserve(32);
		southSlices.reserve(32);
	}

	~Slices()
	{
		rotSlices.clear();
		northSlices.clear();
		southSlices.clear();
		rotSlices.shrink_to_fit();
		northSlices.shrink_to_fit();
		southSlices.shrink_to_fit();
	}
};

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

/*
	generated the mesh of a chunk based on it's ChunkMask
*/
void	Chunk::genMesh()
{

	Slices										ground, water;

	_vertices.reserve(1572864); // 1572864 is 16*16*256*(6*4) because you can have max 16*16 VISIBLE blocks on a chunk's slice with ech having 6*4 vertices
	_indices.reserve(2359296); // 2359296 is (1572864/4) * 6 because for each 4 vertices 6 indices are added

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
		genEdges[0][z] = (sideChunks[0] ? 0 :getFakeNoise(glm::vec2(pos.x - 1, pos.z + z)));   
		genEdges[1][z] = (sideChunks[1] ? 0 :getFakeNoise(glm::vec2(pos.x + 32, pos.z + z)));   
		genEdges[2][z] = (sideChunks[2] ? 0 :getFakeNoise(glm::vec2(pos.x + (31 - z), pos.z + 32)));   
		genEdges[3][z] = (sideChunks[3] ? 0 :getFakeNoise(glm::vec2(pos.x + (31 - z), pos.z - 1)));   
	}

	// Generating the mesh
	for (int y = 0; y <= std::max((int)_maxHeight, WATERLINE); ++y)
	{
		// getting the proper adjacent slice for the y level
		if (sideChunks[0])
			edges[0] = sideChunks[0]->RotChunkMask[y * 32];
		else
			edges[0] = getGenEdgeSlice(genEdges[0], y);

		if (sideChunks[1])
			edges[1] = sideChunks[1]->RotChunkMask[y * 32 + 31];
		else
			edges[1] = getGenEdgeSlice(genEdges[1], y);

		if (sideChunks[2])
			edges[2] = sideChunks[2]->ChunkMask[y * 32];
		else
			edges[2] = getGenEdgeSlice(genEdges[2], y);

		if (sideChunks[3])
			edges[3] = sideChunks[3]->ChunkMask[y * 32 + 31];
		else
			edges[3] = getGenEdgeSlice(genEdges[3], y);

		// same but for water
		edges[4] = (y <= WATERLINE ? char32_t(4294967295) : edges[0]);
		edges[5] = (y <= WATERLINE ? char32_t(4294967295) : edges[1]);
		edges[6] = (y <= WATERLINE ? char32_t(4294967295) : edges[2]);
		edges[7] = (y <= WATERLINE ? char32_t(4294967295) : edges[3]);

		// setting the chunk's rotated slices
		ground.rotSlices.clear();
		getRotSlice(ground.rotSlices, 0, y * 32, ChunkMask);

		// same for water
		if (y <= WATERLINE)
		{
			water.rotSlices.clear();
			getRotSlice(water.rotSlices, 0, y * 32, WaterMask);
		}

		for (int z = 0; z < 32; ++z)
		{
			// culling the slices for the ground (setting which faces will be generated) using the adjacent chunk slices
			ground.slice = ChunkMask[y * 32 + z];
			ground.westFaces = culling(ground.slice, true, ((edges[0] >> (31 - z)) & 1) != 0);
			ground.eastFaces = culling(ground.slice, false, ((edges[1] >> (31 - z)) & 1) != 0);
			ground.northSlices[z] = culling(ground.rotSlices[z], true, ((edges[2] >> (31 - z)) & 1) != 0);
			ground.southSlices[z] = culling(ground.rotSlices[z], false, ((edges[3] >> (31 - z)) & 1) != 0);

			// same for water
			if (y <= WATERLINE)
			{
				water.slice = WaterMask[y * 32 + z];
				water.westFaces = culling(water.slice | ground.slice, true, ((edges[4] >> z) & 1) != 0);
				water.eastFaces = culling(water.slice | ground.slice, false, ((edges[5] >> z) & 1) != 0);
				water.northSlices[z] = culling(water.rotSlices[z] | ground.rotSlices[z], true, ((edges[6] >> z) & 1) != 0);
				water.southSlices[z] = culling(water.rotSlices[z] | ground.rotSlices[z], false, ((edges[7] >> z) & 1) != 0);
			}

			// creating the blocks for the slice
			for (int x = 0; x < 32; ++x)
			{
				placeBlock(glm::vec3(x, y, z), ChunkMask, ground.slice, ground.westFaces, ground.eastFaces, ground.northSlices[z], ground.southSlices[z]);
				if (y <= WATERLINE)
					placeBlock(glm::vec3(x, y, z), WaterMask, water.slice, water.westFaces, water.eastFaces, water.northSlices[z], water.southSlices[z]);
			}
		}
	}

	// freeing up unused space
	_vertices.shrink_to_fit();
	_indices.shrink_to_fit();
}

/*
	Creates the array of blocks and the ChunkMasks using the noise
*/
void	Chunk::genChunk()
{
	int	height;

	// we make the vectors the correct size for a potential fully filled chunk and set everything to 0
	ChunkMask.resize(8192, 0); // 32 * 256
	RotChunkMask.resize(8192, 0);
	Blocks.resize(262144, 0); // 32 * 32 * 256

	// gen highest of each points in chunk based on the noise
	for (int z = 0; z < 32; ++z)
	{
		for (int x = 0; x < 32; ++x)
		{
			height = getFakeNoise(glm::vec2{pos.x + (31 - x), pos.z + z});
			//Pour l'instant si > 65 = grass - 2 dirt - stone, sinon 2 sand - stone.
			//blocks: 0 air, 1 water, 2 stone, 3 dirt, 4 grass, 5 grassSide, 6 sand
			if (height > 65)
				Blocks[height * 1024 + z * 32 + x] = 4;
			else
				Blocks[height * 1024 + z * 32 + x] = 6;

			// adding the block to the chunkmask
			if (ChunkMask[height * 32 + z] != 0)
				ChunkMask[height * 32 + z] = ChunkMask[height * 32 + z] | (char32_t)(((char32_t)1) << (31 - x));
			else
				ChunkMask[height * 32 + z] = (char32_t)(((char32_t)1) << (31 - x));

			if (height > _maxHeight)
				_maxHeight = height;
			if (height < _minHeight)
				_minHeight = height;
			_chunkTop.push_back(height); // this vector stores the y values of the top blocks it's used for placing the dirt under the grass and the stone under the dirt
		}
	}

	// fill from the top to the bottom; no caves yet, need to be added with noise
	for (int y = _maxHeight; y > 0; --y)
	{
		for (int z = 0; z < 32; ++z)
		{
			for (int x = 0; x < 32; ++x)
			{
				// if the top is more than 2 above we put stone else we put the block above
				uint8_t	block = Blocks[y * 1024 + z * 32 + x];
				if (block == 0) // if it's air we skip it
					continue;
				if (_chunkTop[z * 32 + x] - y >= 2)
					Blocks[(y - 1) * 1024 + z * 32 + x] = 2; // 1024 = 32 * 32 aka a horizontal slice
				else
				{ // if it was grass then put dirt else put the above block
					if (block == 4)
						Blocks[(y - 1) * 1024 + z * 32 + x] = 3;
					else
						Blocks[(y - 1) * 1024 + z * 32 + x] = block;
				}
			}

			// updating the chunk mask to fill it
			if (ChunkMask[y * 32 + z] != 0)
			{
				if (ChunkMask[(y - 1) * 32 + z] != 0)
					ChunkMask[(y - 1) * 32 + z] = ChunkMask[(y - 1) * 32 + z] | ChunkMask[y * 32 + z];
				else
					ChunkMask[(y - 1) * 32 + z] = ChunkMask[y * 32 + z];
			}
			else
			{
				if (y > _minHeight)
					ChunkMask[y * 32 + z] = 0;
				else
					ChunkMask[y * 32 + z] = char32_t(2147483648);
			}
		}
		// we fill the rotated ChunkMask using getRotSlice; this vector is used by neightboring chunks and to create the mesh
		getRotSlice(RotChunkMask, y * 32, y * 32, ChunkMask);
	}
	getRotSlice(RotChunkMask, 0, 0, ChunkMask);

	// Add water
	WaterMask.resize(32 * (WATERLINE + 1), 0);

	for (int z = 0; z < 32; ++z)
	{
		for (int x = 0; x < 32; ++x)
		{
			for (int y = WATERLINE; y > _chunkTop[z * 32 + x]; --y)
			{
				Blocks[y * 1024 + z * 32 + x] = 1;
				WaterMask[y * 32 + z] = WaterMask[y * 32 + z] | (char32_t)(((char32_t)1) << (31 - x));
			}
		}
	}


	ChunkMask.shrink_to_fit();
	RotChunkMask.shrink_to_fit();
	Blocks.shrink_to_fit();
	WaterMask.shrink_to_fit();
}

#include "ChunkGenerator.hpp"
extern ChunkGeneratorManager	*CHUNK_GENERATOR;

void	Chunk::draw(Shader &shader)
{
	if (_generating)
		return ;
	if (!_uploaded && _generated)
		upload();

	shader.setMat4("model", model);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, _indicesSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

bool	Chunk::removeBlock(const glm::ivec3 &targetPos)
{
	int			pos = targetPos.y * 32 + (targetPos.z % 32);
	char32_t	slice = (pos >= ChunkMaskSize ? 0 : ChunkMask[pos]);
	if (!((slice >> (targetPos.x % 32)) & 1))
		return (false);

	// this will make this chunk unable to be deleted by the QuadTree
	_edited.store(true);

	// editing the mask slice to remove the desired block
	char32_t rawSlice = slice;
	slice = ((rawSlice << (31 - (targetPos.x % 32))) >> (31 - (targetPos.x % 32))) ^ ((rawSlice >> (targetPos.x % 32)) << (targetPos.x % 32));

	// updating the different vectors of the chunk
	ChunkMask[pos] = slice;
	getRotSlice(RotChunkMask, targetPos.y * 32, targetPos.y * 32, ChunkMask);
	Blocks[targetPos.y * 1024 + (targetPos.z % 32) * 32 + (31 - (targetPos.x % 32))] = 0;
	--_chunkTop[(targetPos.z % 32) * 32 + (31 - (targetPos.x % 32))];
	reset();
	generate();

	// we detect if we are on a border
	glm::vec2	sideReload = {targetPos.x, targetPos.z};
	if (targetPos.x % 32 == 31)
		sideReload.x += 32;
	if (targetPos.x % 32 == 0)
		sideReload.x -= 32;
	if (targetPos.z % 32 == 31)
		sideReload.y += 32;
	if (targetPos.z % 32 == 0)
		sideReload.y -= 32;

	// if we aren't we do nothing
	if (sideReload == glm::vec2(targetPos.x, targetPos.z))
		return (true);

	// but if we are wi force the update of the adjacent chunk so it matches it's faces for the newly broken blocks
	if (targetPos.x != sideReload.x)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(sideReload.x, targetPos.z));
		if (chunk && chunk->_generated)
		{
			chunk->reset();
			chunk->generate();
		}
	}
	if (targetPos.z != sideReload.y)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(targetPos.x, sideReload.y));
		if (chunk && chunk->_generated)
		{
			chunk->reset();
			chunk->generate();
		}
	}

	// the return is important for the raycast so it knows to stop when a block is deleted.
	return (true);
}
