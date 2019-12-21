#include "Constant.h"
#include <math.h>
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

void svd(glm::mat2x2 in, glm::mat2x2* outw, glm::vec2* oute, glm::mat2x2* outv) {
	glm::mat2x2 w, v;
	glm::vec2 e;

	// 如果是对角矩阵，SVD是平凡的
	if (fabs(in[0][1] - in[1][0]) < MATRIX_EPSILON && fabs(in[0][1]) < MATRIX_EPSILON) {
		w = glm::mat2x2(in[0][0] < 0 ? -1 : 1, 0, 0, in[1][1] < 0 ? -1 : 1);
		e = glm::vec2(fabs(in[0][0]), fabs(in[1][1]));
		v = glm::mat2x2(1);
	}

	//否则，我们需要计算A^T*A
	else {
		float j = in[0][0] * in[0][0] + in[0][1] * in[0][1],
			k = in[1][0] * in[1][0] + in[1][1] * in[1][1],
			v_c = in[0][0] * in[1][0] + in[0][1] * in[1][1];
		//若A^T*A为对角阵
		if (fabs(v_c) < MATRIX_EPSILON) {
			float s1 = sqrt(j),
				s2 = fabs(j - k) < MATRIX_EPSILON ? s1 : sqrt(k);
			e = glm::vec2(s1, s2);
			v = glm::mat2x2(1);
			w = glm::mat2x2(
				in[0][0] / s1, in[1][0] / s2,
				in[0][1] / s1, in[1][1] / s2
			);
		}
		//否则解奇异值的二次型
		else {
			float jmk = j - k,
				jpk = j + k,
				root = sqrt(jmk * jmk + 4 * v_c * v_c),
				eig = (jpk + root) / 2,
				s1 = sqrt(eig),
				s2 = fabs(root) < MATRIX_EPSILON ? s1 : sqrt((jpk - root) / 2);
			e = glm::vec2(s1, s2);
			//Use eigenvectors of A^T*A as V
			float v_s = eig - j,
				len = sqrt(v_s * v_s + v_c * v_c);
			v_c /= len;
			v_s /= len;
			v = glm::mat2x2(v_c, -v_s, v_s, v_c);
			//Compute w matrix as Av/s
			w = glm::mat2x2(
				(in[0][0] * v_c + in[1][0] * v_s) / s1,
				(in[1][0] * v_c - in[0][0] * v_s) / s2,
				(in[0][1] * v_c + in[1][1] * v_s) / s1,
				(in[1][1] * v_c - in[0][1] * v_s) / s2
			);
		}
	}

	*outw = w;
	*oute = e;
	*outv = v;
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