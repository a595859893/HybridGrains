#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

struct Grain {
	GLfloat vol, mass, mu, J;
	glm::mat2x2 be, sigma;
	glm::vec2 pos, velocity;
	glm::mat2x2 velocityGard;

	glm::vec2 gridPos;
	glm::vec2 weightGard[16];
	GLfloat weights[16];
};

struct GridNode {
	GLfloat mass;
	bool active;
	glm::vec2 f, p, pNew, vel, a;
};

struct Grid {
	glm::vec2 origin, size, cellSize;
	Grain* grains;
	int grainNum;

	GLfloat nodeArea;
	GridNode* nodes;
	int nodeLen;

};


void InitGrain(Grain* grain, glm::vec2 pos, glm::vec2 velocity, GLfloat mass, GLfloat vol, GLfloat mu);
void InitGrid(Grid* grid, Grain* grain, int grainNum, glm::vec2 pos, glm::vec2 dims, glm::vec2 cells);

void RasterizeMassAndMomentum(Grid* grid);
void ComputeStressAtPoint(Grid* grid);
void ComputeForceOnGrid(Grid* grid, glm::vec2 gravity);
void UpdateMomentumOnGrid(Grid* grid);


void LumpedMassVelocityUpdateOnGrid(Grid* grid);
void ComputeVelocityGradientAtPoint(Grid* grid);
void ElasticPredictionAtPoint(Grid* grid);
void PlasticCorrectionAtPoint(Grid* grid);
void UpdateVelocitesAtPoint(Grid* grid);
void UpdatePositionsAtPoint(Grid* grid);

void collisionGrid(Grid* gird);
void collisionParticles(Grid* gird);