/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/13 08:22:45 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Camera.hpp"

# define LINELEN 7
# define WATERLINE 21
# define GROUND 0
# define WATER 1

extern Camera	*CAMERA;

class Chunk
{
	public:
		Chunk(const glm::vec3 &pos);
		Chunk(const glm::vec3 &pos, bool nocreate);
		~Chunk();

		void	generate();
		void	upload();

		void	draw(Shader &shader);
		float	getDistance() const;

		glm::vec3							pos;
		glm::mat4							model;
		std::unordered_map<int, char32_t>	groundData;
		std::unordered_map<int, char32_t>	waterData;
		std::atomic_bool					generated;
		std::atomic_bool					uploaded;
		bool								rendered = false;
	
	private:
		void	gen();
		void	getRotSlice(std::vector<char32_t> &rotSlice, const int &height);
		void	genMesh();
		void	makeBuffers();
		bool	isInRange();

		unsigned int			_EBO;
		unsigned int			_VAO;
		unsigned int			_VBO;
		glm::mat4				_model;
		uint8_t					_minHeight;
		uint8_t					_maxHeight;
		int						_indicesSize;
		std::vector<int>		_indices;
		std::vector<float>		_vertices;
		std::vector<uint8_t>	_chunkTop;
};

#endif
