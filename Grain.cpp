#include "Grain.h"
#include "Constant.h"
#include <Eigen/Dense>

void InitGrain(Grain* grain, glm::vec2 pos, glm::vec2 velocity, GLfloat mass, GLfloat lambda, GLfloat mu) {
	grain->pos = pos;
	grain->velocity = velocity;
	grain->mass = mass;
	grain->lambda;
	grain->mu;

	grain->color = glm::vec3(1.0f, 0.1f, 0.1f);

	grain->defElastic = glm::mat2x2(1.0);
	grain->defPlastic = glm::mat2x2(1.0);

	grain->svdE = glm::vec2(1.0, 1.0);
	grain->svdW = glm::mat2x2(1.0);
	grain->svdV = glm::mat2x2(1.0);

	grain->polarR = glm::mat2x2(1.0);
	grain->polarS = glm::mat2x2(1.0);
}

void updatePos(Grain* grain) {
	grain->pos += TIMESTEP * grain->velocity;
}
void updateGradient(Grain* grain) {
	grain->velocityGard = TIMESTEP * grain->velocityGard + glm::mat2x2(1.0);
	grain->defElastic = grain->velocityGard * grain->defElastic;
}
void applyPlasticity(Grain* grain) {
	glm::mat2x2 fAll = grain->defElastic + grain->defPlastic;
	Eigen::Matrix2f defElastic = CoverMat2x2(grain->defElastic);
	Eigen::JacobiSVD<Eigen::Matrix2f> svd(defElastic);

	// TODO 映射关系存疑，原先的uv和这里的vw之间的关系是？
	// 目前假设V是一样的，U=W
	grain->svdV = CoverMat2x2(svd.matrixV());
	grain->svdW = CoverMat2x2(svd.matrixU());
	grain->svdE = CoverVec2(svd.singularValues());
	grain->svdE = glm::clamp(grain->svdE, CRIT_COMPRESS, CRIT_COMPRESS);

	grain->defElastic = grain->svdV * glm::transpose(grain->svdW) * fAll;
	grain->defElastic = grain->svdW * glm::transpose(grain->svdV);
}
glm::mat2x2 energyDerivative(Grain* grain) {
	GLfloat harden = exp(HARDENING * (1 - glm::determinant(grain->defPlastic))),
		Je = grain->svdE[0] * grain->svdE[1];
	glm::mat2x2 temp = 2 * grain->mu * \
		(grain->defElastic - grain->svdW * glm::transpose(grain->svdV)) \
		* glm::transpose(grain->defElastic)\
		+ glm::mat2x2(grain->lambda * Je * (Je - 1));
	return grain->vol * harden * temp;
}
//glm::vec2 deltaForce(const glm::vec2 u, const glm::vec2 weightGard);