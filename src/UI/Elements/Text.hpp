/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Text.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 11:48:31 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/15 11:59:06 by mbatty           ###   ########.fr       */
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
			this->_label = label;
			this->offset = offset;
			this->pos = glm::vec2(0);
			getSize();
			this->anchor = anchor;
			this->_drawBG = drawBG;
			this->_update = updatefc;

			loadDefaultAssets();
		}
		void	getSize()
		{
			this->size.x = _label.size() * 15;
			this->size.y = 15;
		}
		void	loadDefaultAssets()
		{
		}
		void	draw()
		{
			getSize();
			if (_drawBG)
				SHADER_MANAGER->get("text")->setBool("drawBackground", true);
			FONT->putString(this->_label, glm::vec2(this->pos.x, this->pos.y - this->size.y / 8), this->size);
			SHADER_MANAGER->get("text")->setBool("drawBackground", false);
		}
		void	update(glm::vec2, bool)
		{
			if (_update)
				_update(_label);
			if (this->anchor != UIAnchor::UI_NONE)
				anchorPos();
		}
	private:
		std::string					_label;
		bool						_drawBG;
		std::function<void(std::string&)>	_update;
};

#endif
