/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/26 12:06:23 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Camera.hpp"

# define STONE_ID 2
# define DIRT_ID 3
# define GRASS_ID 4
# define SAND_ID 10
# define SANDSTONE_ID 11
# define TERRACOTA_ID 17
# define RED_SANDSTONE_ID 13
# define SNOW_ID 7
# define RED_SAND_ID 12
# define RED_TERRACOTTA_ID 18
# define BROWN_TERRACOTTA_ID 19
# define YELLOW_TERRACOTTA_ID 20
# define LIGHT_GRAY_TERRACOTTA_ID 21
# define WHITE_TERRACOTTA_ID 22
# define OAK_LEAVES_ID 34
# define OAK_LOG_ID 49
# define CACTUS_ID 53
# define SPRUCE_LEAVES_ID 35
# define SPRUCE_LOG_ID 50
# define JUNGLE_LEAVES_ID 36
# define JUNGLE_LOG_ID 51
# define MANGROVE_LEAVES_ID 37
# define MANGROVE_LOG_ID 52
# define SNOWY_GRASS_ID 27
# define ICE_ID 9
# define DIAMOND_ORE_ID 16
# define DIAMOND_BLOCK_ID 64
# define GLASS_ID 33
# define OAK_PLANK_ID 63
# define STONE_BRICK_ID 62

# define WATERLINE 63

extern Camera			*CAMERA;

/*
	slice structure, used for generating the mesh of a chunk, stores the slices of a y level in the chunk
*/
struct Slices
{
	uint64_t				slice, westFaces, eastFaces, rotSlice, northFaces, southFaces = 0;

	void	shift()
	{
		slice >>= 2;
		rotSlice >>= 2;
		westFaces >>= 2;
		eastFaces >>= 2;
		northFaces >>= 2;
		southFaces >>= 2;
	}
};

enum BiomeType
{
	OCEAN,
	RIVER,
	PLAINS,
	HILLS,
	MOUNTAINS
};

struct GenInfo
{
	GenInfo() : height(0), biome(0), type(0)
	{}
	GenInfo(uint8_t h, uint8_t b)
	{
		this->height = h;
		this->biome = b;
	}
	GenInfo(uint8_t h, uint8_t b, uint8_t t)
	{
		this->height = h;
		this->biome = b;
		this->type = t;
	}
	uint8_t	height, biome, type;
};

enum	ChunkState
{
	CS_EMPTY,
	CS_GENERATED,
	CS_MESHED,
	CS_UPLOADED,
	CS_EDITED
};

class Chunk
{
	public:
		Chunk(const glm::vec3 &pos);
		~Chunk();

		void	generate();
		void	mesh();
		void	reGenMesh(const bool &isNotThread);
		void	upload();
		void	clear();

		void	draw(Shader &shader);
		float	getDistance() const;

		glm::vec3				pos;
		glm::mat4				model;
		std::atomic_bool		rendered;
		std::atomic_bool		loaded;
		bool					loadedThisFrame;
		std::vector<uint64_t>	ChunkMask;
		std::vector<uint64_t>	RotChunkMask;
		std::vector<uint64_t>	ChunkTrsMask;
		std::vector<uint64_t>	RotChunkTrsMask;
		int						ChunkMaskSize = 8192;
		std::vector<GenInfo>	Blocks;
		std::atomic_bool		_edited;
		float					dist;
		std::atomic_uint8_t		_minHeight = 0;
		std::atomic_uint8_t		_maxHeight = 0;
		uint8_t					_currentMaxHeight = 0;
		uint8_t					_currentBiome;
		std::atomic_bool		_used;
		std::atomic_bool		_isBorder;

		void	initDist();
		float	getDist() const;
		bool	isInRange();

		bool	removeBlock(const glm::ivec3 &targetPos);
		bool	placeBlock(const glm::ivec3 &targetPos, const uint8_t &blockType);
		bool	isBlock(const glm::ivec3 &targetPos);

		std::thread::id	_lastThreadID;

		static float	getErosion(const glm::vec2 &pos);
		static float	getContinentalness(const glm::vec2 &pos);
		static float	getPeaksValleys(const glm::vec2 &pos);
		static float	getTemperature(const glm::vec2 &pos);
		static float	getHumidity(const glm::vec2 &pos);
		BiomeType		getBiomeType();
		uint8_t			getBiomeBlock(float y, BiomeType type);

		float	_currentErosion;
		float	_currentContinentalness;
		float	_currentPeaksValleys;
		ChunkState	getState()
		{
			std::lock_guard<std::mutex> lock(_stateMutex);
			return (this->_state);
		}
		void	setState(const ChunkState state)
		{
			std::lock_guard<std::mutex> lock(_stateMutex);
			this->_state = state;
		}
		void	setGenerating(bool state)
		{
			std::lock_guard<std::mutex> lock(_generatingMutex);
			_generating = state;
		}
		bool	getGenerating()
		{
			std::lock_guard<std::mutex> lock(_generatingMutex);
			return (_generating);
		}
		void	setRemesh(bool state)
		{
			std::lock_guard<std::mutex> lock(_remeshMutex);
			_remesh = state;
		}
		bool	getRemesh()
		{
			std::lock_guard<std::mutex> lock(_remeshMutex);
			return (_remesh);
		}
		void	saveFile();
		GenInfo	getBlock(int x, int y, int z) //Takes world pos
		{
			if (y > 255 || y < 0)
				return GenInfo();
			x -= pos.x;
			z -= pos.z;
			x = 31 - x;
			return (Blocks[y * 1024 + z * 32 + x]);
		}
	private:
		std::mutex	_stateMutex;
		ChunkState	_state = ChunkState::CS_EMPTY;
		std::mutex	_generatingMutex;
		bool		_generating = false;
		std::mutex	_remeshMutex;
		bool		_remesh = false;

		BiomeType	_currentBiomeType;
		float	_currentTemperature;
		float	_currentHumidity;
		void	setBlock(int block, int x, int y, int z);
	private:
		void	growTemperateTree(int x, int y, int z);
		void	growSwampTree(int x, int y, int z);
		void	growJungleTree(int x, int y, int z);
		void	growColdTree(int x, int y, int z);
		void	growIceSpike(int x, int y, int z);
		void	growCactus(int x, int y, int z);
		GenInfo	getGeneration(const glm::vec3 &pos);
		int		getGenerationHeight(const glm::vec2 &pos);
		GenInfo	getGeneration(const glm::vec2 &pos);
		bool	setUsed()
		{
			_used.store(true);
			nbUsing++;
			return(true);
		}
		bool	setUnused()
		{
			nbUsing--;
			if(nbUsing == 0)
				_used.store(false);
			return (true);
		}

		bool	loadFromFile();

		void	addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal);
		void	insertBlock(glm::ivec3 &chunkPos, const std::vector<uint64_t> &usedData, const Slices &slice, const bool &isWater);
		void	genChunk();
		void	getRotSlice(std::vector<uint64_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<uint64_t>	&usedMask);
		void	fatGetRotSlice(std::vector<uint64_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<uint64_t>	&usedMask);
		void	genMesh();
		void	makeBuffers();

		unsigned int			_EBO = 0;
		unsigned int			_VAO = 0;
		unsigned int			_VBO = 0;
		std::atomic_uint			nbUsing = 0;
		uint32_t				_indicesSize;
		std::vector<uint32_t>	_indices;
		std::vector<uint32_t>	_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
