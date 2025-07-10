/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Texture.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 13:49:15 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/09 11:03:51 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Texture.hpp"

Texture::~Texture()
{
	if (ID > 0)
	{
		if (DEBUG)
			consoleLog("Destroying texture: " + std::string(path), LogSeverity::NORMAL);
		glDeleteTextures(1, &ID);
	}
}

void	Texture::cut(Texture &cpy)
{
	this->ID = cpy.ID;
	this->path = cpy.path;
	cpy.ID = 0;
}

typedef struct s_bmp_header
{
	uint16_t	identifier;
	uint32_t	size;
	uint32_t	reserved;
	uint32_t	data_offset;
	uint32_t	dib_size;
	uint16_t	dib_width;
	uint16_t	dib_height;
	uint16_t	dib_cp;
	uint16_t	dib_bpp;

	uint32_t	data_size;
} __attribute__((packed))	bmp_header;

# define BMP_ID		0x4D42
# define HEADER_SIZE	26
# define MAX_SIZE		0x5F5E11A

void	check_header(bmp_header *header)
{
	uint64_t	expected_size;

	if (header->identifier != BMP_ID)
		throw std::runtime_error(std::string("Invalid texture format"));
	if (header->size <= HEADER_SIZE || header->size > MAX_SIZE)
		throw std::runtime_error(std::string("Invalid texture format"));
	if (header->data_offset != HEADER_SIZE)
		throw std::runtime_error(std::string("Invalid texture format"));
	if (header->dib_size != 12)
		throw std::runtime_error(std::string("Invalid texture format"));
	header->data_size = header->size - HEADER_SIZE;
	if (header->dib_width < 5 || header->dib_height < 5)
		throw std::runtime_error(std::string("Invalid texture format"));
	expected_size = 3 * (header->dib_height
			* (header->dib_width + header->dib_width % 4));
	if (header->data_size != expected_size)
		throw std::runtime_error(std::string("Invalid texture format"));
	if (header->dib_bpp != 24)
		throw std::runtime_error(std::string("Invalid texture format"));
	if (header->dib_cp != 1)
		throw std::runtime_error(std::string("Invalid texture format"));
}

void	Texture::LoadImage(const char *path)
{
	std::ifstream	file;
	bmp_header informationHeader;

	file.open(path);
	if (!file.is_open())
		throw std::runtime_error(std::string("Failed to open ") + path);

	file.read((char*)&informationHeader, 26);
	check_header(&informationHeader);
	
	width = informationHeader.dib_width;
	height = informationHeader.dib_height;
	
	data.resize((width * height) * 4);
	
	for (int y = 0; y < height * 4; y += 4)
	{
		for (int x = 0; x < width * 4; x += 4)
		{
			char	color[3];
			file.read(color, 3);

			data[(y * width + x)] = color[2];
			data[(y * width + x) + 1] = color[1];
			data[(y * width + x) + 2] = color[0];
			data[(y * width + x) + 3] = 255;
		}
	}
	file.close();
}

Texture::Texture(const char *path)
{
	if (DEBUG)
		consoleLog("Loading texture: " + std::string(path), LogSeverity::NORMAL);
	this->LoadImage(path);
	glGenTextures(1, &this->ID);
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	this->path = path;
}

void	Texture::use(int offset)
{
	glActiveTexture(GL_TEXTURE0 + offset);
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

void	Texture::use(const char *uniformName, int offset, Shader *shader)
{
	shader->use();
	shader->setInt(uniformName, offset);
	this->use(offset);
}

void	Texture::use(const char *uniformName, int textureID, int offset, Shader *shader)
{
	shader->use();
	shader->setInt(uniformName, offset);
	Texture::use(textureID, offset);
}

void	Texture::use(int textureID, int offset)
{
	glActiveTexture(GL_TEXTURE0 + offset);
	glBindTexture(GL_TEXTURE_2D, textureID);	
}

void	Texture::reset(void)
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
