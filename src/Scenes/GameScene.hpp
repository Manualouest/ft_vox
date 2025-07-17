/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameScene.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 11:12:43 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/17 11:12:58 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GAMESCENE_HPP
# define GAMESCENE_HPP

# include "Scene.hpp"

namespace	GameScene
{
	void	build(Scene *scene);
	void	render(Scene *scene);
	void	update(Scene *scene);
};

#endif
