/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/15 07:04:59 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Camera.hpp"

# define LINELEN 9
# define WATERLINE 63
# define GROUND 1
# define WATER 0

extern Camera	*CAMERA;



struct	Block
{
	Block(int blockID)
	{
		northFace = blockID;
		southFace = blockID;
		eastFace = blockID;
		westFace = blockID;
		topFace = blockID;
		bottomFace = blockID;
	}
	Block(int northID, int southID, int eastID, int westID, int topID, int bottomID)
	{
		northFace = northID;
		southFace = southID;
		eastFace = eastID;
		westFace = westID;
		topFace = topID;
		bottomFace = bottomID;
	}
	int	northFace;
	int	southFace;
	int	eastFace;
	int	westFace;
	int	topFace;
	int	bottomFace;
};


class Chunk
{
	public:
		Chunk(const glm::vec3 &pos);
		~Chunk();

		void	generate();
		void	upload();
		void	clear();

		void	draw(Shader &shader);
		float	getDistance() const;

		glm::vec3				pos;
		glm::mat4				model;
		std::vector<char32_t>	ChunkMask;
		std::vector<char32_t>	RotChunkMask;
		int						ChunkMaskSize = 8192;
		std::vector<char32_t>	WaterMask;
		int						WaterMaskSize;
		std::vector<u_int8_t>	Blocks;
		std::atomic_bool		rendered;
		std::atomic_bool		_edited;
		float					dist;

		bool	isGenerated() {return (this->_generated);}
		bool	isGenerating() {return (this->_generating);}
		bool	isUploaded() {return (this->_uploaded);}

		void	setGenerating(bool state) {this->_generating.store(state);}
		bool	isInRange();

		bool	removeBlock(const glm::ivec3 &targetPos);

		void	initDist();
		float	getDist() const;

		std::thread::id	_lastThreadID;

	private:
		void	addVertices(uint32_t type, const glm::ivec3 &TL, const glm::ivec3 &TR, const glm::ivec3 &BL, const glm::ivec3 &BR, const uint32_t &Normal);
		void	checkSurround(const glm::ivec3 &pos, const Block &block, const char32_t &slice, const char32_t &rotSlice, const char32_t &up, const char32_t &down);
		void	placeBlock(glm::ivec3 pos, const std::vector<char32_t> &usedData, char32_t slice, char32_t westFaces, char32_t eastFaces, char32_t northFaces, char32_t southFaces);
		void	genChunk();
		void	getRotSlice(std::vector<char32_t> &rotSlice, const int &rotOffset, const int &height, const std::vector<char32_t>	&usedMask);
		void	genMesh();
		void	makeBuffers();
		void	reset();

		std::atomic_bool		_generated;
		std::atomic_bool		_needGen;
		std::atomic_bool		_generating;
		std::atomic_bool		_uploaded;

		unsigned int			_EBO = 0;
		unsigned int			_VAO = 0;
		unsigned int			_VBO = 0;
		glm::mat4				_model;
		uint8_t					_minHeight;
		uint8_t					_maxHeight;
		uint32_t				_indicesSize;
		std::vector<uint32_t>	_indices;
		std::vector<uint32_t>	_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
