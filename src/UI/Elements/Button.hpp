/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Button.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 18:34:43 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/29 14:10:42 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUTTON_HPP
# define BUTTON_HPP

#include "UIElement.hpp"

struct	ButtonInfo
{
	void				*data;
	const std::string	id;
	const std::string	label;
	class Button				*button;
};

/*
	@brief	Represents a simple clickable UI element

	When initialized the button will use basic textures/shaders from the Texture/ShaderManager, use the setter functions to use custom shaders
*/
class	Button : public UIElement
{
	public:
		/*
			@brief	Simple anchored button

			Constructor for anchored buttons, the pos argument will be added to the anchor point so
			use positive/negative values to move the button around its anchor

			@param	anchor      Where the button should be anchored
			@param	label       Label of the button to be displayed
			@param	offset      Offset from anchor
			@param	size        Size of the button
			@param	onClick     function to be called when button is clicked, NULL to do nothing
			@param	clickData   Data passed to the onClick function
		*/
		Button(UIAnchor anchor, std::string label, glm::vec2 offset, glm::vec2 size, std::function<void(ButtonInfo)> onClick, void *clickData)
		{
			type = UIElementType::UITYPE_BUTTON;
			this->label = label;
			this->offset = offset;
			this->pos = glm::vec2(0);
			this->size = size;
			this->onClick = onClick;
			this->clickData = clickData;
			this->anchor = anchor;

			anchorPos();
		}
		~Button(){}

		void	draw()
		{
			if (!this->currentTexture)
				return ;

			initButtonQuad();

			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(this->pos.x, this->pos.y, 0.0f));
			model = glm::scale(model, glm::vec3(this->size.x, this->size.y, 1.0f));
			glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

			_shader->use();
			_shader->setMat4("model", model);
			_shader->setMat4("projection", projection);

			this->currentTexture->use(0);

			glBindVertexArray(UIquadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			float	labelWidth = this->label.size() * 16;
			float	labelHeight = 16;

			glm::vec2	buttonCenter;
			buttonCenter.x = (this->pos.x + this->size.x / 2.f) - labelWidth / 2.f;
			buttonCenter.y = (this->pos.y + this->size.y / 2.f) - labelHeight / 2.f;

			FONT->putString(this->label, buttonCenter, glm::vec2(1, 1), glm::vec3(1), false, false);
		}
		void	update(glm::vec2 mousePos, bool mousePressed)
		{
			bool inside = isInside(this->pos, this->size, mousePos);

			this->currentTexture = TEXTURE_MANAGER->get(STONE_TEXTURE_PATH);;

			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();

			if (mousePressed && !this->previousMousePressed)
    			this->wasPressedInside = inside;

    		if (mousePressed)
    		{
    			if (inside && this->wasPressedInside)
    				this->currentTexture = TEXTURE_MANAGER->get(COBBLESTONE_TEXTURE_PATH);
    		}
    		else
    		{
    			if (this->wasPressedInside && inside && onClick)
    				this->onClick({clickData, id, label, this});
    			this->wasPressedInside = false;
    		}

			this->previousMousePressed = mousePressed;
		}
		void	setClickData(void *data)
		{
			this->clickData = data;
		}

		bool						wasPressedInside = false;
		bool						previousMousePressed = false;

		std::function<void(ButtonInfo)>	onClick = NULL;
		void						*clickData = NULL;

		std::string					label;

		Texture						*currentTexture = NULL;
};

#endif
