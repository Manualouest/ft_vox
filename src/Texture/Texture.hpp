/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Texture.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbatty <mbatty@student.42angouleme.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 13:48:51 by mbatty            #+#    #+#             */
/*   Updated: 2025/07/09 15:05:56 by mbatty           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TEXTURE_HPP
# define TEXTURE_HPP

# include "libs.hpp"
# include "Shader.hpp"

class	Texture
{
	public:
		~Texture();
		Texture(void)
		{
			this->ID = 0;
		}
		Texture(const char *path);
		Texture(const Texture &cpy)
		{
			this->ID = cpy.ID;
		}
		Texture(const int ID)
		{
			this->ID = ID;
		}
		//Takes ID from cpy (rendering it useless) its pretty much a moove assignor
		void	cut(Texture &cpy);
		void	LoadImage(const char *path);
		void	use(int offset);
		void	use(const char *uniformName, int offset, Shader *shader);
		
		static void	use(int textureID, int offset);
		static void	use(const char *uniformName, int textureID, int offset, Shader *shader);
		static void	reset(void);

		uint	getID() {return (this->ID);}
		std::vector<unsigned char>	&getData() {return (this->data);}
		int	getWidth() {return (this->width);}
		int	getHeight() {return (this->height);}
		
	private:
		unsigned int				ID;
		std::vector<unsigned char>	data;
		int							width;
		int							height;
		std::string					path;
};

#endif
