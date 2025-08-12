/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   InterfaceManager.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 19:19:36 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 22:24:21 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INTERFACEMANAGER_HPP
# define INTERFACEMANAGER_HPP

# include "Interface.hpp"

class	InterfaceManager
{
	public:
		InterfaceManager()
		{

		}
		~InterfaceManager()
		{
			for (auto &pair : _interfaces)
				delete pair.second;
		}
		void	use(const std::string &name)
		{
			_current = get(name);
		}
		void	update()
		{
			if (_current)
				_current->update();
		}
		void	draw()
		{
			if (_current)
				_current->draw();
		}
		void	reset()
		{
			_current = NULL;
		}
		bool	erase(const std::string &name)
		{
			std::unordered_map<std::string, Interface *>::iterator	finder;
			finder = _interfaces.find(name);
			if (finder == _interfaces.end())
			{
				consoleLog("WARNING Tried to unload an interface thats not loaded: " + name, LogSeverity::WARNING);
				return (0);
			}
			_interfaces.erase(finder);
			return (1);
		}
		Interface	*load(const std::string &name)
		{
			if (_interfaces.find(name) != _interfaces.end())
			{
				consoleLog("WARNING Tried to load an interface thats already loaded (will be using the existing interface): " + name, LogSeverity::WARNING);
				return (this->get(name));
			}
			return (_interfaces.insert(std::make_pair(name, new Interface())).first->second);
		}
		Interface	*get(const std::string &name)
		{
			std::unordered_map<std::string, Interface *>::iterator	finder = _interfaces.find(name);
			if (finder == _interfaces.end())
			{
				consoleLog("ERROR Tried to access an interface thats not loaded, might cause a crash: " + name, LogSeverity::ERROR);
				return (NULL);
			}
			return (finder->second);
		}
		Interface	*operator[](const std::string &name)
		{
			return (this->get(name));
		}
		Interface	*getCurrent() {return (this->_current);}
	private:
		std::unordered_map<std::string, Interface *>	_interfaces;
		Interface							*_current = NULL;
};

#endif
