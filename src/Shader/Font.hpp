/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:45:03 by mbatty            #+#    #+#             */
/*   Updated: 2025/06/12 23:44:01 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FONT_HPP
# define FONT_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Texture.hpp"

extern GLuint fontVAO;
extern GLuint fontVBO;
extern float	SCREEN_WIDTH;
extern float	SCREEN_HEIGHT;

class	Font
{
	public:
		~Font();
		Font();
		Texture	&operator[](char c);
		Texture	&getChar(char c);
		void	initFontModel();
		void    putChar(char c, Shader &shader, glm::vec2 pos, glm::vec2 size);
		void	putString(std::string str, Shader &shader, glm::vec2 pos, glm::vec2 size);

		std::array<Texture, 128> font;	
};

extern Font	*FONT;

#endif
