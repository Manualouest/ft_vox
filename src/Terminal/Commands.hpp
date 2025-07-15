/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 09:51:43 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 11:41:39 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "libs.hpp"

class	Commands
{
	public:
		Commands();
		~Commands() {}
	
		std::string	execute(std::string args)
		{
			if (args[0] != '/')
				return (args);

			std::istringstream	iss(args);
			std::string			command;
			if (!(iss >> command))
				return ("error parsing command: empty");

			if (command == "/help")
				return (this->help());

			std::map<std::string, std::function<std::string(std::istringstream&)>>::iterator	finder = _commands.find(command);
			if (finder != _commands.end())
				return (finder->second(iss));
			return ("command not found");
		}
		std::string	help()
		{
			std::string	res;
			for (auto &command : _commands)
			{
				res += command.first;
				res += ", ";
			}
			return (res);
		}
	private:
		std::map<std::string, std::function<std::string(std::istringstream&)>>	_commands;
};

#endif
