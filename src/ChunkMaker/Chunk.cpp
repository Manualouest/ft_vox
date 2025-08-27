/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:55:10 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/27 09:30:59 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

Chunk::Chunk(const glm::vec3 &nPos) : rendered(false), loaded(false),  _edited(false), _used(false), _isBorder(false)
{
	_minHeight = 255;
	_maxHeight = 0;
	_indicesSize = 0;
	pos = nPos;
	model = glm::mat4(1.0f);
	model = glm::translate(model, pos);
	dist = 0;
}

#include "WorldManager.hpp"
extern WorldManager	*WORLD_MANAGER;

void	Chunk::saveFile()
{
	std::string	worldFile = "saves/" + WORLD_MANAGER->getCurrent()->getID() + "/world/regions/";
	std::string	path = "chunk_" + std::to_string((int)pos.x) + "_" + std::to_string((int)pos.z) + ".chunk";
	std::ofstream	file;

	file.open(worldFile + path);
	if (!file.is_open())
	{
		consoleLog("Could not save world file for " + worldFile + path, LogSeverity::WARNING);
		return ;
	}
	
	for (int y = 0; y <= std::max(_maxHeight.load(), (uint8_t)WATERLINE); y++)
	{
		for (int z = 0; z < 32; ++z)
		{
			for (int x = 0; x < 32; ++x)
			{
				file << std::to_string(Blocks[y * 1024 + z * 32 + x].type) + " ";
			}
		}
		file << std::endl;
	}

	consoleLog("Saved chunk at " + worldFile + path);
}

bool	Chunk::loadFromFile()
{
	std::string	worldFile = "saves/" + WORLD_MANAGER->getCurrent()->getID() + "/world/regions/";
	std::string	path = "chunk_" + std::to_string((int)pos.x) + "_" + std::to_string((int)pos.z) + ".chunk";

	std::ifstream	file;
	std::string line;

	file.open(worldFile + path);
	if (!file.is_open())
		return (false);

	int x = 0;
	int	y = 0;
	int z = 0;
	
	ChunkMask.resize(8192, 0);
	RotChunkMask.resize(8192, 0);
	ChunkTrsMask.resize(8192, 0);
	RotChunkTrsMask.resize(8192, 0);
	Blocks.resize(262144, GenInfo());

	uint	counter = 0;

	while (std::getline(file, line))
	{
		int	block;
		std::istringstream iss(line);
		while (iss >> block && counter++ < 1024)
		{
			if (x > 31)
			{
				x = 0;
				z++;
			}
			if (z > 31)
				break ;

			_chunkTop.push_back(255);

			setBlock(block, x, y, z);
			x++;
		}
		x = 0;
		z = 0;
		y++;
		if (counter != 1024 || y >= 256)
			throw std::runtime_error("Malformed chunk data");
		counter = 0;
	}
	_maxHeight = std::max(y, WATERLINE);

	for (int y = std::max((int)_maxHeight.load(), WATERLINE); y >= 0; --y)
	{
		fatGetRotSlice(RotChunkMask, y * 32, y * 32, ChunkMask);
		fatGetRotSlice(RotChunkTrsMask, y * 32, y * 32, ChunkTrsMask);
	}

	ChunkMask.shrink_to_fit();
	RotChunkMask.shrink_to_fit();
	ChunkTrsMask.shrink_to_fit();
	RotChunkTrsMask.shrink_to_fit();
	Blocks.shrink_to_fit();

	return (true);
}

void	Chunk::generate()
{
	if (Chunk::getState() >= ChunkState::CS_GENERATED)
		return ;

	_chunkTop.reserve(1024);

	if (loadFromFile())
		return ;

	genChunk();
}

void	Chunk::mesh()
{
	if (Chunk::getState() >= ChunkState::CS_MESHED && !getRemesh())
		return ;
	reGenMesh(false);
	_indicesSize = _indices.size();
}

void	Chunk::reGenMesh(const bool &isNotThread)
{
	if (getState() != CS_EMPTY && isNotThread)
		clear();
	_indicesSize = 0;
	genMesh();
	_indicesSize = _indices.size();
}

void	Chunk::clear()
{
	std::stringstream t;
	t << _EBO;
	
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	_EBO = 0;
	
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	_VBO = 0;

	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_VAO = 0;
}

void	Chunk::upload()
{
	makeBuffers();
	this->setState(ChunkState::CS_UPLOADED);
}

Chunk::~Chunk()
{
	if (_edited)
		saveFile();
		
    if (DEBUG)
	{
		std::stringstream sPos;
		sPos << pos.x << ";" << pos.y << ";" << pos.z;
        consoleLog("Destroying the Chunk at " + sPos.str(), LogSeverity::NORMAL);
	}
	if (_EBO)
		glDeleteBuffers(1, &_EBO);
	_EBO = 0;
	if (_VBO)
		glDeleteBuffers(1, &_VBO);
	_VBO = 0;
	if (_VAO)
		glDeleteVertexArrays(1, &_VAO);
	_VAO = 0;
	ChunkMask.clear();
	ChunkMask.shrink_to_fit();
	RotChunkMask.clear();
	RotChunkMask.shrink_to_fit();
	ChunkTrsMask.clear();
	ChunkTrsMask.shrink_to_fit();
	RotChunkTrsMask.clear();
	RotChunkTrsMask.shrink_to_fit();
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
void	Chunk::getRotSlice(std::vector<uint64_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<uint64_t>	&usedMask)
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
	fills a 32 x 32 portion of a vector with the rotated values of another
*/
void	Chunk::fatGetRotSlice(std::vector<uint64_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<uint64_t>	&usedMask)
{
	uint64_t slice;
	for (int i = 0; i < 32; ++i)
	{
		slice = usedMask[height + i];
		for (int ii = 0; ii < 32; ++ii)
			rotSlice[rotOffset + ii] = rotSlice[rotOffset + ii] << 2 | (((slice >> ((31 - ii) * 2)) & 3));
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
	Fast culling of a whole line in the chunk, using the edge of the adjacent chunk on 64 bits
*/
uint64_t	fatCulling(const uint64_t &slice, const bool &dir, const uint64_t &edge)
{
	if (dir)
		return (slice & ~((slice << 2) | edge)); // right faces
	else
		return (slice & ~((slice >> 2) | (edge << 62))); // left faces
}


/*
	generates a bit slice based on a height and a list of height (if under it gives a 1 for each of the list's elements)
*/
// uint64_t	Chunk::getGenEdgeSlice(int edge[32], const int &height)
// {
// 	uint64_t	slice = 0;

// 	// for (int i = 0; i < 32; ++i)
// 	// 	slice |= ((3 * (height <= edge[i])) << ((31 - i) * 2));

// 	(void)edge;
// 	GenInfo	block;
// 	for (int i = 0; i < 32; ++i)
// 	{
// 		block = getGeneration(glm::vec3(pos.x + 31, height, pos.z + i));
// 		if (block.type > 1)
// 			block.type = 3;
// 		slice |= (block.type << ((31 - i) * 2));
// 	}

// 	return (slice);
// }

// the offsets for the bitshift
const int OFFSET1 = 6;  // x
const int OFFSET2 = 15; // xy
const int OFFSET3 = 21; // xyz
const int OFFSET4 = 23; // xyz + texture coord
const int OFFSET5 = 29; // xyz + texture coord + blocktype

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
void	Chunk::insertBlock(glm::ivec3 &chunkPos, const std::vector<uint64_t> &usedData, const Slices &slice, const bool &isWater)
{
	// to add a face we add the offsets of each of it's vertexes to it's position the chunk and then give it's normal based on the face.
	// things change for the north and south slices as the positions are rotated
	if ((isWater || (!isWater && (slice.rotSlice & 3) != 1)) && (slice.northFaces & 3 || slice.southFaces & 3)) // skipping full air
	{
		uint8_t	block = Blocks[(chunkPos.y << 10) + ((31 - chunkPos.x) << 5) + (chunkPos.z)].type - 1;
		glm::ivec3	rotChunkPos = glm::ivec3((31 - chunkPos.z), chunkPos.y, (31 - chunkPos.x));
		if (slice.northFaces & 3)
			addVertices(block + (block == 3), V2 + rotChunkPos, V1 + rotChunkPos, V6 + rotChunkPos, V5 + rotChunkPos, 3);
		if (slice.southFaces & 3)
			addVertices(block + (block == 3), V4 + rotChunkPos, V3 + rotChunkPos, V8 + rotChunkPos, V7 + rotChunkPos, 4);
	}

	if ((isWater || (!isWater && (slice.slice & 3) != 1)) && (slice.westFaces & 3 || slice.eastFaces & 3 || slice.slice & 3)) // same as the other one
	{
		uint8_t	block = Blocks[(chunkPos.y << 10) + (chunkPos.z << 5) + 31 - chunkPos.x].type - 1;
		if (slice.westFaces & 3)
			addVertices(block + (block == 3), V1 + chunkPos, V4 + chunkPos, V5 + chunkPos, V8 + chunkPos, 0);
		if (slice.eastFaces & 3)
			addVertices(block + (block == 3), V3 + chunkPos, V2 + chunkPos, V7 + chunkPos, V6 + chunkPos, 1);

		if (slice.slice & 3 // top face
			&& ((chunkPos.y + 1) * 32 + chunkPos.z >= (int)usedData.size()
				|| ((usedData[(chunkPos.y + 1) * 32 + chunkPos.z] >> (chunkPos.x * 2)) & 3) != (slice.slice & 3)))
			addVertices(block, V1 + chunkPos, V2 + chunkPos, V4 + chunkPos, V3 + chunkPos, 2);

		if (chunkPos.y != 0 && slice.slice & 3
			&& (chunkPos.y - 1) * 32 + chunkPos.z < (int)usedData.size()
				&& !((usedData[(chunkPos.y - 1) * 32 + chunkPos.z] >> (chunkPos.x * 2)) & 3))
			addVertices(block - (block == 3), V7 + chunkPos, V6 + chunkPos, V8 + chunkPos, V5 + chunkPos, 5);
	}

}

/*
	generated the mesh of a chunk based on it's ChunkMask
*/
void	Chunk::genMesh()
{
	// consoleLog("2.1.2.3.1", NORMAL);
	Slices	ground, trs;

	_vertices.reserve(1572864); // 1572864 is 16*16*256*(6*4) because you can have max 16*16 VISIBLE blocks on a chunk's slice with ech having 6*4 vertices
	_indices.reserve(2359296); // 2359296 is (1572864/4) * 6 because for each 4 vertices 6 indices are added

	glm::ivec3	chunkPos(0, 0, 0);
	uint64_t	edges[4][std::max((int)_maxHeight.load(), WATERLINE) + 1]; // these are the slices of adjacent chunks (4-7 is for water only) 0 = west, 1 = east, 2 = north, 3 = south
	Chunk		*sideChunks[4] = {NULL, NULL, NULL, NULL}; // the adjacent chunks 0 = west, 1 = east, 2 = north, 3 = south

	// consoleLog("2.1.2.3.2", NORMAL);
	// getting adjacent chunks
	sideChunks[0] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x - 1, pos.z));
	sideChunks[1] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x + 32, pos.z));
	sideChunks[2] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x, pos.z + 32));
	sideChunks[3] = CHUNKS->getQuadTree()->getLeaf(glm::vec2(pos.x, pos.z - 1));

	// consoleLog("2.1.2.3.3", NORMAL);
	// checking if they are edited
	sideChunks[0] = (sideChunks[0] && sideChunks[0]->getState() != CS_EMPTY && sideChunks[0]->setUsed() ? sideChunks[0] : NULL);
	sideChunks[1] = (sideChunks[1] && sideChunks[1]->getState() != CS_EMPTY && sideChunks[1]->setUsed() ? sideChunks[1] : NULL);
	sideChunks[2] = (sideChunks[2] && sideChunks[2]->getState() != CS_EMPTY && sideChunks[2]->setUsed() ? sideChunks[2] : NULL);
	sideChunks[3] = (sideChunks[3] && sideChunks[3]->getState() != CS_EMPTY && sideChunks[3]->setUsed() ? sideChunks[3] : NULL);

	if ((!sideChunks[0] || !sideChunks[1] || !sideChunks[2] || !sideChunks[3]))
	{
		setRemesh(true);
		if (sideChunks[0])
			sideChunks[0]->setUnused();
		if (sideChunks[1])
			sideChunks[1]->setUnused();
		if (sideChunks[2])
			sideChunks[2]->setUnused();
		if (sideChunks[3])
			sideChunks[3]->setUnused();
		return ;
	}

	for (int y = 0; y <= std::max(_maxHeight.load(), (uint8_t)WATERLINE); ++y)
	{
		// getting the proper adjacent slice for the y level
		if (sideChunks[0])
			edges[0][y] = sideChunks[0]->RotChunkMask[y * 32];

		if (sideChunks[1])
			edges[1][y] = sideChunks[1]->RotChunkMask[y * 32 + 31];

		if (sideChunks[2])
			edges[2][y] = sideChunks[2]->ChunkMask[y * 32];

		if (sideChunks[3])
			edges[3][y] = sideChunks[3]->ChunkMask[y * 32 + 31];
	}

	// consoleLog("2.1.2.3.4", NORMAL);
	// Generating the mesh
	for (chunkPos.y = 0; chunkPos.y <= std::max(_maxHeight.load(), (uint8_t)WATERLINE); ++chunkPos.y)
	{

		for (chunkPos.z = 0; chunkPos.z < 32; ++chunkPos.z)
		{
			// culling the slices for the ground (setting which faces will be generated) using the adjacent chunk slices
			ground.slice = ChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.rotSlice = RotChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.westFaces = fatCulling(ground.slice, true, ((edges[0][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.eastFaces = fatCulling(ground.slice, false, ((edges[1][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.northFaces = fatCulling(ground.rotSlice, true, ((edges[2][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.southFaces = fatCulling(ground.rotSlice, false, ((edges[3][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));

			trs.slice = ChunkTrsMask[chunkPos.y * 32 + chunkPos.z];
			trs.rotSlice = RotChunkTrsMask[chunkPos.y * 32 + chunkPos.z];
			trs.westFaces = trs.slice;
			trs.eastFaces = trs.slice;
			trs.northFaces = trs.rotSlice;
			trs.southFaces = trs.rotSlice;

			// creating the blocks for the slice
			for (chunkPos.x = 0; chunkPos.x < 32; ++chunkPos.x)
			{
				insertBlock(chunkPos, ChunkMask, ground, false);
				insertBlock(chunkPos, ChunkMask, trs, false);
				ground.shift();
				trs.shift();
			}
		}
	}
	for (chunkPos.y = 0; chunkPos.y <= std::max(_maxHeight.load(), (uint8_t)WATERLINE); ++chunkPos.y)
	{

		for (chunkPos.z = 0; chunkPos.z < 32; ++chunkPos.z)
		{
			// culling the slices for the ground (setting which faces will be generated) using the adjacent chunk slices
			ground.slice = ChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.rotSlice = RotChunkMask[chunkPos.y * 32 + chunkPos.z];
			ground.westFaces = fatCulling(ground.slice, true, ((edges[0][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.eastFaces = fatCulling(ground.slice, false, ((edges[1][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.northFaces = fatCulling(ground.rotSlice, true, ((edges[2][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));
			ground.southFaces = fatCulling(ground.rotSlice, false, ((edges[3][chunkPos.y] >> ((31 - chunkPos.z) * 2)) & 3));

			// creating the blocks for the slice
			for (chunkPos.x = 0; chunkPos.x < 32; ++chunkPos.x)
			{
				insertBlock(chunkPos, ChunkMask, ground, true);
				ground.shift();
			}
		}
	}

	// consoleLog("2.1.2.3.5", NORMAL);
	// freeing up unused space
	_vertices.shrink_to_fit();
	_indices.shrink_to_fit();

	setRemesh(false);
	
	// consoleLog("2.1.2.3.6", NORMAL);
	if (sideChunks[0])
		sideChunks[0]->setUnused();
	if (sideChunks[1])
		sideChunks[1]->setUnused();
	if (sideChunks[2])
		sideChunks[2]->setUnused();
	if (sideChunks[3])
		sideChunks[3]->setUnused();
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
		{ -0.3f, 62},  // shallow ocean
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
		{0, 0},
		{0.5, 5},
		{0.7, 10},
		{1.0f, 40},
	}
};

Spline peaksValleysToHeight =
{
	{
		{-1, -25},
		{-0.8, -18},
		{-0.1, -16},
		{0, -15},
		{0.08, 0},
		{1, 0}
	}
};

float	Chunk::getErosion(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.001, 1, 6));
}

float	Chunk::getContinentalness(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.005, 1, 6));
}

float	Chunk::getPeaksValleys(const glm::vec2 &pos)
{
	return (std::abs(calcNoise(pos, 0.003, 1, 4)));
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

BiomeType	Chunk::getBiomeType()
{
	BiomeType	res;

	if (_currentPeaksValleys < 0.08 && _currentContinentalness < 0.15) // RIVER
		res = BiomeType::RIVER;

	else if (_currentContinentalness < 0.13 && _currentErosion < 0.5 && _currentPeaksValleys > 0.07) // PLAINS
		res = BiomeType::PLAINS;

	else if (_currentContinentalness > 0.2 && _currentErosion < 0) //MOUNTAINS
		res = BiomeType::MOUNTAINS;

	else //HILLS
		res = BiomeType::HILLS;

	return (res);
}

uint8_t	Chunk::getBiomeBlock(float y, BiomeType type)
{
	if (type == BiomeType::MOUNTAINS)
	{
		if (_currentTemperature > 0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y >= _currentMaxHeight - 20)
			{
				float	terracottaNoise = calcNoise(glm::vec2(y, y), 0.15, 1, 1);
				if (terracottaNoise > 0.1 && terracottaNoise < 0.2)
					return (RED_TERRACOTTA_ID);
				if (terracottaNoise < 0.1 && terracottaNoise > 0)
					return (BROWN_TERRACOTTA_ID);
				if (terracottaNoise < -0.2 && terracottaNoise > -0.3)
					return (YELLOW_TERRACOTTA_ID);
				if (terracottaNoise > 0.2 && terracottaNoise < 0.3)
					return (LIGHT_GRAY_TERRACOTTA_ID);
				if (terracottaNoise > 0.4 || terracottaNoise < -0.4)
					return (WHITE_TERRACOTTA_ID);
				return (TERRACOTA_ID);
			}
			else
				return (STONE_ID);
		}
		else if (_currentTemperature < -0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y >= _currentMaxHeight - 3)
				return (SNOW_ID);
			else
				return (STONE_ID);
		}
		else
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y == _currentMaxHeight)
				return (STONE_ID);
			if (y == _currentMaxHeight - 1)
				return (STONE_ID);
			else
				return (STONE_ID);
		}
	}
	if (type == BiomeType::HILLS)
	{
		if (_currentTemperature > 0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y == _currentMaxHeight)
				return (RED_SAND_ID);
			if (y == _currentMaxHeight - 1)
				return (RED_SANDSTONE_ID);
			else
				return (STONE_ID);
		}
		else if (_currentTemperature < -0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y == _currentMaxHeight)
				return (SNOW_ID);
			if (y == _currentMaxHeight - 1)
				return (STONE_ID);
			else
				return (STONE_ID);
		}
		else
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (DIRT_ID);
			else if (y == _currentMaxHeight)
				return (GRASS_ID);
			if (y == _currentMaxHeight - 1)
				return (DIRT_ID);
			else
				return (STONE_ID);
		}
	}
	if (type == BiomeType::PLAINS)
	{
		if (_currentTemperature > 0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (SAND_ID);
			else if (y == _currentMaxHeight)
				return (SAND_ID);
			if (y >= _currentMaxHeight - 3)
				return (SANDSTONE_ID);
			else
				return (STONE_ID);
		}
		else if (_currentTemperature < -0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (SAND_ID);
			else if (y == _currentMaxHeight)
				return (SNOW_ID);
			if (y >= _currentMaxHeight - 3)
				return (DIRT_ID);
			else
				return (STONE_ID);
		}
		else
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (SAND_ID);
			else if (y == _currentMaxHeight)
				return (GRASS_ID);
			if (y >= _currentMaxHeight - 3)
				return (DIRT_ID);
			else
				return (STONE_ID);
		}
	}
	if (type == BiomeType::RIVER)
	{
		if (_currentTemperature > 0.2)
		{
			if (y == _currentMaxHeight && _currentMaxHeight + 16 > WATERLINE)
				return (SAND_ID);
			else if (y == _currentMaxHeight)
				return (SAND_ID);
			if (y >= _currentMaxHeight - 3 && _currentMaxHeight + 16 > WATERLINE)
				return (SAND_ID);
			if (y >= _currentMaxHeight - 8)
				return (SANDSTONE_ID);
			else
				return (STONE_ID);
		}
		else if (_currentTemperature < -0.2)
		{
			if (y == _currentMaxHeight && y <= WATERLINE)
				return (STONE_ID);
			else if (y == _currentMaxHeight)
				return (SAND_ID);
			else
				return (STONE_ID);
		}
		else
		{
			if (y == _currentMaxHeight && _currentMaxHeight + 16 > WATERLINE)
				return (SAND_ID);
			else if (y == _currentMaxHeight)
				return (DIRT_ID);
			if (y >= _currentMaxHeight - 3 && _currentMaxHeight + 16 > WATERLINE)
				return (SAND_ID);
			if (y >= _currentMaxHeight - 8)
				return (DIRT_ID);
			else
				return (STONE_ID);
		}
	}
	return (DIAMOND_BLOCK_ID);
}

float	Chunk::getTemperature(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.000375, 1, 4));
}

float	Chunk::getHumidity(const glm::vec2 &pos)
{
	return (calcNoise(pos, 0.0015, 1, 4));
}

GenInfo	Chunk::getGeneration(const glm::vec3 &pos)
{
	if (_currentMaxHeight == 0)
		getGenerationHeight(glm::vec2(pos.x, pos.z));

	GenInfo	res;
	res.type = 0;

	if (pos.y > _currentMaxHeight)
		return res;

	//Gets world shape (caves and height is calculated outside)
	float noise = getCaveValue(pos, 5, (int)_currentMaxHeight + 16);
	if (noise > CAVE_TRESHOLD)
	{
		res.type = 0;
		return (res);
	}

	_currentBiomeType = getBiomeType();
	_currentTemperature = getTemperature(glm::vec2(pos.x, pos.z));
	_currentHumidity = getHumidity(glm::vec2(pos.x, pos.z));

	res.type = getBiomeBlock(pos.y, _currentBiomeType);

	return (res);
}

void	Chunk::setBlock(int type, int x, int y, int z)
{
	GenInfo	block = GenInfo();
	block.type = type;
	block.height = y;
	Blocks[y * 1024 + z * 32 + x] = block;
	if (type == 0)
		return ;
	if (type == GLASS_ID || type == OAK_LEAVES_ID || type == MANGROVE_LEAVES_ID || type == JUNGLE_LEAVES_ID || type == SPRUCE_LEAVES_ID)
	{
		ChunkMask[y * 32 + z] &= ~((uint64_t)(((uint64_t)3) << ((31 - x) * 2)));
		ChunkTrsMask[y * 32 + z] |= (uint64_t)(((uint64_t)3) << ((31 - x) * 2));
	}
	else if (type == 1)	
		ChunkMask[y * 32 + z] |= (uint64_t)(((uint64_t)1) << ((31 - x) * 2));
	else
		ChunkMask[y * 32 + z] |= (uint64_t)(((uint64_t)3) << ((31 - x) * 2));


	if (y > _maxHeight.load())
		_maxHeight = y;
	if (y < _minHeight.load())
		_minHeight = y;
}

void	Chunk::growTemperateTree(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	(void)wx;(void)wy;(void)wz;
	int	treeTop = wy + 10 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 5;
	if (treeTop > wy + 8)
		treeTop = wy + 8;

	int blockInChunkPosX = 0;
	int blockInChunkPosZ = 0;

	for (int sizeX = wx - 2; sizeX <= wx + 2; sizeX++)
		for (int sizeZ = wz - 2; sizeZ <= wz + 2; sizeZ++)
			for (int sizeY = treeTop - 2; sizeY <= treeTop + 2; sizeY++)
			{
				blockInChunkPosX = 31 - (sizeX - pos.x);
				blockInChunkPosZ = sizeZ - pos.z;
				if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
					setBlock(OAK_LEAVES_ID, blockInChunkPosX, sizeY, blockInChunkPosZ);
			}

	blockInChunkPosX = 31 - (wx - pos.x);
	blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(OAK_LOG_ID, blockInChunkPosX, height, blockInChunkPosZ);
}

void	Chunk::growSwampTree(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	(void)wx;(void)wy;(void)wz;
	int	treeTop = wy + 10 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 5;
	if (treeTop > wy + 8)
		treeTop = wy + 8;

	int blockInChunkPosX = 0;
	int blockInChunkPosZ = 0;

	for (int sizeX = wx - 3; sizeX <= wx + 3; sizeX++)
		for (int sizeZ = wz - 3; sizeZ <= wz + 3; sizeZ++)
			for (int sizeY = treeTop - 2; sizeY <= treeTop + 2; sizeY++)
			{
				blockInChunkPosX = 31 - (sizeX - pos.x);
				blockInChunkPosZ = sizeZ - pos.z;
				if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
					setBlock(MANGROVE_LEAVES_ID, blockInChunkPosX, sizeY, blockInChunkPosZ);
			}

	blockInChunkPosX = 31 - (wx - pos.x);
	blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(MANGROVE_LOG_ID, blockInChunkPosX, height, blockInChunkPosZ);
}

void	Chunk::growJungleTree(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	(void)wx;(void)wy;(void)wz;
	int	treeTop = wy + 10 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 10;

	int blockInChunkPosX = 0;
	int blockInChunkPosZ = 0;

	for (int sizeX = wx - 3; sizeX <= wx + 3; sizeX++)
		for (int sizeZ = wz - 3; sizeZ <= wz + 3; sizeZ++)
			for (int sizeY = treeTop - 2; sizeY <= treeTop + 2; sizeY++)
			{
				blockInChunkPosX = 31 - (sizeX - pos.x);
				blockInChunkPosZ = sizeZ - pos.z;
				if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
					setBlock(JUNGLE_LEAVES_ID, blockInChunkPosX, sizeY, blockInChunkPosZ);
			}

	blockInChunkPosX = 31 - (wx - pos.x);
	blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(JUNGLE_LOG_ID, blockInChunkPosX, height, blockInChunkPosZ);
}

void	Chunk::growColdTree(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	(void)wx;(void)wy;(void)wz;
	int	treeTop = wy + 10 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 5;
	if (treeTop > wy + 8)
		treeTop = wy + 8;

	int blockInChunkPosX = 0;
	int blockInChunkPosZ = 0;

	for (int sizeX = wx - 1; sizeX <= wx + 1; sizeX++)
		for (int sizeZ = wz - 1; sizeZ <= wz + 1; sizeZ++)
			for (int sizeY = treeTop - 2; sizeY <= treeTop + 4; sizeY++)
			{
				blockInChunkPosX = 31 - (sizeX - pos.x);
				blockInChunkPosZ = sizeZ - pos.z;
				if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
					setBlock(SPRUCE_LEAVES_ID, blockInChunkPosX, sizeY, blockInChunkPosZ);
			}

	blockInChunkPosX = 31 - (wx - pos.x);
	blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(SPRUCE_LOG_ID, blockInChunkPosX, height, blockInChunkPosZ);

}

void	Chunk::growIceSpike(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	(void)wx;(void)wy;(void)wz;
	int	treeTop = wy + 16 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 5;

	int blockInChunkPosX = 0;
	int blockInChunkPosZ = 0;

	for (int sizeY = maxHeight; sizeY <= treeTop - 3; sizeY++)
		for (int sizeX = wx - 1; sizeX <= wx + 1; sizeX++)
			for (int sizeZ = wz - 1; sizeZ <= wz + 1; sizeZ++)
			{
				blockInChunkPosX = 31 - (sizeX - pos.x);
				blockInChunkPosZ = sizeZ - pos.z;
				if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
					setBlock(ICE_ID, blockInChunkPosX, sizeY, blockInChunkPosZ);
			}

	blockInChunkPosX = 31 - (wx - pos.x);
	blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(ICE_ID, blockInChunkPosX, height, blockInChunkPosZ);

}

void	Chunk::growCactus(int wx, int wy, int wz)
{
	int maxHeight = wy - 1;

	_currentMaxHeight = maxHeight;
	if (getGeneration(glm::vec3(wx, maxHeight, wz)).type ==  0)
		return ;

	int	treeTop = wy + 10 * std::abs(calcNoise(glm::vec2(wx, wz), 0.999, 1, 2)) + 1;
	if (treeTop > wy + 6)
		treeTop = wy + 6;

	int blockInChunkPosX = 31 - (wx - pos.x);
	int blockInChunkPosZ = wz - pos.z;

	for (int height = wy; height < treeTop; height++) //Place trunk
		if (!(blockInChunkPosX < 0 || blockInChunkPosX >= 32 || blockInChunkPosZ < 0 || blockInChunkPosZ >= 32))
			setBlock(CACTUS_ID, blockInChunkPosX, height, blockInChunkPosZ);
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
	ChunkTrsMask.resize(8192, 0);
	RotChunkTrsMask.resize(8192, 0);
	Blocks.resize(262144, newBlock); // 32 * 32 * 256

	for (int z = 0; z < 32; ++z) //Loops over every block to generate the world shape, skipping air blocks
	{
		for (int x = 0; x < 32; ++x)
		{
			height = getGenerationHeight(glm::vec2{(31 - x) + pos.x, pos.z + z});

			if (height > _maxHeight.load())
				_maxHeight = height;
			if (height < _minHeight.load())
				_minHeight = height;
			_chunkTop.push_back(height); // this vector stores the y values of the top blocks

			_currentMaxHeight = height;
			for (int y = height; y >= 0; --y) //Generates terrain shape
			{
				newBlock = getGeneration(glm::vec3((31 - x) + pos.x, y, pos.z + z));

				setBlock(newBlock.type, x, y, z);
			}
		}
	}

	for (int z = -16; z < 32 + 16; ++z) //Generates features on the terrain (trees and all)
	{
		for (int x = -16; x < 32 + 16; ++x)
		{
			if ((31 - x) + pos.x < 0 || pos.z + z < 0)
				continue ;

			height = getGenerationHeight(glm::vec2{(31 - x) + pos.x, pos.z + z});
			_currentBiomeType = getBiomeType();
			_currentTemperature = getTemperature(glm::vec2((31 - x) + pos.x, pos.z + z));
			_currentHumidity = getHumidity(glm::vec2((31 - x) + pos.x, pos.z + z));

			float	noise = calcNoise(glm::vec2((31 - x) + pos.x, pos.z + z), 0.99, 1, 1);
			if (height > WATERLINE && noise > 0.5 && noise < 0.55)
			{
				if ((_currentBiomeType == BiomeType::HILLS || _currentBiomeType == BiomeType::PLAINS) && _currentTemperature < 0.2 && _currentTemperature > -0.2 && _currentHumidity < 0)
					growTemperateTree((31 - x) + pos.x, height + 1, pos.z + z);
				else if ((_currentBiomeType == BiomeType::HILLS || _currentBiomeType == BiomeType::PLAINS) && _currentTemperature < 0.2 && _currentTemperature > -0.2 && _currentHumidity > 0)
					growSwampTree((31 - x) + pos.x, height + 1, pos.z + z);
				else if (_currentBiomeType == BiomeType::PLAINS && _currentTemperature > 0.2)
					growCactus((31 - x) + pos.x, height + 1, pos.z + z);
				else if ((_currentBiomeType == BiomeType::HILLS || _currentBiomeType == BiomeType::PLAINS) && _currentTemperature < -0.2 && _currentHumidity < 0)
					growColdTree((31 - x) + pos.x, height + 1, pos.z + z);
				else if ((_currentBiomeType == BiomeType::HILLS || _currentBiomeType == BiomeType::PLAINS) && _currentTemperature < -0.2 && _currentHumidity > 0)
					growIceSpike((31 - x) + pos.x, height + 1, pos.z + z);
			}
		}
	}

	// Add water
	for (int z = 0; z < 32; ++z)
	{
		for (int x = 0; x < 32; ++x)
		{
			for (int y = WATERLINE; y > _chunkTop[z * 32 + x]; --y)
			{
				Blocks[y * 1024 + z * 32 + x].height = y;
				Blocks[y * 1024 + z * 32 + x].biome = 0;
				Blocks[y * 1024 + z * 32 + x].type = 1;
				ChunkMask[y * 32 + z] |= (uint64_t)(((uint64_t)1) << ((31 - x) * 2));
			}
		}
	}

	for (int y = std::max((int)_maxHeight.load(), WATERLINE); y >= 0; --y)
	{
		fatGetRotSlice(RotChunkMask, y * 32, y * 32, ChunkMask);
		fatGetRotSlice(RotChunkTrsMask, y * 32, y * 32, ChunkTrsMask);
	}

	ChunkMask.shrink_to_fit();
	RotChunkMask.shrink_to_fit();
	ChunkTrsMask.shrink_to_fit();
	RotChunkTrsMask.shrink_to_fit();
	Blocks.shrink_to_fit();
}

#include "ChunkGeneratorManager.hpp"
extern ChunkGeneratorManager	*CHUNK_GENERATOR;

void	Chunk::draw(Shader &shader)
{
	if (getGenerating() || getState() <= CS_GENERATED || getRemesh())
		return ;
	if (getState() == CS_MESHED)
		upload();

	shader.setMat4("model", model);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, _indicesSize, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

bool	Chunk::placeBlock(const glm::ivec3 &targetPos, const uint8_t &blockType)
{
	glm::ivec3 targetPosMod = targetPos % 32;
	int			pos = targetPos.y * 32 + targetPosMod.z;
	uint64_t	slice = (pos >= ChunkMaskSize ? 0 : ChunkMask[pos]);
	uint64_t	sliceTrs = (pos >= ChunkMaskSize ? 0 : ChunkTrsMask[pos]);
	if (((slice >> (targetPosMod.x * 2)) & 3) || ((sliceTrs >> (targetPosMod.x * 2)) & 3))
		return (false);

	_edited = true;

	setBlock(blockType, 31 - targetPosMod.x, targetPos.y, targetPosMod.z);

	fatGetRotSlice(RotChunkMask, targetPos.y * 32, targetPos.y * 32, ChunkMask);
	fatGetRotSlice(RotChunkTrsMask, targetPos.y * 32, targetPos.y * 32, ChunkTrsMask);

	reGenMesh(true);
	upload();

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
		if (chunk && chunk->getState() != CS_EMPTY)
		{
			chunk->reGenMesh(true);
			chunk->upload();
		}
	}
	if (targetPos.z != sideReload.y)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(targetPos.x, sideReload.y));
		if (chunk && chunk->getState() != CS_EMPTY)
		{
			chunk->reGenMesh(true);
			chunk->upload();
		}
	}

	return (true);
}

#include<bitset>

bool	Chunk::removeBlock(const glm::ivec3 &targetPos)
{
	glm::ivec3 targetPosMod = targetPos % 32;
	int			pos = targetPos.y * 32 + targetPosMod.z;
	uint64_t	slice = (pos >= ChunkMaskSize ? 0 : ChunkMask[pos]);
	uint64_t	sliceTrs = (pos >= ChunkMaskSize ? 0 : ChunkTrsMask[pos]);
	if (!((slice >> (targetPosMod.x * 2)) & 3) && !((sliceTrs >> (targetPosMod.x * 2)) & 3))
		return (false);

	// this will make this chunk unable to be deleted by the QuadTree
	_edited.store(true);

	// editing the mask slice to remove the desired block
	uint64_t rawSlice = slice;
	slice = rawSlice & ~((uint64_t)0b11 << ((targetPosMod.x) * 2));


	uint64_t rawSliceTrs = sliceTrs;
	sliceTrs = rawSliceTrs & ~((uint64_t)0b11 << ((targetPosMod.x) * 2));


	// updating the different vectors of the chunk
	ChunkMask[pos] = slice;
	fatGetRotSlice(RotChunkMask, targetPos.y * 32, targetPos.y * 32, ChunkMask);

	ChunkTrsMask[pos] = sliceTrs;
	fatGetRotSlice(RotChunkTrsMask, targetPos.y * 32, targetPos.y * 32, ChunkTrsMask);

	Blocks[targetPos.y * 1024 + targetPosMod.z * 32 + (31 - targetPosMod.x)].type = 0;

	--_chunkTop[targetPosMod.z * 32 + (31 - targetPosMod.x)];

	reGenMesh(true);
	upload();

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
		if (chunk && chunk->getState() != CS_EMPTY)
		{
			chunk->reGenMesh(true);
			chunk->upload();
		}
	}
	if (targetPos.z != sideReload.y)
	{
		Chunk *chunk = CHUNKS->getQuadTree()->getLeaf(glm::vec2(targetPos.x, sideReload.y));
		if (chunk && chunk->getState() != CS_EMPTY)
		{
			chunk->reGenMesh(true);
			chunk->upload();
		}
	}

	// the return is important for the raycast so it knows to stop when a block is deleted.
	return (true);
}

bool	Chunk::isBlock(const glm::ivec3 &targetPos)
{
	glm::ivec3 targetPosMod = targetPos % 32;
	int			pos = targetPos.y * 32 + targetPosMod.z;
	uint64_t	slice = (pos >= ChunkMaskSize ? 0 : ChunkMask[pos]);
	uint64_t	sliceTrs = (pos >= ChunkMaskSize ? 0 : ChunkTrsMask[pos]);
	return (((slice >> (targetPosMod.x * 2)) & 3) || ((sliceTrs >> (targetPosMod.x * 2)) & 3));
}
