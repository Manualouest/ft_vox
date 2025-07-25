/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TextureManager.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 10:28:58 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:25:02 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TextureManager.hpp"

TextureManager::~TextureManager()
{
	for (auto it = loadedTextures.begin(); it != loadedTextures.end(); it++)
		delete it->second;
}

bool	TextureManager::erase(const std::string &path)
{
	std::unordered_map<std::string, Texture*>::iterator	finder;
	finder = loadedTextures.find(path);
	if (finder == loadedTextures.end())
	{
		consoleLog("WARNING Tried to unload a texture thats not loaded: " + path, LogSeverity::WARNING);
		return (0);
	}
	loadedTextures.erase(finder);
	return (1);
}

Texture	*TextureManager::load(const std::string &path)
{
	if (loadedTextures.find(path) != loadedTextures.end())
	{
		consoleLog("WARNING Tried to load a texture thats already loaded (will be using the existing texture): " + path, LogSeverity::WARNING);
		return (this->get(path));
	}
	return (loadedTextures.insert({path, new Texture(path.c_str())}).first->second);
}

Texture	*TextureManager::get(const std::string &path)
{
	std::unordered_map<std::string, Texture *>::iterator	finder;
	finder = loadedTextures.find(path);
	if (finder == loadedTextures.end())
		return (load(path));
	return (finder->second);
}

Texture	*TextureManager::operator[](const std::string &path)
{
	return (this->get(path));
}
