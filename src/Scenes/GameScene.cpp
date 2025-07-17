/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GameScene.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 11:13:19 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/17 17:46:21 by mbatty           ###   ########.fr       */
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

extern SceneManager		*SCENE_MANAGER;
extern FrameBuffer	*MAIN_FRAME_BUFFER;
extern Skybox		*SKYBOX;
extern ShaderManager	*SHADER_MANAGER;
extern RegionManager	*CHUNKS;
extern bool	PAUSED;
extern bool	F3;
extern Window	*WINDOW;

Terminal			*TERMINAL;

void	pauseGame(void*);
void	resumeGame(void*);
void	closeWindow(void*);
void	openOptions(void*);

static void	_keyHookFunc(Scene *, int key, int action)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && !TERMINAL->isActive())
	{
		if (PAUSED)
			resumeGame(NULL);
		else
			pauseGame(NULL);
	}
	else if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		F3 = !F3;
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
	if (PAUSED)
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
	
	if(CAMERA->pitch > 89.0f)
		CAMERA->pitch = 89.0f;
	if(CAMERA->pitch < -89.0f)
		CAMERA->pitch = -89.0f;
}

static void	_updateShaders(ShaderManager *shaders)
{
	Shader	*textShader = shaders->get("text");
	Shader	*postShader = shaders->get("post");
	Shader	*voxelShader = shaders->get("voxel");

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
}

std::string	getFPSString();

static void	_buildInterface(Scene *scene)
{
	InterfaceManager	*interfaces = scene->getInterfaceManager();
	
	Interface	*fps = interfaces->load("fps");

	fps->addElement("text_fps", new Text(UIAnchor::UI_TOP_LEFT, "fps", glm::vec2(0, 0),
		[](std::string &label){label = getFPSString();}, true));

	Interface	*debug = interfaces->load("debug");
		
	debug->addElement("text_cam_pos", new Text(UIAnchor::UI_TOP_LEFT, "camera pos", glm::vec2(0, 15),
		[](std::string &label){label = "world xyz: " + std::to_string((int)CAMERA->pos.x) + "," + std::to_string((int)CAMERA->pos.y) + "," + std::to_string((int)CAMERA->pos.z);}, true));

	debug->addElement("text_chunk_pos", new Text(UIAnchor::UI_TOP_LEFT, "chunk pos", glm::vec2(0, 30),
		[](std::string &label){label = "chunk xz: " + std::to_string((int)CAMERA->pos.x / 32) + "," + std::to_string((int)CAMERA->pos.z / 32);}, true));

	debug->addElement("text_render_distance", new Text(UIAnchor::UI_TOP_RIGHT, "render distance", glm::vec2(0, 0),
		[](std::string &label){label = "render distance (blocks): " + std::to_string(CHUNKS->getRenderDist() * 32);}, true));
			
	debug->addElement("text_used_threads", new Text(UIAnchor::UI_TOP_RIGHT, "used threads", glm::vec2(0, 15),
		[](std::string &label){label = "used threads: " + std::to_string(CHUNK_GENERATOR->workingThreads()) + "/" + std::to_string(GENERATION_THREAD_COUNT);}, true));

	debug->addElement("text_rendered_chunks", new Text(UIAnchor::UI_TOP_RIGHT, "rendered chunks", glm::vec2(0, 30),
		[](std::string &label){label = "rendered chunks: " + std::to_string(CHUNKS->renderCount());}, true));

	Interface	*pause = interfaces->load("pause");

	pause->addElement("button_resume", new Button(UIAnchor::UI_CENTER, "back to game", glm::vec2(0, -90), glm::vec2(300, 80), resumeGame, NULL));
	pause->addElement("button_options", new Button(UIAnchor::UI_CENTER, "options", glm::vec2(0, 0), glm::vec2(300, 80), openOptions, NULL));
	pause->addElement("button_quit_game", new Button(UIAnchor::UI_CENTER, "save and quit", glm::vec2(0, 90), glm::vec2(300, 80), 
		[](void*)
		{
			SCENE_MANAGER->use("title_scene");
		}, NULL));

	Interface	*options = interfaces->load("options");

	options->addElement("button_leave", new Button(UIAnchor::UI_CENTER, "leave", glm::vec2(0, 90), glm::vec2(300, 80), [](void*)
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
		}, [](Slider *slider) {slider->setLabel("fov " + std::to_string((int)FOV));}, 0.80));
}

void	_charHookFunc(Scene *, uint key)
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

    glEnable(GL_DEPTH_TEST);
}

void	GameScene::render(Scene *scene)
{
	MAIN_FRAME_BUFFER->clear();

	MAIN_FRAME_BUFFER->use();
	SKYBOX->draw(*CAMERA);
	Shader	*voxelShader = SHADER_MANAGER->get("voxel");
	Texture::use("stoneTexture", TEXTURE_MANAGER->get("textures/missing.bmp")->getID(), 0, voxelShader);
	Texture::use("stoneTexture", TEXTURE_MANAGER->get("textures/stone.bmp")->getID(), 1, voxelShader);
	Texture::use("dirtTexture", TEXTURE_MANAGER->get("textures/dirt.bmp")->getID(), 2, voxelShader);
	Texture::use("grassTexture", TEXTURE_MANAGER->get("textures/grass.bmp")->getID(), 3, voxelShader);
	Texture::use("grassSideTexture", TEXTURE_MANAGER->get("textures/grass_side.bmp")->getID(), 4, voxelShader);
	Texture::use("sandTexture", TEXTURE_MANAGER->get("textures/sand.bmp")->getID(), 5, voxelShader);
	Texture::use("waterTexture", TEXTURE_MANAGER->get("textures/water.bmp")->getID(), 6, voxelShader);
	CHUNKS->Render(*SHADER_MANAGER->get("voxel"));

	FrameBuffer::reset();

	_updatePostShader(SHADER_MANAGER);
	if (scene->getInterfaceManager()->getCurrent())
		SHADER_MANAGER->get("post")->setBool("blur", true);
	else
		SHADER_MANAGER->get("post")->setBool("blur", false);
	FrameBuffer::drawFrame(SHADER_MANAGER->get("post"), MAIN_FRAME_BUFFER->getColorexture());
	drawUI(scene);
}

void	GameScene::update(Scene *scene)
{
	_frameKeyHook(scene);
	scene->getCamera()->update();
	scene->getInterfaceManager()->update();
	_updateShaders(SHADER_MANAGER);
	CHUNKS->getQuadTree()->pruneDeadLeaves(CHUNKS->getQuadTree());
}
