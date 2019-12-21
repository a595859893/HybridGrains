#pragma once
#include <gl/glew.h>

#include "Grid.h"

struct Simulator {
	GLint width;
	GLint height;
	GLint grainsNum;
	Grain* grains;
	Grid grid;

	GLfloat* grainsPos;
	GLfloat* grainsColor;

	GLfloat* gridPos;
	GLfloat* gridColor;
};

void InitSimulator(Simulator* sim, GLint grainNum);
void SimUpdate(Simulator* sim);