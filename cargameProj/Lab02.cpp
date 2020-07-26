// ffff.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h> 
#include <GLFW\glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



using namespace std;

#define PI       3.14159265359

//实现鼠标交互操作的变量：
static float c = 3.1415926 / 180.0f;
static float r = 1.0f;
static int degree = 90;

bool leftMouseDown = false;
bool rightMouseDown = false;
bool middleMouseDown = false;
bool first_view = false;
bool texture_mode = false;
float mousex, mousey;
float translatex = 0;
float translatey = 0;
float cameraDistance = 40.0f;
float cameraAnglex = 0;
float cameraAngley = 120;

//控制小车移动的变量：
float movementx = 0;
float movementy = 0;
float turnAngle = 0;
float a = 0.005f;
float v_max = 0.05f;
float v = 0;

unsigned int texname;

GLdouble    fovy = 60.0;
GLdouble    nearPlane = 1.0;
GLdouble    farPlane = 1000.0;

GLfloat     angle = 0.0;
GLfloat     torusAngle = 0.0;

GLfloat     lightPos[] = { 25.0, 25.0, 25.0, 1.0 };
GLfloat     lookat[] = { 0.0, 0.0, 0.0 };
GLfloat     up[] = { 0.0, 0.0, 1.0 };

GLboolean showShadow = GL_FALSE;

/*初始化光源与阴影图*/
void init(void)
{
	GLfloat  white[] = { 1.0, 1.0, 1.0, 1.0 };

	//生成深度纹理
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		256, 256, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);

	//设置阴影图相关过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	//自动生成纹理坐标
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glCullFace(GL_BACK);//剔除背面

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_CULL_FACE);
}

//设置纹理
void tex_init(const char * jpg) {
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);	//绑定纹理，后续操作对象都是tex_shadow的纹理进行
	//纹理环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	//纹理过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//深度纹理
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(jpg, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);	//？
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, texname);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)width / height, nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
}

void idle(void)
{
	angle += PI / 10000;
	torusAngle += .1;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'r':
		texture_mode = !texture_mode;
		if (v > v_max) {
			v = v_max;
		}
		if (v < -v_max / 5.0) {
			v = -v_max / 5.0;
		}
		break;
	case 't':
		first_view = !first_view;
		break;
	case ' ':
		v = 0;
		break;
	case 'w':
		v = v + a;
		if (v > v_max) {
			v = v_max;
		}
		cout << "velocity:" << v << '\t' << "acceleration:" << a << endl;
		break;
	case 's':
		v = v - a;
		if (v < -v_max/5.0) {
			v = -v_max/5.0;
		}
		cout << "velocity:" << v << '\t' << "acceleration:" << a << endl;
		break;
	case 'a':
		turnAngle += 15.0;
		break;
	case 'd':
		turnAngle -= 15.0;
		break;
	case 27:  /* Escape */
		exit(0);
		break;
		//开启与关闭纹理贴图
	case 'c': {
		static GLboolean textureOn = GL_TRUE;
		textureOn = !textureOn;
		if (textureOn)
			glEnable(GL_TEXTURE_2D);
		else
			glDisable(GL_TEXTURE_2D);
	}
			  break;
			  //深度纹理贴图的两种模式对比
	case 'm': {
		static GLboolean compareMode = GL_TRUE;
		compareMode = !compareMode;
		printf("Compare mode %s\n", compareMode ? "On" : "Off");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			compareMode ? GL_COMPARE_R_TO_TEXTURE : GL_NONE);
	}
			  break;
			  //比较规则切换
	case 'f': {
		static GLboolean funcMode = GL_TRUE;
		funcMode = !funcMode;
		printf("Operator %s\n", funcMode ? "GL_LEQUAL" : "GL_GEQUAL");
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, funcMode ? GL_LEQUAL : GL_GEQUAL);//当前片段z与阴影图z比较规则(<=, >=)
	}
			  break;
			  //是否显示阴影图
	}

	glutPostRedisplay();
}


void transposeMatrix(GLfloat m[16])
{
	GLfloat  tmp;
#define Swap( a, b )    tmp = a; a = b; b = tmp
	Swap(m[1], m[4]);
	Swap(m[2], m[8]);
	Swap(m[3], m[12]);
	Swap(m[6], m[9]);
	Swap(m[7], m[13]);
	Swap(m[11], m[14]);
#undef Swap
}

/*绘制场景上对象*/
void draw_track() {
	float hight = 0.001f;
	
	if (texture_mode) {
		a = 0.01;
		v_max = 0.5;
		tex_init("grass.jpg");

		//绘制地面
		glNormal3f(0, 0, 1);
		glColor3f(1, 0.6, 0.2);

		glBindTexture(GL_TEXTURE_2D, texname);

		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-20.0f, -20.0f, 0);
		glTexCoord2f(50, 0);
		glVertex3f(20.0f, -20.0f, 0);
		glTexCoord2f(50, 50);
		glVertex3f(20.0f, 20.0f, 0);
		glTexCoord2f(0, 50);
		glVertex3f(-20.0f, 20.0f, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		//跑道的直线部分：

		tex_init("road.jpg");

		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glTexCoord2f(0, 0);
		glVertex3f(-10.0f, -8.0f, hight);
		glTexCoord2f(1, 0);
		glVertex3f(-5.0f, -8.0f, hight);
		glTexCoord2f(1, 1);
		glVertex3f(-5.0f, 8.0f, hight);
		glTexCoord2f(0, 1);
		glVertex3f(-10.0f, 8.0f, hight);
		glEnd();

		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glTexCoord2f(1, 0);
		glVertex3f(10.0f, -8.0f, hight);
		glTexCoord2f(1, 1);
		glVertex3f(10.0f, 8.0f, hight);
		glTexCoord2f(0, 1);
		glVertex3f(5.0f, 8.0f, hight);
		glTexCoord2f(0, 0);
		glVertex3f(5.0f, -8.0f, hight);
		glEnd();

		//跑道的圆环部分：
		float i;
		for (i = 0; i <= PI - 0.01; i += 0.01) {
			glBegin(GL_QUADS);
			glColor3f(0.0, 0.6, 0.2);
			glTexCoord2f(0, 0);
			glVertex3f(-5.0 * cos(i), 8 + 5.0 * sin(i), hight);
			glTexCoord2f(1, 0);
			glVertex3f(-5.0 * cos(i + 0.01), 8 + 5.0 * sin(i + 0.01), hight);
			glTexCoord2f(1, 1);
			glVertex3f(-10.0 * cos(i + 0.01), 8 + 10.0 * sin(i + 0.01), hight);
			glTexCoord2f(0, 1);
			glVertex3f(-10.0 * cos(i), 8 + 10.0 * sin(i), hight);
			glEnd();
		}
		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glTexCoord2f(0, 0);
		glVertex3f(-5.0 * cos(i), 8 + 5.0 * sin(i), hight);
		glTexCoord2f(1, 0);
		glVertex3f(5.0, 8.0, hight);
		glTexCoord2f(1, 1);
		glVertex3f(10.0, 8.0, hight);
		glTexCoord2f(0, 1);
		glVertex3f(-10.0 * cos(i), 8 + 10.0 * sin(i), hight);

		glEnd();

		for (i = 0; i <= PI - 0.01; i += 0.01) {
			glBegin(GL_QUADS);
			glColor3f(0.0, 0.6, 0.2);
			glTexCoord2f(0, 0);
			glVertex3f(-5.0 * cos(i), -8 - 5.0 * sin(i), hight);
			glTexCoord2f(1, 0);
			glVertex3f(-10.0 * cos(i), -8 - 10.0 * sin(i), hight);
			glTexCoord2f(1, 1);
			glVertex3f(-10.0 * cos(i + 0.01), -8 - 10.0 * sin(i + 0.01), hight);
			glTexCoord2f(0, 1);
			glVertex3f(-5.0 * cos(i + 0.01), -8 - 5.0 * sin(i + 0.01), hight);
			glEnd();
		}
		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glTexCoord2f(0, 0);
		glVertex3f(-5.0 * cos(i), -8 - 5.0 * sin(i), hight);
		glTexCoord2f(1, 0);
		glVertex3f(-10.0 * cos(i), -8 - 10.0 * sin(i), hight);
		glTexCoord2f(1, 1);
		glVertex3f(10.0, -8.0, hight);
		glTexCoord2f(0, 1);
		glVertex3f(5.0, -8.0, hight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else {
	a = 0.005;
	v_max = 0.05;
	//绘制地面
	glNormal3f(0, 0, 1);
	glColor3f(1, 0.6, 0.2);
	glBegin(GL_QUADS);
	glVertex3f(-20.0f, -20.0f, 0);
	glVertex3f(20.0f, -20.0f, 0);
	glVertex3f(20.0f, 20.0f, 0);
	glVertex3f(-20.0f, 20.0f, 0);
	glEnd();

	//跑道的直线部分：
	glBegin(GL_QUADS);
	glColor3f(0.0, 0.6, 0.2);
	glVertex3f(-10.0f, -8.0f, hight);
	glVertex3f(-5.0f, -8.0f, hight);
	glVertex3f(-5.0f, 8.0f, hight);
	glVertex3f(-10.0f, 8.0f, hight);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0, 0.6, 0.2);
	glVertex3f(10.0f, -8.0f, hight);
	glVertex3f(10.0f, 8.0f, hight);
	glVertex3f(5.0f, 8.0f, hight);
	glVertex3f(5.0f, -8.0f, hight);
	glEnd();

	//跑道的圆环部分：
	float i;
	for (i = 0; i <= PI - 0.01; i += 0.01) {
		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glVertex3f(-5.0 * cos(i), 8 + 5.0 * sin(i), hight);
		glVertex3f(-5.0 * cos(i + 0.01), 8 + 5.0 * sin(i + 0.01), hight);
		glVertex3f(-10.0 * cos(i + 0.01), 8 + 10.0 * sin(i + 0.01), hight);
		glVertex3f(-10.0 * cos(i), 8 + 10.0 * sin(i), hight);
		glEnd();
	}
	glBegin(GL_QUADS);
	glColor3f(0.0, 0.6, 0.2);
	glVertex3f(-5.0 * cos(i), 8 + 5.0 * sin(i), hight);
	glVertex3f(5.0, 8.0, hight);
	glVertex3f(10.0, 8.0, hight);
	glVertex3f(-10.0 * cos(i), 8 + 10.0 * sin(i), hight);

	glEnd();

	for (i = 0; i <= PI - 0.01; i += 0.01) {
		glBegin(GL_QUADS);
		glColor3f(0.0, 0.6, 0.2);
		glVertex3f(-5.0 * cos(i), -8 - 5.0 * sin(i), hight);
		glVertex3f(-10.0 * cos(i), -8 - 10.0 * sin(i), hight);
		glVertex3f(-10.0 * cos(i + 0.01), -8 - 10.0 * sin(i + 0.01), hight);
		glVertex3f(-5.0 * cos(i + 0.01), -8 - 5.0 * sin(i + 0.01), hight);
		glEnd();
	}
	glBegin(GL_QUADS);
	glColor3f(0.0, 0.6, 0.2);
	glVertex3f(-5.0 * cos(i), -8 - 5.0 * sin(i), hight);
	glVertex3f(-10.0 * cos(i), -8 - 10.0 * sin(i), hight);
	glVertex3f(10.0, -8.0, hight);
	glVertex3f(5.0, -8.0, hight);
	glEnd();
}
}


void draw_car() {

	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	//车底面
	glBegin(GL_POLYGON);
	glColor3f(0.9, 0.2, 0.8);
	glVertex3f(-1.0f, 2.0f, 0.5f);
	glVertex3f(1.0f, 2.0f, 0.5f);
	glVertex3f(1.0f, -2.0f, 0.5f);
	glVertex3f(-1.0f, -2.0f, 0.5f);
	glEnd();

	//车头
	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0.1);
	glVertex3f(-1.0f, 2.0f, 0.5f);
	glVertex3f(-1.0f, 2.0f, 1.5f);
	glColor3f(1, 0, 0);
	glVertex3f(1.0f, 2.0f, 1.5f);
	glVertex3f(1.0f, 2.0f, 0.5f);
	glEnd();

	//车尾
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.2, 1);
	glVertex3f(-1.0f, -2.0f, 1.5f);
	glVertex3f(-1.0f, -2.0f, 0.5f);
	glColor3f(1, 0.2, 0.1);
	glVertex3f(1.0f, -2.0f, 0.5f);
	glVertex3f(1.0f, -2.0f, 1.5f);
	glEnd();
	//左侧面
	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0.1);
	glVertex3f(-1.0f, 2.0f, 1.5f);
	glVertex3f(-1.0f, 2.0f, 0.5f);
	glColor3f(0.1, 0.1, 0.8);
	glVertex3f(-1.0f, -2.0f, 0.5f);
	glVertex3f(-1.0f, -2.0f, 1.5f);
	glEnd();
	//右侧面
	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0.1);
	glVertex3f(1.0f, 2.0f, 0.5f);
	glVertex3f(1.0f, 2.0f, 1.5f);
	glColor3f(0.1, 0.1, 0.8);
	glVertex3f(1.0f, -2.0f, 1.5f);
	glVertex3f(1.0f, -2.0f, 0.5f);
	glEnd();
	//车顶(从头向尾绘制)
	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0);
	glVertex3f(1.0f, 2.0f, 1.5f);
	glVertex3f(-1.0f, 2.0f, 1.5f);
	glVertex3f(-1.0f, 1.0f, 1.5f);
	glVertex3f(1.0f, 1.0f, 1.5f);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0, 1, 1);
	glVertex3f(-1.0f, 1.0f, 1.5f);
	glVertex3f(-1.0f, 0.5f, 2.5f);
	glVertex3f(1.0f, 0.5f, 2.5f);
	glVertex3f(1.0f, 1.0f, 1.5f);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0);
	glVertex3f(1.0f, 0.5f, 2.5f);
	glVertex3f(-1.0f, 0.5f, 2.5f);
	glVertex3f(-1.0f, -1.0f, 2.5f);
	glVertex3f(1.0f, -1.0f, 2.5f);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(0, 1, 1);
	glVertex3f(1.0f, -1.0f, 2.5f);
	glVertex3f(-1.0f, -1.0f, 2.5f);
	glVertex3f(-1.0f, -1.2f, 1.5f);
	glVertex3f(1.0f, -1.2f, 1.5f);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3f(1, 1, 0);
	glVertex3f(1.0f, -1.2f, 1.5f);
	glVertex3f(-1.0f, -1.2f, 1.5f);
	glVertex3f(-1.0f, -2.0f, 1.5f);
	glVertex3f(1.0f, -2.0f, 1.5f);
	glEnd();
	//左车窗
	glBegin(GL_POLYGON);
	glColor3f(0, 1, 1);
	glVertex3f(-1.0f, 0.5f, 2.5f);
	glVertex3f(-1.0f, 1.0f, 1.5f);
	glVertex3f(-1.0f, -1.2f, 1.5f);
	glVertex3f(-1.0f, -1.2f, 2.5f);
	glEnd();
	//右车窗
	glBegin(GL_POLYGON);
	glColor3f(0, 1, 1);
	glVertex3f(1.0f, 1.0f, 1.5f);
	glVertex3f(1.0f, 0.5f, 2.5f);
	glVertex3f(1.0f, -1.2f, 2.5f);
	glVertex3f(1.0f, -1.2f, 1.5f);
	glEnd();

	//车轮	
	glColor3f(1.0f, 0.5f, 0.2f);

	glTranslated(0.6f, 1.3f, 0.25);
	glRotatef(90, 0, 1, 0);
	glutSolidTorus(0.1, 0.25, 5, 100);

	//glRotatef(-90, 0, 1, 0);
	glTranslated(-0.6f, 1.3f, 0.25);
	glRotatef(90, 0, 1, 0);
	glutSolidTorus(0.1, 0.25, 5, 100);

	glRotatef(-90, 0, 1, 0);
	glTranslated(0, -2.6f, 0);
	glRotatef(90, 0, 1, 0);
	glutSolidTorus(0.1, 0.25, 5, 100);

	glRotatef(-90, 0, 1, 0);
	glTranslated(1.2, 0, 0);
	glRotatef(90, 0, 1, 0);
	glutSolidTorus(0.1, 0.25, 5, 100);

	glPopMatrix();
}

void draw_sign() {
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-11, 8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-4, 8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(4, 8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(11, 8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-11, -8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(-4, -8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(4, -8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(11, -8, 0);
	glutSolidCone(0.5, 1.5, 30, 30);
	glPopMatrix();
}

void drawObjects(GLboolean shadowRender)
{
	GLboolean textureOn = glIsEnabled(GL_TEXTURE_2D);

	if (shadowRender)
		glDisable(GL_TEXTURE_2D);

	//绘制地面及跑道
	if (!shadowRender) {
		
		draw_track();
	}
	
	//绘制小车
	glPushMatrix();
	glTranslatef(-movementy-7.5, -movementx, 0);
	glRotatef(turnAngle, 0, 0, 1);
	draw_car();
	//圆光源
	glPushMatrix();
	glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
	glColor3f(1, 1, 1);
	glutWireSphere(0.5, 6, 6);
	glPopMatrix();
	//绘制路标：
	if (texture_mode) {
		tex_init("sign.jpg");
		draw_sign();
		glDisable(GL_TEXTURE_2D);
	}
	else
		draw_sign();
	

	if (shadowRender && textureOn)
		glEnable(GL_TEXTURE_2D);
}

/*生成阴影图*/
void generateShadowMap(void)
{
	GLint    viewport[4];
	GLfloat  lightPos[4];

	glGetLightfv(GL_LIGHT0, GL_POSITION, lightPos);//获得光源的位置
	glGetIntegerv(GL_VIEWPORT, viewport);//获得视口信息

	glViewport(0, 0, 256, 256);//设置视口与阴影图大小匹配

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(80.0, 1.0, 10.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();
	gluLookAt(lightPos[0], lightPos[1], lightPos[2],
		lookat[0], lookat[1], lookat[2],
		up[0], up[1], up[2]);//把摄像机放到光源处

	drawObjects(GL_TRUE);//绘制场景

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, 256, 256, 0);//摄像机移到光源处渲染的阴影图

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);//恢复当前视口

	//绘制阴影图
	//if (showShadow) {
		//GLfloat depthImage[256][256];
		//glReadPixels(0, 0, 256, 256, GL_DEPTH_COMPONENT, GL_FLOAT, depthImage);
		//glWindowPos2f(viewport[2] / 2, 0);//设置阴影图绘制位置
		//glDrawPixels(256, 256, GL_LUMINANCE,
		//	GL_FLOAT, depthImage);
		//glutSwapBuffers();
	//}
}

//自动生成阴影图的纹理坐标
void generateTextureMatrix(void)
{
	GLfloat  tmpMatrix[16];

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.5, 0.5, 0.0);
	glScalef(0.5, 0.5, 1.0);
	gluPerspective(60.0, 1.0, 1.0, 1000.0);
	gluLookAt(lightPos[0], lightPos[1], lightPos[2],
		lookat[0], lookat[1], lookat[2],
		up[0], up[1], up[2]);
	glGetFloatv(GL_MODELVIEW_MATRIX, tmpMatrix);
	glPopMatrix();

	transposeMatrix(tmpMatrix);

	glTexGenfv(GL_S, GL_OBJECT_PLANE, &tmpMatrix[0]);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, &tmpMatrix[4]);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, &tmpMatrix[8]);
	glTexGenfv(GL_Q, GL_OBJECT_PLANE, &tmpMatrix[12]);
}

//渲染
void display(void)
{
	//计算物体的当前位移
	movementx -= v * cos(-c * turnAngle);
	movementy -= v * sin(-c * turnAngle);
	//计算新的摄像机的位置
	GLfloat a = c * (cameraAnglex - 90);
	GLfloat b = c * (270 - cameraAngley);
	GLfloat x = cameraDistance * translatex * cos(a) + cameraDistance * translatey * sin(b) * sin(a) + cameraDistance * cos(c * cameraAnglex) * sin(c * cameraAngley);
	GLfloat y = cameraDistance * translatey * cos(b) + cameraDistance * cos(c * cameraAngley);
	GLfloat z = cameraDistance * translatex * sin(a) - cameraDistance * translatey * cos(a) * sin(b) + cameraDistance * sin(c * cameraAnglex) * sin(c * cameraAngley);


	GLfloat  radius = 30;

	generateShadowMap();
	generateTextureMatrix();

	if (showShadow)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glLoadIdentity();
	if (first_view) {
		gluLookAt(-movementy - 7.5, -movementx, 2.0, 
			-movementy - 7.5 + sin(-c * turnAngle), -movementx + cos(-c * turnAngle), 2.0, 
			up[0], up[1], up[2]);
	}
	else {
		gluLookAt(z, y, x,
			lookat[0], lookat[1], lookat[2],
			up[0], up[1], up[2]);
	}
	drawObjects(GL_FALSE);
	glPopMatrix();

	glutSwapBuffers();
}

void mouseC(int button, int state, int x, int y) {
	mousex = x;
	mousey = y;
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
			leftMouseDown = true;
		else if (state == GLUT_UP)
			leftMouseDown = false;
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN)
			rightMouseDown = true;
		else if (state == GLUT_UP)
			rightMouseDown = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON) {
		if (state == GLUT_DOWN)
			middleMouseDown = true;
		else if (state == GLUT_UP)
			middleMouseDown = false;
	}
}

void mouseMotion(int x, int y) {
	if (leftMouseDown) {
		cameraAngley += (x - mousex);
		cameraAnglex += (y - mousey);
		mousex = x;
		mousey = y;
		/*movementx += 2.5*cos(-c * turnAngle);
		movementy += 2.5*sin(-c * turnAngle);*/
	}
	if (middleMouseDown) {
		cameraDistance += (y - mousey)*0.5f;
		mousey = y;
	}
	if (rightMouseDown) {
		translatex += (x - mousex)*0.1f;
		translatey -= (y - mousey)*0.1f;
		mousex = x;
		mousey = y;
	}
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);

	init();
	

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseC);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	return 0;
}
