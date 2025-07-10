/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 12:17:41 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/09 18:08:08 by mbatty           ###   ########.fr       */
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

class	ShaderManager
{
	public:
		ShaderManager()
		{
		}
		~ShaderManager()
		{
			for (auto it = shaders.begin(); it != shaders.end(); it++)
				delete it->second;
		}

		bool	erase(const std::string &name)
		{
			std::map<std::string, Shader *>::iterator	finder;
			finder = shaders.find(name);
			if (finder == shaders.end())
			{
				consoleLog("WARNING Tried to unload a shader thats not loaded: " + name, LogSeverity::WARNING);
				return (0);
			}
			shaders.erase(finder);
			return (1);
		}
		Shader	*load(ShaderInfos infos)
		{
			if (shaders.find(infos.name) != shaders.end())
			{
				consoleLog("WARNING Tried to load a shader thats already loaded (will be using the existing shader): " + infos.name, LogSeverity::WARNING);
				return (this->get(infos.name));
			}
			return (shaders.insert(std::make_pair(infos.name, new Shader(infos.vertex, infos.fragment))).first->second);
		}
		Shader	*get(const std::string &name)
		{
			std::map<std::string, Shader *>::iterator	finder = shaders.find(name);
			if (finder == shaders.end())
			{
				consoleLog("ERROR Tried to access a shader thats not loaded, might cause a crash: " + name, LogSeverity::ERROR);
				return (NULL);
			}
			return (finder->second);
		}
		Shader	*operator[](const std::string &name)
		{
			return (this->get(name));
		}
	private:
		std::map<std::string, Shader *>	shaders;
};

extern	ShaderManager	*SHADER_MANAGER;

#endif
