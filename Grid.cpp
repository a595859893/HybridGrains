#include "Grid.h"
#include "Constant.h"

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
void InitMass(Grid* grid) {
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

				// 统计周围样条插值后的质量之和
				grid->nodes[(int)(y * grid->size[0] + x)].mass += weight * grain->mass;
			}
		}
	}
}

// 初始化每个步骤下每个Grid下的速度
void InitVelocity(Grid* grid) {
	// 同InitMass，遍历每个粒子并得到它们对应的大概Grid
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		int ox = grain->gridPos[0], oy = grain->gridPos[1];
		// 然后遍历周围4x4的方格
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				// 获取之前计算好的权重
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					// 统计周围样条插值后的速度之和
					// 参考代码上写的是速度，但是这里应该是动量和
					// 后续基于质量取平均后才是速度
					int n = (int)(y * grid->size[0] + x);
					grid->nodes[n].v += grain->velocity * w * grain->mass;
					grid->nodes[n].active = true;
				}
			}
		}
	}
	// 将动量和换成速度均值
	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->v /= node->mass;
	}

	// 撞上边界的检测
	collisionGrid(grid);
}
// 将Grain的速度映射到Grain上
// 只需要初始化模拟的时候调用一次即可
void CalculateVolumes(Grid* grid) {
	// 遍历每个粒子
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		int ox = grain->gridPos[0], oy = grain->gridPos[1];

		// 计算Grid内具有的粒子密度
		grain->density = 0;
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					// 所有在该Grid内的粒子的质量加权和
					grain->density += w * grid->nodes[(int)(y * grid->size[0] + x)].mass;
				}
			}
		}
		// 除以Grid的体积得到Grid内平均密度
		grain->density /= grid->nodeArea;

		// 利用Grid密度计算出Gird内该Grain所占的体积
		grain->vol = grain->mass / grain->density;
	}
}

void ExplicitVelocities(Grid* grid, glm::vec2 gravity) {
	// 计算每个粒子的受力情况
	// 将即将更新的速度存在vNew里
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// 获取Grain所在Grid内的受力情况
		glm::mat2x2 energy = energyDerivative(grain);
		if (isinf(energy[0][0])) {
			int a = 0;
		}
		// 将内部受力情况加权映射到周围的4x4网格内
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


	// 网格内的新速度可以根据映射好的受力重新计算
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

	// 撞上边界的检测
	collisionGrid(grid);
}

// 将Grid的速度映射回Grain
void UpdateVelocites(Grid* grid) {
	// 遍历每个粒子
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		// 分别计算PIC与FLIP(Fluid implicit particle)
		glm::vec2 pic(0, 0), flip = grain->velocity;

		// 速度梯度
		grain->velocityGard = glm::mat2x2(0);

		// 计算密度（可视化用）
		grain->density = 0;

		// 遍历粒子周围4x4的格子
		int ox = grain->gridPos[0], oy = grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					GridNode* node = grid->nodes + (int)(y * grid->size[0] + x);
					// 统计这个格子的带权速度（刚体碰撞）
					pic += node->vNew * w;
					// 统计速度的带权改变量（流体碰撞）
					flip += (node->vNew - node->v) * w;
					// 速度梯度
					grain->velocityGard += glm::outerProduct(node->vNew, grain->weightGard[idx]);
					if (!(grain->velocityGard[0][0] == grain->velocityGard[0][0])) {
						int a = 0;
					}
					//（可视化用）: 更新密度，在这里是先统计质量
					grain->density += w * node->mass;
				}
			}
		}

		// 将两部分的速度混合
		grain->velocity = flip * FLIP_PERCENT + pic * (1 - FLIP_PERCENT);
		// 质量除以体积得到密度
		grain->density /= grid->nodeArea;
	}

	collisionParticles(grid);
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
				glm::vec2 newPos = node->vNew * deltaScale + glm::vec2(x, y);

				// 撞到窗口边界的反弹
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
