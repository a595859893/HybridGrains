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

void InitMass(Grid* grid) {
	memset(grid->nodes, 0, sizeof(GridNode) * grid->nodeLen);

	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		grain->gridPos = (grain->pos - grid->origin) / grid->cellSize;

		float ox = grain->gridPos[0], oy = grain->gridPos[1];
		//Shape function gives a blending radius of two;
		//so we do computations within a 2x2 square for each particle
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			if (y < 0 || y > grid->size[1]) continue;

			//Y-dimension interpolation
			float y_pos = oy - y,
				wy = bspline(y_pos),
				dy = bsplineSlope(y_pos);

			for (int x = (int)ox - 1, x_end = (int)x + 3; x <= x_end; x++, idx++) {
				if (x < 0 || x > grid->size[0]) continue;

				//X-dimension interpolation
				float x_pos = ox - x,
					wx = bspline(x_pos),
					dx = bsplineSlope(x_pos);

				//Final weight is dyadic product of weights in each dimension
				float weight = wx * wy;
				grain->weights[idx] = weight;

				//Weight gradient is a vector of partial derivatives
				grain->weightGard[idx] = glm::vec2(dx * wy, wx * dy);
				//I don't know why we need to do this... JT did it, doesn't appear in tech paper
				grain->weightGard[idx] /= grid->cellSize;

				//Interpolate mass
				grid->nodes[(int)(y * grid->size[0] + x)].mass += weight * grain->mass;
			}
		}
	}

}
void InitVelocity(Grid* grid) {
	//We interpolate velocity after mass, to conserve momentum
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		int ox = grain->gridPos[0], oy = grain->gridPos[1];

		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {

				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					//Interpolate velocity
					int n = (int)(y * grid->size[0] + x);
					//We could also do a separate loop to divide by nodes[n].mass only once
					grid->nodes[n].v += grain->velocity * w * grain->mass;
					grid->nodes[n].active = true;
				}
			}
		}
	}

	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->v /= node->mass;
	}

	collisionGrid(grid);
}

// Maps volume from the grid to particles
// This should only be called once, at the beginning of the simulation
void CalculateVolumes(Grid* grid) {
	//First, compute the forces
//We store force in velocity_new, since we're not using that variable at the moment
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		int ox = grain->gridPos[0], oy = grain->gridPos[1];

		//First compute particle density
		grain->density = 0;
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					//Node density is trivial
					grain->density += w * grid->nodes[(int)(y * grid->size[0] + x)].mass;
				}
			}
		}

		grain->density /= grid->nodeArea;

		//Volume for each particle can be found from density
		grain->vol = grain->mass / grain->density;
	}
}

void ExplicitVelocities(Grid* grid, glm::vec2 gravity) {
	//First, compute the forces
//We store force in velocity_new, since we're not using that variable at the moment
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;


		//Solve for grid internal forces
		glm::mat2x2 energy = energyDerivative(grain);

		int ox = (int)grain->gridPos[0], oy = (int)grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					//Weight the force onto nodes
					int n = (int)(y * grid->size[0] + x);
					grid->nodes[n].vNew += energy * grain->weightGard[idx];
				}
			}
		}
	}

	for (int i = 0; i < grid->nodeLen; i++) {
		GridNode* node = grid->nodes + i;
		if (node->active)
			node->vNew = node->v + TIMESTEP * (gravity - node->vNew / node->mass);
	}

	collisionGrid(grid);
}
void UpdateVelocites(Grid* grid) {
	//Estimate each particles volume (for force calculations)
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;

		//We calculate PIC and FLIP velocities separately
		glm::vec2 pic(0, 0), flip = grain->velocity;

		//Also keep track of velocity gradient
		grain->velocityGard = glm::mat2x2(0);

		//VISUALIZATION PURPOSES ONLY:
		//Recompute density
		grain->density = 0;

		int ox = grain->gridPos[0], oy = grain->gridPos[1];
		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
				float w = grain->weights[idx];
				if (w > BSPLINE_EPSILON) {
					GridNode* node = grid->nodes + (int)(y * grid->size[0] + x);
					//Particle in cell
					pic += node->vNew * w;
					//Fluid implicit particle
					flip += (node->vNew - node->v) * w;
					//Velocity gradient

					grain->velocityGard += glm::outerProduct(node->vNew, grain->weightGard[idx]);
					//VISUALIZATION ONLY: Update density
					grain->density += w * node->mass;
				}
			}
		}

		grain->velocity = flip * FLIP_PERCENT + pic * (1 - FLIP_PERCENT);
		grain->density /= grid->nodeArea;
	}

	collisionParticles(grid);
}

void collisionGrid(Grid* grid) {
	glm::vec2 deltaScale = glm::vec2(TIMESTEP, TIMESTEP);
	deltaScale /= grid->cellSize;

	GridNode* node = grid->nodes;
	for (int y = 0, idx = 0; y < grid->size[1]; y++) {
		for (int x = 0; x < grid->size[0]; x++, node++) {
			//Check to see if this node needs to be computed
			if (node->active) {
				//Collision response
				//TODO: make this work for arbitrary collision geometry
				glm::vec2 new_pos = node->vNew * deltaScale + glm::vec2(x, y);

				//Left border, right border
				if (new_pos[0] < BSPLINE_RADIUS || new_pos[0] > grid->size[0] - BSPLINE_RADIUS - 1) {
					node->vNew[0] = 0;
					node->vNew[1] *= STICKY;
				}
				//Bottom border, top border
				if (new_pos[1] < BSPLINE_RADIUS || new_pos[1] > grid->size[1] - BSPLINE_RADIUS - 1) {
					node->vNew[0] *= STICKY;
					node->vNew[1] = 0;
				}
			}
		}
	}
}
void collisionParticles(Grid* grid) {
	for (int i = 0; i < grid->grainNum; i++) {
		Grain* grain = grid->grains + i;
		glm::vec2 new_pos = grain->gridPos + TIMESTEP * grain->velocity / grid->cellSize;
		//Left border, right border
		if (new_pos[0] < BSPLINE_RADIUS - 1 || new_pos[0] > grid->size[0] - BSPLINE_RADIUS)
			grain->velocity[0] = -STICKY * grain->velocity[0];
		//Bottom border, top border
		if (new_pos[1] < BSPLINE_RADIUS - 1 || new_pos[1] > grid->size[1] - BSPLINE_RADIUS)
			grain->velocity[1] = -STICKY * grain->velocity[1];
	}
}
