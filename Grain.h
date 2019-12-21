#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

struct Grain {
	GLfloat vol, mass, density;
	glm::vec2 pos, velocity;
	glm::mat2x2 velocityGard;

	GLfloat lambda, mu;

	glm::mat2x2 defElastic, defPlastic;
	glm::mat2x2 svdW, svdV;
	glm::vec2 svdE;

	glm::mat2x2 polarR, polarS;
	glm::vec2 gridPos;
	glm::vec2 weightGard[16];
	GLfloat weights[16];
};

void InitGrain(Grain* grain, glm::vec2 pos, glm::vec2 velocity, GLfloat mass, GLfloat lambda, GLfloat mu);
void updatePos(Grain* grain);
void updateGradient(Grain* grain);
void applyPlasticity(Grain* grain);
glm::mat2x2 energyDerivative(Grain* grain);
glm::vec2 deltaForce(const glm::vec2 u, const glm::vec2 weightGard);