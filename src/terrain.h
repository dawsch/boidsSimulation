#pragma once

#include "glm.hpp"
#include "Render_Utils.h"
#include "Shader_Loader.h"

extern Core::Shader_Loader shaderLoader;

class Terrain
{
public:
	aiMesh mesh;
	Core::RenderContext context;
	Terrain();
};