/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:16:19 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/14 18:47:05 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHADER_HPP
# define SHADER_HPP

# include "libs.hpp"

class Shader
{
	public:
		~Shader();
		Shader(const Shader &cpy)
		{
			this->ID = cpy.ID;
		}
		Shader(unsigned int ID)
		{
			this->ID = ID;
		}
		Shader(const char *vertexPath, const char *fragmentPath);
		
		void	use() const;
		void	setBool(const std::string &name, bool value) const;
		void	setInt(const std::string &name, int value) const;
		void	setFloat(const std::string &name, float value) const;
		void	setMat4(const std::string &name, glm::mat4 value) const;
		void	setVec3(const std::string &name, glm::vec3 value) const;
	private:
		unsigned int	ID;
		std::string		vertPath;
		std::string		fragPath;
};

#endif
