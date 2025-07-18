/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TitleScreen.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 10:39:14 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/18 16:11:13 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"
#include "TitleScreen.hpp"
#include "SceneManager.hpp"
#include "ShaderManager.hpp"
#include "FrameBuffer.hpp"
#include "Skybox.hpp"
#include "WorldManager.hpp"

extern WorldManager	*WORLD_MANAGER;

extern Skybox	*SKYBOX;
extern ShaderManager	*SHADER_MANAGER;
void	closeWindow(void*);
extern SceneManager		*SCENE_MANAGER;

void	resumeGame(void*);

//A FAIRE: 1/42 d'avoir ft_xov

#define TITLES_COUNT 9
#define TITLE_TIME 8

std::string	popupTitles[TITLES_COUNT] =
{
	"by mbatty and mbirou!",
	"outstanding project!",
	"ft_minecraft?",
	"42 angouleme",
	"also try cub3d!",
	"chicken jockey!",
	"scraeyme approved",
	"dont forget to git push"
};

extern uint	seed;

void	startGame(void *)
{
	SCENE_MANAGER->swap("game_scene");
	SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("leaving");
}



void	startWorld1(void *)
{
	World	*world = WORLD_MANAGER->get("world_1");
	if (!world)
		world = WORLD_MANAGER->load("world_1", rand());
	seed = world->getSeed();
	startGame(NULL);
}

void	startWorld2(void *)
{
	World	*world = WORLD_MANAGER->get("world_2");
	if (!world)
		world = WORLD_MANAGER->load("world_2", rand());
	seed = world->getSeed();
	startGame(NULL);
}

void	startWorld3(void *)
{
	World	*world = WORLD_MANAGER->get("world_3");
	if (!world)
		world = WORLD_MANAGER->load("world_3", rand());
	seed = world->getSeed();
	startGame(NULL);
}



static void	_buildInterface(Scene *scene)
{
	InterfaceManager	*manager = scene->getInterfaceManager();

	Interface	*main = manager->load("main");

	main->addElement("button_singleplayer", new Button(UIAnchor::UI_CENTER, "singleplayer", glm::vec2(0, -90), glm::vec2(300, 80), []
		(void*)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_selection");
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

	
		
	Interface	*worldSelection = manager->load("world_selection");

	worldSelection->addElement("select_text", new Text(UIAnchor::UI_TOP_CENTER_HALF, "select world", glm::vec2(0, -50), NULL, false));

	worldSelection->addElement("button_cancel", new Button(UIAnchor::UI_BOTTOM_CENTER, "cancel", glm::vec2(0, -10), glm::vec2(300, 80), [](void*)
	{
		SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("main");
	}, NULL));

	worldSelection->addElement("button_world1", new Button(UIAnchor::UI_CENTER, "- empty -", glm::vec2(0, -90), glm::vec2(300, 80), startWorld1, NULL));
	worldSelection->addElement("button_world2", new Button(UIAnchor::UI_CENTER, "- empty -", glm::vec2(0, 0), glm::vec2(300, 80), startWorld2, NULL));
	worldSelection->addElement("button_world3", new Button(UIAnchor::UI_CENTER, "- empty -", glm::vec2(0, 90), glm::vec2(300, 80), startWorld3, NULL));

	worldSelection->setUpdateFunc([]
		(Interface *)
		{
			World	*world1 = WORLD_MANAGER->get("world_1");
			World	*world2 = WORLD_MANAGER->get("world_2");
			World	*world3 = WORLD_MANAGER->get("world_3");

			if (world1)
				static_cast<Button*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection")->getElement("button_world1"))->label = "world_1";
			if (world2)
				static_cast<Button*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection")->getElement("button_world2"))->label = "world_2";
			if (world3)
				static_cast<Button*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection")->getElement("button_world3"))->label = "world_3";
		});


		
	Interface	*options = manager->load("options");

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

	Interface	*leaving = manager->load("leaving");

	leaving->addElement("leaving", new Text(UIAnchor::UI_CENTER, "joining world...", glm::vec2(0, 0), NULL, false));
}

static void	_frameKeyHook(Scene *scene)
{
	(void)scene;
}

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

void	TitleScreen::build(Scene *scene)
{
	_buildInterface(scene);
	scene->getCamera()->pos = glm::vec3(0, 0, 0);
	scene->getCamera()->pitch = 0;
	scene->getCamera()->yaw = 0;
	scene->getInterfaceManager()->use("main");
}

void	TitleScreen::destructor(Scene *scene)
{
	(void)scene;
}

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

void	TitleScreen::close(Scene *scene)
{
	(void)scene;
}

void	TitleScreen::open(Scene *scene)
{
	(void)scene;
	scene->getInterfaceManager()->use("main");
}
