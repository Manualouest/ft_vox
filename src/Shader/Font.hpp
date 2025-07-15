/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:45:03 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 11:37:04 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FONT_HPP
# define FONT_HPP

# include "libs.hpp"
# include "Shader.hpp"
# include "Texture.hpp"
# include "ShaderManager.hpp"

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
		void    putChar(char c, glm::vec2 pos, glm::vec2 size);
		void	putString(std::string str, glm::vec2 pos, glm::vec2 size);

	private:
		std::array<Texture, 128>	font;
		Shader						*_shader;
};

extern Font	*FONT;

#endif
