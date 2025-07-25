/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ShaderManager.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:23:08 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:25:30 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ShaderManager.hpp"

ShaderManager::~ShaderManager()
{
	for (auto it = shaders.begin(); it != shaders.end(); it++)
		delete it->second;
}

bool	ShaderManager::erase(const std::string &name)
{
	std::unordered_map<std::string, Shader *>::iterator	finder;
	finder = shaders.find(name);
	if (finder == shaders.end())
	{
		consoleLog("WARNING Tried to unload a shader thats not loaded: " + name, LogSeverity::WARNING);
		return (0);
	}
	shaders.erase(finder);
	return (1);
}

Shader	*ShaderManager::load(ShaderInfos infos)
{
	if (shaders.find(infos.name) != shaders.end())
	{
		consoleLog("WARNING Tried to load a shader thats already loaded (will be using the existing shader): " + infos.name, LogSeverity::WARNING);
		return (this->get(infos.name));
	}
	return (shaders.insert(std::make_pair(infos.name, new Shader(infos.vertex, infos.fragment))).first->second);
}

Shader	*ShaderManager::get(const std::string &name)
{
	std::unordered_map<std::string, Shader *>::iterator	finder = shaders.find(name);
	if (finder == shaders.end())
	{
		consoleLog("ERROR Tried to access a shader thats not loaded, might cause a crash: " + name, LogSeverity::ERROR);
		return (NULL);
	}
	return (finder->second);
}

Shader	*ShaderManager::operator[](const std::string &name)
{
	return (this->get(name));
}
