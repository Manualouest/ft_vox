#ifndef SKYBOX_HPP
# define SKYBOX_HPP

# include "libs.hpp"
# include "Texture.hpp"
# include "Camera.hpp"
# include "Shader.hpp"
# include "ShaderManager.hpp"

#define DAY glm::vec3(0.75, 0.75, 0.75)
#define SUNSET glm::vec3(1, 0.43529411764705883, 0.00392156862745098)
#define NIGHT glm::vec3(0.21372549, 0.264705882, 0.329411765)

#define SKYDAYUP glm::vec3(0.4, 0.73, 1.0)
#define SKYDAYDOWN glm::vec3(0.6, 0.8, 1.0)
#define SKYSUNSETUP glm::vec3(0.5882352941176471, 0.49019607843137253, 0.9764705882352941)
#define SKYSUNSETDOWN glm::vec3(0.984313725490196, 0.5647058823529412, 0.3843137254901961)
#define SKYNIGHTUP glm::vec3(0.023529411764705882, 0.03137254901960784, 0.0392156862745098)
#define SKYNIGHTDOWN glm::vec3(0.08235294117647059, 0.10980392156862745, 0.1411764705882353)

class	Skybox
{
	public:
		~Skybox();
		Skybox(const std::vector<std::string> &faces);
		void	draw(Camera &camera);

		void		setTime(const float &newTime);
		float		getTime() const;
		glm::vec3	getDirectLightPos(const bool &getSun) const;
		glm::vec3	getDirectLightColor(const bool &getSun) const;
		glm::vec3	getAmbientLight() const;
		glm::vec3	getSkyboxColorUp() const;
		glm::vec3	getSkyboxColorDown() const;

	private:
		unsigned int	ID;
		unsigned int	VAO;
		unsigned int	VBO;
		glm::mat4		model;
		Shader			*_shader;
		float			_time = 0;
		glm::vec3		_sunPos;
		glm::vec3		_moonPos;
};

#endif
