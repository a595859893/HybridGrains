//
////Map grid velocities back to particles
//void Grid::updateVelocities() const {
//	for (int i = 0; i < obj->size; i++) {
//		Particle& p = obj->particles[i];
//		//We calculate PIC and FLIP velocities separately
//		Vector2f pic, flip = p.velocity;
//		//Also keep track of velocity gradient
//		Matrix2f& grad = p.velocity_gradient;
//		grad.setData(0.0);
//		//VISUALIZATION PURPOSES ONLY:
//		//Recompute density
//		p.density = 0;
//
//		int ox = p.grid_position[0],
//			oy = p.grid_position[1];
//		for (int idx = 0, y = oy - 1, y_end = y + 3; y <= y_end; y++) {
//			for (int x = ox - 1, x_end = x + 3; x <= x_end; x++, idx++) {
//				float w = p.weights[idx];
//				if (w > BSPLINE_EPSILON) {
//					GridNode& node = nodes[(int)(y * size[0] + x)];
//					//Particle in cell
//					pic += node.velocity_new * w;
//					//Fluid implicit particle
//					flip += (node.velocity_new - node.velocity) * w;
//					//Velocity gradient
//					grad += node.velocity_new.outer_product(p.weight_gradient[idx]);
//					//VISUALIZATION ONLY: Update density
//					p.density += w * node.mass;
//				}
//			}
//		}
//		//Final velocity is a linear combination of PIC and FLIP components
//		p.velocity = flip * FLIP_PERCENT + pic * (1 - FLIP_PERCENT);
//		//VISUALIZATION: Update density
//		p.density /= node_area;
//	}
//	collisionParticles();
//}
//
//
//void Grid::collisionGrid() {
//	Vector2f delta_scale = Vector2f(TIMESTEP);
//	delta_scale /= cellsize;
//
//	for (int y = 0, idx = 0; y < size[1]; y++) {
//		for (int x = 0; x < size[0]; x++, idx++) {
//			//Get grid node (equivalent to (y*size[0] + x))
//			GridNode& node = nodes[idx];
//			//Check to see if this node needs to be computed
//			if (node.active) {
//				//Collision response
//				//TODO: make this work for arbitrary collision geometry
//				Vector2f new_pos = node.velocity_new * delta_scale + Vector2f(x, y);
//				//Left border, right border
//				if (new_pos[0] < BSPLINE_RADIUS || new_pos[0] > size[0] - BSPLINE_RADIUS - 1) {
//					node.velocity_new[0] = 0;
//					node.velocity_new[1] *= STICKY;
//				}
//				//Bottom border, top border
//				if (new_pos[1] < BSPLINE_RADIUS || new_pos[1] > size[1] - BSPLINE_RADIUS - 1) {
//					node.velocity_new[0] *= STICKY;
//					node.velocity_new[1] = 0;
//				}
//			}
//		}
//	}
//}
//
//
//void Grid::collisionParticles() const {
//	for (int i = 0; i < obj->size; i++) {
//		Particle& p = obj->particles[i];
//		Vector2f new_pos = p.grid_position + TIMESTEP * p.velocity / cellsize;
//		//Left border, right border
//		if (new_pos[0] < BSPLINE_RADIUS - 1 || new_pos[0] > size[0] - BSPLINE_RADIUS)
//			p.velocity[0] = -STICKY * p.velocity[0];
//		//Bottom border, top border
//		if (new_pos[1] < BSPLINE_RADIUS - 1 || new_pos[1] > size[1] - BSPLINE_RADIUS)
//			p.velocity[1] = -STICKY * p.velocity[1];
//	}
//}
//
//
//void Grid::draw() {
//
//	if (SUPPORTS_POINT_SMOOTH) glDisable(GL_POINT_SMOOTH);
//
//	//Grid nodes
//	glPointSize(1);
//	glColor3f(.2, .2, .2);
//
//	glBegin(GL_POINTS);
//	for (int i = 0; i < size[0]; i++) {
//		for (int j = 0; j < size[1]; j++)
//			glVertex2fv((origin + cellsize * Vector2f(i, j)).data);
//	}
//	glEnd();
//}