/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UIElement.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 12:42:04 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/29 14:04:39 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UIELEMENT_HPP
# define UIELEMENT_HPP

# include "libs.hpp"
# include "TextureManager.hpp"
# include "ShaderManager.hpp"
# include "Window.hpp"
# include "Font.hpp"

extern Window	*WINDOW;

extern unsigned int	UIquadVAO;
extern unsigned int	UIquadVBO;

enum UIAnchor
{
	UI_NONE, //Not anchored
	UI_TOP_LEFT, //Anchored on the top left of the screen
	UI_TOP_RIGHT, //Anchored on the top right of the screen
	UI_TOP_CENTER, //Anchored on the top center of the screen
	UI_TOP_CENTER_HALF, //Anchored on the half top of the screen
	UI_BOTTOM_CENTER_HALF, //Anchored on the half bottom of the screen
	UI_CENTER, //Anchored on the center of the screen
	UI_CENTER_LEFT, //Anchored on the center left of the screen
	UI_CENTER_RIGHT, //Anchored on the center right of the screen
	UI_BOTTOM_LEFT, //Anchored on the bottom left of the screen
	UI_BOTTOM_RIGHT, //Anchored on the bottom left of the screen
	UI_BOTTOM_CENTER, //Anchored on the bottom left of the screen
	UI_CENTER_HALF_LEFT, //Anchored on the half left of the screen
	UI_CENTER_HALF_RIGHT //Anchored on the half right of the screen
};

enum UIElementType
{
	UITYPE_UNSET,
	UITYPE_BUTTON,
	UITYPE_IMAGE,
	UITYPE_SLIDER,
	UITYPE_TEXT,
	UITYPE_TEXTBOX,
	UITYPE_TOGGLE,
};

class	UIElement
{
	public:
		UIElement()
		{
			type = UIElementType::UITYPE_UNSET;
			this->_shader = SHADER_MANAGER->get("gui");
		}
		virtual ~UIElement(){}
		virtual void	draw() = 0;
		/*
			@brief update button's variables

			Updates button's variables like its current used texture (normal/pressed)
		*/
        virtual void	update(glm::vec2 mousePos, bool mousePressed) = 0;
		/*
			Returns wether the mouse is in the button or not
		*/
		bool	isInside(glm::vec2 buttonPos, glm::vec2 size, glm::vec2 mousePos);
		/*
			Simple function to init the quad "model" used for all buttons
			If the quad is already loaded, just returns
		*/
		static void	initButtonQuad();
		void	anchorPos();
		void	setID(const std::string &id)
		{
			this->id = id;
		}
		/*
			Simply draws a quad using any shader. Its up the the caller to define the uniforms used by the shader outside of this function
		*/
		static void	draw(Shader *shader, glm::vec2 pos, glm::vec2 size)
		{
			initButtonQuad();

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
			model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
			glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

			shader->use();
			shader->setMat4("model", model);
			shader->setMat4("projection", projection);

			glBindVertexArray(UIquadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}

		glm::vec2	offset;
		glm::vec2	pos;
		glm::vec2	size;
		std::string	id; //ID of the ui element inside of an interface (Set by the interface).
		UIAnchor	anchor = UIAnchor::UI_NONE;
		Shader		*_shader;
		UIElementType	type;
};

# include "Button.hpp"
# include "Text.hpp"
# include "Slider.hpp"
# include "Image.hpp"
# include "Toggle.hpp"
# include "TextBox.hpp"

#endif
