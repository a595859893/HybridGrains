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