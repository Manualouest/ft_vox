// void	drawDepthTex(glm::vec3 offset, const unsigned int texture)
// {
// 	glDisable(GL_DEPTH_TEST);
// 	glDisable(GL_CULL_FACE);
// 	Shader	*test = SHADER_MANAGER->get("test");
// 	CAMERA->setViewMatrix(*test);

// 	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(160 + 16, 128 + 16, 0.0f) * offset);
//     model = glm::scale(model, glm::vec3(160, 128, 1.0f));

//     glm::mat4 projection = glm::ortho(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
				
// 	test->setMat4("projection", projection);
// 	test->setMat4("model", model);

// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, texture);

// 	glBindVertexArray(quadVAO);
// 	glDrawArrays(GL_TRIANGLES, 0, 6);

// 	glEnable(GL_DEPTH_TEST);
// 	glEnable(GL_CULL_FACE);
// }

class	Chunk
{
	public:
		Chunk(glm::vec3 pos): terrain(pos), water(pos)
		{
			terrain.generatePlane(16.f, 16.f);
			terrain.setScale(glm::vec3(2, 1, 2));
			terrain.upload();
			water.generatePlane(16.f, 16.f);
			water.setScale(glm::vec3(2, 1, 2));
			water.upload();
			this->pos = pos;
		}
		~Chunk(){}
		void	drawTerrain(Shader *shader)
		{
			shader->use();
			TEXTURE_MANAGER->get("textures/dirt.bmp")->use(0);
			TEXTURE_MANAGER->get("textures/stone.bmp")->use(1);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, TERRAIN_DEPTH_BUFFER->getTexture());
			terrain.draw(shader);
		}
		void	drawWater(Shader *shader)
		{
			shader->use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, TERRAIN_DEPTH_BUFFER->getTexture());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, WATER_DEPTH_BUFFER->getTexture());
			water.draw(shader);
		}
		glm::vec3	getPos() {return (this->pos);}
		bool		isInRange()
		{
			return (glm::length(CAMERA->pos - (this->pos + glm::vec3(16.f, 0.f, 16.f))) < RENDER_DISTANCE - 64.f);
		}
		float		getDistance() const
		{
			return (glm::length(CAMERA->pos - (this->pos + glm::vec3(16.f, 0.f, 16.f))));
		}
	private:
		Mesh	terrain;
		Mesh	water;
		glm::vec3	pos;
};

class	Region
{
	public:
		Region(glm::vec3 pos)
		{
			this->pos = pos;
			chunks.resize(4 * 4);
		}
		~Region()
		{
			for (auto *chunk : chunks)
				delete chunk;
		}
		void	generate()
		{
			int	chunk = 0;
			for (int x = 0; x < 4; x++)
				for (int y = 0; y < 4; y++)
					chunks[chunk++] = new Chunk((this->pos * glm::vec3(4.f) * glm::vec3(32.f)) + glm::vec3(x * 32.f, 0, y * 32.f));
		}
		void	drawTerrain(Shader *shader)
		{
			for (auto *chunk : visibleChunks)
				if (chunk->isInRange())
					chunk->drawTerrain(shader);
		}
		void	drawWater(Shader *shader)
		{
			sortChunks();
			for (auto *chunk : visibleChunks)
				if (chunk->isInRange())
					chunk->drawWater(shader);
		}
		void	drawBoth(Shader *terrainShader, Shader *waterShader)
		{
			sortChunks();
			for (auto *chunk : visibleChunks)
				if (chunk->isInRange())
				{
					chunk->drawTerrain(terrainShader);
					chunk->drawWater(waterShader);
				}
		}
		bool isInRange(const glm::vec3& cameraPos) const
		{
			glm::vec3 center = (this->pos * glm::vec3(4.f) * glm::vec3(32.f)) + glm::vec3(64.0f, 0.0f, 64.0f);
			float distance = glm::length(cameraPos - center);
			
			return (distance < RENDER_DISTANCE + 64.0f);
		}
		float	getDistance() const
		{
			glm::vec3 center = (this->pos * glm::vec3(4.f) * glm::vec3(32.f)) + glm::vec3(64.0f, 0.0f, 64.0f);
			float distance = glm::length(CAMERA->pos - center);
			
			return (distance);
		}
		void	computeVisibility()
		{
			visibleChunks.clear();
			visibleChunks.reserve(chunks.size());
			for (auto *chunk : chunks)
				if (chunk->isInRange())
					visibleChunks.emplace_back(chunk);
			visibleChunks.shrink_to_fit();
			sortChunks();
		}
	private:
		void	sortChunks()
		{
			std::sort(visibleChunks.begin(), visibleChunks.end(),
			[](const Chunk *cp1, const Chunk *cp2)
			{
				return (cp1->getDistance() > cp2->getDistance());
			});
		}
		std::vector<Chunk *>	chunks;
		std::vector<Chunk *>	visibleChunks;
		glm::vec3				pos;
};

class	World
{
	public:
		World()
		{
			consoleLog("Generating terrain", NORMAL);
			for (uint x = 0; x < WORLD_SIZE; x++)
			{
				for (uint y = 0; y < WORLD_SIZE; y++)
				{
					regions.push_back(new Region(glm::vec3(x, 0, y)));
					regions.back()->generate();
				}
			}
			consoleLog("Generating terrain, done", SUCCESS);
		}
		~World()
		{
			visibleRegions.clear();
			for (auto *region : regions)
				delete region;
		}
		void	computeVisibility()
		{
			visibleRegions.clear();
			visibleRegions.reserve(regions.size());
			for (auto *region : regions)
				if (region->isInRange(CAMERA->pos))
				{
					region->computeVisibility();
					visibleRegions.emplace_back(region);
				}
			visibleRegions.shrink_to_fit();
			sortChunks();
		}
		void	drawTerrain()
		{
			for (auto *region : visibleRegions)
				region->drawTerrain(SHADER_MANAGER->get("mesh"));
		}
		void	drawWater()
		{
			for (auto *region : visibleRegions)
				region->drawWater(SHADER_MANAGER->get("water"));
		}
		void	drawBoth()
		{
			for (auto *region : visibleRegions)
				region->drawBoth(SHADER_MANAGER->get("mesh"), SHADER_MANAGER->get("water"));
		}
	private:
		void	sortChunks()
		{
			std::sort(visibleRegions.begin(), visibleRegions.end(),
			[](const Region *cp1, const Region *cp2)
			{
				return (cp1->getDistance() > cp2->getDistance());
			});
		}
		std::vector<Region *>	regions;
		std::vector<Region *>	visibleRegions;
};

class	Mesh
{
	public:
		Mesh()
		{
			model = glm::mat4(1.0);
			pos = glm::vec3(0);
			scale = glm::vec3(1);
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
		}
		Mesh(glm::vec3 pos): Mesh()
		{
			this->pos = pos;
		}
		~Mesh()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}
		void	addVertice(const glm::vec3 &pos)
		{
			vertices.push_back(pos);
		}
		void	addIndex(unsigned int index)
		{
			indices.push_back(index);
		}
		void	upload()
		{
			glBindVertexArray(VAO);
		
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
		
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}
		void	draw(Shader	*shader)
		{
			shader->use();

			model = glm::mat4(1.f);

			model = glm::translate(model, pos);

			model = glm::scale(model, scale);

			shader->setMat4("model", model);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		void generatePlane(int sizeX, int sizeZ)
		{
			vertices.clear();
			indices.clear();

			for (int z = 0; z <= sizeZ; z++)
				for (int x = 0; x <= sizeX; x++)
					vertices.emplace_back(glm::vec3(x, 0, z));

			for (int z = 0; z < sizeZ; z++)
			{
				for (int x = 0; x < sizeX; x++)
				{
					int start = z * (sizeX + 1) + x;
					int nextRow = start + sizeX + 1;

					// Top triangle
					addIndex(start);
					addIndex(start + 1);
					addIndex(nextRow);

					// Bottom triangle
					addIndex(start + 1);
					addIndex(nextRow + 1);
					addIndex(nextRow);
				}
			}
		}
		void generateCube()
		{
			vertices.clear();
			indices.clear();
		
			// Cube vertices (same as before)
			vertices = {
				{-0.5f, -0.5f, -0.5f}, // 0
				{ 0.5f, -0.5f, -0.5f}, // 1
				{ 0.5f,  0.5f, -0.5f}, // 2
				{-0.5f,  0.5f, -0.5f}, // 3
				{-0.5f, -0.5f,  0.5f}, // 4
				{ 0.5f, -0.5f,  0.5f}, // 5
				{ 0.5f,  0.5f,  0.5f}, // 6
				{-0.5f,  0.5f,  0.5f}  // 7
			};
		
			// Fixed clockwise winding for each face (when viewed from outside)
			indices = {
				// Back face
				0, 1, 2,
				2, 3, 0,
				// Front face
				6, 5, 4,
				4, 7, 6,
				// Left face
				7, 4, 0,
				0, 3, 7,
				// Right face
				1, 5, 6,
				6, 2, 1,
				// Bottom face
				0, 4, 5,
				5, 1, 0,
				// Top face
				3, 2, 6,
				6, 7, 3
			};
		}
		glm::vec3	getPos(){return (this->pos);}
		glm::vec3	getScale(){return (this->scale);}
		void	setPos(glm::vec3 pos){this->pos = pos;}
		void	setScale(glm::vec3 scale){this->scale = scale;}
	private:
		std::vector<glm::vec3>	vertices;
		std::vector<unsigned int>	indices;
		glm::vec3		pos;
		glm::vec3		scale;
		unsigned int	VAO;
		unsigned int	VBO;
		unsigned int	EBO;
		glm::mat4		model;
};
