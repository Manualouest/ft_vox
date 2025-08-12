/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Font.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/21 14:48:45 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/29 18:21:09 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Font.hpp"

#include "TextureManager.hpp"

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
    this->_atlas = TEXTURE_MANAGER->get(ASCII_FONT_TEXTURE_PATH);
}

void	Font::putString(std::string str, glm::vec2 pos, glm::vec2 scale, glm::vec3 color, bool background, bool shadow)
{
    putString(str, pos, scale, color, glm::vec3(1), 0, background, shadow);
}

#define FONT_SIZE 16.0

void	Font::putString(std::string str, glm::vec2 pos, glm::vec2 scale, glm::vec3 color, glm::vec3 rotation, float angle, bool background, bool shadow)
{
    float   charSizeX = FONT_SIZE * scale.x;
    float   charSizeY = FONT_SIZE * scale.y;
    float   strSizeX = charSizeX * (float)str.size() - (charSizeX / 2);
    float   strSizeY = charSizeY;

    glm::vec3   center = glm::vec3(strSizeX / 2.0f, strSizeY / 2.0f, 1.0f);

    glm::mat4   model(1);

    model = glm::translate(model, glm::vec3(pos.x, pos.y, 1.0f));

    model = glm::translate(model, center);
    model = rotate(model, glm::radians(angle), rotation);
    model = glm::translate(model, center * -1.0f);

    model = glm::scale(model, glm::vec3(charSizeX, charSizeY, 1.0f));

    glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

    uint    count = 0;

    for (std::string::iterator it = str.begin(); it != str.end(); it++)
    {
        initFontModel();
        _shader->use();
        _atlas->use(0);

        _shader->setMat4("projection", projection);
        _shader->setInt("charIndex", (int)*it);

        glm::mat4 charModel = model;

        charModel = translate(charModel, glm::vec3(count++, 0, 0));

        if (shadow)
        {
            glm::mat4   shadowModel = glm::translate(charModel, glm::vec3(0.125, 0.125, 0.0));
            SHADER_MANAGER->get("text")->setBool("drawBackground", false);
            _shader->setVec3("color", glm::vec3(0));
            _shader->setMat4("model", shadowModel);
            glBindVertexArray(fontVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        if (background)
            SHADER_MANAGER->get("text")->setBool("drawBackground", true);
        else
            SHADER_MANAGER->get("text")->setBool("drawBackground", false);

        _shader->setVec3("color", color);
        _shader->setMat4("model", charModel);
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
        0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        0.0f, 1.0f,  0.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &fontVAO);
    glGenBuffers(1, &fontVBO);

    glBindVertexArray(fontVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}
