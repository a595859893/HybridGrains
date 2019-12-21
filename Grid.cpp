#include "Grid.h"
#include "Constant.h"

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
void InitMass(Grid* grid) {
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

				// ͳ����Χ������ֵ�������֮��
				grid->nodes[(int)(y * grid->size[0] + x)].mass += weight * grain->mass;
			}
		}
	}
}

// ��ʼ��ÿ��������ÿ��Grid�µ��ٶ�
void InitVelocity(Grid* grid) {
	// ͬInitMass������ÿ�����Ӳ��õ����Ƕ�Ӧ�Ĵ��Grid
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		int ox = grain->gridPos[0], oy = grain->gridPos[1];
		// Ȼ�������Χ4x4�ķ���
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				// ��ȡ֮ǰ����õ�Ȩ��
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					// ͳ����Χ������ֵ����ٶ�֮��
					// �ο�������д�����ٶȣ���������Ӧ���Ƕ�����
					// ������������ȡƽ��������ٶ�
					int n = (int)(y * grid->size[0] + x);
					grid->nodes[n].v += grain->velocity * w * grain->mass;
					grid->nodes[n].active = true;
				}
			}
		}
	}
	// �������ͻ����ٶȾ�ֵ
	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->v /= node->mass;
	}

	// ײ�ϱ߽�ļ��
	collisionGrid(grid);
}
// ��Grain���ٶ�ӳ�䵽Grain��
// ֻ��Ҫ��ʼ��ģ���ʱ�����һ�μ���
void CalculateVolumes(Grid* grid) {
	// ����ÿ������
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		int ox = grain->gridPos[0], oy = grain->gridPos[1];

		// ����Grid�ھ��е������ܶ�
		grain->density = 0;
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					// �����ڸ�Grid�ڵ����ӵ�������Ȩ��
					grain->density += w * grid->nodes[(int)(y * grid->size[0] + x)].mass;
				}
			}
		}
		// ����Grid������õ�Grid��ƽ���ܶ�
		grain->density /= grid->nodeArea;

		// ����Grid�ܶȼ����Gird�ڸ�Grain��ռ�����
		grain->vol = grain->mass / grain->density;
	}
}

void ExplicitVelocities(Grid* grid, glm::vec2 gravity) {
	// ����ÿ�����ӵ��������
	// ���������µ��ٶȴ���vNew��
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// ��ȡGrain����Grid�ڵ��������
		glm::mat2x2 energy = energyDerivative(grain);
		if (isinf(energy[0][0])) {
			int a = 0;
		}
		// ���ڲ����������Ȩӳ�䵽��Χ��4x4������
		int ox = (int)grain->gridPos[0], oy = (int)grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					int n = (int)(y * grid->size[0] + x);
					if (!(energy[0][0] == energy[0][0])) {
						int a = 0;
					}
					if (!(grid->nodes[n].vNew[0] == grid->nodes[n].vNew[0])) {
						int a = 0;
					}

					grid->nodes[n].vNew += energy * grain->weightGard[idx];

					if (!(grid->nodes[n].vNew[0] == grid->nodes[n].vNew[0])) {
						int a = 0;
					}
				}
			}
		}
	}


	// �����ڵ����ٶȿ��Ը���ӳ��õ��������¼���
	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (!(node->vNew[0] == node->vNew[0])) {
			int a = 0;
		}

		if (node->active)
			node->vNew = node->v + TIMESTEP * (gravity - node->vNew / node->mass);

		if (!(node->vNew[0] == node->vNew[0])) {
			int a = 0;
		}
	}

	// ײ�ϱ߽�ļ��
	collisionGrid(grid);
}

// ��Grid���ٶ�ӳ���Grain
void UpdateVelocites(Grid* grid) {
	// ����ÿ������
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// �ֱ����PIC��FLIP(Fluid implicit particle)
		glm::vec2 pic(0, 0), flip = grain->velocity;

		// �ٶ��ݶ�
		grain->velocityGard = glm::mat2x2(0);

		// �����ܶȣ����ӻ��ã�
		grain->density = 0;

		// ����������Χ4x4�ĸ���
		int ox = grain->gridPos[0], oy = grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					GridNode* node = grid->nodes + (int)(y * grid->size[0] + x);
					// ͳ��������ӵĴ�Ȩ�ٶȣ�������ײ��
					pic += node->vNew * w;
					// ͳ���ٶȵĴ�Ȩ�ı�����������ײ��
					flip += (node->vNew - node->v) * w;
					// �ٶ��ݶ�
					grain->velocityGard += glm::outerProduct(node->vNew, grain->weightGard[idx]);
					if (!(grain->velocityGard[0][0] == grain->velocityGard[0][0])) {
						int a = 0;
					}
					//�����ӻ��ã�: �����ܶȣ�����������ͳ������
					grain->density += w * node->mass;
				}
			}
		}

		// �������ֵ��ٶȻ��
		grain->velocity = flip * FLIP_PERCENT + pic * (1 - FLIP_PERCENT);
		// ������������õ��ܶ�
		grain->density /= grid->nodeArea;
	}

	collisionParticles(grid);
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
				glm::vec2 newPos = node->vNew * deltaScale + glm::vec2(x, y);

				// ײ�����ڱ߽�ķ���
				if (newPos[0] < BSPLINE_RADIUS || newPos[0] > grid->size[0] - BSPLINE_RADIUS - 1) {
					node->vNew[0] = 0;
					node->vNew[1] *= STICKY;
				}
				if (newPos[1] < BSPLINE_RADIUS || newPos[1] > grid->size[1] - BSPLINE_RADIUS - 1) {
					node->vNew[0] *= STICKY;
					node->vNew[1] = 0;
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
