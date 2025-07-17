NAME := ft_vox

OBJ_DIR := ./obj/
INCLUDE_DIRS := ./includes/ ./GLFW/include/GLFW/ ./includes/glad/. ./includes/render ./src/ ./glm/glm/ ./glm/glm/gtc/
CLASSE_HEADERS := Camera/ ChunkMaker/ FrameBuffer/ Texture/ Shader/ Camera/ Window/ Managers/ UI/ UI/Elements UI/Interfaces Terminal/ Scenes/

GLFWARCHIVE = GLFW/build/src/libglfw3.a

INCLUDE_DIRS := $(addprefix -I, $(INCLUDE_DIRS))
CLASSE_HEADERS := $(addprefix -I./src/, $(CLASSE_HEADERS))

MAKE := @make --no-print-directory

SOURCE_DIR := ./src/

OBJ_DIR := ./obj/

OBJECTS = $(SOURCES:.cpp=.o)

CPP_FILES :=	main \
				glad/glad \
				Window/Window \
				Camera/Camera \
				Shader/Shader \
				Shader/Font \
				Texture/Texture \
				Texture/Skybox \
				FrameBuffer/FrameBuffer \
				ChunkMaker/Chunk \
				ChunkMaker/RegionManager \
				ChunkMaker/ChunkGenerator \
				ChunkMaker/ChunkGeneratorManager \
				UI/UIElement \
				Terminal/Terminal \
				Terminal/Commands \
				Managers/Quadtree \
				Managers/ShaderManager \
				Managers/TextureManager \
				Scenes/TitleScreen \
				Scenes/GameScene

CPP_FILES := $(addsuffix .cpp, $(CPP_FILES))

SOURCES := $(addprefix $(SOURCE_DIR), $(CPP_FILES))

OBJECTS := $(addprefix $(OBJ_DIR), $(CPP_FILES:.cpp=.o))
DEPS := $(addprefix $(OBJ_DIR), $(CPP_FILES:.cpp=.d))

CFLAGS = -MP -MMD -g -Wall -Wextra -Werror

GLAD_PATH = libs/glad

all: glfw glad glm $(NAME)

run: all
	@./$(NAME) 42

vrun: all
	@valgrind ./$(NAME) 42

glfw:
	@if ls | grep -q "GLFW"; then \
		echo "\033[32;1;4mGLFW Found\033[0m"; \
	else \
		echo "\033[31;1;4mGLFW Not Found\033[0m"; \
		echo "\033[31;1mDownloading GLFW from github \033[0m"; \
		git clone https://github.com/glfw/glfw.git GLFW; \
		echo "\033[31;1mCompiling GLFW\033[0m"; \
		cmake -S GLFW -B GLFW/build; \
		cmake --build GLFW/build; \
	fi

glad:
	@if ls src | grep -q "glad"; then \
		echo "\033[32;1;4mglad Found\033[0m"; \
	else \
		echo "\033[31;1;4mglad Not Found\033[0m"; \
		echo "\033[31;1mDownloading glad from github \033[0m"; \
		pip install glad; \
		git clone https://github.com/Dav1dde/glad.git glad; \
		python -m glad --out-path=glad/build --generator=c; \
		mkdir -p glad2; \
		cp glad/build/include/glad/glad.h glad2/.; \
		cp glad/build/src/glad.c glad2/.; \
		rm -rf glad; \
		mv glad2 glad; \
		mv glad/glad.c glad/glad.cpp; \
		mkdir -p src/glad; \
		mkdir -p includes/glad; \
		mv glad/glad.cpp src/glad/; \
		mv glad/glad.h includes/glad/; \
		rm -rf glad; \
	fi

glm:
	@if ls | grep -q "glm"; then \
		echo "\033[32;1;4mGLM Found\033[0m"; \
	else \
		echo "\033[31;1;4mGLM Not Found\033[0m"; \
		echo "\033[31;1mCloning GLM from github\033[0m"; \
		git clone https://github.com/g-truc/glm.git glm; \
	fi

$(OBJECTS): $(OBJ_DIR)%.o : $(SOURCE_DIR)%.cpp
	@echo "\033[0;32mCompiling $<\033[0m"
	@c++ $(CFLAGS) $(CLASSE_HEADERS) $(INCLUDE_DIRS) -c $< -o $@

$(OBJ_DIR):
	@(cd $(SOURCE_DIR) && find . -type d -exec mkdir -p -- $(shell pwd)/$(OBJ_DIR){} \;)

$(NAME): $(OBJ_DIR) $(OBJECTS)
	@c++ $(OBJECTS) $(GLFWARCHIVE) -o $(NAME)
	@echo "\033[0;32mCompiled $(NAME)\033[0m"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "\033[0;32mCleaned objects\033[0m"

fclean:
	$(MAKE) clean
	@rm -f $(NAME)
	@echo "\033[0;32mCleaned $(NAME)\033[0m"

dclean: fclean
	@rm -rf src/glad
	@rm -rf includes/glad
	@rm -rf GLFW
	@rm -rf glm
	@echo "\033[0;32mCleaned external libraries\033[0m"

re: fclean all

.PHONY: all clean fclean dclean re GLFW glad run glm

-include $(DEPS)
