/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 13:33:29 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 18:23:09 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libs.hpp"
#include "Font.hpp"
#include "ShaderManager.hpp"
#include "TextureManager.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Skybox.hpp"
#include "FrameBuffer.hpp"
#include "Chunk.hpp"
#include "RegionManager.hpp"
#include "ChunkGeneratorManager.hpp"
#include "InterfaceManager.hpp"
#include "UIElement.hpp"
#include "Terminal.hpp"

float	FOV = 80;
float	SCREEN_WIDTH = 860;
float	SCREEN_HEIGHT = 520;
float	RENDER_DISTANCE = 1024;

bool		F3 = false;
bool		PAUSED = false;
bool		SKYBOX_ACTIVE = false;

int		currentFPS = 60;

uint	seed = 0;

Font				*FONT;
Window				*WINDOW;
Camera				*CAMERA;
Skybox				*SKYBOX;
Terminal			*TERMINAL;

RegionManager		*CHUNKS; //!testing

TextureManager		*TEXTURE_MANAGER;
ShaderManager		*SHADER_MANAGER;
InterfaceManager	*INTERFACE_MANAGER;

FrameBuffer	*MAIN_FRAME_BUFFER;
FrameBuffer	*DEPTH_FRAME_BUFFER;
FrameBuffer	*WATER_DEPTH_FRAME_BUFFER;

ChunkGeneratorManager	*CHUNK_GENERATOR;

/*
	Keyboard input as the char so like typing on a keyboard
*/
void	keyboard_input(GLFWwindow *, unsigned int key)
{
	TERMINAL->input(key);
}

void	pauseGame(void*)
{
	PAUSED = true;
	INTERFACE_MANAGER->use("pause");
	glfwSetInputMode(WINDOW->getWindowData(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void	resumeGame(void*)
{
	PAUSED = false;
	INTERFACE_MANAGER->reset();
	WINDOW->setDefaultMousePos();
}

void	closeWindow(void*)
{
	if (!WINDOW->up())
		return ;
	glfwSetWindowShouldClose(WINDOW->getWindowData(), true);
	consoleLog("Quitting game.", LogSeverity::WARNING);
}

/*
	Basic keyhooks when you dont need it to be executed every frame
*/
void	key_hook(GLFWwindow *window, int key, int, int action, int)
{
	if (TERMINAL->specialInput(key, action))
		return ;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && WINDOW->up())
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
			consoleLog("Quitting game.", LogSeverity::WARNING);
		}
		if (!TERMINAL->isActive() && PAUSED)
			resumeGame(NULL);
		else if (!TERMINAL->isActive())
			pauseGame(NULL);
	}
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
	{
		F3 = !F3;
		if (!F3)
			glfwSwapInterval(1);
		else
		{
			consoleLog("WARNING: Debug mode is ON, more resources will be used.", LogSeverity::WARNING);
			glfwSwapInterval(0);
		}
	}
	if (TERMINAL->isActive())
		return ;
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		uint	current = CHUNKS->getRenderDist();
		if (current + 1 < 32)
		{
			RENDER_DISTANCE = ((current + 1) * 32) * 2;
			CHUNKS->setRenderDist(current + 1);
		}
	}
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		int	current = CHUNKS->getRenderDist();
		if (current - 1 > 0)
		{
			RENDER_DISTANCE = ((current - 1) * 32) * 2;
			CHUNKS->setRenderDist(current - 1);
		}
	}
}

/*
	Function used to load all textures, as with the shaders, id recommend loading textures at the start especially if they are big
*/
void	build(TextureManager *textures)
{
	consoleLog("Loading textures...", LogSeverity::NORMAL);
	textures->load("textures/mbatty.bmp");
	textures->load("textures/stone.bmp");
	textures->load("textures/dirt.bmp");
	textures->load("textures/grass.bmp");
	textures->load("textures/grass_side.bmp");
	textures->load("textures/cobblestone.bmp");
	consoleLog("Finished loading textures", LogSeverity::SUCCESS);
}

void	openOptions(void*)
{
	pauseGame(NULL);
	INTERFACE_MANAGER->use("options");
}

/*
	Function used to build and load all shaders used in the program, shaders can be built later on during execution but I wouldnt recommend it
*/
void	build(ShaderManager *shader)
{
	consoleLog("Building shaders...", LogSeverity::NORMAL);
	Shader *textShader = shader->load({"text", TEXT_VERT_SHADER, TEXT_FRAG_SHADER});
	Shader *skyboxShader = shader->load({"skybox", SKYBOX_VERT_SHADER, SKYBOX_FRAG_SHADER});
	Shader *postShader = shader->load({"post", "shaders/post.vs", "shaders/post.fs"});
	shader->load({"voxel", "shaders/voxel.vs", "shaders/voxel.fs"});
	Shader *guiShader = shader->load({"gui", "shaders/gui_shader.vs", "shaders/gui_shader.fs"});

	guiShader->setInt("tex0", 0);

	Texture::use("terrainDepthTex", 0, 1, SHADER_MANAGER->get("voxel"));
	Texture::use("waterDepthTex", 0, 2, SHADER_MANAGER->get("voxel"));

	shader->load({"test", "shaders/test.vs", "shaders/test.fs"});
	Texture::use("screenTexture", 0, 0, (*shader)["test"]);

	Texture::use("tex0", 0, 0, textShader);

	Texture::use("skybox", 0, 0, skyboxShader);

	Texture::use("screenTexture", 0, 0, postShader);
	Texture::use("depthTex", 0, 1, postShader);

	consoleLog("Finished building shaders", LogSeverity::SUCCESS);
}

/*
	Returns current FPS as a string
*/
std::string	getFPSString()
{
	static int			frame = 0;
	static std::string	fpsString = "0 fps";
	if (frame++ >= currentFPS / 10)
	{
		currentFPS = (int)(1.0f / WINDOW->getDeltaTime());
		fpsString = std::to_string(currentFPS) + " fps";
		frame = 0;
	}
	return (fpsString);
}

void	build(InterfaceManager *interfaces)
{
	consoleLog("Loading interfaces...", LogSeverity::NORMAL);

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

	pause->addElement("button_resume", new Button(UIAnchor::UI_CENTER, "resume", glm::vec2(0, -90), glm::vec2(300, 80), resumeGame, NULL));
	pause->addElement("button_options", new Button(UIAnchor::UI_CENTER, "options", glm::vec2(0, 0), glm::vec2(300, 80), openOptions, NULL));
	pause->addElement("button_quit_game", new Button(UIAnchor::UI_CENTER, "quit game", glm::vec2(0, 90), glm::vec2(300, 80), closeWindow, NULL));
	
	pause->addElement("text_popup", new Text(UIAnchor::UI_TOP_CENTER_HALF, "by mbatty and mbirou", glm::vec2(175, -40), NULL, false));
	pause->addElement("image_icon", new Image(UIAnchor::UI_TOP_CENTER_HALF, glm::vec2(0, 0), glm::vec2(400, 150)));

	pause->setUpdateFunc([]
		(Interface *interface)
		{
			UIElement	*elem = interface->getElement("text_popup");
			Text		*text_popup = static_cast<Text*>(elem);

			float	scale = 1 + std::abs(cos(glfwGetTime())) / 10;

			text_popup->setScale(glm::vec2(scale, scale));
			text_popup->setRotation(glm::vec3(0.0, 0.0, 1.0));
			text_popup->setAngle(-10 + cos(glfwGetTime()) * 10);
		});

	Interface	*options = interfaces->load("options");

	options->addElement("button_leave", new Button(UIAnchor::UI_CENTER, "leave", glm::vec2(0, 90), glm::vec2(300, 80), [](void*)
		{
			INTERFACE_MANAGER->use("pause");
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
			FOV = glm::clamp((int)(val * 100), 1, 100);
		}, [](Slider *slider) {slider->setLabel("fov " + std::to_string((int)FOV));}, 0.80));

	consoleLog("Finished loading interfaces", LogSeverity::SUCCESS);
}

/*
	Draws all 2D elements on screen for now just FPS
*/
void    drawUI()
{
    glDisable(GL_DEPTH_TEST);

	Interface	*debug = INTERFACE_MANAGER->get("debug");
	Interface	*fps = INTERFACE_MANAGER->get("fps");

	fps->update();
	fps->draw();
	if (!PAUSED && F3)
	{
		debug->update();
		debug->draw();
	}

	INTERFACE_MANAGER->draw();
	TERMINAL->draw();

    glEnable(GL_DEPTH_TEST);
}

/*
	Update post shader separatly because I needed to at some point (Mainly for viewPos if you use multiple cameras at once)
*/
void	updatePostShader(ShaderManager *shaders)
{
	Shader	*postShader = shaders->get("post");

	postShader->use();
	postShader->setVec3("viewPos", CAMERA->pos);
	postShader->setFloat("time", glfwGetTime());
}

/*
	Used to update the shaders with infos they need for the frame (screen size, time and all)
*/
void	update(ShaderManager *shaders)
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

/*
	Handles player movement in the world
*/
void	frame_key_hook(Window &window)
{
	if (PAUSED || TERMINAL->isActive())
		return ;
	float cameraSpeed = 15 * window.getDeltaTime();
	float	speedBoost = 1.0f;

	if (glfwGetKey(window.getWindowData(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		speedBoost = 20.0f;

	if (glfwGetKey(window.getWindowData(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		speedBoost *= 15.0f;
	
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_W) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_S) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_SPACE) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->worldUp * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->worldUp * (cameraSpeed * speedBoost);
		
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_A) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - glm::normalize(glm::cross(CAMERA->front, CAMERA->worldUp)) * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_D) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + glm::normalize(glm::cross(CAMERA->front, CAMERA->worldUp)) * (cameraSpeed * speedBoost);
}

/*
	Handles rotating the camera around using the mouse
*/
void	move_mouse_hook(GLFWwindow*, double xpos, double ypos)
{
	if (PAUSED || TERMINAL->isActive())
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

/*
	Small structure used to load all necessary classes in order so no weird stuff should happen
*/
struct	Engine
{
	Engine()
	{
		srand(std::time(NULL));
		WINDOW = new Window();
		CAMERA = new Camera();
		CAMERA->pos.y += 40;
		TEXTURE_MANAGER = new TextureManager();
		build(TEXTURE_MANAGER);
		SHADER_MANAGER = new ShaderManager();
		build(SHADER_MANAGER);
		FONT = new Font();
		INTERFACE_MANAGER = new InterfaceManager();
		build(INTERFACE_MANAGER);
		CHUNK_GENERATOR = new ChunkGeneratorManager();
		MAIN_FRAME_BUFFER = new FrameBuffer();
		DEPTH_FRAME_BUFFER = new FrameBuffer();
		WATER_DEPTH_FRAME_BUFFER = new FrameBuffer();
		SKYBOX = new Skybox({SKYBOX_PATHES});
		CHUNKS = new RegionManager();
		TERMINAL = new Terminal();
	}
	~Engine()
	{
		delete CHUNK_GENERATOR;
		delete CHUNKS;
		delete TERMINAL;
		delete SKYBOX;
		delete MAIN_FRAME_BUFFER;
		delete DEPTH_FRAME_BUFFER;
		delete WATER_DEPTH_FRAME_BUFFER;
		delete FONT;
		delete SHADER_MANAGER;
		delete TEXTURE_MANAGER;
		delete INTERFACE_MANAGER;
		delete CAMERA;
		consoleLog("Done.", LogSeverity::SUCCESS);
		delete WINDOW;
	}
};

/*
	Renders main scene (Not post processing effects)
*/
void	render()
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
}

/*
	Game updates
*/
void	update()
{
	INTERFACE_MANAGER->update();
}

Quadtree	*prevBranch = NULL;

int	getBlock(const glm::vec3 &pos)
{
	CHUNKS->getQuadTree()->getLeaf(pos);
	return (0);
}

int	main(int ac, char **av)
{
	consoleLog("Starting...", NORMAL);

	try {
		Engine	engine;

		consoleLog("Starting rendering...", NORMAL);

		if (ac >= 2)
			seed = std::atoi(av[1]);
		else
			seed = rand();

		CAMERA->yaw = 45;
		CAMERA->pitch = -10;
		CAMERA->pos = {WORLD_SIZE / 2, 130, WORLD_SIZE / 2};

		pauseGame(NULL);

		while (WINDOW->up())
		{
			WINDOW->loopStart();
			CAMERA->update();
			update(SHADER_MANAGER);
			update();

			CHUNKS->getQuadTree()->pruneDeadLeaves(CHUNKS->getQuadTree());

			render();

			FrameBuffer::reset();

			updatePostShader(SHADER_MANAGER);
			if (INTERFACE_MANAGER->getCurrent())
				SHADER_MANAGER->get("post")->setBool("blur", true);
			else
				SHADER_MANAGER->get("post")->setBool("blur", false);
			FrameBuffer::drawFrame(SHADER_MANAGER->get("post"), MAIN_FRAME_BUFFER->getColorexture());

			drawUI();
			
			frame_key_hook(*WINDOW);
			WINDOW->loopEnd();
		}
	} catch (const std::exception &e) {
		consoleLog("Program terminated: " + std::string(e.what()), LogSeverity::MELTDOWN);
		return (1);
	}
}
