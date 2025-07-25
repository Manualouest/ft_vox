/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TextureManager.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 12:22:51 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:24:50 by mbatty           ###   ########.fr       */
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
		~TextureManager();

		/*
			Erases a texture from the manager, freeing all its data

			@param path Texture to erase
		*/
		bool	erase(const std::string &path);

		/*
			Loads a texture from the given path, if cant be created, Texture will throw

			@param path Texture to load
		*/
		Texture	*load(const std::string &path);

		/*
			Returns a texture based on its path, if not found it will return the load method

			@param path Path of the texture to load
		*/
		Texture	*get(const std::string &path);

		//Wrapper around get method
		Texture	*operator[](const std::string &path);
	private:
		std::unordered_map<std::string, Texture*>	loadedTextures;
};

extern TextureManager	*TEXTURE_MANAGER;

#endif
