#include "Simulator.h"
#include "Constant.h"
#include "Grid.h"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <stdlib.h>

void InitSimulator(Simulator* sim, GLint grainsNum) {
	int i, j, k, idx;
	sim->width = 2;
	sim->height = 2;

	sim->grainsNum = grainsNum;
	sim->grains = (Grain*)malloc(sizeof(Grain) * grainsNum);
	sim->grainsPos = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * GRAINS_DIM);
	sim->grainsColor = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * 3);



	for (i = 0; i < grainsNum; i++) {
		glm::vec2 randPos = glm::clamp(glm::gaussRand(glm::vec2(0, 0), glm::vec2(0.1, 0.1)), -1.0f, 1.0f);
		glm::vec2 randVel = glm::gaussRand(glm::vec2(0, 0), glm::vec2(1, 1));
		//glm::vec2 randVel = glm::vec2(0, 1);
		InitGrain(sim->grains + i, randPos, randVel, 1, MU, LAMBDA);
	}

	InitGrid(&sim->grid, sim->grains, grainsNum,
		glm::vec2(-1, -1), glm::vec2(2, 2), glm::vec2(256, 256));
	InitMass(&sim->grid);
	CalculateVolumes(&sim->grid);

	int GridNum = sim->grid.size[0] * sim->grid.size[1];
	sim->gridPos = (GLfloat*)malloc(sizeof(GLfloat) * GridNum * 6 * GRAINS_DIM);
	sim->gridColor = (GLfloat*)malloc(sizeof(GLfloat) * GridNum * 6 * 3);

	for (i = 0; i < sim->grid.size[0]; i++) {
		for (j = 0; j < sim->grid.size[1]; j++) {
			idx = i * sim->grid.size[0] + j;
			sim->gridPos[idx * 6 * GRAINS_DIM + 0] = sim->grid.cellSize[0] * j;
			sim->gridPos[idx * 6 * GRAINS_DIM + 1] = sim->grid.cellSize[1] * i;

			sim->gridPos[idx * 6 * GRAINS_DIM + 2] = sim->grid.cellSize[0] * (j + 1);
			sim->gridPos[idx * 6 * GRAINS_DIM + 3] = sim->grid.cellSize[1] * i;

			sim->gridPos[idx * 6 * GRAINS_DIM + 4] = sim->grid.cellSize[0] * j;
			sim->gridPos[idx * 6 * GRAINS_DIM + 5] = sim->grid.cellSize[1] * (i + 1);

			sim->gridPos[idx * 6 * GRAINS_DIM + 6] = sim->grid.cellSize[0] * (j + 1);
			sim->gridPos[idx * 6 * GRAINS_DIM + 7] = sim->grid.cellSize[1] * i;

			sim->gridPos[idx * 6 * GRAINS_DIM + 8] = sim->grid.cellSize[0] * j;
			sim->gridPos[idx * 6 * GRAINS_DIM + 9] = sim->grid.cellSize[1] * (i + 1);

			sim->gridPos[idx * 6 * GRAINS_DIM + 10] = sim->grid.cellSize[0] * (j + 1);
			sim->gridPos[idx * 6 * GRAINS_DIM + 11] = sim->grid.cellSize[1] * (i + 1);

			for (k = 0; k < 6; k++) {
				sim->gridPos[idx * 6 * GRAINS_DIM + k * 2 + 0] -= 1;
				sim->gridPos[idx * 6 * GRAINS_DIM + k * 2 + 1] -= 1;

				sim->gridColor[idx * 6 * 3 + k * 3 + 0] = 0;
				sim->gridColor[idx * 6 * 3 + k * 3 + 1] = 0;
				sim->gridColor[idx * 6 * 3 + k * 3 + 2] = 0;
			}
		}
	}
}

void SimUpdate(Simulator* sim) {
	int i, j, k, idx;
	Grain* cur;

	InitMass(&sim->grid);
	InitVelocity(&sim->grid);
	ExplicitVelocities(&sim->grid, GRAVITY);
	UpdateVelocites(&sim->grid);

	for (i = 0; i < sim->grainsNum; i++) {
		// 更新状态
		cur = sim->grains + i;
		updatePos(cur);
		updateGradient(cur);
		applyPlasticity(cur);

		// 更新绘制位置和颜色
		for (j = 0; j < GRAINS_DIM; j++)
			sim->grainsPos[i * GRAINS_DIM + j] = sim->grains[i].pos[j];
		sim->grainsColor[i * 3 + 0] = sim->grains[i].density / 10000.0f;
		sim->grainsColor[i * 3 + 1] = 1;
		sim->grainsColor[i * 3 + 2] = glm::length(sim->grains[i].velocity) / 10.0f;
	}

	for (i = 0; i < sim->grid.size[0]; i++) {
		for (j = 0; j < sim->grid.size[1]; j++) {
			idx = i * sim->grid.size[0] + j;
			for (k = 0; k < 6; k++) {
				sim->gridColor[idx * 6 * 3 + k * 3 + 0] = 0;
				sim->gridColor[idx * 6 * 3 + k * 3 + 1] = sim->grid.nodes[idx].active * 0.2f;
				sim->gridColor[idx * 6 * 3 + k * 3 + 2] = 0;
			}
		}
	}

}