/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/14 12:24:34 by mbatty           ###   ########.fr       */
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

		bool	isGenerated() {return (this->_generated);}
		bool	isGenerating() {return (this->_generating);}
		bool	isUploaded() {return (this->_uploaded);}

		void	setGenerating(bool state) {this->_generating.store(state);}
		bool	isInRange();

		std::thread::id	_lastThreadID;

	private:
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
