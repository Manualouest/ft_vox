/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameScene.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 11:13:19 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/13 14:05:27 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GameScene.hpp"
#include "RegionManager.hpp"
#include "ShaderManager.hpp"
#include "Skybox.hpp"
#include "FrameBuffer.hpp"
#include "Window.hpp"
#include "SceneManager.hpp"
#include "Terminal.hpp"
#include "ChunkGeneratorManager.hpp"
#include "WorldManager.hpp"
#include "TextureManager.hpp"

/*
Variables used by this scene only
*/
ChunkGeneratorManager			*CHUNK_GENERATOR;
RegionManager					*CHUNKS = NULL;
Terminal						*TERMINAL;

/*
Global variables for the whole program
*/
extern WorldManager	*WORLD_MANAGER;
extern SceneManager				*SCENE_MANAGER;
extern FrameBuffer				*MAIN_FRAME_BUFFER;
extern Skybox					*SKYBOX;
extern ShaderManager			*SHADER_MANAGER;
extern RegionManager			*CHUNKS;
extern bool						PAUSED;
extern bool						F3;
extern Window					*WINDOW;

static bool	leavingScene = false;
static bool	enteringWorld = false;

static bool crosshair = false;

struct	DebugTimes
{
	double	fullFrameTIme;
	double	renderTime;
	double	updateTime;
};

std::vector<DebugTimes>	frameTimes;

void	pauseGame()
{
	PAUSED = true;
	SCENE_MANAGER->get("game_scene")->getInterfaceManager()->use("pause");
	glfwSetInputMode(WINDOW->getWindowData(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void	pauseGameButton(ButtonInfo)
{
	pauseGame();
}

void	resumeGame()
{
	PAUSED = false;
	SCENE_MANAGER->get("game_scene")->getInterfaceManager()->reset();
	WINDOW->setDefaultMousePos();
}

void	resumeGameButton(ButtonInfo)
{
	resumeGame();
}

void	closeWindow(ButtonInfo);

static void	_keyHookFunc(Scene *, int key, int action)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && !TERMINAL->isActive())
	{
		if (PAUSED)
			resumeGame();
		else
			pauseGame();
	}
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
	{
		F3 = !F3;
		frameTimes.clear();
	}
	else
		TERMINAL->specialInput(key, action);
}

/*
	Update post shader separatly because I needed to at some point (Mainly for viewPos if you use multiple cameras at once)
*/
static void	_updatePostShader(ShaderManager *shaders)
{
	Shader	*postShader = shaders->get("post");

	postShader->use();
	postShader->setVec3("viewPos", CAMERA->pos);
	postShader->setFloat("time", glfwGetTime());
}

static void	_frameKeyHook(Scene *)
{
	if (PAUSED || TERMINAL->isActive())
		return ;

	float cameraSpeed = 15 * WINDOW->getDeltaTime();
	float	speedBoost = 1.0f;

	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		speedBoost = 20.0f;

	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		speedBoost *= 15.0f;

	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_2) == GLFW_PRESS)
		speedBoost = 20.f / 15.f;

	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_W) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_S) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_SPACE) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->worldUp * (cameraSpeed * speedBoost);
	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->worldUp * (cameraSpeed * speedBoost);

	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_A) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - glm::normalize(glm::cross(CAMERA->front, CAMERA->worldUp)) * (cameraSpeed * speedBoost);
	if (glfwGetKey(WINDOW->getWindowData(), GLFW_KEY_D) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + glm::normalize(glm::cross(CAMERA->front, CAMERA->worldUp)) * (cameraSpeed * speedBoost);
}

void	_moveMouseHookFunc(Scene*, double xpos, double ypos)
{
	if (PAUSED || enteringWorld || leavingScene)
		return ;

	float xoffset = xpos - WINDOW->getLastMouseX();
	float yoffset = WINDOW->getLastMouseY() - ypos;

	WINDOW->setLastMouseX(xpos);
	WINDOW->setLastMouseY(ypos);

	const float sensitivity = 0.1f;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	CAMERA->yaw += xoffset;
	CAMERA->pitch += yoffset;

	if (CAMERA->pitch > 89.0f)
		CAMERA->pitch = 89.0f;
	if (CAMERA->pitch < -89.0f)
		CAMERA->pitch = -89.0f;
	if (CAMERA->yaw > 360)
		CAMERA->yaw = 0;
	if (CAMERA->yaw < 0)
		CAMERA->yaw = 360;
}

void	_mouseBtnHookFunc(Scene*, int button, int action, int)
{
	if (PAUSED || enteringWorld || leavingScene)
		return ;

	if (CHUNKS && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glm::vec3	rayDir = CAMERA->front;
		glm::vec3	rayPos = CAMERA->pos;
		glm::ivec3	mapPos = CAMERA->pos;
		glm::vec3	deltaDist = glm::abs(glm::vec3(glm::length(rayDir)) / rayDir);
		glm::ivec3	rayStep = glm::ivec3(glm::sign(rayDir));
		glm::vec3	sideDist = (sign(rayDir) * (glm::vec3(mapPos) - rayPos) + (glm::sign(rayDir) * 0.5f) + 0.5f) * deltaDist;

		int	MAX_RAY_STEPS = 8;
		for (int i = 0; i < MAX_RAY_STEPS; ++i)
		{
			if (sideDist.x < sideDist.y) {
				if (sideDist.x < sideDist.z)
				{
					sideDist.x += deltaDist.x;
					mapPos.x += rayStep.x;
				}
				else
				{
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
				}
			}
			else
			{
				if (sideDist.y < sideDist.z)
				{
					sideDist.y += deltaDist.y;
					mapPos.y += rayStep.y;
				}
				else
				{
					sideDist.z += deltaDist.z;
					mapPos.z += rayStep.z;
				}
			}
			Chunk	*chunk = CHUNKS->getQuadTree()->getLeaf({mapPos.x, mapPos.z});
			if (chunk && chunk->removeBlock(mapPos))
				break;
		}
	}
}

static void	_updateShaders(ShaderManager *shaders)
{
	Shader	*textShader = shaders->get("text");
	Shader	*postShader = shaders->get("post");
	Shader	*voxelShader = shaders->get("voxel");
	Shader	*skyboxShader = shaders->get("skybox");

	textShader->use();
	textShader->setFloat("time", glfwGetTime());
	textShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	textShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);

	postShader->use();
	postShader->setFloat("RENDER_DISTANCE", RENDER_DISTANCE);
	postShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	postShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);

	voxelShader->use();
	voxelShader->setVec3("viewPos", CAMERA->pos);
	voxelShader->setFloat("RENDER_DISTANCE", RENDER_DISTANCE);
	voxelShader->setFloat("time", glfwGetTime());

	skyboxShader->use();
	skyboxShader->setFloat("time", glfwGetTime());
}

int		currentFPS = 60;

std::string	getFPSString(bool debug)
{
	double	currentTime = glfwGetTime();
	static double		lastUpdate = 0;
	static double		lastMinMaxUpdate = 0;
	static std::string	fpsString = "0 fps";

	static int	minFPS = INT_MAX;
	static int	maxFPS = 0;

	if (currentTime - lastMinMaxUpdate >= 10)
	{
		minFPS = INT_MAX;
		maxFPS = 0;
		lastMinMaxUpdate = currentTime;
	}

	currentFPS = (int)(1.0f / WINDOW->getDeltaTime());
	if (currentFPS > maxFPS)
		maxFPS = currentFPS;
	if (currentFPS < minFPS)
		minFPS = currentFPS;

	if (currentTime - lastUpdate >= 0.1)
	{
		fpsString = std::to_string(currentFPS) + " fps";
		if (debug)
		{
			fpsString += " |" + std::to_string(minFPS) + " min";
			fpsString += " |" + std::to_string(maxFPS) + " max";
		}
		lastUpdate = currentTime;
	}
	return (fpsString);
}

static void	_buildInterface(Scene *scene)
{
	InterfaceManager	*interfaces = scene->getInterfaceManager();

	Interface	*fps = interfaces->load("fps");

	fps->addElement("text_fps", new Text(UIAnchor::UI_TOP_LEFT, "fps", glm::vec2(0, 0),
		[](std::string &label)
		{
			label = getFPSString(F3 && !PAUSED);
		}, true));

	Interface	*waiting = interfaces->load("waiting");
	waiting->addElement("generated_chunks", new Text(UIAnchor::UI_CENTER, "generated chunks", glm::vec2(0, -128),
		[](std::string &label)
		{
			uint	generatedChunks = CHUNKS->getGeneratingChunksCount();
			uint	loadedChunks = CHUNKS->getLoadedChunkCount();

			label = "generated " + std::to_string(loadedChunks - generatedChunks) + "/" + std::to_string(loadedChunks);
		}, false));

	waiting->setDrawFunc([]
		(Interface*)
		{
			Shader	*shader = SHADER_MANAGER->get("colored_quad");

			uint	generatedChunks = CHUNKS->getGeneratingChunksCount();
			uint	loadedChunks = CHUNKS->getLoadedChunkCount();

			float	barSizeX = 300;
			float	barSizeY = 5;

			float	barPosX = (SCREEN_WIDTH / 2) - barSizeX / 2;
			float	barPosY = (SCREEN_HEIGHT / 2 - 112) - barSizeY / 2;

			shader->setVec3("color", glm::vec3(0.5, 0.5, 0.5));
			UIElement::draw(shader, glm::vec2(barPosX, barPosY), glm::vec2(barSizeX, barSizeY));

			float	ratio = 0;
			if (loadedChunks != 0)
				ratio = (float)(loadedChunks - generatedChunks) / (float)loadedChunks;

			shader->setVec3("color", glm::vec3(0, 1, 0));
			UIElement::draw(shader, glm::vec2(barPosX, barPosY), glm::vec2(barSizeX * ratio, barSizeY));



			uint renderDist = CHUNKS->getRenderDist() * 2;
			const std::vector<Chunk*> &chunks = CHUNKS->getLoadedChunks();

			float	chunkSizeX = 200.f / (float)renderDist;
			float	chunkSizeY = 200.f / (float)renderDist;

			float	chunkOriginX = SCREEN_WIDTH / 2.f - 100.f;
			float	chunkOriginY = SCREEN_HEIGHT / 2.f - 100.f;

			uint	x = 0;
			uint	y = 0;

			shader->setVec3("color", glm::vec3(0.5, 0.5, 0.5));
			UIElement::draw(shader, glm::vec2(chunkOriginX, chunkOriginY), glm::vec2(200, 200));
			for (Chunk * chunk: chunks)
			{
				if (chunk->isGenerated())
				{
					shader->setVec3("color", glm::vec3(0, 1, 0));
					UIElement::draw(shader, glm::vec2(chunkOriginX + (chunkSizeX * (float)x), chunkOriginY + (chunkSizeY * (float)y)), glm::vec2(chunkSizeX, chunkSizeY));
				}
				else if (chunk->waiting)
				{
					shader->setVec3("color", glm::vec3(1, 1, 0));
					UIElement::draw(shader, glm::vec2(chunkOriginX + (chunkSizeX * (float)x), chunkOriginY + (chunkSizeY * (float)y)), glm::vec2(chunkSizeX, chunkSizeY));
				}
				x++;
				if (x >= renderDist)
				{
					x = 0;
					y++;
				}
			}

		});

	Interface	*debug = interfaces->load("debug");

	debug->addElement("text_cam_pos", new Text(UIAnchor::UI_TOP_LEFT, "pos", glm::vec2(0, 16),
		[](std::string &label){label = "world xyz: " + std::to_string((int)CAMERA->pos.x) + " | " + std::to_string((int)CAMERA->pos.y) + " | " + std::to_string((int)CAMERA->pos.z);}, true));

	debug->addElement("text_cam_pitch_yaw", new Text(UIAnchor::UI_TOP_LEFT, "pitch/yaw", glm::vec2(0, 32),
		[](std::string &label){label = "pitch/yaw: " + std::to_string(CAMERA->pitch) + " | " + std::to_string(CAMERA->yaw);}, true));

	debug->addElement("text_chunk_pos", new Text(UIAnchor::UI_TOP_LEFT, "chunk pos", glm::vec2(0, 48),
		[](std::string &label){label = "chunk xz: " + std::to_string((int)CAMERA->pos.x / 32) + " | " + std::to_string((int)CAMERA->pos.z / 32);}, true));

	debug->addElement("text_camera_speed", new Text(UIAnchor::UI_TOP_LEFT, "speed", glm::vec2(0, 64),
		[](std::string &label)
		{
			static double	lastUpdate = 0;
			static float	speed = 0;

			if (glfwGetTime() - lastUpdate >= 0.1) //JUST FOR READABILITY NOT PERFORMANCE
			{
				speed = CAMERA->getSpeed();
				lastUpdate = glfwGetTime();
			}
			label = "speed: " + std::to_string(speed);
		}, true));

	debug->addElement("text_continentalness", new Text(UIAnchor::UI_TOP_LEFT, "continentalness", glm::vec2(0, 96),
		[](std::string &label){label = "continentalness: " + std::to_string(Chunk::getContinentalness(glm::vec2(CAMERA->pos.x, CAMERA->pos.z)));}, true));

	debug->addElement("text_erosion", new Text(UIAnchor::UI_TOP_LEFT, "erosion", glm::vec2(0, 112),
		[](std::string &label){label = "erosion: " + std::to_string(Chunk::getErosion(glm::vec2(CAMERA->pos.x, CAMERA->pos.z)));}, true));

	debug->addElement("text_peaksvalleys", new Text(UIAnchor::UI_TOP_LEFT, "peaks & valleys", glm::vec2(0, 128),
		[](std::string &label){label = "peaks & valleys: " + std::to_string(Chunk::getPeaksValleys(glm::vec2(CAMERA->pos.x, CAMERA->pos.z)));}, true));

	debug->addElement("text_render_distance", new Text(UIAnchor::UI_TOP_RIGHT, "render distance", glm::vec2(0, 0),
		[](std::string &label){label = "render distance (blocks): " + std::to_string(CHUNKS->getRenderDist() * 32);}, true));

	debug->addElement("text_used_threads", new Text(UIAnchor::UI_TOP_RIGHT, "used threads", glm::vec2(0, 16),
		[](std::string &label){label = "used threads: " + std::to_string(CHUNK_GENERATOR->workingThreads()) + "/" + std::to_string(GENERATION_THREAD_COUNT);}, true));

	debug->addElement("text_rendered_chunks", new Text(UIAnchor::UI_TOP_RIGHT, "rendered chunks", glm::vec2(0, 32),
		[](std::string &label){label = "rendered chunks: " + std::to_string(CHUNKS->renderCount());}, true));

	debug->addElement("text_loaded_chunks", new Text(UIAnchor::UI_TOP_RIGHT, "loaded chunks", glm::vec2(0, 48),
		[]
		(std::string &label)
		{
			static uint	size = 0;
			static double	lastUpdate = 0;

			if (glfwGetTime() - lastUpdate >= 1)
			{
				size = CHUNKS->getQuadTree()->size().leaves;
				lastUpdate = glfwGetTime();
			}
			label = "loaded chunks: " + std::to_string(size);
		}, true));

	debug->addElement("text_render_time_label", new Text(UIAnchor::UI_BOTTOM_RIGHT, "time per frame", glm::vec2(-320, 0), NULL, true));

	debug->setDrawFunc([]
		(Interface*)
		{
			Shader	*shader = SHADER_MANAGER->get("colored_quad");
			int	i = 1;
			float	graphScale = 10000.0f;
			for (DebugTimes time : frameTimes)
			{
				float	barSize = time.fullFrameTIme * graphScale;
				float	posX = SCREEN_WIDTH - (i * 5.0f);
				float	posY = SCREEN_HEIGHT - barSize;
				shader->setVec3("color", glm::vec3(1, 0, 0));
				UIElement::draw(shader, glm::vec2(posX, posY), glm::vec2(5, barSize));
				if (time.renderTime > time.updateTime)
				{
					barSize = time.renderTime * graphScale;
					posY = SCREEN_HEIGHT - barSize;
					shader->setVec3("color", glm::vec3(0, 1, 0));
					UIElement::draw(shader, glm::vec2(posX, posY), glm::vec2(5, barSize));
					barSize = time.updateTime * graphScale;
					posY = SCREEN_HEIGHT - barSize;
					shader->setVec3("color", glm::vec3(0, 0, 1));
					UIElement::draw(shader, glm::vec2(posX, posY), glm::vec2(5, barSize));
				}
				else
				{
					barSize = time.updateTime * graphScale;
					posY = SCREEN_HEIGHT - barSize;
					shader->setVec3("color", glm::vec3(0, 0, 1));
					UIElement::draw(shader, glm::vec2(posX, posY), glm::vec2(5, barSize));
					barSize = time.renderTime * graphScale;
					posY = SCREEN_HEIGHT - barSize;
					shader->setVec3("color", glm::vec3(0, 1, 0));
					UIElement::draw(shader, glm::vec2(posX, posY), glm::vec2(5, barSize));
				}
				i++;
			}
		});

	Interface	*pause = interfaces->load("pause");

	pause->addElement("button_resume", new Button(UIAnchor::UI_CENTER, "back to game", glm::vec2(0, -90), glm::vec2(300, 80), resumeGameButton, NULL));
	pause->addElement("button_options", new Button(UIAnchor::UI_CENTER, "options", glm::vec2(0, 0), glm::vec2(300, 80), []
		(ButtonInfo)
		{
			pauseGame();
			SCENE_MANAGER->get("game_scene")->getInterfaceManager()->use("options");
		}, NULL));

	pause->addElement("button_quit_game", new Button(UIAnchor::UI_CENTER, "save and quit", glm::vec2(0, 90), glm::vec2(300, 80),
		[](ButtonInfo)
		{
			SCENE_MANAGER->swap("title_scene");
			SCENE_MANAGER->get("game_scene")->getInterfaceManager()->use("leaving");
			leavingScene = true;
		}, NULL));

	Interface	*options = interfaces->load("options");

	options->addElement("button_done", new Button(UIAnchor::UI_CENTER, "done", glm::vec2(0, 90), glm::vec2(300, 80), []
		(ButtonInfo)
		{
			SCENE_MANAGER->get("game_scene")->getInterfaceManager()->use("pause");
		}, NULL));

	options->addElement("slider_render_distance", new Slider(UIAnchor::UI_CENTER, "render distance", glm::vec2(0, 0), glm::vec2(300, 80),
		[](float val)
		{
			int	newRenderDistance = glm::clamp((int)(val * 32.f), 1, 32);
			RENDER_DISTANCE = newRenderDistance * 32 * 2;
			CHUNKS->setRenderDist(newRenderDistance);
		}, [](Slider *slider) {slider->setLabel("render distance " + std::to_string(CHUNKS->getRenderDist()));}, 0.55));

	options->addElement("slider_fov", new Slider(UIAnchor::UI_CENTER, "fov", glm::vec2(0, -90), glm::vec2(300, 80),
		[](float val)
		{
			FOV = glm::clamp((int)(val * 120), 1, 120);
		}, [](Slider *slider) {slider->setLabel("fov " + std::to_string((int)FOV));}, 80.f / 120.f));

	options->addElement("button_crosshair", new Button(UIAnchor::UI_CENTER, "crosshair: off", glm::vec2(-310, 0), glm::vec2(300, 80), []
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

	Interface	*leaving = interfaces->load("leaving");

	leaving->addElement("leaving", new Text(UIAnchor::UI_CENTER, "leaving world...", glm::vec2(0, 0), NULL, false));
}

static void	_charHookFunc(Scene *, uint key)
{
	TERMINAL->input(key);
}

void	GameScene::build(Scene *scene)
{
	TERMINAL = new Terminal();
	_buildInterface(scene);
	scene->getCamera()->pos = glm::vec3(0, 0, 0);
	scene->getCamera()->pitch = 0;
	scene->getCamera()->yaw = 0;
	scene->getInterfaceManager()->reset();
	scene->setKeyHook(_keyHookFunc);
	scene->setCharHook(_charHookFunc);
	scene->setMoveMouseHook(_moveMouseHookFunc);
	scene->setMouseBtnHookFunc(_mouseBtnHookFunc);
}

void	GameScene::destructor(Scene *)
{
	if (CHUNK_GENERATOR)
	{
		delete CHUNK_GENERATOR;
		CHUNK_GENERATOR = NULL;
	}
	if (CHUNKS)
	{
		delete CHUNKS;
		CHUNKS = NULL;
	}
	delete TERMINAL;
}

static void	drawUI(Scene *scene)
{
	glDisable(GL_DEPTH_TEST);

	Interface	*debug = scene->getInterfaceManager()->get("debug");
	Interface	*fps = scene->getInterfaceManager()->get("fps");

	fps->update();
	fps->draw();
	if (!PAUSED && F3)
	{
		debug->update();
		debug->draw();
	}

	scene->getInterfaceManager()->draw();
	TERMINAL->draw();

	if (crosshair && !PAUSED)
	{
		float	crossHairSize = 20;
		Shader	*shader = SHADER_MANAGER->get("crosshair");
		Texture::use("screenTexture", MAIN_FRAME_BUFFER->getColorexture(), 0, shader);
		Texture::use("crossHairTexture", TEXTURE_MANAGER->get("assets/textures/ui/crosshair.bmp")->getID(), 1, shader);
		UIElement::draw(shader, glm::vec2(SCREEN_WIDTH / 2 - crossHairSize / 2, SCREEN_HEIGHT / 2 - crossHairSize / 2), glm::vec2(crossHairSize));
	}

    glEnable(GL_DEPTH_TEST);
}

double	frameStartTime = 0;
double	renderStartTime = 0;
double	updateTime = 0;

void	GameScene::render(Scene *scene)
{
	renderStartTime = glfwGetTime();

	if (enteringWorld)
	{
		glDisable(GL_DEPTH_TEST);
		FrameBuffer::drawFrame(SHADER_MANAGER->get("title_bg"), TEXTURE_MANAGER->get(DIRT_TEXTURE_PATH)->getID());
		scene->getInterfaceManager()->get("waiting")->draw();
		glEnable(GL_DEPTH_TEST);
		return ;
	}

	if (leavingScene)
	{
		glDisable(GL_DEPTH_TEST);
		FrameBuffer::drawFrame(SHADER_MANAGER->get("title_bg"), TEXTURE_MANAGER->get(DIRT_TEXTURE_PATH)->getID());
		scene->getInterfaceManager()->draw();
		glEnable(GL_DEPTH_TEST);
		return ;
	}
	MAIN_FRAME_BUFFER->clear();

	MAIN_FRAME_BUFFER->use();
	SKYBOX->draw(*CAMERA);
	Shader	*voxelShader = SHADER_MANAGER->get("voxel");
	Texture::use("stoneTexture", TEXTURE_MANAGER->get(MISSING_TEXTURE_PATH)->getID(), 0, voxelShader);
	Texture::use("stoneTexture", TEXTURE_MANAGER->get(STONE_TEXTURE_PATH)->getID(), 1, voxelShader);
	Texture::use("dirtTexture", TEXTURE_MANAGER->get(DIRT_TEXTURE_PATH)->getID(), 2, voxelShader);
	Texture::use("grassTexture", TEXTURE_MANAGER->get(GRASS_TOP_TEXTURE_PATH)->getID(), 3, voxelShader);
	Texture::use("grassSideTexture", TEXTURE_MANAGER->get(GRASS_SIDE_TEXTURE_PATH)->getID(), 4, voxelShader);
	Texture::use("sandTexture", TEXTURE_MANAGER->get(SAND_TEXTURE_PATH)->getID(), 5, voxelShader);
	Texture::use("waterTexture", TEXTURE_MANAGER->get(WATER_TEXTURE_PATH)->getID(), 6, voxelShader);
	CHUNKS->Render(*SHADER_MANAGER->get("voxel"));

	FrameBuffer::reset();

	_updatePostShader(SHADER_MANAGER);
	if (scene->getInterfaceManager()->getCurrent())
		SHADER_MANAGER->get("post")->setBool("blur", true);
	else
		SHADER_MANAGER->get("post")->setBool("blur", false);
	FrameBuffer::drawFrame(SHADER_MANAGER->get("post"), MAIN_FRAME_BUFFER->getColorexture());
	drawUI(scene);

	if (F3 && frameTimes.size() >= 64)
		frameTimes.pop_back();

	double	currentTime = glfwGetTime();
	if (F3)
		frameTimes.insert(frameTimes.begin(), {currentTime - frameStartTime, currentTime - renderStartTime, updateTime});
}

uint	renderDist = 0;

void	_checkWorldLoaded()
{
	if (CHUNKS->isFullyGenerated())
	{
		enteringWorld = false;
		CHUNKS->setRenderDist(renderDist);
		resumeGame();
	}
}

void	GameScene::update(Scene *scene)
{
	frameStartTime = glfwGetTime();
	double updateStartTime = glfwGetTime();

	if (enteringWorld)
	{
		scene->getCamera()->update();
		CHUNKS->UpdateChunks();
		scene->getInterfaceManager()->update();
		_checkWorldLoaded();
		return ;
	}

	_frameKeyHook(scene);
	scene->getCamera()->update();
	scene->getInterfaceManager()->update();
	_updateShaders(SHADER_MANAGER);
	CHUNKS->UpdateChunks();
	updateTime = glfwGetTime() - updateStartTime;
}

void	GameScene::close(Scene *)
{
	WORLD_MANAGER->getCurrent()->save();
	WORLD_MANAGER->reset();
	if (CHUNK_GENERATOR)
	{
		delete CHUNK_GENERATOR;
		CHUNK_GENERATOR = NULL;
	}
	if (CHUNKS)
	{
		delete CHUNKS;
		CHUNKS = NULL;
	}
	leavingScene = false;
	enteringWorld = false;
	consoleLog("Closed a world", LogSeverity::NORMAL);
}

extern std::string	currentWorldID;

void	GameScene::open(Scene *)
{
	World	*world = WORLD_MANAGER->get(currentWorldID);
	if (!world)
		world = WORLD_MANAGER->load(currentWorldID, rand());
	seed = world->getSeed();
	WORLD_MANAGER->use(currentWorldID);

	CAMERA->pos = WORLD_MANAGER->getCurrent()->getPlayerPos();
	CAMERA->pitch = WORLD_MANAGER->getCurrent()->getFloatInfo("pitch");
	CAMERA->yaw = WORLD_MANAGER->getCurrent()->getFloatInfo("yaw");

	if (!CHUNK_GENERATOR)
		CHUNK_GENERATOR = new ChunkGeneratorManager();
	if (!CHUNKS)
		CHUNKS = new RegionManager();
	enteringWorld = true;
	renderDist = CHUNKS->getRenderDist();
	CHUNKS->setRenderDist(10);
	SCENE_MANAGER->get("game_scene")->getInterfaceManager()->use("waiting");
	consoleLog("Opened a world", LogSeverity::NORMAL);
}
