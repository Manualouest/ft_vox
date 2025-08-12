/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Toggle.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 13:07:11 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 20:01:50 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOGGLE_HPP
# define TOGGLE_HPP

struct	ToggleInfo
{
	void				*data;
	bool				pressed;
	const std::string	id;
	const std::string	label;
};

class	Toggle : public UIElement
{
	public:
		Toggle(UIAnchor anchor, std::string label, glm::vec2 offset, glm::vec2 size, std::function<void(ToggleInfo)> onClick, void *clickData)
		{
			type = UIElementType::UITYPE_TOGGLE;
			this->label = label;
			this->offset = offset;
			this->pos = glm::vec2(0);
			this->size = size;
			this->onClick = onClick;
			this->clickData = clickData;
			this->anchor = anchor;

			anchorPos();
		}
		~Toggle(){}

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

			float	labelWidth = this->label.size() * 15;
			float	labelHeight = 15;

			glm::vec2	buttonCenter;
			buttonCenter.x = (this->pos.x + this->size.x / 2.f) - labelWidth / 2.f;
			buttonCenter.y = (this->pos.y + this->size.y / 2.f) - labelHeight / 2.f;

			FONT->putString(this->label, buttonCenter, glm::vec2(1, 1), glm::vec3(1), false, false);
		}
		void	update(glm::vec2 mousePos, bool mousePressed)
		{
			bool inside = isInside(this->pos, this->size, mousePos);

			if (pressed)
				this->currentTexture = TEXTURE_MANAGER->get(COBBLESTONE_TEXTURE_PATH);
			else
				this->currentTexture = TEXTURE_MANAGER->get(STONE_TEXTURE_PATH);

			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();

			if (mousePressed && !this->previousMousePressed)
    			this->wasPressedInside = inside;

    		else
    		{
    			if (this->wasPressedInside && inside)
    				click();
    			this->wasPressedInside = false;
    		}

			this->previousMousePressed = mousePressed;
		}
		void	setClickData(void *data)
		{
			this->clickData = data;
		}
		void	click()
		{
			pressed = !pressed;
			if (onClick)
				this->onClick({clickData, pressed, id, label});
		}

		bool						wasPressedInside = false;
		bool						previousMousePressed = false;
		bool						pressed = false;

		std::function<void(ToggleInfo)>	onClick = NULL;
		void						*clickData = NULL;

		std::string					label;

		Texture						*currentTexture = NULL;
};

#endif
