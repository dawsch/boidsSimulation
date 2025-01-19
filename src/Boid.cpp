#include "Boid.h"
#include "glm.hpp"
#include <gtc/matrix_transform.hpp>
#include <gtx/quaternion.hpp>
#include <gtc/random.hpp>
#include <gtc/type_ptr.hpp>
#include <random>
#include <iostream>
#include <gtx/vector_angle.hpp>

glm::vec3 getRandomVec3(float min = -1.0f, float max = 1.0f);
glm::vec3 setLength(const glm::vec3& vector, float newLength);
glm::vec3 clampLength(const glm::vec3& vector, float maxLength);
glm::quat computeRotation(const glm::vec3& velocity, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));
float calculateAngleToBoid(glm::vec3& boidPosition, glm::vec3& boidVelocity, glm::vec3& otherBoidPosition);

Boid::Boid(int id, float perceptionRadias, glm::vec2 startPositionBoundes)
{
	
	this->id = id;
	this->perceptionRadias = perceptionRadias;
	this->position = getRandomVec3(startPositionBoundes.x, startPositionBoundes.y);
	this->velocity = getRandomVec3(-0.8f, 0.8f);
	this->acceleration = glm::vec3(0.0f);
	this->scale = glm::vec3(0.05f);
}
void Boid::update(Boid* boids, int boidsNumber)
{
	this->position = this->position + this->velocity;
	this->velocity = this->velocity + this->acceleration;
	this->velocity = clampLength(this->velocity, this->maxSpeed);
	this->acceleration = glm::vec3(0.0f);
}
void Boid::checkEdges(glm::vec2 boundies)
{
	if (this->position.x < boundies.x)
		this->position.x = boundies.y;
	if (this->position.x > boundies.y)
		this->position.x = boundies.x;

	if (this->position.y < boundies.x)
		this->position.y = boundies.y;
	if (this->position.y > boundies.y)
		this->position.y = boundies.x;

	if (this->position.z < boundies.x)
		this->position.z = boundies.y;
	if (this->position.z > boundies.y)
		this->position.z = boundies.x;
}

glm::vec3 Boid::align(Boid* boids, int boidsNumber)
{
	glm::vec3 steering = glm::vec3(0.0f);
	int total = 0;
	for (int i = 0; i < boidsNumber; i++)
	{
		if (boids[i].id != this->id)
		{
			float dis = glm::distance(this->position, boids[i].position);
			if (dis <= perceptionRadias && dis != 0)
			{
				if (calculateAngleToBoid(this->position, this->velocity, boids[i].position) < 3)
				{
					steering = steering + boids[i].velocity;
					total++;
				}
				
			}
		}
	}
	if (total > 0)
	{
		steering = steering / (float)total;
		steering = setLength(steering, this->maxSpeed);
		steering = steering - this->velocity;
		steering = clampLength(steering, this->maxForce);
	}
	if (glm::isinf(steering.x) || glm::isinf(steering.y) || glm::isinf(steering.z) ||
		glm::isnan(steering.x) || glm::isnan(steering.z) || glm::isnan(steering.z))
		return glm::vec3(0.0f);
	return steering;
}
glm::vec3 Boid::cohesion(Boid* boids, int boidsNumber)
{
	glm::vec3 steering = glm::vec3(0.0f);
	int total = 0;
	for (int i = 0; i < boidsNumber; i++)
	{
		if (boids[i].id != this->id)
		{
			float dis = glm::distance(this->position, boids[i].position);
			if (dis <= perceptionRadias * 2.0f && dis != 0)
			{
				if (calculateAngleToBoid(this->position, this->velocity, boids[i].position) < 3)
				{
					steering = steering + boids[i].position;
					total++;
				}
			}
		}
	}
	if (total > 0)
	{
		this->color = glm::vec3(1.0f, 0.0f, 0.0f);
		steering = steering / (float)total;
		steering = steering - this->position;
		steering = setLength(steering, this->maxSpeed);
		steering = steering - this->velocity;
		steering = clampLength(steering, this->maxForce);
	}
	if (glm::isinf(steering.x) || glm::isinf(steering.y) || glm::isinf(steering.z) ||
		glm::isnan(steering.x) || glm::isnan(steering.z) || glm::isnan(steering.z))
		return glm::vec3(0.0f);
	return steering;
}
glm::vec3 Boid::seperation(Boid* boids, int boidsNumber)
{
	glm::vec3 steering = glm::vec3(0.0f);
	int total = 0;
	for (int i = 0; i < boidsNumber; i++)
	{
		if (boids[i].id != this->id)
		{
			float dis = glm::distance(this->position, boids[i].position);
			if (dis <= perceptionRadias * 0.9f && dis != 0)
			{
				if (calculateAngleToBoid(this->position, this->velocity, boids[i].position) < 3)
				{
					glm::vec3 diff = this->position - boids[i].position;
					diff = diff / (dis * dis);
					steering = steering + diff;
					total++;
				}
			}
		}
	}
	if (total > 0)
	{
		steering = steering / (float)total;
		steering = setLength(steering, this->maxSpeed);
		steering = steering - this->velocity;
		steering = clampLength(steering, this->maxForce);
	}
	if (glm::isinf(steering.x) || glm::isinf(steering.y) || glm::isinf(steering.z) ||
		glm::isnan(steering.x) || glm::isnan(steering.z) || glm::isnan(steering.z))
		return glm::vec3(0.0f);
	return steering;
}

void Boid::flock(Boid* boids, int boidsNumber, float alignF, float cohesionF, float separationF)
{
	
	glm::vec3 aligment = this->align(boids, boidsNumber) * alignF;
	glm::vec3 cohensia = this->cohesion(boids, boidsNumber) * cohesionF;
	glm::vec3 seperation = this->seperation(boids, boidsNumber) * separationF;
	this->acceleration = aligment + cohensia + seperation;
}

glm::mat4 Boid::getModel()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->position);
	model = model * glm::toMat4(computeRotation(this->velocity));
	model = glm::scale(model, glm::vec3(0.3f));
	return model;
}

glm::vec3 getRandomVec3(float min, float max) 
{
	glm::vec3 result(glm::linearRand(min, max), glm::linearRand(min, max), glm::linearRand(min, max));

	return result;
}
glm::vec3 clampLength(const glm::vec3& vector, float maxLength) 
{
	float length = glm::length(vector);

	if (length > maxLength) 
	{
		return glm::normalize(vector) * maxLength;
	}

	return vector;
}
glm::vec3 setLength(const glm::vec3& vector, float newLength) {
	float length = glm::length(vector);

	if (length == 0.0f) {
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}

	return glm::normalize(vector) * newLength;
}
glm::quat computeRotation(const glm::vec3& velocity, const glm::vec3& up) 
{
    // Sprawdzenie, czy wektor prêdkoœci nie jest zerowy
    if (glm::length(velocity) == 0.0f) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Brak rotacji
    }

    // Normalizacja wektora prêdkoœci
    glm::vec3 forward = glm::normalize(velocity);

    // Obliczanie osi obrotu jako iloczyn wektorowy miêdzy 'up' a 'forward'
    glm::vec3 right = glm::normalize(glm::cross(up, forward));

    // Obliczanie nowego wektora "up" (prostopad³ego do forward i right)
    glm::vec3 newUp = glm::cross(forward, right);

    // Tworzenie macierzy orientacji
    glm::mat3 rotationMatrix(right, newUp, forward);

    // Konwersja macierzy orientacji na kwaternion
    return glm::quat_cast(rotationMatrix);
}
float calculateAngleToBoid(glm::vec3& boidPosition, glm::vec3& boidVelocity, glm::vec3& otherBoidPosition) 
{
	// Wektor prowadz¹cy do innego boida
	glm::vec3 directionToOther = otherBoidPosition - boidPosition;

	// Sprawdzenie, czy wektor prowadz¹cy do innego boida nie jest zerowy
	if (glm::length(directionToOther) > 0.0f) {
		// Normalizacja wektorów
		glm::vec3 normalizedVelocity = glm::normalize(boidVelocity);
		glm::vec3 normalizedDirection = glm::normalize(directionToOther);

		// Obliczenie k¹ta miêdzy wektorem prêdkoœci a kierunkiem do innego boida
		return glm::angle(normalizedVelocity, normalizedDirection);
	}
	else {
		// Jeœli boidy maj¹ identyczn¹ pozycjê, zwróæ -1 (lub inn¹ wartoœæ oznaczaj¹c¹ brak k¹ta)
		return -1.0f;
	}
}