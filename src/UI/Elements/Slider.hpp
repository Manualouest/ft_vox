/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Slider.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 13:36:46 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 20:01:28 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SLIDER_HPP
# define SLIDER_HPP

# include "UIElement.hpp"

/*
	@brief	Represents a simple slider UI element

	When initialized the slider will use basic textures/shaders from the Texture/ShaderManager, use the setter functions to use custom shaders
*/
class	Slider : public UIElement
{
	public:
		Slider(UIAnchor anchor, std::string label, glm::vec2 offset, glm::vec2 size, std::function<void(float)> onChange, std::function<void(Slider *)> onUpdate, float startValue)
		{
			type = UIElementType::UITYPE_SLIDER;
			_buttonShader = SHADER_MANAGER->get("gui");
			backgroundTexture = TEXTURE_MANAGER->get(DIRT_TEXTURE_PATH);
			currentTexture = TEXTURE_MANAGER->get(STONE_TEXTURE_PATH);
			this->label = label;
			this->offset = offset;
			this->pos = glm::vec2(0);
			this->size = size;
			this->onChange = onChange;
			this->onUpdate = onUpdate;
			this->anchor = anchor;
			setSlider(startValue);
		}
		~Slider(){}
		void	draw()
		{
			initButtonQuad();

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(this->pos.x, this->pos.y, 0.0f));
			model = glm::scale(model, glm::vec3(this->size.x, this->size.y, 1.0f));

			glm::mat4 sliderModel = glm::translate(glm::mat4(1.0f), glm::vec3(this->sliderPos.x, this->sliderPos.y, 0.0f));
			sliderModel = glm::scale(sliderModel, glm::vec3(this->sliderWidth, this->size.y, 1.0f));

			glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

			_buttonShader->use();
			_buttonShader->setMat4("projection", projection);

			glBindVertexArray(UIquadVAO);

			this->backgroundTexture->use(0);
			_buttonShader->setMat4("model", model); //Draws the background
			glDrawArrays(GL_TRIANGLES, 0, 6);

			this->currentTexture->use(0);
			_buttonShader->setMat4("model", sliderModel); //Draws the slider
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
			if (this->anchor != UI_NONE)
			{
				anchorPos();
				setSlider(value);
			}

			if (onUpdate)
				onUpdate(this);

			float	minCenter = pos.x + sliderWidth * 0.5f;
    		float	maxCenter = pos.x + this->size.x - sliderWidth * 0.5f;
			bool	insideSlider = isInside(this->sliderPos, glm::vec2(sliderWidth, this->size.y), mousePos);
			bool	insideButton = isInside(this->pos, size, mousePos);
			float	previousValue = value;

			this->currentTexture = TEXTURE_MANAGER->get(STONE_TEXTURE_PATH);;

			if (mousePressed && !this->previousMousePressed)
			{
				if (insideSlider || insideButton)
					this->dragging = true;
			}

			if (mousePressed && this->dragging)
			{
				this->currentTexture = TEXTURE_MANAGER->get(COBBLESTONE_TEXTURE_PATH);;
				this->sliderPos.x = mousePos.x - (sliderWidth / 2);
			}

			if (!mousePressed)
				this->dragging = false;

			sliderPos.x = glm::clamp(sliderPos.x, pos.x, pos.x + size.x - (sliderWidth));
			this->previousMousePressed = mousePressed;
			float sliderCenter = sliderPos.x + (sliderWidth / 2);
    		float normalizedValue = (sliderCenter - minCenter) / (maxCenter - minCenter);
    		normalizedValue = glm::clamp(normalizedValue, 0.0f, 1.0f);

    		value = normalizedValue;
			if (value != previousValue)
				onChange(value);
		}
		void    setSlider(float value)
		{
			value = glm::clamp(value, 0.0f, 1.0f);
			this->value = value;

			float minCenter = pos.x + (sliderWidth / 2);
			float maxCenter = pos.x + size.x - (sliderWidth / 2);
			float centerX = minCenter + value * (maxCenter - minCenter);

			sliderPos.x = centerX - (sliderWidth / 2);
			sliderPos.y = pos.y;
		}

		void	setLabel(const std::string &newLabel)
		{
			this->label = newLabel;
		}

		bool						wasPressedInside = false;
		bool						previousMousePressed = false;
		bool						dragging = false;

		std::function<void(float)>	onChange = NULL;
		std::function<void(Slider *)>	onUpdate = NULL;
		void								*clickData = NULL;

		Texture						*currentTexture;
		Texture						*backgroundTexture;

		glm::vec2						sliderPos;
		float						sliderWidth = 30;
		float						value = 0;
		std::string					label;

		Shader						*_buttonShader;
};

#endif
