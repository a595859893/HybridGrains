#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <Eigen/Dense>

glm::mat2x2 CoverMat2x2(Eigen::Matrix2f mat);
Eigen::Matrix2f CoverMat2x2(glm::mat2x2  mat);
glm::vec2 CoverVec2(Eigen::Vector2f vec);

GLfloat bspline(GLfloat x);
GLfloat bsplineSlope(GLfloat x);

static const glm::vec2 GRAVITY(0, -9.8);
static const GLint GRAINS_DIM = 2;
static const GLint BSPLINE_RADIUS = 2;

static const GLfloat
PARTICLE_DIAM = .005f,		//Diameter of each particle; smaller = higher resolution
MAX_TIMESTEP = 5e-4f,		//Upper timestep limit
FLIP_PERCENT = .95f,			//Weight to give FLIP update over PIC (.95)
CRIT_COMPRESS = 1 - 1.9e-2f,	//Fracture threshold for compression (1-2.5e-2)
CRIT_STRETCH = 1 + 7.5e-3f,	//Fracture threshold for stretching (1+7.5e-3)
HARDENING = 5.0f,			//How much plastic deformation strengthens material (10)
DENSITY = 100,				//Density of snow in kg/m^2 (400 for 3d)
YOUNGS_MODULUS = 1.5e5f,		//Young's modulus (springiness) (1.4e5)
POISSONS_RATIO = .2f,		//Poisson's ratio (transverse/axial strain ratio) (.2)
IMPLICIT_RATIO = 0,			//Percentage that should be implicit vs explicit for velocity update
MAX_IMPLICIT_ITERS = 30,	//Maximum iterations for the conjugate residual
MAX_IMPLICIT_ERR = 1e4f,		//Maximum allowed error for conjugate residual
MIN_IMPLICIT_ERR = 1e-4f,	//Minimum allowed error for conjugate residual
STICKY = .9f,				//Collision stickiness (lower = stickier)
TIMESTEP = 0.0001f,
BSPLINE_EPSILON = 1e-4f;