#pragma once

#include "glm.hpp"
#include "Render_Utils.h"

class Terrain
{
public:
	float* vertices;
	unsigned int* indices;
	Core::RenderContext context;
	GLuint program;
	Terrain();
	void render();
};