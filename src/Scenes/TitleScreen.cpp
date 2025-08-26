/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TitleScreen.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 10:39:14 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/25 20:04:16 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Scene.hpp"
#include "TitleScreen.hpp"
#include "SceneManager.hpp"
#include "ShaderManager.hpp"
#include "FrameBuffer.hpp"
#include "Skybox.hpp"
#include "WorldManager.hpp"

//SETTINGS
extern uint	settingRenderDistance;

extern WorldManager	*WORLD_MANAGER;

extern Skybox	*SKYBOX;
extern ShaderManager	*SHADER_MANAGER;
void	closeWindow(ButtonInfo);
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
	"dont forget to git push",
	"c++ > all"
};

extern uint	seed;

void	startGame()
{
	SCENE_MANAGER->swap("game_scene");
	SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("leaving");
}

void	startGame(ButtonInfo)
{
	startGame();
}

std::string	selectedWorld;

void	selectWorld(ButtonInfo infos)
{
	selectedWorld = infos.id;
	SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_creation");
}

static void	_buildWorldSelection(Interface *interface);

std::string	currentWorldID;

void	startWorldButton(ButtonInfo infos)
{
	Toggle	*deleteToggle = static_cast<Toggle*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection")->getElement("toggle_delete"));

	if (deleteToggle->pressed)
	{
		WORLD_MANAGER->clear(infos.id);
		std::filesystem::path	path = "./saves/" + infos.id;
		std::error_code	ec;
		std::filesystem::remove_all(path, ec); //!!!!!! Check the error code !!!!!!!!!
		_buildWorldSelection(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection"));
		return ;
	}

	currentWorldID = infos.id;

	startGame();
}

static void	_buildWorldSelection(Interface *interface)
{
	interface->clear();

	interface->addElement("select_text", new Text(UIAnchor::UI_TOP_CENTER, "select world", glm::vec2(0, 10), NULL, false));

	interface->addElement("button_cancel", new Button(UIAnchor::UI_BOTTOM_CENTER, "cancel", glm::vec2(210, -10), glm::vec2(200, 60), []
		(ButtonInfo)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("main");
		}, NULL));

	interface->addElement("toggle_delete", new Toggle(UIAnchor::UI_BOTTOM_CENTER, "delete", glm::vec2(0, -10), glm::vec2(200, 60), NULL, NULL));
	interface->addElement("button_new", new Button(UIAnchor::UI_BOTTOM_CENTER, "new", glm::vec2(-210, -10), glm::vec2(200, 60), []
		(ButtonInfo)
		{
			static_cast<TextBox*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_creation")->getElement("textbox_world_name"))->clear();
			static_cast<TextBox*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_creation")->getElement("textbox_world_seed"))->clear();
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_creation");
		}, NULL));

	if (WORLD_MANAGER->data().size() <= 0)
	{
		interface->addElement("no_world_text", new Text(UIAnchor::UI_CENTER, "no worlds... go create one!", glm::vec2(0, 0), NULL, false));
		return ;
	}

	int	offset = -90 * (WORLD_MANAGER->data().size() - 1) / 2;

	for (auto &pair : WORLD_MANAGER->data())
	{
		std::string	id = pair.second->getID();
		interface->addElement(id, new Button(UIAnchor::UI_CENTER, id, glm::vec2(0, offset), glm::vec2(300, 80), startWorldButton, NULL));
		offset += 90;
	}

	interface->setUpdateFunc([]
		(Interface *)
		{
			for (auto &pair : WORLD_MANAGER->data())
			{
				std::string	id = pair.second->getID();
				World	*world = WORLD_MANAGER->get(id);

				if (world)
					static_cast<Button*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection")->getElement(id))->label = world->getWorldInfo("display_name");
			}

		});
}

#include "RegionManager.hpp"

static void	_createNewWorld(std::string name, uint seed)
{
	if (name.size() <= 0)
		return ;

	WORLD_MANAGER->load(name, seed);
	WORLD_MANAGER->get(name)->saveWorldInfo("pos_x", std::to_string(WORLD_SIZE / 2));
	WORLD_MANAGER->get(name)->saveWorldInfo("pos_y", std::to_string(100));
	WORLD_MANAGER->get(name)->saveWorldInfo("pos_z", std::to_string(WORLD_SIZE / 2));
	_buildWorldSelection(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection"));
}

static void	_buildMainInterface(Interface *interface)
{
	interface->addElement("button_singleplayer", new Button(UIAnchor::UI_CENTER, "singleplayer", glm::vec2(0, -90), glm::vec2(300, 80), []
		(ButtonInfo)
		{
			WORLD_MANAGER->reload();
			_buildWorldSelection(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_selection"));
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_selection");
		}, NULL));

	interface->addElement("button_options", new Button(UIAnchor::UI_CENTER, "options", glm::vec2(0, 0), glm::vec2(300, 80), []
		(ButtonInfo)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("options");
		}, NULL));

	interface->addElement("button_quit_game", new Button(UIAnchor::UI_CENTER, "quit game", glm::vec2(0, 90), glm::vec2(300, 80), closeWindow, NULL));

	Text	*text_popup = static_cast<Text*>(interface->addElement("text_popup", new Text(UIAnchor::UI_TOP_CENTER_HALF, "by mbatty and mbirou!", glm::vec2(175, -40), NULL, false)));
	interface->addElement("image_icon", new Image(UIAnchor::UI_TOP_CENTER_HALF, glm::vec2(0, 0), glm::vec2(400, 150)));

	text_popup->setRotation(glm::vec3(0.0, 0.0, 1.0));
	text_popup->setAngle(-10);
	text_popup->setColor(glm::vec3(1.0, 1.0, 0.0));

	interface->setUpdateFunc([]
		(Interface *interface)
		{
			static double	lastUpdate = 0;
			Text		*text_popup = static_cast<Text*>(interface->getElement("text_popup"));

			if (glfwGetTime() - lastUpdate >= TITLE_TIME)
			{
				text_popup->setText(popupTitles[rand() % TITLES_COUNT]);
				lastUpdate = glfwGetTime();
			}

			float	scale = 1 + std::abs(cos(glfwGetTime() * 5)) / 10;

			text_popup->setScale(glm::vec2(scale, scale));
		});
}

extern bool	crosshair;

static void	_buildOptionsInterface(Interface *interface)
{
	interface->addElement("button_leave", new Button(UIAnchor::UI_BOTTOM_CENTER, "leave", glm::vec2(0, -10), glm::vec2(200, 60), []
		(ButtonInfo)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("main");
		}, NULL));

	interface->addElement("slider_render_distance", new Slider(UIAnchor::UI_CENTER, "render distance", glm::vec2(0, 0), glm::vec2(300, 80),
		[](float val)
		{
			int	newRenderDistance = glm::clamp((int)(val * 32.f), 2, 32);
			RENDER_DISTANCE = newRenderDistance * 32 * 2;
			settingRenderDistance = newRenderDistance;
		}, [](Slider *slider) {slider->setLabel("render distance " + std::to_string(settingRenderDistance));}, 0.55));

	interface->addElement("slider_fov", new Slider(UIAnchor::UI_CENTER, "fov", glm::vec2(0, -90), glm::vec2(300, 80),
		[](float val)
		{
			FOV = glm::clamp((int)(val * 120), 2, 120);
		}, [](Slider *slider) {slider->setLabel("fov " + std::to_string((int)FOV));}, 80.f / 120.f));

	interface->addElement("button_crosshair", new Button(UIAnchor::UI_CENTER, "crosshair: off", glm::vec2(-310, 0), glm::vec2(300, 80), []
		(ButtonInfo info)
		{
			if (!crosshair)
			{
				crosshair = true;
				info.button->label = "crosshair: on";
			}
			else
			{
				crosshair = false;
				info.button->label = "crosshair: off";
			}
		}, NULL));

	interface->setUpdateFunc([]
		(Interface *interface)
		{
			(void)interface;
			Slider		*render_distance_slider = static_cast<Slider*>(interface->getElement("slider_render_distance"));
			Slider		*fov_slider = static_cast<Slider*>(interface->getElement("slider_fov"));
			Button		*button_crosshair = static_cast<Button*>(interface->getElement("button_crosshair"));

			render_distance_slider->setLabel("render distance " + std::to_string(settingRenderDistance));
			render_distance_slider->setSlider((float)settingRenderDistance / 32.f);
			
			fov_slider->setLabel("fov " + std::to_string(FOV));
			fov_slider->setSlider((float)FOV / 120.f);

			if (crosshair)
				button_crosshair->label = "crosshair: on";
			else
				button_crosshair->label = "crosshair: off";
		});
}

static void	_buildWorldCreationInterface(Interface *interface)
{
	interface->addElement("text_world_name", new Text(UIAnchor::UI_CENTER, "world name", glm::vec2(0, -170), NULL, false));
	interface->addElement("textbox_world_name", new TextBox(UIAnchor::UI_CENTER, glm::vec2(0, -120), glm::vec2(200, 60), NULL, NULL));

	interface->addElement("text_world_seed", new Text(UIAnchor::UI_CENTER, "world seed", glm::vec2(0, -50), NULL, false));
	interface->addElement("textbox_world_seed", new TextBox(UIAnchor::UI_CENTER, glm::vec2(0, 0), glm::vec2(200, 60), NULL, NULL));

	interface->addElement("button_cancel", new Button(UIAnchor::UI_BOTTOM_CENTER, "cancel", glm::vec2(110, -10), glm::vec2(200, 60), []
		(ButtonInfo)
		{
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_selection");
		}, NULL));

	interface->addElement("create_world_button", new Button(UIAnchor::UI_BOTTOM_CENTER, "create new world", glm::vec2(-110, -10), glm::vec2(200, 60), []
		(ButtonInfo)
		{
			TextBox	*worldNameTextBox = static_cast<TextBox*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_creation")->getElement("textbox_world_name"));
			TextBox	*worldSeedTextBox = static_cast<TextBox*>(SCENE_MANAGER->get("title_scene")->getInterfaceManager()->get("world_creation")->getElement("textbox_world_seed"));

			std::string	name = worldNameTextBox->getInput();
			std::string	seedString = worldSeedTextBox->getInput();

			if (!name.size() || WORLD_MANAGER->get(name))
				return ;

			uint		seed = 0;
			if (!seedString.size())
				seed = rand();
			else
			{
				try
				{
					seed = std::stoul(seedString.c_str(), NULL, 10);
				} catch (...) {}
			}

			_createNewWorld(name, seed);
			SCENE_MANAGER->get("title_scene")->getInterfaceManager()->use("world_selection");
		}, NULL));
}

static void	_buildInterface(Scene *scene)
{
	InterfaceManager	*manager = scene->getInterfaceManager();

	Interface	*main = manager->load("main");
	_buildMainInterface(main);

	Interface	*worldSelection = manager->load("world_selection");
	_buildWorldSelection(worldSelection);

	Interface	*options = manager->load("options");
	_buildOptionsInterface(options);

	Interface	*worldCreation = manager->load("world_creation");
	_buildWorldCreationInterface(worldCreation);

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

	textShader->use();
	textShader->setFloat("time", glfwGetTime());
	textShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	textShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);
}

static void	_keyHookFunc(Scene *scene, int key, int action)
{
	scene->getInterfaceManager()->getCurrent()->specialInput(key, action);
}

static void	_charHookFunc(Scene *scene, uint key)
{
	scene->getInterfaceManager()->getCurrent()->charInput(key);
}

void	TitleScreen::build(Scene *scene)
{
	_buildInterface(scene);
	scene->getCamera()->pos = glm::vec3(0, 0, 0);
	scene->getCamera()->pitch = 0;
	scene->getCamera()->yaw = 0;
	scene->setKeyHook(_keyHookFunc);
	scene->setCharHook(_charHookFunc);
	scene->getInterfaceManager()->use("main");
}

void	TitleScreen::destructor(Scene *scene)
{
	(void)scene;
}

void	TitleScreen::render(Scene *scene)
{
    glDisable(GL_DEPTH_TEST);
	FrameBuffer::drawFrame(SHADER_MANAGER->get("title_bg"), TEXTURE_MANAGER->get(DIRT_TEXTURE_PATH)->getID());
	scene->getInterfaceManager()->draw();
    glEnable(GL_DEPTH_TEST);
}

void	TitleScreen::update(Scene *scene)
{
	_frameKeyHook(scene);
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
