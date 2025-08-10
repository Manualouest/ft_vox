/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/14 18:13:32 by mbatty           ###   ########.fr       */
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
	char32_t				slice, westFaces, eastFaces, rotSlice, northSlices, southSlices = 0;
	std::vector<char32_t>	rotSlices;

	Slices()
	{
		rotSlices.reserve(32);
	}

	~Slices()
	{
		rotSlices.clear();
		rotSlices.shrink_to_fit();
	}
};

enum Biome
{
	PLAINS,
	MESA,
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
		std::vector<char32_t>	ChunkMask;
		std::vector<char32_t>	RotChunkMask;
		int						ChunkMaskSize = 8192;
		std::vector<char32_t>	WaterMask;
		std::vector<GenInfo>	Blocks;
		std::atomic_bool		_edited;
		float					dist;

		bool	isGenerated() {return (this->_generated);}
		bool	isGenerating() {return (this->_generating);}
		bool	isUploaded() {return (this->_uploaded);}

		void	initDist();
		float	getDist() const;
		void	setGenerating(bool state) {this->_generating.store(state);}
		bool	isInRange();

		bool	removeBlock(const glm::ivec3 &targetPos);

		std::thread::id	_lastThreadID;

	private:
		void	addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal);
		void	placeBlock(glm::ivec3 &pos, const std::vector<char32_t> &usedData, char32_t &slice, char32_t &westFaces, char32_t &eastFaces, char32_t &northFaces, char32_t &southFaces);
		void	genChunk();
		void	getRotSlice(std::vector<char32_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<char32_t>	&usedMask);
		void	genMesh();
		void	makeBuffers();

		std::atomic_bool		_generated;
		std::atomic_bool		_generating;
		std::atomic_bool		_uploaded;

		unsigned int			_EBO = 0;
		unsigned int			_VAO = 0;
		unsigned int			_VBO = 0;
		uint8_t					_minHeight;
		uint8_t					_maxHeight;
		uint32_t				_indicesSize;
		std::vector<uint32_t>	_indices;
		std::vector<uint32_t>	_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
