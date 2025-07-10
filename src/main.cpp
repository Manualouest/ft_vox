/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 13:33:29 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/10 09:34:48 by mbatty           ###   ########.fr       */
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

#define WORLD_SIZE 32

float	FOV = 65;
float	SCREEN_WIDTH = 860;
float	SCREEN_HEIGHT = 520;
float	RENDER_DISTANCE = 448;

bool	F3 = false;
bool	PAUSED = false;
bool	SKYBOX_ACTIVE = false;

int		currentFPS = 60;

Font				*FONT;
Window				*WINDOW;
Camera				*CAMERA;
Skybox				*SKYBOX;


TextureManager		*TEXTURE_MANAGER;
ShaderManager		*SHADER_MANAGER;

FrameBuffer	*MAIN_FRAME_BUFFER;

/*
	Keyboard input as the char so like typing on a keyboard
*/
void	keyboard_input(GLFWwindow *window, unsigned int key)
{
	(void)window;(void)key;
}

/*
	Basic keyhooks when you dont need it to be executed every frame
*/
void	key_hook(GLFWwindow *window, int key, int, int action, int)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS && WINDOW->up())
	{
		consoleLog("Quitting game.", LogSeverity::WARNING);
		glfwSetWindowShouldClose(window, true);
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
	consoleLog("Finished loading textures", LogSeverity::SUCCESS);
}

/*
	Function used to build and load all shaders used in the program, shaders can be built later on during execution but I wouldnt recommend it
*/
void	build(ShaderManager *shader)
{
	consoleLog("Building shaders...", LogSeverity::NORMAL);
	Shader *textShader = shader->load({"text", TEXT_VERT_SHADER, TEXT_FRAG_SHADER});
	Shader *skyboxShader = shader->load({"skybox", SKYBOX_VERT_SHADER, SKYBOX_FRAG_SHADER});
	Shader *waterShader = shader->load({"water", "shaders/water.vs", "shaders/water.fs"});
	Shader *postShader = shader->load({"post", "shaders/post.vs", "shaders/post.fs"});

	Texture::use("tex0", 0, 0, textShader);

	Texture::use("skybox", 0, 0, skyboxShader);

	Texture::use("screenTexture", 0, 0, postShader);
	Texture::use("depthTex", 0, 1, postShader);

	Texture::use("depthTex", 0, 0, waterShader);
	Texture::use("waterDepthTex", 0, 1, waterShader);

	consoleLog("Finished building shaders", LogSeverity::SUCCESS);
}

/*
	Returns current FPS as a string
*/
std::string	getFPSString()
{
	currentFPS = (int)(1.0f / WINDOW->getDeltaTime());
	return (std::to_string(currentFPS) + " fps");
}

/*
	Draws all 2D elements on screen for now just FPS
*/
void	drawUI()
{
	glDisable(GL_DEPTH_TEST);

	static int frame = 0;
	static std::string	fps = "0 fps";
	std::string			particles_count;
	std::string			load_particles;

	if (frame++ >= currentFPS / 10)
	{
		frame = 0;
		fps = getFPSString();
	}
	FONT->putString(fps, *SHADER_MANAGER->get("text"),
		glm::vec2((SCREEN_WIDTH / 2) - (fps.length() * 15) / 2, 0),
		glm::vec2(fps.length() * 15, 15));
	
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
	Shader	*waterShader = shaders->get("water");
	Shader	*postShader = shaders->get("post");

	textShader->use();
	textShader->setFloat("time", glfwGetTime());
	textShader->setFloat("SCREEN_WIDTH", SCREEN_WIDTH);
	textShader->setFloat("SCREEN_HEIGHT", SCREEN_HEIGHT);

	waterShader->use();
	CAMERA->setViewMatrix(*waterShader);
	waterShader->setVec3("viewPos", CAMERA->pos);
	waterShader->setFloat("time", glfwGetTime());
	waterShader->setFloat("RENDER_DISTANCE", RENDER_DISTANCE);

	postShader->use();
	postShader->setFloat("RENDER_DISTANCE", RENDER_DISTANCE);
}

/*
	Handles player movement in the world
*/
void	frame_key_hook(Window &window)
{
	float cameraSpeed = 15.0f * window.getDeltaTime();
	float	speedBoost = 1.0f;

	if (glfwGetKey(window.getWindowData(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		speedBoost = 20.0f;
	
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_W) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_S) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->front * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_SPACE) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + CAMERA->up * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - CAMERA->up * (cameraSpeed * speedBoost);
		
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_A) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos - glm::normalize(glm::cross(CAMERA->front, CAMERA->up)) * (cameraSpeed * speedBoost);
	if (glfwGetKey(window.getWindowData(), GLFW_KEY_D) == GLFW_PRESS)
		CAMERA->pos = CAMERA->pos + glm::normalize(glm::cross(CAMERA->front, CAMERA->up)) * (cameraSpeed * speedBoost);
}

/*
	Handles rotating the camera around using the mouse
*/
void	move_mouse_hook(GLFWwindow* window, double xpos, double ypos)
{
	(void)window;

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
		WINDOW = new Window();
		CAMERA = new Camera();
		FONT = new Font();
		SHADER_MANAGER = new ShaderManager();
		build(SHADER_MANAGER);
		TEXTURE_MANAGER = new TextureManager();
		build(TEXTURE_MANAGER);
		MAIN_FRAME_BUFFER = new FrameBuffer(FrameBufferType::DEFAULT);
		SKYBOX = new Skybox({SKYBOX_PATHES});
	}
	~Engine()
	{
		delete SKYBOX;
		delete MAIN_FRAME_BUFFER;
		delete TEXTURE_MANAGER;
		delete SHADER_MANAGER;
		delete FONT;
		delete CAMERA;
		delete WINDOW;
	}
};

/*
	Renders main scene (Not post processing effects)
*/
void	render()
{
	SKYBOX->draw(*CAMERA, *SHADER_MANAGER->get("skybox"));
}

/*
	Game updates
*/
void	update()
{
	
}

int	main(void)
{
	consoleLog("Starting...", NORMAL);

	try {
		Engine	engine;

		consoleLog("Starting rendering...", NORMAL);
		while (WINDOW->up())
		{
			WINDOW->loopStart();
			CAMERA->update();
			update(SHADER_MANAGER);
			update();

			
			MAIN_FRAME_BUFFER->use();
			render();

			
			FrameBuffer::reset();

			updatePostShader(SHADER_MANAGER);
			FrameBuffer::drawFrame(SHADER_MANAGER->get("post"), MAIN_FRAME_BUFFER->getTexture());

			drawUI();

			
			frame_key_hook(*WINDOW);
			WINDOW->loopEnd();
		}
	} catch (const std::exception &e) {
		consoleLog("Program terminated: " + std::string(e.what()), LogSeverity::MELTDOWN);
		return (1);
	}
}
