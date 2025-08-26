/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Window.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:11:45 by mbatty            #+#    #+#             */
/*   Updated: 2025/08/26 13:41:10 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Window.hpp"
#include "Camera.hpp"
#include "FrameBuffer.hpp"

extern FrameBuffer	*MAIN_FRAME_BUFFER;

static void resize_hook(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	MAIN_FRAME_BUFFER->resize(width, height);
}

void	key_hook(GLFWwindow *window, int key, int scancode, int action, int mods);
void	keyboard_input(GLFWwindow *window, unsigned int key);
void	move_mouse_hook(GLFWwindow* window, double xpos, double ypos);
void	press_mouse_hook(GLFWwindow* window, int button, int action, int mods);
void	scroll_callback(GLFWwindow *, double xoffset, double yoffset);

Window::Window() : _lastFrame(0)
{
	consoleLog("Creating window...", LogSeverity::NORMAL);
	//Inits GLFW settings
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize glfw");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creates and opens window
	GLFWmonitor	*monitor = NULL;
	#if FULL_SCREEN
		monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode	*monitorInfos = glfwGetVideoMode(monitor);
		SCREEN_HEIGHT = monitorInfos->height;
		SCREEN_WIDTH = monitorInfos->width;
	#endif
	_windowData = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WIN_NAME, monitor, NULL);
	if (!_windowData)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}
	glfwMakeContextCurrent(_windowData);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		throw std::runtime_error("Failed to init GLAD");
	}
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	glfwSetFramebufferSizeCallback(_windowData, resize_hook);
	glfwSetKeyCallback(_windowData, key_hook);
	glfwSetCharCallback(_windowData, keyboard_input);
	glfwSetInputMode(this->getWindowData(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(this->getWindowData(), move_mouse_hook);
	glfwSetMouseButtonCallback(this->getWindowData(), press_mouse_hook);
	glfwSetScrollCallback(this->getWindowData(), scroll_callback);

	glClearColor(0.6, 0.8, 1.0, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	this->center();
	this->setIcon(MBATTY_TEXTURE_PATH);
	consoleLog("Creating window, done.", LogSeverity::SUCCESS);
}

Window::~Window()
{
	glfwDestroyWindow(this->_windowData);
	glfwTerminate();
}

GLFWwindow	*Window::getWindowData(void)
{
	return (this->_windowData);
}

void		Window::loopStart(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_currentFrame = glfwGetTime();
	_deltaTime = _currentFrame - _lastFrame;
}

void		Window::loopEnd()
{
	glfwSwapBuffers(_windowData);
	glfwPollEvents();

	_lastFrame = _currentFrame;
}

bool	Window::up(void)
{
	return (!glfwWindowShouldClose(_windowData));
}

void		Window::setIcon(const char *path)
{
	Texture	tex(path);
	GLFWimage	image[1];
	image[0].pixels = tex.getData().data();
	image[0].width = tex.getWidth();
	image[0].height = tex.getHeight();
	glfwSetWindowIcon(_windowData, 1, image);
}

void		Window::center()
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwSetWindowPos(_windowData, (mode->width / 2) - (SCREEN_WIDTH / 2), (mode->height / 2) - (SCREEN_HEIGHT / 2));
}

void	Window::setDefaultMousePos()
{
	double	x,y;
	glfwSetInputMode(this->_windowData, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(this->_windowData, &x, &y);
	lastMouseX = x;
	lastMouseY = y;
}

float		Window::getDeltaTime(void)
{
	return (this->_deltaTime);
}
