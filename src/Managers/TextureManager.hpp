/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TextureManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 12:22:51 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/09 18:07:33 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEXTUREMANAGER_HPP
# define TEXTUREMANAGER_HPP

#include "libs.hpp"
#include "Texture.hpp"

/*

	Stores textures in a map for easy access.

	load() is used to load the texture given by the path.

	get() returns the texture linked to the given path, if not found, will load a new texture and return it. You can also access via the [] operator

*/
class	TextureManager
{
	public:
		TextureManager(){}
		~TextureManager()
		{
			for (auto it = loadedTextures.begin(); it != loadedTextures.end(); it++)
				delete it->second;	
		}

		bool	erase(const std::string &path)
		{
			std::map<std::string, Texture*>::iterator	finder;
			finder = loadedTextures.find(path);
			if (finder == loadedTextures.end())
			{
				consoleLog("WARNING Tried to unload a texture thats not loaded: " + path, LogSeverity::WARNING);
				return (0);
			}
			loadedTextures.erase(finder);
			return (1);
		}

		Texture	*load(const std::string &path)
		{
			if (loadedTextures.find(path) != loadedTextures.end())
			{
				consoleLog("WARNING Tried to load a texture thats already loaded (will be using the existing texture): " + path, LogSeverity::WARNING);
				return (this->get(path));
			}
			return (loadedTextures.insert({path, new Texture(path.c_str())}).first->second);
		}

		Texture	*get(const std::string &path)
		{
			std::map<std::string, Texture *>::iterator	finder;
			finder = loadedTextures.find(path);
			if (finder == loadedTextures.end())
				return (load(path));
			return (finder->second);
		}
		//Since TEXTURE_MANAGER is used as a pointer use this operator like (*TEXTURE_MANAGER)["path"]
		Texture	*operator[](const std::string &path)
		{
			return (this->get(path));
		}
	private:
		std::map<std::string, Texture*>	loadedTextures;
};

extern TextureManager	*TEXTURE_MANAGER;

#endif
