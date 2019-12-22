#include "Grid.h"
#include "Constant.h"

void InitGrain(Grain* grain, glm::vec2 pos, glm::vec2 velocity, GLfloat mass, GLfloat vol, GLfloat mu) {
	grain->pos = pos;
	grain->velocity = velocity;
	grain->vol = vol;
	grain->mass = mass;
	grain->mu = mu;

	grain->J = 0.5;
	grain->be = glm::mat2x2(1);
}


void InitGrid(Grid* grid, Grain* grain, int grainNum, glm::vec2 pos, glm::vec2 dims, glm::vec2 cells) {
	grid->grains = grain;
	grid->grainNum = grainNum;

	// gridԭ��
	grid->origin = pos;
	// ����node�߳�
	grid->cellSize = dims / cells;
	// node���з�ʽ������
	grid->size = cells + glm::vec2(1, 1);

	// node������������
	grid->nodeLen = (int)(grid->size[0] * grid->size[1]);
	grid->nodes = (GridNode*)malloc(sizeof(GridNode) * grid->nodeLen);
	// ����node�����
	grid->nodeArea = grid->cellSize[0] * grid->cellSize[1];
}

// ��ʼ�����β���ʱ��ÿ��Grid�ڵ�����
void RasterizeMassAndMomentum(Grid* grid) {
	memset(grid->nodes, 0, sizeof(GridNode) * grid->nodeLen);

	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// ��������Ӵ�Ŵ����ĸ�Grid��
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;

		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// �ٶ�ÿ������ռ������λ����Χ4x4��Grid
		// ������Ҫ������ЩGrid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// ��������߽磬����
			if (y < 0 || y > grid->size[1]) continue;

			// Y��������ֵ
			float y_pos = oy - y,
				wy = bspline(y_pos),
				dy = bsplineSlope(y_pos);

			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// ��������߽磬����
				if (x < 0 || x > grid->size[0]) continue;

				// X��������ֵ
				float x_pos = ox - x,
					wx = bspline(x_pos),
					dx = bsplineSlope(x_pos);

				// Ȩ�ص���xy������ֵ֮��
				float weight = wx * wy;
				grain->weights[idx] = weight;

				// ����Ȩ����������ֵ�µ��ݶ�
				grain->weightGard[idx] = glm::vec2(dx * wy, wx * dy);
				// �ݶȳ��Ե��������������õ���λ������ݶ�
				// �����ο�������û˵ΪʲôҪ��ô��
				grain->weightGard[idx] /= grid->cellSize;

				int n = (int)(y * grid->size[0] + x);
				// ͳ����Χ������ֵ�������֮��
				grid->nodes[n].mass += weight * grain->mass;
				grid->nodes[n].p += weight * grain->mass * grain->velocity;
				grid->nodes[n].active = true;
			}
		}
	}

	// ײ�ϱ߽�ļ��
	collisionGrid(grid);
}

// �����������ܵ�ѹ��
void ComputeStressAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::mat2x2 tau(0);
		if (grain->J <= 1) {
			tau = 0.5f * KAPPA * (grain->J * grain->J - 1) * glm::mat2x2(1) + \
				grain->mu * 0.5f * (grain->be - 0.5f * (grain->be[0][0] * grain->be[1][1]) * glm::mat2x2(1));
		}

		grain->sigma = tau / grain->J;
	}
}
// ���������������
void ComputeForceOnGrid(Grid* grid, glm::vec2 gravity) {
	// ����ÿ�����ӵ��������
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// ���ڲ����������Ȩӳ�䵽��Χ��4x4������
		int ox = (int)grain->gridPos[0], oy = (int)grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					int n = (int)(y * grid->size[0] + x);
					grid->nodes[n].f += grain->vol * grain->J * grain->sigma * grain->weightGard[idx];
				}
			}
		}
	}

	// �������ٶ�
	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->f += node->mass * gravity;
	}

}
// ��������Ķ���
void UpdateMomentumOnGrid(Grid* grid) {
	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active) {// �������д������ӵ�Grid
				node->pNew = node->p + TIMESTEP * node->f;
			}
		}
	}

	// ײ�ϱ߽�ļ��
	collisionGrid(grid);
}

// �����������ٶȵĸ���
void LumpedMassVelocityUpdateOnGrid(Grid* grid) {
	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active && node->mass > 0) {// �������д������ӵ�Grid
				node->vel = node->pNew / node->mass;
			}
		}
	}
}
// �������ٶ��ݶ�
void ComputeVelocityGradientAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		grain->velocityGard = glm::mat2x2(0);
		// ��������Ӵ�Ŵ����ĸ�Grid��
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// �ٶ�ÿ������ռ������λ����Χ4x4��Grid
		// ������Ҫ������ЩGrid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// ��������߽磬����
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// ��������߽磬����
				if (x < 0 || x > grid->size[0]) continue;
				glm::mat2x2 vwmat(
					grain->velocity[0] * grain->weightGard[idx][0],
					grain->velocity[0] * grain->weightGard[idx][1],
					grain->velocity[1] * grain->weightGard[idx][0],
					grain->velocity[1] * grain->weightGard[idx][1]
				);

				grain->velocityGard += vwmat;
			}
		}
	}
}
// ���Ƶ������
void ElasticPredictionAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		grain->be = grain->J * grain->be;
		glm::mat2x2 beStar = grain->be + TIMESTEP * (grain->velocityGard * grain->be + grain->be * grain->velocityGard);
		if (glm::determinant(beStar) > 0) {
			grain->J = sqrt(glm::determinant(beStar));
			grain->be = beStar / grain->J;
		}

	}
}
// �����������
void PlasticCorrectionAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		GLfloat yieldThreshold = 0.5 * ALPHA * KAPPA * (grain->J * grain->J - 1);
		glm::mat2x2 devBe = grain->be - 0.5f * (grain->be[0][0] * grain->be[1][1]) * glm::mat2x2(1);
		devBe /= grain->J;
		GLfloat fnorm = devBe[0][0] * devBe[0][0] + devBe[0][1] * devBe[1][0]\
			+ devBe[1][0] * devBe[0][1] + devBe[1][1] * devBe[1][1];
		fnorm = sqrt(fnorm);

		if (MU * fnorm > yieldThreshold) {
			GLfloat lambda2 = yieldThreshold / (MU * fnorm);
			GLfloat lambda1 = sqrt(glm::determinant(grain->be) - lambda2 * lambda2 * glm::determinant(devBe));
			grain->be = lambda1 * glm::mat2x2(1) + lambda2 * devBe;
		}
	}

}
// ���µ���ٶ�
void UpdateVelocitesAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::vec2 pic(0), aflip(0), vflip;

		// ��������Ӵ�Ŵ����ĸ�Grid��
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// �ٶ�ÿ������ռ������λ����Χ4x4��Grid
		// ������Ҫ������ЩGrid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// ��������߽磬����
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// ��������߽磬����
				if (x < 0 || x > grid->size[0]) continue;
				int n = (int)(y * grid->size[0] + x);
				pic += grain->weights[idx] * grid->nodes[n].vel;
				aflip += grain->weights[idx] * grid->nodes[n].a;
			}
		}

		vflip = grain->velocity + TIMESTEP * aflip;
		grain->velocity = (1 - BETA) * pic + BETA * vflip;
	}
}
// ���µ��λ��
void UpdatePositionsAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::vec2 pic(0);

		// ��������Ӵ�Ŵ����ĸ�Grid��
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// �ٶ�ÿ������ռ������λ����Χ4x4��Grid
		// ������Ҫ������ЩGrid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// ��������߽磬����
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// ��������߽磬����
				if (x < 0 || x > grid->size[0]) continue;
				int n = (int)(y * grid->size[0] + x);
				pic += grain->weights[idx] * grid->nodes[n].vel;
			}
		}

		grain->pos += TIMESTEP * pic;
	}
}


// Gridײǽ�������
void collisionGrid(Grid* grid) {
	glm::vec2 deltaScale = glm::vec2(TIMESTEP, TIMESTEP);
	deltaScale /= grid->cellSize;

	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active) {// �������д������ӵ�Grid
				// ��ײ��Ӧ
				// TODO: make this work for arbitrary collision geometry

				// �������Grid��һ�ε����Ŀ���λ��
				glm::vec2 newPos = node->pNew * deltaScale + glm::vec2(x, y);

				// ײ�����ڱ߽�ķ���
				if (newPos[0] < BSPLINE_RADIUS || newPos[0] > grid->size[0] - BSPLINE_RADIUS - 1) {
					node->pNew[0] = 0;
					node->pNew[1] *= STICKY;
				}
				if (newPos[1] < BSPLINE_RADIUS || newPos[1] > grid->size[1] - BSPLINE_RADIUS - 1) {
					node->pNew[0] *= STICKY;
					node->pNew[1] = 0;
				}
			}
		}
	}
}

// Grainײǽ�������
void collisionParticles(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// �������Grid��һ�ε����Ŀ���λ��
		glm::vec2 new_pos = grain->gridPos + TIMESTEP * grain->velocity / grid->cellSize;

		// ײ�����ڱ߽�ķ���
		if (new_pos[0] < BSPLINE_RADIUS - 1 || new_pos[0] > grid->size[0] - BSPLINE_RADIUS)
			grain->velocity[0] = -STICKY * grain->velocity[0];
		if (new_pos[1] < BSPLINE_RADIUS - 1 || new_pos[1] > grid->size[1] - BSPLINE_RADIUS)
			grain->velocity[1] = -STICKY * grain->velocity[1];
	}
}
