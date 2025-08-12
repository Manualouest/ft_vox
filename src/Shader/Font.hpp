/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:45:03 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/22 12:48:53 by mbatty           ###   ########.fr       */
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
		void	initFontModel();
		void	putString(std::string str, glm::vec2 pos, glm::vec2 size, glm::vec3 color, bool background, bool shadow);
		void	putString(std::string str, glm::vec2 pos, glm::vec2 scale, glm::vec3 color, glm::vec3 rotation, float angle, bool background, bool shadow);
		void	putChar(char c, glm::vec2 offset, glm::mat4 model);

	private:
		Shader						*_shader;
		Texture						*_atlas;
};

extern Font	*FONT;

#endif
