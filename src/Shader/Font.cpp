/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:48:45 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/17 15:00:43 by mbatty           ###   ########.fr       */
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
    this->_shader = SHADER_MANAGER->get("text");
	std::ifstream	tmpFile;
	for (unsigned char c = 32; c < 128; ++c)
	{
		std::string path = "src/assets/textures/font/" + std::to_string((int)c) + ".bmp";
		tmpFile.open(path);
		if (tmpFile.is_open())
		{
			Texture	tmp(path.c_str());
			font[c].cut(tmp);
		}
		tmpFile.close();
	}
	Texture	tmp("src/assets/textures/font/63.bmp");
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

void    Font::putChar(char c, glm::vec2 pos, glm::vec2 size)
{
    initFontModel();
    _shader->use();
    getChar(c).use(0);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
    
    _shader->setMat4("projection", projection);
    _shader->setMat4("model", model);

    glBindVertexArray(fontVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void	Font::putString(std::string str, glm::vec2 pos, glm::vec2 size)
{
	float	offset = size.x / str.size();
	float	charPosX = pos.x;
	float	charPosY = pos.y;
	for (std::string::iterator it = str.begin(); it != str.end(); it++)
	{
		putChar(*it, glm::vec2(charPosX, charPosY), glm::vec2(offset, size.y));
		charPosX += offset;
	}
}

void	Font::putString(std::string str, glm::vec2 pos, glm::vec2 scale, glm::vec3 rotation, float angle)
{
    float   fontSizeX = 16.0f * scale.x;
    float   fontSizeY = 16.0f * scale.y;

    glm::vec3   center = glm::vec3((float)str.size() * fontSizeX / 2.0f, fontSizeY / 2.0f, 1.0); //Rotate around center of string

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 1.0f));

    model = glm::translate(model, center);
    model = rotate(model, glm::radians(angle), rotation);
    model = glm::translate(model, center * -1.0f);

    model = glm::scale(model, glm::vec3(fontSizeX, fontSizeY, 1.0f));
    glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

    int charit = 0;
    for (std::string::iterator it = str.begin(); it != str.end(); it++)
	{
        initFontModel();
        _shader->use();
        getChar(*it).use(0);

        glm::mat4   model2 = model;

        model2 = glm::translate(model2, glm::vec3(charit++, 0.0, 0.0));

        _shader->setMat4("projection", projection);
        _shader->setMat4("model", model2);
        
        glBindVertexArray(fontVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
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
