/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunk.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/10 09:44:25 by mbirou            #+#    #+#             */
/*   Updated: 2025/07/10 10:46:55 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNK_HPP
# define CHUNK_HPP

# include "libs.hpp"
# include "Shader.hpp"
#include "Camera.hpp"

class Chunk
{
	public:
		Chunk(const glm::vec3 &pos);
		~Chunk();

		void	draw(Camera &camera, Shader &shader);

		glm::vec3							pos;
		std::unordered_map<int, char32_t>	chunkData;
	
	private:
		void	gen();
		void	getRotSlice(std::vector<char32_t> &rotSlice, const int &height);
		void	genMesh();
		void	makeBuffers();

		unsigned int		_EBO;
		unsigned int		_VAO;
		unsigned int		_VBO;
		glm::mat4			_model;
		unsigned int		_minHeight:8;
		unsigned int		_maxHeight:8;
		std::vector<float>	_vertices;
		std::vector<int>	_indices;
		int					_indicesSize;
};

#endif