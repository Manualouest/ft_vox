/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Skybox.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 15:22:58 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/15 18:57:37 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Skybox.hpp"

float skyboxVertices[] = {
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,

     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::~Skybox()
{
    if (DEBUG)
        consoleLog("Destroying skybox", NORMAL);
    glDeleteTextures(1, &ID);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

Skybox::Skybox(const std::vector<std::string> &faces)
{
    _shader = SHADER_MANAGER->get("skybox");
    model = glm::mat4(1);
    if (DEBUG)
	    consoleLog("Loading skybox", NORMAL);

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        Texture	tmp;
        tmp.LoadImage(faces[i].c_str());

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, tmp.getWidth(), tmp.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp.getData().data());
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

#include "UIElement.hpp"

void	Skybox::draw(Camera &camera)
{
    glDisable(GL_DEPTH_TEST);
    _shader->use();

    glm::mat4 view = camera.getViewMatrix();

    view[3] = glm::vec4(0, 0, 0, 1); //Remove translation

    camera.setViewMatrix(*_shader);
    _shader->setMat4("model", model);
    _shader->setMat4("view", view);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDisable(GL_CULL_FACE);

    Shader  *sunShader = SHADER_MANAGER->get("sun");

    glm::mat4   sunModel = glm::mat4(1);
    sunModel = glm::translate(sunModel, glm::vec3(-0.5, 0, 8));

    sunShader->use();
    camera.setViewMatrix(*sunShader);
    sunShader->setMat4("model", sunModel);
    sunShader->setMat4("view", view);

	Texture::use("tex", TEXTURE_MANAGER->get("assets/textures/skybox/sun.bmp")->getID(), 1, sunShader);
    glBindVertexArray(UIElement::getQuadVAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

    // sunModel = glm::mat4(1);
    // sunModel = glm::translate(sunModel, glm::vec3(-0.5, 0, -8));
    // sunShader->use();
    // sunShader->setMat4("model", sunModel);

    // Texture::use("tex", TEXTURE_MANAGER->get("assets/textures/skybox/moon.bmp")->getID(), 1, sunShader);
    // glBindVertexArray(UIElement::getQuadVAO());
	// glDrawArrays(GL_TRIANGLES, 0, 6);
	// glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
