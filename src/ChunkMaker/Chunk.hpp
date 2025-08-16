/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/08/16 02:01:51 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Camera.hpp"




# include <bitset>




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

enum Biome
{
	PLAINS,
	DESERT,
	SNOWY
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

class Chunk
{
	public:
		Chunk(const glm::vec3 &pos);
		~Chunk();

		void	generate();
		void	reGenMesh();
		void	upload();
		void	clear();

		void	draw(Shader &shader);
		float	getDistance() const;

		glm::vec3				pos;
		glm::mat4				model;
		std::atomic_bool		rendered;
		std::vector<uint64_t>	ChunkMask;
		std::vector<uint64_t>	RotChunkMask;
		int						ChunkMaskSize = 8192;
		std::vector<char32_t>	WaterMask;
		std::vector<GenInfo>	Blocks;
		std::atomic_bool		_used;
		std::atomic_bool		_created;
		std::atomic_bool		_needRemesh;
		std::atomic_bool		_edited;
		float					dist;
		uint8_t					_minHeight = 0;
		uint8_t					_maxHeight = 0;
		uint8_t					_currentMaxHeight = 0;
		uint8_t					_currentBiome;

		bool	isGenerated() {return (this->_generated);}
		bool	isGenerating() {return (this->_generating);}
		bool	isUploaded() {return (this->_uploaded);}
		bool	setUsed() {_used.store(true); nbUsing++; return(true);}
		bool	setUnused() {nbUsing--; if(nbUsing == 0){_used.store(false);}; return(true);}

		void	initDist();
		float	getDist() const;
		void	setGenerating(bool state) {this->_generating.store(state);}
		bool	isInRange();
		void	genMesh();

		bool	removeBlock(const glm::ivec3 &targetPos);

		std::thread::id	_lastThreadID;

		static float	getErosion(glm::vec2 pos);
		static float	getContinentalness(glm::vec2 pos);
		static float	getPeaksValleys(glm::vec2 pos);

		float	_currentErosion;
		float	_currentContinentalness;
		float	_currentPeaksValleys;
	private:
		GenInfo	getGeneration(glm::vec3 pos);
		int	getGenerationHeight(glm::vec2 pos);
		GenInfo	getGeneration(glm::vec2 pos);

		void		addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal);
		void		placeBlock(glm::ivec3 &chunkPos, const std::vector<uint64_t> &usedData, const Slices &slice);
		void		genChunk();
		void		getRotSlice(std::vector<char32_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<char32_t>	&usedMask);
		void		fatGetRotSlice(std::vector<uint64_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<uint64_t>	&usedMask);
		void		makeBuffers();
		uint64_t	getGenEdgeSlice(int edge[32], const int &height);

		std::atomic_bool		_generated;
		std::atomic_bool		_generating;
		std::atomic_bool		_uploaded;

		unsigned int			_EBO = 0;
		unsigned int			_VAO = 0;
		unsigned int			_VBO = 0;
		unsigned int			nbUsing = 0;
		uint32_t				_indicesSize;
		std::vector<uint32_t>	_indices;
		std::vector<uint32_t>	_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
