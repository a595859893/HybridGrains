#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>

#define MATRIX_EPSILON 1e-6

void svd(glm::mat2x2 in, glm::mat2x2* outw, glm::vec2* oute, glm::mat2x2* outv);
GLfloat bspline(GLfloat x);
GLfloat bsplineSlope(GLfloat x);

static const glm::vec2 GRAVITY(0, -9.8);
static const GLint GRAINS_DIM = 2;
static const GLint BSPLINE_RADIUS = 2;

static const GLfloat
STICKY = .8f,				//Collision stickiness (lower = stickier)
TIMESTEP = 3e-3f,
BSPLINE_EPSILON = 1e-3f,
KAPPA = 0.0001f,
ALPHA = 0.10f,
BETA = 0.4,//越低越倾向于聚在一起
MU = 0.99;