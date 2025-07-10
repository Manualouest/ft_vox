/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:48:45 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/09 15:05:05 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Font.hpp"

GLuint fontVAO = 0;
GLuint fontVBO = 0;

Font::~Font()
{
    if (fontVAO != 0)
    {
        if (DEBUG)
            consoleLog("Destroying font quad", LogSeverity::NORMAL);
        glDeleteBuffers(1, &fontVBO);
        glDeleteVertexArrays(1, &fontVAO);
        fontVBO = 0;
        fontVAO = 0;
    }
}

Font::Font()
{
	std::ifstream	tmpFile;
	for (unsigned char c = 33; c < 128; ++c)
	{
		std::string path = "src/assets/textures/font/" + std::string(1, c) + ".bmp";
		tmpFile.open(path);
		if (tmpFile.is_open())
		{
			Texture	tmp(path.c_str());
			font[c].cut(tmp);
		}
		tmpFile.close();
	}
	Texture	tmp("src/assets/textures/font/?.bmp");
	font[127].cut(tmp);
}

Texture &Font::operator[](char c)
{
	if (c < 0 || c > 127 || font[c].getID() <= 0)
		return (font[127]);
	return (font[c]);
}

Texture	&Font::getChar(char c)
{
	if (c < 0 || c > 127 || font[c].getID() <= 0)
		return (font[127]);
	return (font[c]);
}

void    Font::putChar(char c, Shader &shader, glm::vec2 pos, glm::vec2 size)
{
    if (c == ' ')
        return ;
    initFontModel();
    shader.use();
    getChar(c).use(0);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
    
    shader.setMat4("projection", projection);
    shader.setMat4("model", model);

    glBindVertexArray(fontVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void	Font::putString(std::string str, Shader &shader, glm::vec2 pos, glm::vec2 size)
{
	float	offset = size.x / str.size();
	float	charPos = pos.x;
	for (std::string::iterator it = str.begin(); it != str.end(); it++)
	{
		putChar(*it, shader, glm::vec2(charPos, pos.y), glm::vec2(offset, size.y));
		charPos += offset;
	}
}

void	Font::initFontModel()
{
    if (fontVAO != 0) return;
        
    if (DEBUG)
        consoleLog("Loading font quad", LogSeverity::NORMAL);

    float vertices[] = {
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    glGenVertexArrays(1, &fontVAO);
    glGenBuffers(1, &fontVBO);

    glBindVertexArray(fontVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}
