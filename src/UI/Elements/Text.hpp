/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Text.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 11:48:31 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/16 17:37:27 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEXT_HPP
# define TEXT_HPP

# include "UIElement.hpp"

class	Text : public UIElement
{
	public:
		~Text(){}
		Text(UIAnchor anchor, std::string label, glm::vec2 offset, std::function<void(std::string&)> updatefc, bool drawBG)
		{
			this->_angle = 0;
			this->_rotation = glm::vec3(1);
			this->_scale = glm::vec2(1);
			this->_label = label;
			this->offset = offset;
			this->pos = glm::vec2(0);
			getSize();
			this->anchor = anchor;
			this->_drawBG = drawBG;
			this->_update = updatefc;
		}
		void	getSize()
		{
			this->size.x = _label.size() * 16;
			this->size.y = 16;
		}
		void	draw()
		{
			getSize();
			if (_drawBG)
				SHADER_MANAGER->get("text")->setBool("drawBackground", true);

			FONT->putString(this->_label, glm::vec2(this->pos.x, this->pos.y - this->size.y / 8), this->_scale, _rotation, _angle);
			SHADER_MANAGER->get("text")->setBool("drawBackground", false);
		}
		void	update(glm::vec2, bool)
		{
			if (_update)
				_update(_label);
			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();
		}

		void	setScale(glm::vec2 scale) {_scale = scale;}
		void	setRotation(glm::vec3 rotation) {_rotation = rotation;}
		void	setAngle(float angle) {_angle = angle;}
	private:
		glm::vec2							_scale;
		glm::vec3							_rotation;
		float								_angle;
		std::string							_label;
		bool								_drawBG;
		std::function<void(std::string&)>	_update;
};

#endif
