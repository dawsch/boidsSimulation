#pragma once

#include "glm.hpp"
#include "Render_Utils.h"

class Boid
{
public:
	int id;
	//float perceptionRadias;
	float maxForce = 0.16f;
	float maxSpeed = 1.4f;
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 scale;
	glm::vec3 color = glm::vec3(1.0, 0.4, 0.0);
	glm::mat4 getModel();
	Core::RenderContext context;
	Boid(int id, float perceptionRadias, glm::vec2 startPositionBoundes);
	void update(Boid* boids, int boidsNumber);
	void flock(Boid* boids, int boidsNumber, float alignF, float cohesionF, float separationF, float perceptionRadias);
	void checkEdges(glm::vec2 boundies);
	void collisons(Boid* boids, int boidsNumber);
private:
	glm::vec3 align(Boid* boids, int boidsNumber, float perceptionRadias);
	glm::vec3 cohesion(Boid* boids, int boidsNumber, float perceptionRadias);
	glm::vec3 seperation(Boid* boids, int boidsNumber, float perceptionRadias);
};

