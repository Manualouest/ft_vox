/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Shader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/03 12:15:58 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/14 18:46:51 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Shader.hpp"

Shader::~Shader()
{
	if (DEBUG)
        consoleLog("Destroying shader: " + vertPath + " " + fragPath, LogSeverity::NORMAL);
	glDeleteProgram(ID);
}

static int	loadVertexShader(const char *shader)
{
	int				success;
	char			infoLog[512];
	unsigned int	res = glCreateShader(GL_VERTEX_SHADER);
	
	glShaderSource(res, 1, &shader, NULL);
	glCompileShader(res);
	glGetShaderiv(res, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(res, 512, NULL, infoLog);
		std::cout << shader << std::endl;
		std::cout << infoLog << std::endl;
		return (0);
	}
	return (res);
}

static int	loadFragmentShader(const char *shader)
{
	int				success;
	char			infoLog[512];
	unsigned int	res = glCreateShader(GL_FRAGMENT_SHADER);
	
	glShaderSource(res, 1, &shader, NULL);
	glCompileShader(res);
	glGetShaderiv(res, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(res, 512, NULL, infoLog);
		std::cout << shader << std::endl;
		std::cout << infoLog << std::endl;
		return (0);
	}
	return (res);
}

static int	loadShaderProgram(const char *vertex, const char *fragment)
{
	unsigned int	res;
	int				success;
	char			infoLog[512];
	int				vertexShader;
	int				fragmentShader;
	
	vertexShader = loadVertexShader(vertex);
	if (!vertexShader)
		return (0);
	fragmentShader = loadFragmentShader(fragment);
	if (!fragmentShader)
		return (0);

	res = glCreateProgram();
	glAttachShader(res, vertexShader);
	glAttachShader(res, fragmentShader);
	glLinkProgram(res);
	glGetProgramiv(res, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(res, 512, NULL, infoLog);
		std::cout << vertex << std::endl;
		std::cout << infoLog << std::endl;
		return (0);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return (res);
}

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
	if (DEBUG)
        consoleLog("Loading shader: " + std::string(vertexPath) + " " + std::string(fragmentPath), LogSeverity::NORMAL);
		
	std::string	vertexCode;
	std::string	fragmentCode;
	std::ifstream	vShaderFile;
	std::ifstream	fShaderFile;

	vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		vShaderFile.close();
		fShaderFile.close();
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch(...)
	{
		throw std::runtime_error("Failed to read shader");
	}

	const char	*vShaderCode = vertexCode.c_str();
	const char	*fShaderCode = fragmentCode.c_str();
	
	this->ID = loadShaderProgram(vShaderCode, fShaderCode);
	if (!this->ID)
		throw std::runtime_error("Failed to load shader program");

	this->fragPath = fragmentPath;
	this->vertPath = vertexPath;
}

void	Shader::use() const
{
	glUseProgram(this->ID);
}

void	Shader::setBool(const std::string &name, bool value) const
{
	this->use();
	glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
}

void	Shader::setInt(const std::string &name, int value) const
{
	this->use();
	glUniform1i(glGetUniformLocation(this->ID, name.c_str()), value);
}

void	Shader::setFloat(const std::string &name, float value) const
{
	this->use();
	glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
}

void	Shader::setMat4(const std::string &name, glm::mat4 value) const
{
	this->use();
	glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void	Shader::setVec3(const std::string &name, glm::vec3 value) const
{
	this->use();
	glUniform3fv(glGetUniformLocation(this->ID, name.c_str()), 1, glm::value_ptr(value));
}
