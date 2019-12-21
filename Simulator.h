#pragma once
#include <gl/glew.h>

#include "Grain.h"
#include "Grid.h"

struct Simulator {
	GLint width;
	GLint height;
	GLint grainsNum;
	Grain* grains;
	GLfloat* grainsPos;
	GLfloat* grainsColor;
	Grid grid;
};

void InitSimulator(Simulator* sim, GLint width, GLint height, GLint grainNum);
void SimUpdate(Simulator* sim);