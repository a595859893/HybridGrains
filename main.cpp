// Create by ���쿡 16307064

// ����_USE_MATH_DEFINES��ȷ������ʹ��math���е�PI����
#define _USE_MATH_DEFINES

// �Դ�������
#include <stdio.h>
#include <memory.h>

// opengl��ؿ�����
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// �Զ���ͷ�ļ�����
#include "Simulator.h"
#include "Constant.h"

#define WINDOWS_HEIGHT 800
#define WINDOWS_WIDTH 800

void Cleanup();

Simulator sim;
GLuint VaoIdx, VboIdx, ColorIdx;
GLuint GridVaoIdx, GridVboIdx, GridColorIdx;
GLuint VertexShaderId, FragmentShaderId, ProgramId;

char* ReadShader(const char* path) {
	// ���ļ��ж�ȡ�����ַ����ĺ���
	FILE* fp = fopen(path, "r");
	fseek(fp, 0L, SEEK_END);
	long fileSize = ftell(fp);

	char* fileCtx = (char*)malloc(sizeof(char) * fileSize);
	if (fileCtx) {
		memset(fileCtx, 0, sizeof(char) * fileSize);
		fseek(fp, 0L, SEEK_SET);
		fread(fileCtx, 1, fileSize, fp);
	}
	fclose(fp);
	return fileCtx;
}

void InitShaders() {
	// ��shader�ļ��ж�ȡ�ַ���
	GLchar* VertexShader = ReadShader("./shader.vert");
	GLchar* FragmentShader = ReadShader("./shader.frag");

	// ���벢��shader
	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
	glCompileShader(VertexShaderId);

	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
	glCompileShader(FragmentShaderId);


	// ��shader����program��
	ProgramId = glCreateProgram();
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId);
	glUseProgram(ProgramId);

	char buf[1024];
	int len;
	GLint compileResult = GL_TRUE;
	glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &compileResult);
	glGetShaderInfoLog(VertexShaderId, 1024, &len, buf);
	printf(buf);

	// �ͷ��ڴ�
	free(VertexShader);
	free(FragmentShader);

	// ������Ϣ
	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr, "(%d) %s \n", ErrorCheckValue, gluErrorString(ErrorCheckValue));
		exit(-1);
	}
}

void InitWindow(int argc, char* argv[]) {
	// ���ڳ�ʼ��ģ��
	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitWindowSize(WINDOWS_WIDTH, WINDOWS_HEIGHT);
	glutInitDisplayMode(GLUT_RGB);
	glutCreateWindow("Hybird Grains");
}

void Initialize(int argc, char* argv[]) {
	// ��ʼ������
	InitWindow(argc, argv);

	GLenum GlewInitResult = glewInit();
	if (GLEW_OK != GlewInitResult) {
		exit(EXIT_FAILURE);
	}

	// ������ɫ�͹رմ��ڵ��ڴ��ͷ�
	glClearColor(0, 0, 0, 0);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glutCloseFunc(Cleanup);
}

void InitBuffer() {
	// ����չʾBuffer
	glGenVertexArrays(1, &VaoIdx);
	glGenBuffers(1, &VboIdx);
	glGenBuffers(1, &ColorIdx);

	glBindVertexArray(VaoIdx);

	glBindBuffer(GL_ARRAY_BUFFER, VboIdx);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sim.grainsNum * GRAINS_DIM, sim.grainsPos, GL_STREAM_DRAW);
	glVertexAttribPointer(0, GRAINS_DIM, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, ColorIdx);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sim.grainsNum * 3, sim.grainsColor, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// Grid���ӻ�Buffer
	glGenVertexArrays(1, &GridVaoIdx);
	glGenBuffers(1, &GridVboIdx);
	glGenBuffers(1, &GridColorIdx);

	glBindVertexArray(GridVaoIdx);

	glBindBuffer(GL_ARRAY_BUFFER, GridVboIdx);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sim.grid.size[0] * sim.grid.size[1] * 6 * GRAINS_DIM, sim.gridPos, GL_STATIC_DRAW);
	glVertexAttribPointer(0, GRAINS_DIM, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, GridColorIdx);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * sim.grid.size[0] * sim.grid.size[1] * 6 * 3, sim.gridColor, GL_STREAM_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr, "(%d) %s \n", ErrorCheckValue, gluErrorString(ErrorCheckValue));
		exit(-1);
	}
}

void RenderFunction(void) {
	// �������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ��������
	SimUpdate(&sim);

	// Grid���ӻ�
	glBindVertexArray(GridVaoIdx);
	glBindBuffer(GL_ARRAY_BUFFER, GridVboIdx);
	glBindBuffer(GL_ARRAY_BUFFER, GridColorIdx);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * sim.grid.size[0] * sim.grid.size[1] * 6 * 3, sim.gridColor);
	glDrawArrays(GL_TRIANGLES, 0, sim.grid.size[0] * sim.grid.size[1] * 6);

	// Grain����
	glBindVertexArray(VaoIdx);
	glBindBuffer(GL_ARRAY_BUFFER, ColorIdx);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * sim.grainsNum * 3, sim.grainsColor);
	glBindBuffer(GL_ARRAY_BUFFER, VboIdx);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * sim.grainsNum * GRAINS_DIM, sim.grainsPos);
	glDrawArrays(GL_POINTS, 0, sim.grainsNum * 3);

	//for (int i = 0; i < sim.grainsNum ; i++) {
	//	printf("%f %f %f \n", sim.grainsColor[i * 3 + 0], sim.grainsColor[i * 3 + 1], sim.grainsColor[i * 3 + 2]);
	//}


	glutSwapBuffers();
	glutPostRedisplay();

	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr, "(%d) %s \n", ErrorCheckValue, gluErrorString(ErrorCheckValue));
		exit(-1);
	}
}

void Cleanup()
{
	// �ڴ��ͷ�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &VaoIdx);

	GLenum ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(stderr, "ERROR: Could not destroy the VBO: %s \n", gluErrorString(ErrorCheckValue));
		exit(-1);
	}
}

int main(int argc, char* argv[])
{
	// ��ʼ�����ں�ģ���������
	Initialize(argc, argv);
	InitSimulator(&sim, 100);
	InitShaders();
	InitBuffer();

	// �󶨺���
	glutDisplayFunc(RenderFunction);

	// ��ѭ��
	glutMainLoop();
	return 0;
}