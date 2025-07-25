/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SceneManager.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 11:05:13 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:33:47 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCENEMANAGER_HPP
# define SCENEMANAGER_HPP

# include "Scene.hpp"

class	SceneManager
{
	public:
		SceneManager()
		{

		}
		~SceneManager()
		{
			for (auto &pair : _scenes)
				delete pair.second;
		}
		/*
			Swap is like use() but instead of switching scene instantly it switches on the next frame.
		*/
		void	swap(const std::string &name)
		{
			_swap = get(name);
		}
		void	use(const std::string &name)
		{
			if (_current)
				_current->close();

			_current = get(name);
			_current->use();
		}
		void	update()
		{
			if (_swap)
			{
				if (_current)
					_current->close();

				_current = _swap;
				_swap = NULL;
				_current->use();
			}
			if (_current)
				_current->update();
		}
		void	render()
		{
			if (_current)
				_current->render();
		}
		void	reset()
		{
			_current->close();
			_current = NULL;
		}
		bool	erase(const std::string &name)
		{
			std::unordered_map<std::string, Scene *>::iterator	finder;
			finder = _scenes.find(name);
			if (finder == _scenes.end())
			{
				consoleLog("WARNING Tried to unload a scene thats not loaded: " + name, LogSeverity::WARNING);
				return (0);
			}
			_scenes.erase(finder);
			return (1);
		}
		Scene	*load(std::string name, std::function<void(Scene *)> build, std::function<void(Scene *)> destructor, std::function<void(Scene*)> onRender, std::function<void(Scene*)> onUpdate)
		{
			if (_scenes.find(name) != _scenes.end())
			{
				consoleLog("WARNING Tried to load a scene thats already loaded (will be using the existing scene): " + name, LogSeverity::WARNING);
				return (this->get(name));
			}
			return (_scenes.insert(std::make_pair(name, new Scene(build, destructor, onRender, onUpdate))).first->second);
		}
		Scene	*get(const std::string &name)
		{
			std::unordered_map<std::string, Scene *>::iterator	finder = _scenes.find(name);
			if (finder == _scenes.end())
			{
				consoleLog("ERROR Tried to access a scene thats not loaded, might cause a crash: " + name, LogSeverity::ERROR);
				return (NULL);
			}
			return (finder->second);
		}
		Scene	*operator[](const std::string &name)
		{
			return (this->get(name));
		}
		Scene	*getCurrent() {return (this->_current);}
	private:
		std::unordered_map<std::string, Scene *>		_scenes;
		Scene								*_current = NULL;
		Scene								*_swap = NULL;
};

#endif
