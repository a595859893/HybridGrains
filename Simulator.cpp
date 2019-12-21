#include "Simulator.h"
#include "Constant.h"
#include "Grid.h"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <stdlib.h>

void InitSimulator(Simulator* sim, GLint width, GLint height, GLint grainsNum) {
	sim->width = width;
	sim->height = height;

	sim->grainsNum = grainsNum;
	sim->grains = (Grain*)malloc(sizeof(Grain) * grainsNum);
	sim->grainsPos = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * GRAINS_DIM);
	sim->grainsColor = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * 3);


	for (int i = 0; i < grainsNum; i++) {
		glm::vec2 randPos = glm::clamp(glm::gaussRand(glm::vec2(0, 0), glm::vec2(0.25, 0.25)), -0.8f, 0.8f);
		glm::vec2 randVel = glm::gaussRand(glm::vec2(0, 0), glm::vec2(1, 1));
		InitGrain(sim->grains + i, randPos, randVel, 1, 0.2f, 0.2f);
	}

	InitGrid(&sim->grid, sim->grains, grainsNum,
		glm::vec2(-0.5, -0.5), glm::vec2(width, height), glm::vec2(128, 128));
	InitMass(&sim->grid);
	CalculateVolumes(&sim->grid);

}

void SimUpdate(Simulator* sim) {
	int i, j;
	Grain* cur;

	//Initialize FEM grid
	InitMass(&sim->grid);
	InitVelocity(&sim->grid);
	//Compute grid velocities
	ExplicitVelocities(&sim->grid, GRAVITY);
	//Map back to particles
	UpdateVelocites(&sim->grid);

	for (i = 0; i < sim->grainsNum; i++) {
		// 更新状态
		cur = sim->grains + i;
		updatePos(cur);
		updateGradient(cur);
		applyPlasticity(cur);

		// 更新绘制位置和颜色
		for (j = 0; j < GRAINS_DIM; j++) sim->grainsPos[i * GRAINS_DIM + j] = sim->grains[i].pos[j];
		for (j = 0; j < 3; j++) sim->grainsColor[i * 3 + j] = sim->grains[i].color[j];
	}

}