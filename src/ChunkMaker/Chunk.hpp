/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/25 11:03:02 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Camera.hpp"

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
		int						ChunkMaskSize = 8192;
		std::vector<char32_t>	WaterMask;
		std::vector<GenInfo>	Blocks;
		std::atomic_bool		_edited;
		float					dist;
		uint8_t					_minHeight = 0;
		uint8_t					_maxHeight = 0;
		uint8_t					_currentMaxHeight = 0;
		uint8_t					_currentBiome;
		std::atomic_bool		_used;
		std::atomic_bool		_isBorder;

		void	initDist();
		float	getDist() const;
		bool	isInRange();

		bool	removeBlock(const glm::ivec3 &targetPos);

		std::thread::id	_lastThreadID;

		static float	getErosion(const glm::vec2 &pos);
		static float	getContinentalness(const glm::vec2 &pos);
		static float	getPeaksValleys(const glm::vec2 &pos);
		static float	getTemperature(const glm::vec2 &pos);
		static float	getHumidity(const glm::vec2 &pos);
		BiomeType	getBiomeType();
		uint8_t	getBiomeBlock(float y, BiomeType type);

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

		void	addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal);
		void	placeBlock(glm::ivec3 &chunkPos, const std::vector<uint64_t> &usedData, const Slices &slice);
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
