#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>

#define WIN_DIM 800
#define GRAVITY -0.0981
#define BULLET_SPEED 3.0
#define MAX_PORTALS 2
#define NUM_WALLS 1

void init(void);
void initWalls(void);
void display(void);
void reshape(int, int);
void moveCamera(void);
void keyIn(unsigned char, int, int);
void keyOut(unsigned char, int, int);
void timerFunc(int);
void keyCheck(void);
void mouseMove(int, int);
void updatePosition(void);
void activeMouseFunction(int, int, int, int);
void shootPBall(int);
void movePBalls(void);
void displayPBalls(void);
void displayWalls(void);
void buildWall(float,float,float,float,float,float);




struct box{
	int xMin,xMax, yMin, yMax, zMin, zMax; 
};

typedef box wall;

struct ball{
	GLfloat pos[3], velocity[3], color[3];
	int exists;
};

typedef ball pBall;

float cameraPos[3];
float cameraXRot, cameraYRot;
float velocity;
int keyMap[256];
wall walls[NUM_WALLS];
int lastX, lastY;

pBall balls[2];

int main (int argc, char **argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);

	glutInitWindowSize(WIN_DIM, WIN_DIM);
	glutInitWindowPosition(100,100);
	
	glutCreateWindow(
		"Portal Balls - Harry Mosher, Matt McCarrick, Jared Dickson");
	
	init();
	glutWarpPointer(WIN_DIM/2,WIN_DIM/2);
	glutTimerFunc(10, timerFunc, 0);
	glutMainLoop();

}

void init(){
	glEnable(GL_DEPTH_TEST);
	
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyIn);
	glutKeyboardUpFunc(keyOut);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutPassiveMotionFunc(mouseMove);
	glutMouseFunc(activeMouseFunction);

	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	cameraPos[0] = 0.0f;
	cameraPos[1] = 0.0f;
	cameraPos[2] = 10.0f;
	cameraXRot = 0;
	cameraYRot = 0;
	lastX = WIN_DIM / 2;
	lastY = WIN_DIM / 2;
	velocity = 0;

	balls[0].exists = 0;
	balls[1].exists = 0;
	
	balls[0].color[0] = 0.0;
	balls[0].color[1] = 0.0;
	balls[0].color[2] = 1.0;
	balls[1].color[0] = 1.0;
	balls[1].color[1] = 0.5;
	balls[1].color[2] = 0.0;
	
	initWalls();

	int i;
	for(i = 0; i < 256; i++)
		keyMap[i] = 0;

}

void display(){
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	
	keyCheck();
	updatePosition();
	moveCamera();
	movePBalls();
	displayPBalls();
	displayWalls();

	glColor3f(1.0f,0.0f,0.0f);
	glutSolidCube(5);

	glutSwapBuffers();

}

void movePBalls(){
	int i;
	for (i = 0; i < MAX_PORTALS; i++){
		if(balls[i].exists){
			balls[i].pos[0] += balls[i].velocity[0];
			balls[i].pos[1] += balls[i].velocity[1];
			balls[i].pos[2] += balls[i].velocity[2];
		}
	}
}

void moveCamera(){
	glRotatef(cameraXRot, 1.0, 0.0, 0.0);
	glRotatef(cameraYRot, 0.0, 1.0, 0.0);
	
	if(cameraPos[1] < 0)
		cameraPos[1] = 0;
	glTranslatef(-cameraPos[0],-cameraPos[1],-cameraPos[2]);
	
}

void displayPBalls(){
	int i;
	for(i = 0; i < MAX_PORTALS; i++){
		if(balls[i].exists){
			glPushMatrix();
				glColor3f(balls[i].color[0], balls[i].color[1], balls[i].color[2]);
				glTranslatef(balls[i].pos[0], balls[i].pos[1], balls[i].pos[2]);
				glutSolidSphere(2.0, 20.0, 20.0);
			glPopMatrix();
		}
	}
	
}

void displayWalls(){
	int i;
	for (i < 0; i < NUM_WALLS; i++){
		glColor3f(1.0,1.0,0.0);
		buildWall(walls[i].xMin,walls[i].xMax,walls[i].yMin,walls[i].xMax,
			walls[i].zMin, walls[i].zMax);
	}

}

void updatePosition(){
	velocity += GRAVITY;
	cameraPos[1] += velocity;
}

void reshape(int width, int height){
	glViewport(0,0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(60, (GLfloat)width/(GLfloat)height, 1.0f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
}

void keyCheck(){
	float yRotRad, xRotRad;

	if(keyMap['a'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		cameraPos[0] -= float(cos(yRotRad)) * 0.75;
		cameraPos[2] -= float(sin(yRotRad)) * 0.75;
	}
	if(keyMap['d'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		cameraPos[0] += float(cos(yRotRad)) * 0.75;
		cameraPos[2] += float(sin(yRotRad)) * 0.75;
	}
	if(keyMap['w'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		xRotRad = (cameraXRot / 180 * 3.141592654f); 
		cameraPos[0] += float(sin(yRotRad)) * 0.75;
		cameraPos[2] -= float(cos(yRotRad)) * 0.75;
		cameraPos[1] -= float(sin(xRotRad)) * 0.75;
	}
	if(keyMap['s'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		xRotRad = (cameraXRot / 180 * 3.141592654f); 
		cameraPos[0] -= float(sin(yRotRad)) * 0.75;
		cameraPos[2] += float(cos(yRotRad)) * 0.75;
		cameraPos[1] += float(sin(xRotRad)) * 0.75;
	}
	if(keyMap['r'] == 1){
		cameraPos[1] += 1;
	}
	if(keyMap['f'] == 1){
		cameraPos[1] -= 1;
	}
	if(keyMap[' '] == 1){
		velocity = 1.0;
	}
}

void keyIn(unsigned char key, int x, int y){
	if (key == 27)
		exit(EXIT_SUCCESS);
	keyMap[key] = 1;
}

void keyOut(unsigned char key, int x, int y){
	keyMap[key] = 0;
}

void timerFunc(int value){
	glutPostRedisplay();
	glutTimerFunc(10, timerFunc, 0);
}

void mouseMove(int x, int y){
	float temp;
	//check the difference between the current x and the last x position
	 int diffx = (x-lastX) * .5;
	 //check the difference between the current y and the last y position
    int diffy = (y-lastY) * .5; 
	//set lastx to the current x position
    lastX = x; 
	//set lasty to the current y position
    lastY = y; 
	//set the xrot to xrot with the addition of the difference in the y position
	temp = cameraXRot + (float) diffy;
	if(temp >= -80 && temp <= 90){
		cameraXRot = temp;
	}
	//set the xrot to yrot with the addition of the difference in the x position

    cameraYRot += (float) diffx;
	if ((x >= (WIN_DIM/2) + 5 || x <= (WIN_DIM/2) - 5 || 
		y >= (WIN_DIM/2) + 5 || y <= (WIN_DIM/2) - 5)){
		lastX = WIN_DIM/2;
		lastY = WIN_DIM/2;
		glutWarpPointer(lastX, lastY);
	}
}

void activeMouseFunction(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		shootPBall(0);
		balls[0].exists = 1;
	}
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		shootPBall(1);
		balls[1].exists = 1;
	}
}

void shootPBall(int which){
	float shotPos[3], shotInc[3];
	float xRotRad, yRotRad;
	yRotRad = (cameraYRot / 180 * M_PI);
	xRotRad = (cameraXRot / 180 * M_PI);
	shotInc[0] = (float)(sin(yRotRad));
	shotInc[1] = - (float)(sin(xRotRad));
	shotInc[2] = - (float)(cos(yRotRad));

	shotPos[0] = cameraPos[0] + shotInc[0];
	shotPos[1] = cameraPos[1] + shotInc[1];
	shotPos[2] = cameraPos[2] + shotInc[2];
	
	balls[which].pos[0] = shotPos[0];
	balls[which].pos[1] = shotPos[1];
	balls[which].pos[2] = shotPos[2];
	balls[which].velocity[0] = 
		BULLET_SPEED * (-1*(shotPos[0] - (shotPos[0] + shotInc[0])));
	balls[which].velocity[1] = 
		BULLET_SPEED * (-1*(shotPos[1] - (shotPos[1] + shotInc[1])));
	balls[which].velocity[2] = 
		BULLET_SPEED * (-1*(shotPos[2] - (shotPos[2] + shotInc[2])));
}

void initWalls(){
	walls[0].xMin = -5;
	walls[0].xMax = 5;
	walls[0].yMin = 0;
	walls[0].yMax = 10;
	walls[0].zMin = -1;
	walls[0].zMax = 1;
}

void buildWall(float xMin,float xMax,float yMin,float yMax,float zMin,float zMax){
	glBegin(GL_POLYGON);
		glVertex3f(xMin,yMax,zMin);
		glVertex3f(xMin,yMax,zMax);
		glVertex3f(xMax,yMax,zMax);
		glVertex3f(xMax,yMax,zMin);
		glVertex3f(xMax,yMin,zMin);
		glVertex3f(xMax,yMin,zMax);
		glVertex3f(xMin,yMin,zMax);
		glVertex3f(xMin,yMin,zMin);
	glEnd();
}
