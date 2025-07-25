#ifndef SKYBOX_HPP
# define SKYBOX_HPP

# include "libs.hpp"
# include "Texture.hpp"
# include "Camera.hpp"
# include "Shader.hpp"
# include "ShaderManager.hpp"

class	Skybox
{
	public:
		~Skybox();
		Skybox(const std::vector<std::string> &faces);
		void	draw(Camera &camera);

	private:
		unsigned int		ID;
		unsigned int		VAO;
		unsigned int		VBO;
		glm::mat4			model;
		Shader				*_shader;
};

#endif
