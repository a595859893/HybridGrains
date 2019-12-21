#include "Simulator.h"
#include "Constant.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <stdlib.h>

void InitSimulator(Simulator* sim, GLint width, GLint height, GLint grainsNum) {
	sim->width = width;
	sim->height = height;
	sim->grid = (Grid*)malloc(sizeof(Grid) * width * height);

	sim->grainsNum = grainsNum;
	sim->grains = (Grain*)malloc(sizeof(Grain) * grainsNum);
	sim->grainsPos = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * GRAINS_DIM);
	sim->grainsColor = (GLfloat*)malloc(sizeof(GLfloat) * grainsNum * 3);

	for (int i = 0; i < grainsNum; i++) {
		InitGrain(sim->grains + i,
			glm::gaussRand(glm::vec2(0.25, 0.25), glm::vec2(0.25, 0.25)),
			glm::gaussRand(glm::vec2(0, 0), glm::vec2(1, 1)),
			1, 0.2f, 0.2f);
	}
}

void SimUpdate(Simulator* sim) {
	int i, j, idx;
	for (i = 0; i < sim->grainsNum; i++) {
		// Î»ÖÃ
		updatePos(sim->grains + i);
		for (j = 0; j < GRAINS_DIM; j++) {
			sim->grainsPos[i * GRAINS_DIM + j] = sim->grains[i].pos[j];
		}

		// ÑÕÉ«
		for (j = 0; j < 3; j++) {
			sim->grainsColor[i * 3 + j] = sim->grains[i].color[j];
		}
	}

}