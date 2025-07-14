/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/14 19:40:03 by mbirou           ###   ########.fr       */
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

		glm::vec3							pos;
		glm::mat4							model;
		std::unordered_map<int, char32_t>	groundData;
		std::unordered_map<int, char32_t>	waterData;
		std::atomic_bool					rendered;
		std::atomic_bool					_edited;

		bool	isGenerated() {return (this->_generated);}
		bool	isGenerating() {return (this->_generating);}
		bool	isUploaded() {return (this->_uploaded);}

		void	setGenerating(bool state) {this->_generating.store(state);}
		bool	isInRange();

		bool	isOnBlock(const glm::vec3 &targetPos);
		float	distToBlock(const glm::vec3 &targetPos);
		bool	removeBlock(const glm::vec3 &targetPos);

		std::thread::id	_lastThreadID;

	private:
		void	checkSurround(const glm::ivec3 &pos, const Block &block, const char32_t &slice, const char32_t &rotSlice, const char32_t &up, const char32_t &down);
		void	gen();
		void	getRotSlice(std::vector<char32_t> &rotSlice, const int &height);
		void	genMesh();
		void	makeBuffers();

		std::atomic_bool		_generated;
		std::atomic_bool		_generating;
		std::atomic_bool		_uploaded;

		unsigned int			_EBO = 0;
		unsigned int			_VAO = 0;
		unsigned int			_VBO = 0;
		glm::mat4				_model;
		uint8_t					_minHeight;
		uint8_t					_maxHeight;
		int						_indicesSize;
		std::vector<int>		_indices;
		std::vector<float>		_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
