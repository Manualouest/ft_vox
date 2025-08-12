/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 12:17:41 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:25:20 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHADERMANAGER_HPP
# define SHADERMANAGER_HPP

# include "libs.hpp"
# include "Shader.hpp"

struct	ShaderInfos
{
	const std::string	&name;
	const char	*vertex;
	const char	*fragment;
};

/*
	Shader manager to store all shaders globally in a scene.
*/
class	ShaderManager
{
	public:
		ShaderManager() {}
		~ShaderManager();

		/*
			Erases a shader from the manager, freing all its data

			@param name Key of the shader to remove
		*/
		bool	erase(const std::string &name);

		/*
			Loads a shader giving it a key and 2 shaders (vertex/fragment)

			@param infos Struct containing name, vertex and fragment
			@param infos.name Key to name the shader, will be used to retrieve it
			@param infos.vertex Vertex shader to load
			@param infos.fragment Fragment shader to load
		*/
		Shader	*load(ShaderInfos infos);

		/*
			Returns a shader based on the key, if no shader exists under that name returns NULL

			@param name Key of the shader to get
		*/
		Shader	*get(const std::string &name);

		//Wrapper around get method
		Shader	*operator[](const std::string &name);
	private:
		std::unordered_map<std::string, Shader *>	shaders;
};

extern	ShaderManager	*SHADER_MANAGER;

#endif
