#include "Constant.h"

glm::mat2x2 CoverMat2x2(Eigen::Matrix2f mat) {
	return glm::mat2x2(mat(0, 0), mat(0, 1), mat(1, 0), mat(1, 1));
}

Eigen::Matrix2f CoverMat2x2(glm::mat2x2  mat) {
	Eigen::Matrix2f retmat;
	retmat << mat[0][0], mat[0][1],
		mat[1][0], mat[1][1];
	return retmat;
}

glm::vec2 CoverVec2(Eigen::Vector2f vec) {
	return glm::vec2(vec(0), vec(1));
}

GLfloat bspline(GLfloat x) {
	x = fabs(x);
	GLfloat w;
	if (x < 1)
		w = x * x * (x / 2 - 1) + 2 / 3.0;
	else if (x < 2)
		w = x * (x * (-x / 6 + 1) - 2) + 4 / 3.0;
	else return 0;
	//Clamp between 0 and 1... if needed
	if (w < BSPLINE_EPSILON) return 0;
	return w;
}

GLfloat bsplineSlope(GLfloat x) {
	GLfloat abs_x = fabs(x), w;
	if (abs_x < 1)
		return 1.5 * x * abs_x - 2 * x;
	else if (x < 2)
		return -x * abs_x / 2 + 2 * x - 2 * x / abs_x;
	else return 0;
}