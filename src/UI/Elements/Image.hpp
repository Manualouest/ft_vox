/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Image.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 17:43:25 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/25 20:00:05 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IMAGE_HPP
# define IMAGE_HPP

# include "UIElement.hpp"

class	Image : public UIElement
{
	public:
		Image(UIAnchor anchor, glm::vec2 offset, glm::vec2 size)
		{
			type = UIElementType::UITYPE_IMAGE;
			this->offset = offset;
			this->pos = glm::vec2(0);
			this->size = size;
			this->anchor = anchor;

			anchorPos();
		}
		~Image(){}

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
		}
		void	update(glm::vec2, bool)
		{
			this->currentTexture = TEXTURE_MANAGER->get(TITLE_TEXTURE_PATH);;
			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();
		}

		bool						wasPressedInside = false;
		bool						previousMousePressed = false;

		Texture						*currentTexture = NULL;
};

#endif
