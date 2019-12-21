#include "Grain.h"
#include "Constant.h"
#include <Eigen/Dense>

void InitGrain(Grain* grain, glm::vec2 pos, glm::vec2 velocity, GLfloat mass, GLfloat lambda, GLfloat mu) {
	grain->pos = pos;
	grain->velocity = velocity;
	grain->mass = mass;
	grain->lambda = LAMBDA;
	grain->mu = MU;

	grain->defElastic = glm::mat2x2(1.0);
	grain->defPlastic = glm::mat2x2(1.0);

	grain->svdE = glm::vec2(1.0, 1.0);
	grain->svdW = glm::mat2x2(1.0);
	grain->svdV = glm::mat2x2(1.0);

	grain->polarR = glm::mat2x2(1.0);
	grain->polarS = glm::mat2x2(1.0);
}

// 根据计算出来的速度更新本帧所在的位置
void updatePos(Grain* grain) {
	grain->pos += TIMESTEP * grain->velocity;
}

// 更新速度梯度
void updateGradient(Grain* grain) {
	if (!(grain->velocityGard[0][0] == grain->velocityGard[0][0])) {
		int a = 0;
	}

	grain->velocityGard = TIMESTEP * grain->velocityGard + glm::mat2x2(1.0);

	grain->defElastic = grain->velocityGard * grain->defElastic;

	if (!(grain->defElastic[0][0] == grain->defElastic[0][0])) {
		int a = 0;
	}
}

// 应用塑性形变
void applyPlasticity(Grain* grain) {
	glm::mat2x2 fAll = grain->defElastic + grain->defPlastic;
	if (!(grain->defElastic[0][0] == grain->defElastic[0][0])) {
		int a = 0;
	}

	svd(grain->defElastic, &(grain->svdV), &(grain->svdE), &(grain->svdW));
	grain->svdE = glm::clamp(grain->svdE, CRIT_COMPRESS, CRIT_STRETCH);



	glm::mat2x2 vCopy(grain->svdV), wCopy(grain->svdW);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++)
			wCopy[i][j] *= grain->svdE[i];
	}

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++)
			vCopy[i][j] /= grain->svdE[i];
	}

	// 得到新的弹性形变和塑性形变
	if (glm::determinant(grain->defPlastic) < -50) {
		int a = 0;
	}
	glm::mat2x2 org = grain->defPlastic;
	grain->defPlastic = vCopy * glm::transpose(grain->svdW) * fAll;
	grain->defElastic = wCopy * glm::transpose(grain->svdV);

	if (glm::determinant(grain->defPlastic) < -50) {
		int a = org[0][0];
	}
}

// 能量的导数，通过弹性和塑性形变，计算内部力之和
glm::mat2x2 energyDerivative(Grain* grain) {
	GLfloat lnHarden = HARDENING * (1 - glm::determinant(grain->defPlastic)),
		Je = grain->svdE[0] * grain->svdE[1];

	glm::mat2x2 temp = 2 * grain->mu * \
		(grain->defElastic - grain->svdW * glm::transpose(grain->svdV)) \
		* glm::transpose(grain->defElastic);

	temp += glm::mat2x2(grain->lambda * Je * (Je - 1));

	GLfloat lnVol = log(grain->vol);
	GLfloat expCal = exp(lnVol + lnHarden);

	if (isinf(expCal)) {
		expCal = exp(10);
	}

	return expCal * temp;
}
//glm::vec2 deltaForce(const glm::vec2 u, const glm::vec2 weightGard);