#version 430 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec3 worldPos;


void main()
{
	worldPos = (modelMatrix* vec4(vertexPosition,1)).xyz;
	gl_Position = transformation * vec4(vertexPosition, 1.0);

}
