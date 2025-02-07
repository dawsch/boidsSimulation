#pragma once

#include "glm.hpp"
#include "Render_Utils.h"
#include "Shader_Loader.h"

extern Core::Shader_Loader shaderLoader;

class Terrain
{
public:
	//int verticesNumber;
	//float* vertices;
	//unsigned int* indices;
	aiMesh mesh;
	Core::RenderContext context;
	//GLuint program;
	Terrain();
	//void render(glm::mat4 cameraMatrix, glm::mat4 modelMatrix, glm::mat4 perspectiveMatrix);
};