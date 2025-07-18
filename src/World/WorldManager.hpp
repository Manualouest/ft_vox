/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WorldManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 13:12:52 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/18 15:42:21 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORLDMANAGER_HPP
# define WORLDMANAGER_HPP

# include "World.hpp"

# define SAVES_PATH "./saves/"

class	WorldManager
{
	public:
	WorldManager()
	{
		loadSaveFiles();
	}
	~WorldManager()
	{
		for (auto &pair : _worlds)
			delete pair.second;
	}
	World	*get(const std::string &name)
	{
		std::map<std::string, World *>::iterator	finder = _worlds.find(name);
		if (finder == _worlds.end())
		{
			return (NULL);
		}
		return (finder->second);
	}
	World	*load(std::string id, uint seed)
	{
		if (_worlds.find(id) != _worlds.end())
		{
			consoleLog("WARNING Tried to load a world thats already loaded (will be using the existing scene): " + id, LogSeverity::WARNING);
			return (this->get(id));
		}
		return (_worlds.insert(std::make_pair(id, new World(id, seed))).first->second);
	}


	
	void	loadSaveFiles()
	{
		std::filesystem::create_directories(SAVES_PATH);
		for (const auto& entry : std::filesystem::directory_iterator(SAVES_PATH))
			if (entry.is_directory())
			{
				std::string	worldName = entry.path().filename().string();
				_worlds.insert({worldName, new World(worldName, SAVES_PATH + worldName)});
			}
	}
	private:
		std::map<std::string, World*>	_worlds;
};

#endif
