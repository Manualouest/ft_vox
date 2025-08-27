/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   World.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 12:41:57 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/27 11:11:00 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORLD_HPP
# define WORLD_HPP

# include "libs.hpp"

class	World
{
	public:
		World(const std::string &id, uint seed)
		{
			this->_id = id;
			saveWorldInfo("seed", std::to_string(seed));
			saveWorldInfo("display_name", _id);
			this->_path = "./saves/" + id;
			initFiles();
		}
		World(const std::string &id, const std::string &path)
		{
			this->_id = id;
			this->_path = path;
			load();
		}
		~World()
		{
		}
		/*
			Saves informating inside of the world_info.txt file
		*/
		void	saveWorldInfo(const std::string &key, const std::string &value)
		{
			_worldInfos[key] = value;
		}
		void	initFiles()
		{
			_createWorldFolders();

			std::ofstream	file;

			file.open(_path + "/world_info.txt");
			for (auto &pair : _worldInfos)
			{
				std::string	line = pair.first + " " + pair.second + "\n";
				file.write(line.c_str(), line.size());
			}
		}
		void	save()
		{
			_createWorldFolders();
			_saveWorldInfos();

			std::ofstream	file;

			file.open(_path + "/world_info.txt");
			for (auto &pair : _worldInfos)
			{
				std::string	line = pair.first + " " + pair.second + "\n";
				file.write(line.c_str(), line.size());
			}
		}
		void	saveChunk()
		{

		}
		void	load()
		{
			std::ifstream	file;
			std::string		line;

			consoleLog("Loading save: " + _path);
			file.open(_path + "/world_info.txt");
			while (std::getline(file, line))
			{
				std::istringstream iss(line);
				std::string	key;
				std::string	value;

				if (!(iss >> key))
					throw std::runtime_error("Bad format in world file.");

				try {
					std::getline(iss, value);
					value.erase(0, value.find_first_not_of(" \t\r\n"));
					value.erase(value.find_last_not_of(" \t\r\n") + 1);
				} catch (const std::exception &e) {
					throw std::runtime_error(e.what());
				}

				_worldInfos.insert({key, value});
			}
		}
		float	getFloatInfo(std::string id)
		{
			auto finder = _worldInfos.find(id);

			if (finder == _worldInfos.end())
				return (0);
			return (std::atof(finder->second.c_str()));
		}
		uint	getUintInfo(std::string id)
		{
			auto finder = _worldInfos.find(id);

			if (finder == _worldInfos.end())
				return (0);
			return (std::strtoul(finder->second.c_str(), NULL, 10));
		}
		std::string	getWorldInfo(std::string id)
		{
			return (_worldInfos[id]);
		}
		glm::vec3	getPlayerPos()
		{
			glm::vec3	res;

			res.x = getFloatInfo("pos_x");
			res.y = getFloatInfo("pos_y");
			res.z = getFloatInfo("pos_z");
			return (res);
		}
		uint	getSeed() {return (getUintInfo("seed"));}
		std::string	getID() {return (this->_id);}
	private:
		void	_createWorldFolders()
		{
			std::filesystem::create_directories(_path);
			std::filesystem::create_directories(_path + "/playerdata");
			std::filesystem::create_directories(_path + "/world");
			std::filesystem::create_directories(_path + "/world/regions");
			std::filesystem::create_directories(_path + "/world/entities");
		}
		void	_saveWorldInfos()
		{
			saveWorldInfo("display_name", _id);
			saveWorldInfo("pos_x", std::to_string(CAMERA->pos.x));
			saveWorldInfo("pos_y", std::to_string(CAMERA->pos.y));
			saveWorldInfo("pos_z", std::to_string(CAMERA->pos.z));
			saveWorldInfo("yaw", std::to_string(CAMERA->yaw));
			saveWorldInfo("pitch", std::to_string(CAMERA->pitch));
		}
		std::string							_path;
		std::string							_id;
		std::map<std::string, std::string>	_worldInfos;
};

#endif
