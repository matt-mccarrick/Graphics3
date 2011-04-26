#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#define WIN_DIM 800
#define GRAVITY -0.0981
#define BULLET_SPEED 3.0
#define MAX_PORTALS 2
#define WALL_BALL_BORDER 3.0
#define WALL_PLAYER_BORDER 1.0
#define NUM_WALLS 35
#define PORTAL_SIZE 5.0
#define dimwid 216
#define dimhgt 286

enum WallType { Wall, Panel, Gray};

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
void setupWall(int, float,float,float,float,float,float,WallType);
void checkShotCollision(void);
bool wallCollision(int, float);
void portalCollision(void);
void readTextures();

struct box{
	float xMin,xMax, yMin, yMax, zMin, zMax;
	WallType type;
};

struct ball{
	GLfloat pos[3], velocity[3], color[3];
	bool isPortal;
	int exists;
};

typedef box wall;
typedef ball pBall;

float cameraPos[3];
float cameraXRot, cameraYRot;
float velocity;
int keyMap[256];
wall walls[NUM_WALLS];
int lastX, lastY;
bool wallColl, inPortal[MAX_PORTALS], canJump;

GLubyte walltexture[dimwid][dimhgt][3];
GLubyte paneltexture[71][141][3];
GLubyte blueball[216][286][3];
GLubyte orangeball[216][286][3];

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

	cameraPos[0] = 5.0f;
	cameraPos[1] = 0.0f;
	cameraPos[2] = 40.0f;
	cameraXRot = 0;
	cameraYRot = 0;
	lastX = WIN_DIM / 2;
	lastY = WIN_DIM / 2;
	velocity = 0;

	balls[0].exists = 0;
	balls[1].exists = 0;
	balls[0].isPortal = false;
	balls[1].isPortal = false;
	canJump = true;
	wallColl = false;
	inPortal[0] = false;
	inPortal[1] = false;

	
	balls[0].color[0] = 0.0;
	balls[0].color[1] = 0.0;
	balls[0].color[2] = 1.0;
	balls[1].color[0] = 1.0;
	balls[1].color[1] = 0.5;
	balls[1].color[2] = 0.0;
	
	readTextures();

	// specify combination of texture with surface color
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	// specify texture parameters - wrapping/filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
	checkShotCollision();
	//wallCollision();
	portalCollision();

	updatePosition();
	moveCamera();

	movePBalls();
	displayPBalls();
	
	//Display the walls
	displayWalls();
	
	glutSwapBuffers();

}

void movePBalls(){
	int i;
	for (i = 0; i < MAX_PORTALS; i++){
		if(balls[i].exists && !balls[i].isPortal){
			balls[i].pos[0] += balls[i].velocity[0];
			balls[i].pos[1] += balls[i].velocity[1];
			balls[i].pos[2] += balls[i].velocity[2];
		}
	}
}

void moveCamera(){
	glRotatef(cameraXRot, 1.0, 0.0, 0.0);
	glRotatef(cameraYRot, 0.0, 1.0, 0.0);
	/*
	if(cameraPos[1] < 0){
		cameraPos[1] = 0;
		canJump = true;	
	}else{
		canJump = false;
	}*/
	glTranslatef(-cameraPos[0],-cameraPos[1],-cameraPos[2]);
	
}

void displayPBalls(){
	int i;
	for(i = 0; i < MAX_PORTALS; i++){
		if(balls[i].exists){
			glPushMatrix();

				glColor3f(balls[i].color[0], balls[i].color[1], balls[i].color[2]);
				glTranslatef(balls[i].pos[0], balls[i].pos[1], balls[i].pos[2]);
				if(balls[i].isPortal)
					glScalef(PORTAL_SIZE, PORTAL_SIZE, PORTAL_SIZE);
				
				glEnable(GL_TEXTURE_2D);
					
				if(i % 2 == 0)
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 216, 286, 0, GL_RGB, GL_UNSIGNED_BYTE, blueball);
				else
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 216,286, 0, GL_RGB, GL_UNSIGNED_BYTE, orangeball);

				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				
				glutSolidSphere(1.0, 20.0, 20.0);
				
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_TEXTURE_GEN_S); 
				glDisable(GL_TEXTURE_GEN_T);
			glPopMatrix();
		}
	}
	
}

void displayWalls(){
	int i;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dimwid,dimhgt, 0, GL_RGB, GL_UNSIGNED_BYTE, walltexture);
	for (i = 0; i < NUM_WALLS; i++){
		if(walls[i].type == Wall){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dimwid,dimhgt, 0, GL_RGB, GL_UNSIGNED_BYTE, walltexture);
			glEnable(GL_TEXTURE_2D);
		}
		else if(walls[i].type == Panel){
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 71,141, 0, GL_RGB, GL_UNSIGNED_BYTE, paneltexture);
			glEnable(GL_TEXTURE_2D);
		}
		else if(walls[i].type == Gray)
			glColor3f(0.89,0.89,0.89);
		else
			glColor3f(1.0,1.0,0.0);
		buildWall(walls[i].xMin,walls[i].xMax,walls[i].yMin,walls[i].yMax,
			walls[i].zMin, walls[i].zMax);
		if(walls[i].type == Wall || walls[i].type == Panel)
			glDisable(GL_TEXTURE_2D);
	}
}

void checkShotCollision(){
	int i;
	int k;
	for(i = 0; i < MAX_PORTALS; i ++){
		for(k = 0; k < NUM_WALLS; k++){
			if(!balls[i].isPortal && balls[i].exists){
				bool xOk = balls[i].pos[0] > walls[k].xMin - WALL_BALL_BORDER
					&& balls[i].pos[0] < walls[k].xMax + WALL_BALL_BORDER;
				bool yOk = balls[i].pos[1] > walls[k].yMin - WALL_BALL_BORDER
					&& balls[i].pos[1] < walls[k].yMax + WALL_BALL_BORDER;
				bool zOk = balls[i].pos[2] > walls[k].zMin - WALL_BALL_BORDER
					&& balls[i].pos[2] < walls[k].zMax + WALL_BALL_BORDER;

				if(xOk && yOk && zOk){
					balls[i].isPortal = true;
				}
			}		
		}
	}	
}

void wallCollision(){
	int k;
	for(k = 0; k < NUM_WALLS; k++){
		bool xOk = cameraPos[0] > walls[k].xMin - WALL_BALL_BORDER
			&& cameraPos[0] < walls[k].xMax + WALL_BALL_BORDER;
		bool yOk = cameraPos[1] > walls[k].yMin - WALL_BALL_BORDER
			&& cameraPos[1] < walls[k].yMax + WALL_BALL_BORDER;
		bool zOk = cameraPos[2] > walls[k].zMin - WALL_BALL_BORDER
			&& cameraPos[2] < walls[k].zMax + WALL_BALL_BORDER;

		if(xOk && yOk && zOk){
			//printf("%g, %g, %g\n", 
			//			cameraPos[0], cameraPos[1], cameraPos[2]);
			wallColl = true;
		}	
	}
}

bool wallCollision(int which, float increment){
	int k;
	for(k = 0; k < NUM_WALLS; k++){
		switch(which){
			case 0:
				if(cameraPos[0] + increment > walls[k].xMin - WALL_PLAYER_BORDER && 
					cameraPos[0] + increment < walls[k].xMax + WALL_PLAYER_BORDER && 
					cameraPos[1] > walls[k].yMin - WALL_PLAYER_BORDER && 
					cameraPos[1] < walls[k].yMax + WALL_PLAYER_BORDER && 
					cameraPos[2] > walls[k].zMin - WALL_PLAYER_BORDER && 
					cameraPos[2] < walls[k].zMax + WALL_PLAYER_BORDER){
					wallColl = true;
					return true;
				}
				break;
			case 1:
				if(cameraPos[0] > walls[k].xMin - WALL_PLAYER_BORDER && 
					cameraPos[0] < walls[k].xMax + WALL_PLAYER_BORDER && 
					cameraPos[1] + increment > walls[k].yMin - WALL_PLAYER_BORDER && 
					cameraPos[1] + increment < walls[k].yMax + WALL_PLAYER_BORDER && 
					cameraPos[2] > walls[k].zMin - WALL_PLAYER_BORDER && 
					cameraPos[2] < walls[k].zMax + WALL_PLAYER_BORDER){
					if(cameraPos[1] > walls[k].yMax){
						cameraPos[1] = walls[k].yMax + WALL_PLAYER_BORDER + .1;
						canJump = true;
					}
					return true;
				}else{
					//canJump = false;
				}
				break;
			case 2:
				if(cameraPos[0] > walls[k].xMin - WALL_PLAYER_BORDER && 
					cameraPos[0] < walls[k].xMax + WALL_PLAYER_BORDER && 
					cameraPos[1] > walls[k].yMin - WALL_PLAYER_BORDER && 
					cameraPos[1] < walls[k].yMax + WALL_PLAYER_BORDER && 
					cameraPos[2] + increment > walls[k].zMin - WALL_PLAYER_BORDER && 
					cameraPos[2] + increment < walls[k].zMax + WALL_PLAYER_BORDER){
					wallColl = true;
					return true;
				}
				break;

		}
	}
	wallColl = false;
	return false;
}

void portalCollision(){
	int i;
	for(i = 0; i < MAX_PORTALS; i++){
		if(balls[i].isPortal && balls[((i + 1) % 2)].isPortal){
			float distance = sqrt(pow(cameraPos[0] - balls[i].pos[0], 2) + 
				pow(cameraPos[1] - balls[i].pos[1], 2) + 
				pow(cameraPos[2] - balls[i].pos[2], 2));
			if(distance < PORTAL_SIZE){
				if(!inPortal[i]){
					cameraPos[0] = balls[((i + 1) % 2)].pos[0];
					cameraPos[1] = balls[((i + 1) % 2)].pos[1];
					cameraPos[2] = balls[((i + 1) % 2)].pos[2];
					inPortal[((i + 1) % 2)] = true;
					inPortal[i] = false;
				}


				break;
			}

		inPortal[i] = false;
		}
	}

}

void updatePosition(){
	if(!wallCollision(1,velocity * 2)){
		velocity += GRAVITY;
		cameraPos[1] += velocity;
		canJump = false;
	}else{
		velocity = 0;
		canJump = true;
	}
}

void reshape(int width, int height){
	glViewport(0,0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(60, (GLfloat)width/(GLfloat)height, 1.0f, 1250.0f);

	glMatrixMode(GL_MODELVIEW);
}

void keyCheck(){
	float yRotRad, xRotRad;

	if(keyMap['a'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		float xInc = -float(cos(yRotRad)) * 0.75;
		float zInc = -float(sin(yRotRad)) * 0.75;

		if(!wallCollision(0,xInc))
			cameraPos[0] += xInc;
		if(!wallCollision(2, zInc))
			cameraPos[2] += zInc;
	}
	if(keyMap['d'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		float xInc = float(cos(yRotRad)) * 0.75;
		float zInc = float(sin(yRotRad)) * 0.75;

		if(!wallCollision(0,xInc))
			cameraPos[0] += xInc;
		if(!wallCollision(2, zInc))
			cameraPos[2] += zInc;
	}
	if(keyMap['w'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		xRotRad = (cameraXRot / 180 * 3.141592654f);
		float xInc = float(sin(yRotRad)) * 0.75;
		float zInc = -float(cos(yRotRad)) * 0.75;

		if(!wallCollision(0,xInc))
			cameraPos[0] += xInc;
		if(!wallCollision(2,zInc))
			cameraPos[2] += zInc;
	}
	if(keyMap['s'] == 1){
		yRotRad = (cameraYRot / 180 * 3.141592654f);
		xRotRad = (cameraXRot / 180 * 3.141592654f); 
		float xInc = -float(sin(yRotRad)) * 0.75;
		float zInc = float(cos(yRotRad)) * 0.75;

		if(!wallCollision(0,xInc))
			cameraPos[0] += xInc;
		if(!wallCollision(2,zInc))
			cameraPos[2] += zInc;

	}
	if(keyMap['r'] == 1){
		cameraPos[1] += 1;
	}
	if(keyMap['f'] == 1){
		cameraPos[1] -= 1;
	}
	if(keyMap[' '] == 1){
		if(canJump){
			velocity = 3.0;
		}
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
    
    //if it hasn't moved too far, don't warp the pointer
	if ((x >= (WIN_DIM/2) + 1 || x <= (WIN_DIM/2) - 1 || 
		y >= (WIN_DIM/2) + 1 || y <= (WIN_DIM/2) - 1)){
		lastX = WIN_DIM/2;
		lastY = WIN_DIM/2;
		glutWarpPointer(lastX, lastY);
	}
}

void activeMouseFunction(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		shootPBall(0);
		balls[0].exists = 1;
		balls[0].isPortal = false;
	}
	if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
		shootPBall(1);
		balls[1].exists = 1;
		balls[1].isPortal = false;
	}
}

void shootPBall(int which){
	float shotPos[3], shotInc[3];
	float xRotRad, yRotRad;
	yRotRad = (cameraYRot / 180 * M_PI);
	xRotRad = (cameraXRot / 180 * M_PI);
	shotInc[0] = (float)(sin(yRotRad));
	shotInc[1] = - (float)(tan(xRotRad));
	shotInc[2] = - (float)(cos(yRotRad));

	shotPos[0] = cameraPos[0] + shotInc[0];
	shotPos[1] = cameraPos[1] + shotInc[1];
	shotPos[2] = cameraPos[2] + shotInc[2];
	
	balls[which].pos[0] = shotPos[0];
	balls[which].pos[1] = shotPos[1];
	balls[which].pos[2] = shotPos[2];

	balls[which].velocity[0] = 
		BULLET_SPEED * (shotInc[0]);
	balls[which].velocity[1] = 
		BULLET_SPEED *(shotInc[1]);
	balls[which].velocity[2] = 
		BULLET_SPEED * (shotInc[2]);
}

void initWalls(){

	int i;
	
	int width=dimwid, height=dimhgt, depth=4;	
	int xposinit=-50, yposinit=-10, zposinit=-500;
	int xpos=-50, ypos=-10, zpos=-500;
	
	// Four main walls
	for(i = 0; i < 4; i++){
		setupWall(i, xpos + (i*width), xpos + ((i + 1) * width), ypos, ypos + height, zpos, zpos + depth, Wall);		
	}	
	zpos += (4 * width);
	for(i = 0; i < 4; i++){
		setupWall(i + 4, xpos + (i*width), xpos + ((i + 1) * width), ypos, ypos + height, zpos, zpos + depth, Wall);		
	}
	zpos = zposinit;
	for(i = 0 ; i < 4; i++){
		setupWall(i + 8, xpos, xpos + depth, ypos, ypos + height, zpos + (i*width), zpos + ((i + 1) * width), Wall);	
	}
	xpos += (4 * width);
	for(i = 0 ; i < 4; i++){
		setupWall(i + 12, xpos, xpos + depth, ypos, ypos + height, zpos + (i*width), zpos + ((i + 1) * width), Wall);	
	}	
	xpos = xposinit;
	
	// Floor and Ceiling
	setupWall(16, xpos, xpos + (4*width), -12, -4, zpos, zpos + (4*width), Wall);
	setupWall(17, xpos, xpos + (4*width), height -12, height, zpos, zpos + (4*width), Wall);
	
	// Pillar
	setupWall(18, xpos + width, xpos + width + 50, ypos, ypos + height, zpos + width, zpos + width + 50, Wall);
	
	// Wall within room
	setupWall(19, xpos + width, xpos + width + 4, ypos, ypos + height, zpos + width*2, zpos + width*3, Wall);
	
	// Wall stair-like platform
	for(i = 0; i < 8; i++){
		setupWall(i + 20, xpos + (i*width/2), xpos + width/2 + (i*width/2), ypos + 20 + (i * 20), ypos + 28 + (i * 20), zpos, zpos + 100, Wall);
	}
	
	// Back corner structure
	xpos += (3*width);
	zpos += (3*width);
	setupWall(28, xpos + 8, xpos + 200, ypos, ypos + height/2, zpos, zpos + 8, Wall);
	setupWall(29, xpos, xpos + 8, ypos, ypos + height/2, zpos, zpos + width/2, Wall); 
	setupWall(30, xpos + 8, xpos + 200, ypos + height/2 -13, ypos + height/2 - 5, zpos + 8, zpos + width/2, Wall); 
	setupWall(31, xpos - (2*width), xpos, ypos + height/2 - 50, ypos + height/2, zpos, zpos + 50, Wall);	
	xpos = xposinit;
	zpos = zposinit;
}

void setupWall(int which, float xMin,float xMax,float yMin,float yMax,float zMin,float zMax, WallType type){
	walls[which].xMin = xMin;
	walls[which].xMax = xMax;
	walls[which].yMin = yMin;
	walls[which].yMax = yMax;
	walls[which].zMin = zMin;
	walls[which].zMax = zMax;
	walls[which].type = type;
}

void buildWall(float xMin,float xMax,float yMin,float yMax,float zMin,float zMax){
	
	glBegin(GL_QUADS);
	
		//face 1 top, floor	
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMin,yMax,zMin);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMin,yMax,zMax);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMax,yMax,zMax);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMax,yMax,zMin);		
		
		//face 2, back
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMin,yMax,zMin);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMax,yMax,zMin);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMax,yMin,zMin);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMin,yMin,zMin);
		
		//face 3, right		
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMin,yMax,zMin);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMin,yMax,zMax);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMin,yMin,zMax);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMin,yMin,zMin);		

		//face 4, front	
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMin,yMax,zMax);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMax,yMax,zMax);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMax,yMin,zMax);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMin,yMin,zMax);
		
		//face 5, left			
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMax,yMax,zMax);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMax,yMax,zMin);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMax,yMin,zMin);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMax,yMin,zMax);		
		
		//face 6, bottom, floor
		glTexCoord2f(1.0, 0.0);
		glVertex3f(xMax,yMin,zMin);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(xMax,yMin,zMax);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(xMin,yMin,zMax);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(xMin,yMin,zMin);
		
	glEnd();
}

// digitally scans-in dim x dim RGB image data
void readTextures()
{
	int i, j;
	unsigned char data[3];
	FILE *fp_dat;
	char f1[256] = "wall.dat";
	char f2[256] = "blue.dat";
	char f3[256] = "orange.dat";
	char f4[256] = "panel.dat";

	if ((fp_dat = fopen (f1, "rb")) == NULL) {
		printf ("file not found\n");
		abort();
	}

	for (i=0; i< dimwid; i++) {
		for (j=0; j<dimhgt; j++) {
			fread (data, sizeof(unsigned char), 3, fp_dat);
			walltexture[i][j][0] = (GLubyte) data[0];
			walltexture[i][j][1] = (GLubyte) data[1];
			walltexture[i][j][2] = (GLubyte) data[2];
		}
	}
	
	if ((fp_dat = fopen (f2, "rb")) == NULL) {
		printf ("file not found\n");
		abort();
	}
	
	for (i=0; i< 216; i++) {
		for (j=0; j<286; j++) {
			fread (data, sizeof(unsigned char), 3, fp_dat);
			blueball[i][j][0] = (GLubyte) data[0];
			blueball[i][j][1] = (GLubyte) data[1];
			blueball[i][j][2] = (GLubyte) data[2];
		}
	}
	
	if ((fp_dat = fopen (f3, "rb")) == NULL) {
		printf ("file not found\n");
		abort();
	}
	
	for (i=0; i< 216; i++) {
		for (j=0; j<286; j++) {
			fread (data, sizeof(unsigned char), 3, fp_dat);
			orangeball[i][j][0] = (GLubyte) data[0];
			orangeball[i][j][1] = (GLubyte) data[1];
			orangeball[i][j][2] = (GLubyte) data[2];
		}
	}
	
	if ((fp_dat = fopen (f4, "rb")) == NULL) {
		printf ("file not found\n");
		abort();
	}
	
	for (i=0; i< 71; i++) {
		for (j=0; j<141; j++) {
			fread (data, sizeof(unsigned char), 3, fp_dat);
			paneltexture[i][j][0] = (GLubyte) data[0];
			paneltexture[i][j][1] = (GLubyte) data[1];
			paneltexture[i][j][2] = (GLubyte) data[2];
		}
	}
}
