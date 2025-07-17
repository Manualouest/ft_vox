/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TitleScreen.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 10:57:06 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/17 11:00:29 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TITLESCREEN_HPP
# define TITLESCREEN_HPP

# include "Scene.hpp"

namespace	TitleScreen
{
	void	build(Scene *scene);
	void	destructor(Scene *scene);
	void	render(Scene *scene);
	void	update(Scene *scene);
	void	close(Scene *scene);
	void	open(Scene *scene);
};

#endif
