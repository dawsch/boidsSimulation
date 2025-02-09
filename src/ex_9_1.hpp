#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"

#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
//#include "Texture.h"

#include "terrain.h"
#include "Boid.h"
//#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

#include "../imGUI/imgui_impl_glfw.h"

#include "../imGUI/imgui.h"
#include "../imGUI/imgui_impl_glfw.h"
#include "../imGUI/imgui_impl_opengl3.h"

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

int WIDTH = 900, HEIGHT = 700;

namespace models {
	Core::RenderContext spaceshipContext;
	Core::RenderContext sphereContext;
}

GLuint depthMapFBO;
GLuint depthMap;

GLuint program;
GLuint programSun;
GLuint programTest;
GLuint programTex;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

glm::vec3 sunPos = glm::vec3(0, 0, 0);
//glm::vec3 sunDir = glm::vec3(-0.93633f, 0.351106, 0.003226f);
glm::vec3 sunDir = glm::vec3(0.0f, -1.0f, 0.0f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f)*5;

//glm::vec3 cameraPos = glm::vec3(0.479490f, 0.750000f, -2.124680f);
//glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);


void mouse_callback(GLFWwindow* window, double xpos, double ypos);

glm::vec3 cameraPos = glm::vec3(30.0f, 30.0f, 30.0f);
glm::vec3 target = glm::vec3(70.0f, 100.0f, 20.0f);
glm::vec3 cameraFront = glm::normalize(target - cameraPos);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 startDir = glm::normalize(target - cameraPos);

int rdFieldOfView = 45;

float lastX = 400, lastY = 300, yaw = -90.0f, pitch = 0.0f;

bool firstMouse = true;

bool mouseControle = false;

glm::vec2 operationsBoxSize = glm::vec2(10.0f, 400.0f);


glm::vec3 spaceshipPos = glm::vec3(0.065808f, 1.250000f, -2.189549f);
glm::vec3 spaceshipDir = glm::vec3(-0.490263f, 0.000000f, 0.871578f);
GLuint VAO,VBO;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 pointlightPos = glm::vec3(0, 2, 0);
glm::vec3 pointlightColor = glm::vec3(0.9, 0.6, 0.6);

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9)*3;
float spotlightPhi = 3.14 / 4;


const int boidsNumber = 150;
Boid* boids[boidsNumber];
Terrain* terrain;

float alignFactor = 2.4f, cohesionFactor = 0.3f, separationFactor = 4.0f;
float perceptionRadias = 30.0f;


float currentFPS;
float lastFrame = 0.0f;
float lastTime = -1.f;
float deltaTime = 0.f;

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}
glm::mat4 createCameraMatrix()
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 createPerspectiveMatrix()
{

	return glm::perspective(glm::radians((float)rdFieldOfView), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);
}

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);

	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);

}
glm::mat4 getLightSpaceMatrix() {
	float near_plane = 1.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	glm::mat4 lightView = glm::lookAt(sunPos, sunPos + sunDir, glm::vec3(0.0f, 1.0f, 0.0f));
	return lightProjection * lightView;
}

void renderShadowapSun() {
	glm::mat4 lightSpaceMatrix = getLightSpaceMatrix();

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Use a simple shader for depth rendering
	glUseProgram(programTest); // Ensure you have a simple shader for depth rendering

	// Render the scene from the light's perspective
	for (int i = 0; i < boidsNumber; i++) {
		glm::mat4 model = boids[i]->getModel();
		glm::mat4 transformation = lightSpaceMatrix * model;
		glUniformMatrix4fv(glGetUniformLocation(programTest, "transformation"), 1, GL_FALSE, glm::value_ptr(transformation));
		Core::DrawContext(boids[i]->context);
	}

	// Render terrain
	glm::mat4 terrainModel = glm::mat4(1.0f);
	glm::mat4 terrainTransformation = lightSpaceMatrix * terrainModel;
	glUniformMatrix4fv(glGetUniformLocation(programTest, "transformation"), 1, GL_FALSE, glm::value_ptr(terrainTransformation));
	Core::DrawContext(terrain->context);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}

void renderScene(GLFWwindow* window)
{

	glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float time = glfwGetTime();
	updateDeltaTime(time);

	// Render the depth map from the light's perspective
	renderShadowapSun();

	// Bind the depth map texture and pass the light space matrix to the shader
	glUseProgram(program); // Use the main shader program
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, depthMap); // Bind the depth map texture
	glUniform1i(glGetUniformLocation(program, "shadowMap"), 0); // Set the shadowMap uniform to texture unit 0

	// Calculate and pass the light space matrix to the shader
	glm::mat4 lightSpaceMatrix = getLightSpaceMatrix();
	glUniformMatrix4fv(glGetUniformLocation(program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	// Render the space lamp
	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x / 2, sunColor.y / 2, sunColor.z / 2);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(sphereContext);

	// Switch back to the main shader program
	glUseProgram(program);

	// Render objects with PBR lighting and shadows
	drawObjectPBR(sphereContext, glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), glm::vec3(0.2, 0.7, 0.3), 0.3, 0.0);

	drawObjectPBR(sphereContext,
		glm::translate(pointlightPos) * glm::scale(glm::vec3(0.1)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(0.5, 0.5, 0.5), 0.7, 0.0);

	// Update and render boids
	for (int i = 0; i < boidsNumber; i++)
	{
		boids[i]->flock(*boids, boidsNumber, alignFactor, cohesionFactor, separationFactor, perceptionRadias);
		boids[i]->update(*boids, boidsNumber);
		boids[i]->checkEdges(operationsBoxSize);
		drawObjectPBR(boids[i]->context,
			boids[i]->getModel(),
			boids[i]->color,
			0.8, 0.1
		);
	}

	// Render terrain
	drawObjectPBR(terrain->context, glm::mat4(1.0f), glm::vec3(0.5f, 0.7f, 0.9f), 0.4f, 0.1f);

	// Update spotlight position and direction
	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;

	// Calculate delta time for FPS
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Render ImGui UI
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::SetNextWindowBgAlpha(0.8f);

	currentFPS = 1 / deltaTime;
	ImGui::Begin("Window");
	ImGui::Text("FPS:");
	ImGui::SameLine();
	ImGui::Text("%s", std::to_string(currentFPS).c_str());

	ImGui::Text("align:     ");
	ImGui::SameLine();
	ImGui::SliderFloat("##align", &alignFactor, 0, 3);

	ImGui::Text("cohesion:  ");
	ImGui::SameLine();
	ImGui::SliderFloat("##cohesion", &cohesionFactor, 0, 3);

	ImGui::Text("separation:");
	ImGui::SameLine();
	ImGui::SliderFloat("##separation", &separationFactor, 0, 4);

	ImGui::Text("perception radius:");
	ImGui::SameLine();
	ImGui::SliderFloat("##perception", &perceptionRadias, 0, 100);

	// Add a checkbox to toggle shadow mapping
	static bool enableShadows = true; // Default to shadows enabled
	ImGui::Checkbox("Enable Shadows", &enableShadows);

	// Pass the shadow toggle state to the shader
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "enableShadows"), enableShadows ? 1 : 0);

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Swap buffers
	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_9_1.vert", "shaders/shader_9_1.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_8_sun.vert", "shaders/shader_8_sun.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);


	for (int i = 0; i < boidsNumber; i++)
	{
		boids[i] = new Boid(i, 30.0f, operationsBoxSize);
		boids[i]->context = shipContext;
	}

	terrain = new Terrain();
	yaw = glm::degrees(atan2(startDir.z, startDir.x));  // Kąt azymutu
	pitch = glm::degrees(asin(startDir.y));        // Kąt elewacji

	// Create depth map FBO
	glGenFramebuffers(1, &depthMapFBO);

	// Create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

//obsluga wejscia
bool cameraButtonPressed = false;
void processInput(GLFWwindow* window)
{
	float cameraSpeed = 25.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && cameraButtonPressed == false)
	{
		cameraButtonPressed = true;
		if (!mouseControle)
		{
			mouseControle = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			mouseControle = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE && cameraButtonPressed == true)
	{
		cameraButtonPressed = false;
		firstMouse = true;
	}
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (mouseControle)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);
	}

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glfwPollEvents();
		renderScene(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
//}