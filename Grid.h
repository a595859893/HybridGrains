#pragma once
#include "Grain.h"

struct GridNode {
	GLfloat mass;
	bool active;
	glm::vec2 v, vNew;
};

struct Grid {
	glm::vec2 origin, size, cellSize;
	Grain* grains;
	int grainNum;

	GLfloat nodeArea;
	GridNode* nodes;
	int nodeLen;

};


void InitGrid(Grid* grid, Grain* grain, int grainNum, glm::vec2 pos, glm::vec2 dims, glm::vec2 cells);

void InitMass(Grid* grid);
void InitVelocity(Grid* grid);

void CalculateVolumes(Grid* grid);

void ExplicitVelocities(Grid* grid, glm::vec2 gravity);
void UpdateVelocites(Grid* grid);

void collisionGrid(Grid* gird);
void collisionParticles(Grid* gird);