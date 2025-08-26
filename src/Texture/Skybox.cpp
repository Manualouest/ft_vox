/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Skybox.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbirou <mbirou@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 15:22:58 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/26 20:44:35 by mbirou           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Skybox.hpp"

#include "Window.hpp"
extern Window	*WINDOW;

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
    _sunPos = glm::vec3(std::cos((_time / 1000.0f) * (M_PI * 2.0f)) * 7, std::cos((_time / 1000.0f) * (M_PI * 2.0f)) * 12, std::sin((_time / 1000.0f) * (M_PI * 2.0f)) * 12);
	sunModel = glm::translate(sunModel, _sunPos);
    sunModel = glm::rotate(sunModel, (float)std::atan2(std::sin((_time / 1000.0f) * (M_PI * 2.0f)), std::cos((_time / 1000.0f) * (M_PI * 2.0f))) + (float)(M_PI / 2.0f), glm::vec3(1, 0, 0));

    sunShader->use();
    camera.setViewMatrix(*sunShader);
    sunShader->setMat4("model", sunModel);
    sunShader->setMat4("view", view);

	Texture::use("tex", TEXTURE_MANAGER->get("assets/textures/skybox/sun.bmp")->getID(), 1, sunShader);
    glBindVertexArray(UIElement::getQuadVAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

    glm::mat4   moonModel = glm::mat4(1);
	_moonPos = glm::vec3(-std::cos((_time / 1000.0f) * (M_PI * 2.0f)) * 7, -std::cos((_time / 1000.0f) * (M_PI * 2.0f)) * 10, -std::sin((_time / 1000.0f) * (M_PI * 2.0f)) * 10);
    moonModel = glm::translate(moonModel, _moonPos);
    moonModel = glm::rotate(moonModel, (float)std::atan2(std::sin((_time / 1000.0f) * (M_PI * 2.0f)), std::cos((_time / 1000.0f) * (M_PI * 2.0f))) - (float)(M_PI / 2.0f), glm::vec3(1, 0, 0));
    sunShader->use();
    sunShader->setMat4("model", moonModel);

    Texture::use("tex", TEXTURE_MANAGER->get("assets/textures/skybox/moon.bmp")->getID(), 1, sunShader);
    glBindVertexArray(UIElement::getQuadVAO());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	_time += WINDOW->getDeltaTime() * 8.3333333335;
	if (_time > 1000)
		_time -= 1000;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void	Skybox::setTime(const float &newTime)
{
	_time = newTime;
}

float	Skybox::getTime() const
{
	return (_time);
}


glm::vec3	Skybox::getDirectLightPos(const bool &getSun) const
{
	if (getSun)
		return (-_sunPos);
	return (-_moonPos);
}

glm::vec3	Skybox::getDirectLightColor(const bool &getSun) const
{
	if (getSun)
	{
		float		height = std::cos((_time / 1000.0f) * (M_PI * 2.0f));
		float		heightAbs = std::abs(height);
		glm::vec3	color = glm::mix(SUNSET, DAY, heightAbs);
		return (glm::mix(color / 5.0f, color, std::clamp(height + 0.2f, 0.0f, 1.0f)));
	}
	float	height = -std::cos((_time / 1000.0f) * (M_PI * 2.0f));
	return (glm::mix(NIGHT / 5.0f, NIGHT, std::clamp(height + 0.05f, 0.0f, 1.0f)));
}

glm::vec3	Skybox::getAmbientLight() const
{
	float	height = std::abs(std::cos((_time / 1000.0f) * (M_PI * 2.0f)));
	glm::vec3	color = glm::mix(NIGHT / 2.0f, glm::vec3(0.2f, 0.2f, 0.2f), height);
	return (color);
}

glm::vec3	Skybox::getSkyboxColorUp() const
{
	float		height = std::cos((_time / 1000.0f) * (M_PI * 2.0f));
	glm::vec3	color = glm::mix(SKYSUNSETUP, SKYDAYUP, std::clamp(height * 2.0f, 0.0f, 1.0f));
	if (height < 0)
		color = glm::mix(SKYSUNSETUP, SKYNIGHTUP, std::abs(std::clamp(std::abs(height) * 2.0f, 0.0f, 1.0f)));
	return (color);
}

glm::vec3	Skybox::getSkyboxColorDown() const
{
	float		height = std::cos((_time / 1000.0f) * (M_PI * 2.0f));
	glm::vec3	color = glm::mix(SKYSUNSETDOWN, SKYDAYDOWN, std::clamp(height * 2.0f, 0.0f, 1.0f));
	if (height < 0)
		color = glm::mix(SKYSUNSETDOWN, SKYNIGHTDOWN, std::clamp(std::abs(height) * 2.0f, 0.0f, 1.0f));
	return (color);
}