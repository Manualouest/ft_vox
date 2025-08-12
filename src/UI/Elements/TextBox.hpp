/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TextBox.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/20 13:17:26 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 20:01:39 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEXTBOX_HPP
# define TEXTBOX_HPP

struct	TextBoxInfo
{
	void				*data;
	const std::string	input;
	const std::string	id;
};

class	TextBox : public UIElement
{
	public:
		TextBox(UIAnchor anchor, glm::vec2 offset, glm::vec2 size, std::function<void(TextBoxInfo)> onClick, void *clickData)
		{
			type = UIElementType::UITYPE_TEXTBOX;
			this->offset = offset;
			this->pos = glm::vec2(0);
			this->size = size;
			this->onClick = onClick;
			this->clickData = clickData;
			this->anchor = anchor;

			anchorPos();
		}
		~TextBox(){}

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

			float	labelWidth = this->input.size() * 15;
			float	labelHeight = 15;

			glm::vec2	buttonCenter;
			buttonCenter.x = (this->pos.x + this->size.x / 2.f) - labelWidth / 2.f;
			buttonCenter.y = (this->pos.y + this->size.y / 2.f) - labelHeight / 2.f;

			FONT->putString(this->input, buttonCenter, glm::vec2(1, 1), glm::vec3(1), false, false);
		}
		void	update(glm::vec2 mousePos, bool mousePressed)
		{
			bool inside = isInside(this->pos, this->size, mousePos);

			if (mousePressed && !inside && pressed)
				validate();

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
				{
    				pressed = !pressed;
					if (!pressed)
						validate();
				}
    			this->wasPressedInside = false;
    		}

			this->previousMousePressed = mousePressed;
		}
		void	setClickData(void *data)
		{
			this->clickData = data;
		}
		void	charInput(unsigned int key)
		{
			(void)key;
			if (!pressed)
				return ;

			if (key >= 32 && key <= 126)
				input.insert(input.end(), 1, (char)key);
		}
		void	specialInput(int key, int action)
		{
			(void)key;(void)action;
			if (!pressed || (action != GLFW_PRESS && action != GLFW_REPEAT))
				return ;

			if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER)
				validate();
			if (key == GLFW_KEY_BACKSPACE && input.size() > 0)
				input.pop_back();
		}
		void	validate()
		{
			pressed = false;
			if (onClick)
				this->onClick({clickData, input, id});
		}
		std::string	getInput()
		{
			return (this->input);
		}
		void	clear()
		{
			input.clear();
		}

		bool						wasPressedInside = false;
		bool						previousMousePressed = false;
		bool						pressed = false;
		std::string					input;

		std::function<void(TextBoxInfo)>	onClick = NULL;
		void						*clickData = NULL;

		Texture						*currentTexture = NULL;
};

#endif
