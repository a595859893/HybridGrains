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

	// grid原点
	grid->origin = pos;
	// 单个node边长
	grid->cellSize = dims / cells;
	// node排列方式（长宽）
	grid->size = cells + glm::vec2(1, 1);

	// node总数（标量）
	grid->nodeLen = (int)(grid->size[0] * grid->size[1]);
	grid->nodes = (GridNode*)malloc(sizeof(GridNode) * grid->nodeLen);
	// 单个node的体积
	grid->nodeArea = grid->cellSize[0] * grid->cellSize[1];
}

// 初始化本次步骤时在每个Grid内的质量
void RasterizeMassAndMomentum(Grid* grid) {
	memset(grid->nodes, 0, sizeof(GridNode) * grid->nodeLen);

	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// 计算该粒子大概处于哪个Grid内
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;

		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// 假定每个粒子占其所在位置周围4x4的Grid
		// 所以需要遍历这些Grid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// 超过计算边界，跳过
			if (y < 0 || y > grid->size[1]) continue;

			// Y轴样条插值
			float y_pos = oy - y,
				wy = bspline(y_pos),
				dy = bsplineSlope(y_pos);

			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// 超过计算边界，跳过
				if (x < 0 || x > grid->size[0]) continue;

				// X轴样条插值
				float x_pos = ox - x,
					wx = bspline(x_pos),
					dx = bsplineSlope(x_pos);

				// 权重等于xy样条插值之积
				float weight = wx * wy;
				grain->weights[idx] = weight;

				// 计算权重在样条插值下的梯度
				grain->weightGard[idx] = glm::vec2(dx * wy, wx * dy);
				// 梯度除以单个方块的体积，得到单位体积的梯度
				// 不过参考代码里没说为什么要这么做
				grain->weightGard[idx] /= grid->cellSize;

				int n = (int)(y * grid->size[0] + x);
				// 统计周围样条插值后的质量之和
				grid->nodes[n].mass += weight * grain->mass;
				grid->nodes[n].p += weight * grain->mass * grain->velocity;
				grid->nodes[n].active = true;
			}
		}
	}

	// 撞上边界的检测
	collisionGrid(grid);
}

// 计算粒子所受的压力
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
// 更新网格受力情况
void ComputeForceOnGrid(Grid* grid, glm::vec2 gravity) {
	// 计算每个粒子的受力情况
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// 将内部受力情况加权映射到周围的4x4网格内
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

	// 重力加速度
	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->f += node->mass * gravity;
	}

}
// 更新网格的动量
void UpdateMomentumOnGrid(Grid* grid) {
	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active) {// 遍历所有存在粒子的Grid
				node->pNew = node->p + TIMESTEP * node->f;
			}
		}
	}

	// 撞上边界的检测
	collisionGrid(grid);
}

// 网格质量和速度的更新
void LumpedMassVelocityUpdateOnGrid(Grid* grid) {
	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active && node->mass > 0) {// 遍历所有存在粒子的Grid
				node->vel = node->pNew / node->mass;
			}
		}
	}
}
// 计算点的速度梯度
void ComputeVelocityGradientAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		grain->velocityGard = glm::mat2x2(0);
		// 计算该粒子大概处于哪个Grid内
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// 假定每个粒子占其所在位置周围4x4的Grid
		// 所以需要遍历这些Grid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// 超过计算边界，跳过
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// 超过计算边界，跳过
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
// 估计点的塑性
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
// 矫正点的塑性
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
// 更新点的速度
void UpdateVelocitesAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::vec2 pic(0), aflip(0), vflip;

		// 计算该粒子大概处于哪个Grid内
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// 假定每个粒子占其所在位置周围4x4的Grid
		// 所以需要遍历这些Grid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// 超过计算边界，跳过
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// 超过计算边界，跳过
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
// 更新点的位置
void UpdatePositionsAtPoint(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::vec2 pic(0);

		// 计算该粒子大概处于哪个Grid内
		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;
		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		// 假定每个粒子占其所在位置周围4x4的Grid
		// 所以需要遍历这些Grid
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			// 超过计算边界，跳过
			if (y < 0 || y > grid->size[1]) continue;
			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				// 超过计算边界，跳过
				if (x < 0 || x > grid->size[0]) continue;
				int n = (int)(y * grid->size[0] + x);
				pic += grain->weights[idx] * grid->nodes[n].vel;
			}
		}

		grain->pos += TIMESTEP * pic;
	}
}


// Grid撞墙反弹检测
void collisionGrid(Grid* grid) {
	glm::vec2 deltaScale = glm::vec2(TIMESTEP, TIMESTEP);
	deltaScale /= grid->cellSize;

	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			if (node->active) {// 遍历所有存在粒子的Grid
				// 碰撞回应
				// TODO: make this work for arbitrary collision geometry

				// 计算这个Grid下一次迭代的可能位置
				glm::vec2 newPos = node->pNew * deltaScale + glm::vec2(x, y);

				// 撞到窗口边界的反弹
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

// Grain撞墙反弹检测
void collisionParticles(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// 计算这个Grid下一次迭代的可能位置
		glm::vec2 new_pos = grain->gridPos + TIMESTEP * grain->velocity / grid->cellSize;

		// 撞到窗口边界的反弹
		if (new_pos[0] < BSPLINE_RADIUS - 1 || new_pos[0] > grid->size[0] - BSPLINE_RADIUS)
			grain->velocity[0] = -STICKY * grain->velocity[0];
		if (new_pos[1] < BSPLINE_RADIUS - 1 || new_pos[1] > grid->size[1] - BSPLINE_RADIUS)
			grain->velocity[1] = -STICKY * grain->velocity[1];
	}
}
