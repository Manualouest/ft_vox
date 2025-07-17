/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TitleScreen.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 10:39:14 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/17 17:54:01 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"

void	closeWindow(void*);

#include "SceneManager.hpp"

extern SceneManager		*SCENE_MANAGER;

void	resumeGame(void*);

#define TITLES_COUNT 7
#define TITLE_TIME 8

std::string	popupTitles[TITLES_COUNT] =
{
	"by mbatty and mbirou!",
	"outstanding project!",
	"ft_minecraft?",
	"42 angouleme",
	"also try cub3d!",
	"chicken jockey!",
	"scraeyme approved"
};

//A FAIRE: 1/42 d'avoir ft_xov

static void	_buildInterface(Scene *scene)
{
	Interface	*main = scene->getInterfaceManager()->load("main");

	main->addElement("button_singleplayer", new Button(UIAnchor::UI_CENTER, "singleplayer", glm::vec2(0, -90), glm::vec2(300, 80), []
		(void*)
		{
			SCENE_MANAGER->use("game_scene");
			resumeGame(NULL);
		}, NULL));
	main->addElement("button_options", new Button(UIAnchor::UI_CENTER, "options", glm::vec2(0, 0), glm::vec2(300, 80), []
		(void*)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("options");
		}, NULL));
	main->addElement("button_quit_game", new Button(UIAnchor::UI_CENTER, "quit game", glm::vec2(0, 90), glm::vec2(300, 80), closeWindow, NULL));

	main->addElement("text_popup", new Text(UIAnchor::UI_TOP_CENTER_HALF, "by mbatty and mbirou!", glm::vec2(175, -40), NULL, false));
	main->addElement("image_icon", new Image(UIAnchor::UI_TOP_CENTER_HALF, glm::vec2(0, 0), glm::vec2(400, 150)));

	main->setUpdateFunc([]
		(Interface *interface)
		{
			static double	lastUpdate = 0;
			UIElement	*elem = interface->getElement("text_popup");
			Text		*text_popup = static_cast<Text*>(elem);

			if (glfwGetTime() - lastUpdate >= TITLE_TIME)
			{
				text_popup->setText(popupTitles[rand() % TITLES_COUNT]);
				lastUpdate = glfwGetTime();
			}

			float	scale = 1 + std::abs(cos(glfwGetTime() * 5)) / 10;

			text_popup->setScale(glm::vec2(scale, scale));
			text_popup->setRotation(glm::vec3(0.0, 0.0, 1.0));
			text_popup->setAngle(-10);
		});

	Interface	*options = scene->getInterfaceManager()->load("options");

	options->addElement("button_leave", new Button(UIAnchor::UI_BOTTOM_CENTER, "leave", glm::vec2(0, -10), glm::vec2(300, 80), [](void*)
	{
		SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("main");
	}, NULL));

	options->addElement("fun_text", new Text(UIAnchor::UI_CENTER, "why are you here? ... there are no options...", glm::vec2(0, 0), NULL, false));

	options->setUpdateFunc([]
		(Interface *interface)
		{
			Text		*text_popup = static_cast<Text*>(interface->getElement("fun_text"));

			text_popup->setRotation(glm::vec3(0.0, 0.0, 1.0));
			text_popup->setAngle(cos(glfwGetTime() * 25));
		});
}

static void	_frameKeyHook(Scene *scene)
{
	(void)scene;
}

#include "ShaderManager.hpp"
extern ShaderManager	*SHADER_MANAGER;

static void	_updateShaders(ShaderManager *shaders)
{
	Shader	*textShader = shaders->get("text");
	Shader	*postShader = shaders->get("post");

	textShader->use();
	textShader->setFloat("time", glfwGetTime());
	textShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	textShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);

	postShader->use();
	postShader->setFloat("RENDER_DISTANCE", RENDER_DISTANCE);
	postShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	postShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);
}

#include "TitleScreen.hpp"

void	TitleScreen::build(Scene *scene)
{
	_buildInterface(scene);
	scene->getCamera()->pos = glm::vec3(0, 0, 0);
	scene->getCamera()->pitch = 0;
	scene->getCamera()->yaw = 0;
	scene->getInterfaceManager()->use("main");
}

#include "FrameBuffer.hpp"
#include "Skybox.hpp"

extern Skybox	*SKYBOX;

void	TitleScreen::render(Scene *scene)
{
    glDisable(GL_DEPTH_TEST);
	FrameBuffer::drawFrame(SHADER_MANAGER->get("title_bg"), TEXTURE_MANAGER->get("textures/dirt.bmp")->getID());
	scene->getInterfaceManager()->draw();
    glEnable(GL_DEPTH_TEST);
}

void	TitleScreen::update(Scene *scene)
{
	_frameKeyHook(scene);
	scene->getCamera()->update();
	scene->getInterfaceManager()->update();
	_updateShaders(SHADER_MANAGER);
}
