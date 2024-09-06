#include "TextureBuilder.h"
#include "Model_3DS.h"
#include <vector>
#include <cstdlib>
#include < string >
#include <iostream>
#include "GLTexture.h"
#include <glut.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <playsoundapi.h>
#pragma comment(lib,"winmm.lib")

#define BACKGROUND_MUSIC_PATH "sfx/game_music.wav"
#define DEATH_PATH "sfx/roblox-death-sound-effect.wav"
#define JET_PATH "sfx/airplane-95615.wav"
#define CHEER_PATH "sfx/crowd-cheer-ii-6263.wav"
#define LEVEL_2_PATH "sfx/hungarian_dance_slowed-17216.wav"
GLfloat rotationAngle1 = 0.0f;

#define DEG2RAD(a) (a * 0.0174532925)


int windowWidth = 1280;
int windowHeight = 720;

int powerUpTimer = 0;
int gameTimer = 120 + 1;
int score = 0;
const int mazeSize = 15;
const float wallSize = 25;
bool lose = false;
bool shield = false;
bool speed = false;
float moveSpeed = 3;
bool isFullScreen = false;
float newPlayerPosX = 0.0;
float newPlayerPosZ = 0.0;
float playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
float playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
float playerRotation = 0;
float cameraRotation = 0;
bool firstPerson = false;
float cameraDistance = 13;  
float cameraHeight = 0;    
float playerHeight = 17;
int mousePosX = windowWidth / 2;
int mousePosY = windowHeight / 2;
bool level1 = true;
bool level2 = true;

Model_3DS model_player;
Model_3DS model_water;
Model_3DS model_trap;
Model_3DS model_spider;
Model_3DS model_speed;
Model_3DS model_food;
Model_3DS model_shield;

struct Collectible {
	float x;
	float y;
	float z;
	bool c;
};

struct Collectible2 {
	float x;
	float y;
	float z;
	bool c;
};
struct Obstacle {
	float x;
	float y;
	float z;
};
struct Powerup {
	float x;
	float y;
	float z;
	bool c;
};
struct Boundary {
	float x;
	float y;
	float z;
};
std::vector<Collectible> Collectibles;
std::vector<Collectible2> Collectibles2;
std::vector<Obstacle> Obstacles;
std::vector<Powerup> Powerups;
std::vector<Boundary> Boundaries;


GLTexture tex_ground;
GLTexture tex_concrete;
GLTexture tex_wall;
GLTexture tex_portal;
GLTexture tex_exit;

void drawText() {
	glColor3f(0.0f, 0.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 640.0, 0.0, 480.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2i(500, 450);
	std::string timerText = "Score: "+std::to_string(score) +"  " + "\n\n\nTime: " + std::to_string(gameTimer);
	for (char c : timerText) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawLoseText() {
	glColor3f(0.0f, 0.0f, 0.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, 640.0, 0.0, 480.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glRasterPos2i(300, 300);
	std::string timerText = "GAME OVER";
	for (char c : timerText) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


void RenderGround()
{
	glDisable(GL_LIGHTING);	

	glColor3f(0.6, 0.6, 0.6);	

	glEnable(GL_TEXTURE_2D);	

	if (level1) {
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	

	}
	else if (level2){
	glBindTexture(GL_TEXTURE_2D, tex_concrete.texture[0]);	

	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);    
	glTexCoord2f(0, 0);    
	glVertex3f(-400, 0, -400);
	glTexCoord2f(5, 0);
	glVertex3f(400, 0, -400);
	glTexCoord2f(5, 5);
	glVertex3f(400, 0, 400);
	glTexCoord2f(0, 5);
	glVertex3f(-400, 0, 400);
	glEnd();
	glPopMatrix();


	glEnable(GL_LIGHTING);	

	glColor3f(1, 1, 1);	
}


bool checkObstacleCollision(float playerX, float playerZ) {
	for (const auto& obstacle : Obstacles) {
		float obstacleX = obstacle.x;
		float obstacleZ = obstacle.z;

		if (playerX > obstacleX - 5 &&
			playerX < obstacleX + 5 &&
			playerZ > obstacleZ - 5 &&
			playerZ < obstacleZ + 5) {
			return true;
		}
	}
	return false;
}

bool checkGoalCollision(float playerX, float playerZ) {
	float playerRadius = 3.0;
	float wallX = 0 * wallSize - mazeSize / 2.0;
	float wallZ = 6 * wallSize - mazeSize / 2.0;


	float minPlayerX = playerPosX + 20 - playerRadius;
	float maxPlayerX = playerPosX + 20 + playerRadius;
	float minPlayerZ = playerPosZ + 20 - playerRadius;
	float maxPlayerZ = playerPosZ + 20 + playerRadius;

	if ((maxPlayerX > (wallX - wallSize / 2.0)) && ((minPlayerX < wallX + wallSize / 2.0)) &&
		(maxPlayerZ > (wallZ - wallSize / 2.0)) && ((minPlayerZ < wallZ + wallSize / 2.0))) {
		return true;
	}
	return false;
}

bool checkCollectibleCollision(float playerX, float playerZ) {
	for ( auto& collectible : Collectibles) {
		if (!collectible.c) {
			float collectibleX = collectible.x;
			float collectibleZ = collectible.z;

			if (playerX > collectibleX - 5 &&
				playerX < collectibleX + 5 &&
				playerZ > collectibleZ - 5 &&
				playerZ < collectibleZ + 5) {
				collectible.c = true; 
				return true;
			}
		}
	}

	/*for (auto& collectible : Collectibles2) {
		if (!collectible.c) {
			float collectibleX = collectible.x;
			float collectibleZ = collectible.z;

			if (playerX > collectibleX - 5 &&
				playerX < collectibleX + 5 &&
				playerZ > collectibleZ - 5 &&
				playerZ < collectibleZ + 5) {
				// Handle collectible collision logic (e.g., increase score)
				collectible.c = true; // Mark the collectible as collected
				return true;
			}
		}
	}*/
	return false;
}

bool checkPowerupCollision(float playerX, float playerZ) {
	for ( auto& powerup : Powerups) {
		if (!powerup.c) {
			float powerupX = powerup.x;
			float powerupZ = powerup.z;
			if (level1) {
				if (playerX > powerupX - 5 &&
					playerX < powerupX + 5 &&
					playerZ > powerupZ - 5 &&
					playerZ < powerupZ + 5) {
					powerup.c = true; 
					return true;
				}
			}
			else if (level2) {
				if (playerX > powerupX - 25 &&
					playerX < powerupX + 25 &&
					playerZ > powerupZ - 25 &&
					playerZ < powerupZ + 25) {
					
					powerup.c = true; 
					return true;
				}
			}
			
		}
	}
	return false;
}

void drawCube(float x, float y, float z, float size, bool maze) {
	glPushMatrix();
	glTranslatef(x, y, z);

	glEnable(GL_TEXTURE_2D);

	if (maze) {
		if (level1) {
			glBindTexture(GL_TEXTURE_2D, tex_wall.texture[0]);
		}
		else if (level2) {
			glBindTexture(GL_TEXTURE_2D, tex_concrete.texture[0]);
		}
	}
	else {
		if (level1) {
			glBindTexture(GL_TEXTURE_2D, tex_portal.texture[0]);
		}
		else if (level2) {
			glBindTexture(GL_TEXTURE_2D, tex_exit.texture[0]);
		}
	}

	glBegin(GL_QUADS);
	// Front face
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glTexCoord2f(1, 0);
	glVertex3f(size / 2, -size / 2, size / 2);
	glTexCoord2f(1, 1);
	glVertex3f(size / 2, size / 2, size / 2);
	glTexCoord2f(0, 1);
	glVertex3f(-size / 2, size / 2, size / 2);

	// Back face
	glNormal3f(0, 0, -1);
	glTexCoord2f(1, 0);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glTexCoord2f(0, 1);
	glVertex3f(size / 2, size / 2, -size / 2);
	glTexCoord2f(0, 0);
	glVertex3f(size / 2, -size / 2, -size / 2);

	// Left face
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glTexCoord2f(0, 0);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glTexCoord2f(1, 0);
	glVertex3f(-size / 2, -size / 2, size / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-size / 2, size / 2, size / 2);

	// Right face
	glNormal3f(1, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(size / 2, size / 2, size / 2);
	glTexCoord2f(0, 1);
	glVertex3f(size / 2, -size / 2, size / 2);
	glTexCoord2f(0, 0);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glTexCoord2f(1, 0);
	glVertex3f(size / 2, size / 2, -size / 2);

	// Top face
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-size / 2, size / 2, -size / 2);
	glTexCoord2f(1, 1);
	glVertex3f(size / 2, size / 2, -size / 2);
	glTexCoord2f(1, 0);
	glVertex3f(size / 2, size / 2, size / 2);
	glTexCoord2f(0, 0);
	glVertex3f(-size / 2, size / 2, size / 2);

	// Bottom face
	glNormal3f(0, -1, 0);
	glTexCoord2f(1, 0);
	glVertex3f(-size / 2, -size / 2, -size / 2);
	glTexCoord2f(0, 0);
	glVertex3f(size / 2, -size / 2, -size / 2);
	glTexCoord2f(0, 1);
	glVertex3f(size / 2, -size / 2, size / 2);
	glTexCoord2f(1, 1);
	glVertex3f(-size / 2, -size / 2, size / 2);

	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}

void drawMaze() {
	int maze[mazeSize][mazeSize] = {
			{1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1},
			{1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
			{1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1},
			{1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1},
			{1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1},
			{1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};
	for (int i = 0; i < mazeSize; ++i) {
		for (int j = 0; j < mazeSize; ++j) {
			if (maze[i][j] == 1) {
				float x = i * wallSize - mazeSize / 2.0;
				float y = j * wallSize - mazeSize / 2.0;
				drawCube(x, 0, y, wallSize, true);
				Boundary b;
				b.x = x;
				b.y = 0;
				b.z = y;
				Boundaries.push_back(b);
			}
		}
	}


	/*std::cout << "PlayerX: " << playerX << std::endl;
	std::cout << "PlayerY: " << playerY << std::endl;
	std::cout << "PlayerZ: " << playerZ << std::endl;*/
	//drawCube(playerX, playerY, playerZ, 15);
}






GLuint tex;
GLuint tex1;
char title[] = "Maze Runner 2099";

GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)windowWidth / (GLdouble)windowHeight;
GLdouble zNear = 0.1;
GLdouble zFar = 1000;



class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

int cameraZoom = 0;

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 0.0f, float eyeY = 4.0f, float eyeZ = 0.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 0.0f, float upZ = -1.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void rotateY2(float a) {
		if (a == 0) return;
		if (firstPerson) {
			Vector3f player = Vector3f(playerPosX, playerHeight, playerPosZ + cameraDistance);
			Vector3f view = (center - eye).unit();
			Vector3f right = up.cross(view).unit();
			view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
			right = view.cross(up);
			center = eye + view;
		}
		else {
			Vector3f player = Vector3f(playerPosX, 20 + 5, playerPosZ);
			Vector3f view = (eye - center).unit();
			Vector3f right = up.cross(view).unit();
			view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
			right = view.cross(up);
			eye = center + view*cameraDistance;
		}
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

	void moveCamera(float eyeX, float eyeY, float eyeZ) {
		eye = Vector3f(eye.x + eyeX, eye.y + eyeY, eye.z + eyeZ);
	}

	void moveCamera(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ) {
		eye = Vector3f(eye.x + eyeX, eye.y + eyeY, eye.z + eyeZ);
		center = Vector3f(center.x + centerX, center.y + centerY, center.z + centerZ);
	}
};


Camera camera = Camera(
	playerPosX, playerHeight + cameraHeight, playerPosZ - cameraDistance,
	playerPosX, playerHeight, playerPosZ,
	0.0, 1.0, 0.0
);


//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	//gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************//
	camera.look();

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);



	glEnable(GL_NORMALIZE);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspectRatio, zNear, zFar);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

void drawLevel1() {

	glPushMatrix();
	glTranslatef(-20, 15, -20);
	drawCube((0 * wallSize - mazeSize / 2.0), 0, (6 * wallSize - mazeSize / 2.0), wallSize, false);
	glPopMatrix();

	/*for (auto it = Collectibles2.begin(); it != Collectibles2.end(); ++it) {
		Collectible2& c = *it;
		if (!c.c) {
			glPushMatrix();
			glTranslatef(c.x, c.y, c.z);
			model_food.Draw();
			glPopMatrix();
		}
	}*/


	for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
		Collectible& c = *it;
		if (!c.c) {
			glColor3f(0, 0.2, 0.8);
			glPushMatrix();
			glTranslatef(c.x, c.y+5, c.z);
			glScalef(3, 3, 3);
			model_water.Draw();
			glPopMatrix();
		}
	}

	for (auto it = Obstacles.begin(); it != Obstacles.end(); ++it) {
		Obstacle& c = *it;
		glColor3f(0.7,0, 0);
			glPushMatrix();
			glTranslatef(c.x, c.y, c.z);
			glScalef(0.2, 0.2, 0.2);
			model_trap.Draw();
			glPopMatrix();
		
	}

	for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
		Powerup& c = *it;
		if (!c.c) {
			glColor3f(0.3, 0.3, 0.3);
			glPushMatrix();
			glTranslatef(c.x, c.y, c.z);
			glScalef(5, 5, 5);
			model_speed.Draw();
			glPopMatrix();
		}
	}
	glColor3f(1, 1, 1);


}

void drawLevel2() {
	glPushMatrix();
	glTranslatef(-20, 15, -20);
	drawCube((0 * wallSize - mazeSize / 2.0), 0, (6 * wallSize - mazeSize / 2.0), wallSize, false);
	glPopMatrix();

	for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
		Collectible& c = *it;
		if (!c.c) {
			glPushMatrix();
			glTranslatef(c.x, c.y, c.z);
			model_food.Draw();
			glPopMatrix();
		}
	}

	for (auto it = Obstacles.begin(); it != Obstacles.end(); ++it) {
		Obstacle& c = *it;
		glColor3f(0, 0, 0);

			glPushMatrix();
			glTranslatef(c.x, c.y, c.z);
			model_spider.Draw();
			glPopMatrix();
		
	}

	for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
		Powerup& c = *it;
		if (!c.c) {
			glColor3f(1, 1, 1);

			glPushMatrix();
			glTranslatef(c.x + 25, c.y + 3, c.z - 42.5);
			glRotatef(90, 0, 1, 0);
			glScalef(5, 5, 5);
			model_shield.Draw();
			glPopMatrix();
		}
	}
	glColor3f(1, 1, 1);

}

static bool checkWallCollision() {
	float playerRadius = 3.0;
	int maze[mazeSize][mazeSize] = {
			{1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
			{1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1},
			{1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1},
			{1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1},
			{1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1},
			{1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1},
			{1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1},
			{1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1},
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};
	for (int i = 0; i < mazeSize; ++i) {
		for (int j = 0; j < mazeSize; ++j) {
			if (maze[i][j] == 1) {
				float wallX = i * wallSize - mazeSize / 2.0;
				float wallZ = j * wallSize - mazeSize / 2.0;


				// Check collision with walls
				float minPlayerX = playerPosX + 20 - playerRadius;
				float maxPlayerX = playerPosX + 20 + playerRadius;
				float minPlayerZ = playerPosZ + 20 - playerRadius;
				float maxPlayerZ = playerPosZ + 20 + playerRadius;

				if ((maxPlayerX > (wallX - wallSize / 2.0)) && ((minPlayerX < wallX + wallSize / 2.0)) &&
					(maxPlayerZ > (wallZ - wallSize / 2.0)) && ((minPlayerZ < wallZ + wallSize / 2.0))) {
					return true;
				}
			}
		}
	}
	return false;
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{	
	setupCamera();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	double x = 0.7;
	double y = 0.7;
	double z = 0.7;
	if (level2) {
		x = 0.2;
		y = 0.2;
		z = 0.4;
	}

	GLfloat lightIntensity[] = { x, y, z, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	if (!lose) {
		drawText();
	}
	
	RenderGround();
	glPushMatrix();
	glTranslatef(-20, 15, -20);
	drawMaze();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(playerPosX, 0, playerPosZ);
	glRotatef(playerRotation, 0, 1, 0);
	if (!firstPerson) {
		glScalef(0.2, 0.2, 0.2);
		model_player.Draw();
	}
	glPopMatrix();



	if (level1) {
		drawLevel1();
	}
	else if (level2) {
		drawLevel2();
	}
	else {
		drawLoseText();
	}
	//glPushMatrix();
	//glRotatef(rotationAngle1, 0.0f, 1.0f, 0.0f);
	GLfloat lightIntensity1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat lightPosition1[] = {0*wallSize -mazeSize/2.0, 10.0f,6 * wallSize - mazeSize/2.0, 0.0f  + rotationAngle1 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity1);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity1);
	//glPopMatrix();
	
	//sky box
	glPushMatrix();
		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		if (level1) {
			glBindTexture(GL_TEXTURE_2D, tex);

		}
		else if (level2){
			glBindTexture(GL_TEXTURE_2D, tex1);
		}
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 500, 500, 500);
		gluDeleteQuadric(qobj);
	glPopMatrix();



	glutSwapBuffers();
	glFlush();
}

void update(int value) {
	// Update the rotation angle for the light intensity
	rotationAngle1 += 1.0f;

	if (rotationAngle1 > 360.0f) {
		rotationAngle1 -= 360.0f;
	}

	// Request the next update
	glutTimerFunc(16, update, 0);

	// Trigger a redraw
	glutPostRedisplay();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void Special(int key, int x, int y) {
	float rotateSpeed = 10.0;
	float playerPosZ_new;
	float playerPosX_new;
	switch (key) {
	case GLUT_KEY_UP:
		playerRotation = cameraRotation;
		playerPosX += moveSpeed * sin(DEG2RAD(playerRotation));
		playerPosZ += moveSpeed * cos(DEG2RAD(playerRotation));
		if (checkWallCollision()) {
			playerPosX -= moveSpeed * sin(DEG2RAD(playerRotation));
			playerPosZ -= moveSpeed * cos(DEG2RAD(playerRotation));
		}
		else
			camera.moveCamera(moveSpeed * sin(DEG2RAD(playerRotation)), 0, moveSpeed * cos(DEG2RAD(playerRotation)), moveSpeed * sin(DEG2RAD(playerRotation)), 0, moveSpeed * cos(DEG2RAD(playerRotation)));
		if (checkGoalCollision(playerPosX, playerPosZ)) {
			if (level1) {
				PlaySound(CHEER_PATH, NULL, SND_FILENAME | SND_ASYNC);
				level1 = false;
				level2 = true;
				playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
				playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
				glPushMatrix();
				glTranslatef(playerPosX, 0, playerPosZ);
				glRotatef(playerRotation, 0, 1, 0);
				if (!firstPerson) {
					glScalef(0.2, 0.2, 0.2);
					model_player.Draw();
				}
				glPopMatrix();
				gameTimer = 120 + 1;
				for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
					Collectible& c = *it;
					c.c = false;
				}

				for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
					Powerup& c = *it;
					c.c = false;
				}
				camera.look();
				PlaySound(LEVEL_2_PATH, NULL, SND_FILENAME | SND_ASYNC);
			}
			else if (level2) {
				playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
				playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
				glPushMatrix();
				glTranslatef(playerPosX, 0, playerPosZ);
				glRotatef(playerRotation, 0, 1, 0);
				if (!firstPerson) {
					glScalef(0.2, 0.2, 0.2);
					model_player.Draw();
				}
				glPopMatrix();
				camera.look();
			}
		}
		if (checkObstacleCollision(playerPosX, playerPosZ)) {

			if ((level2 && !shield) || level1) {
				PlaySound(DEATH_PATH, NULL, SND_FILENAME | SND_ASYNC);
				lose = true;
				level1 = false;
				level2 = false;
			}
		}
		else if (checkCollectibleCollision(playerPosX, playerPosZ)) {
			score += 10;
		}
		else if (checkPowerupCollision(playerPosX, playerPosZ)) {
			if (level1) {
				PlaySound(JET_PATH, NULL, SND_FILENAME | SND_ASYNC);
				speed = true;
			}
			else if (level2) {
				shield = true;
			}
			powerUpTimer += 20;
		}
		break;
	case GLUT_KEY_DOWN:
		playerRotation = cameraRotation;
		playerPosX += -moveSpeed * sin(DEG2RAD(playerRotation));
		playerPosZ += -moveSpeed * cos(DEG2RAD(playerRotation));
		if (checkWallCollision()) {
			playerPosX -= -moveSpeed * sin(DEG2RAD(playerRotation));
			playerPosZ -= -moveSpeed * cos(DEG2RAD(playerRotation));
		}
		else
			camera.moveCamera(-moveSpeed * sin(DEG2RAD(playerRotation)), 0, -moveSpeed * cos(DEG2RAD(playerRotation)), -moveSpeed * sin(DEG2RAD(playerRotation)), 0, -moveSpeed * cos(DEG2RAD(playerRotation)));
		playerRotation += 180;
		if (checkGoalCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(CHEER_PATH, NULL, SND_FILENAME | SND_ASYNC);
					level1 = false;
					level2 = true;
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					gameTimer = 120 + 1;
					for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
						Collectible& c = *it;
						c.c = false;
					}

					for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
						Powerup& c = *it;
						c.c = false;
					}
					camera.look();
					PlaySound(LEVEL_2_PATH, NULL, SND_FILENAME | SND_ASYNC);

				}
				else if (level2) {
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					camera.look();
				}
			}
			if (checkObstacleCollision(playerPosX, playerPosZ)) {
				if ((level2 && !shield) || level1) {
					PlaySound(DEATH_PATH, NULL, SND_FILENAME | SND_ASYNC);
					lose = true;
					level1 = false;
					level2 = false;
				}
			}
			else if (checkCollectibleCollision(playerPosX, playerPosZ)) {
				score += 10;
			}
			else if (checkPowerupCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(JET_PATH, NULL, SND_FILENAME | SND_ASYNC);
					speed = true;
				}
				else if (level2) {
					shield = true;
				}
				powerUpTimer += 20;
			}
		break;
	case GLUT_KEY_LEFT:
		playerRotation = cameraRotation;
		playerPosX += moveSpeed * cos(DEG2RAD(playerRotation));
		playerPosZ += -moveSpeed * sin(DEG2RAD(playerRotation));
		if (checkWallCollision()) {
			playerPosX -= moveSpeed * cos(DEG2RAD(playerRotation));
			playerPosZ -= -moveSpeed * sin(DEG2RAD(playerRotation));
		}
		else
			camera.moveCamera(moveSpeed * cos(DEG2RAD(playerRotation)), 0, -moveSpeed * sin(DEG2RAD(playerRotation)), moveSpeed * cos(DEG2RAD(playerRotation)), 0, -moveSpeed * sin(DEG2RAD(playerRotation)));
		playerRotation += 90;
		if (checkGoalCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(CHEER_PATH, NULL, SND_FILENAME | SND_ASYNC);
					level1 = false;
					level2 = true;
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					gameTimer = 120 + 1;
					for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
						Collectible& c = *it;
						c.c = false;
					}

					for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
						Powerup& c = *it;
						c.c = false;
					}
					camera.look();
					PlaySound(LEVEL_2_PATH, NULL, SND_FILENAME | SND_ASYNC);

				}
				else if (level2) {
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					camera.look();
				}
			}
			if (checkObstacleCollision(playerPosX, playerPosZ)) {
				if ((level2 && !shield) || level1) {
					PlaySound(DEATH_PATH, NULL, SND_FILENAME | SND_ASYNC);

					lose = true;
					level1 = false;
					level2 = false;
				}
			}
			else if (checkCollectibleCollision(playerPosX, playerPosZ)) {
				score += 10;
			}
			else if (checkPowerupCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(JET_PATH, NULL, SND_FILENAME | SND_ASYNC);
					speed = true;
				}
				else if (level2) {
					shield = true;
				}
				powerUpTimer += 20;
			}
		break;
	case GLUT_KEY_RIGHT:
		playerRotation = cameraRotation;
		playerPosX += -moveSpeed * cos(DEG2RAD(playerRotation));
		playerPosZ += moveSpeed * sin(DEG2RAD(playerRotation));
		if (checkWallCollision()) {
			playerPosX -= -moveSpeed * cos(DEG2RAD(playerRotation));
			playerPosZ -= moveSpeed * sin(DEG2RAD(playerRotation));
		}
		else
			camera.moveCamera(-moveSpeed * cos(DEG2RAD(playerRotation)), 0, moveSpeed * sin(DEG2RAD(playerRotation)), -moveSpeed * cos(DEG2RAD(playerRotation)), 0, moveSpeed * sin(DEG2RAD(playerRotation)));
		playerRotation += 270;
		if (checkGoalCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(CHEER_PATH, NULL, SND_FILENAME | SND_ASYNC);
					level1 = false;
					level2 = true;
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					gameTimer = 120 + 1;
					for (auto it = Collectibles.begin(); it != Collectibles.end(); ++it) {
						Collectible& c = *it;
						c.c = false;
					}

					for (auto it = Powerups.begin(); it != Powerups.end(); ++it) {
						Powerup& c = *it;
						c.c = false;
					}
					camera.look();
					PlaySound(LEVEL_2_PATH, NULL, SND_FILENAME | SND_ASYNC);

				}
				else if (level2) {
					playerPosX = (1 * wallSize - mazeSize / 2.0) - 20;
					playerPosZ = (13 * wallSize - mazeSize / 2.0) - 40;
					glPushMatrix();
					glTranslatef(playerPosX, 0, playerPosZ);
					glRotatef(playerRotation, 0, 1, 0);
					if (!firstPerson) {
						glScalef(0.2, 0.2, 0.2);
						model_player.Draw();
					}
					glPopMatrix();
					camera.look();
				}
			}else 
			if (checkObstacleCollision(playerPosX, playerPosZ)) {
				PlaySound(DEATH_PATH, NULL, SND_FILENAME | SND_ASYNC);

				lose = true;
				level1 = false;
				level2 = false;
			}
			else if (checkCollectibleCollision(playerPosX, playerPosZ)) {
				score += 10;
			}
			else if (checkPowerupCollision(playerPosX, playerPosZ)) {
				if (level1) {
					PlaySound(JET_PATH, NULL, SND_FILENAME | SND_ASYNC);
					speed = true;
				}
				else if (level2) {
					shield = true;
				}
				powerUpTimer += 20;
			}
		break;
	case GLUT_KEY_F1:
		printf("hello");
		glutDestroyWindow(glutGetWindow());
		exit(0);
		break;
	case GLUT_KEY_F5:
		if (!firstPerson) {
			cameraRotation = 0;
			camera = Camera(
				playerPosX, playerHeight, playerPosZ,
				playerPosX, playerHeight, playerPosZ + 2 * cameraDistance,
				0.0, 1.0, 0.0
			);
			firstPerson = true;
		}
		else {
			cameraRotation = 0;
			camera = Camera(
				playerPosX, playerHeight + cameraHeight, playerPosZ - cameraDistance,
				playerPosX, playerHeight, playerPosZ,
				0.0, 1.0, 0.0
			);
			firstPerson = false;
		}
		break;
	}



	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y) {
	if (key == 27) {
		printf("hello");
		glutDestroyWindow(glutGetWindow());
		exit(0);
	}
	else if (key == 'f' || key == 'F') {
		if (isFullScreen) {
			glutReshapeWindow(windowWidth, windowHeight);
			isFullScreen = false;
		}
		else {
			glutFullScreen();
			isFullScreen = true;
		}
	}

}

// Function to handle mouse movement
void mouse_callback(int xpos, int ypos) {
	mousePosX = xpos;
	mousePosY = ypos;
}

void gTime(int value) {
	if (value == 0) {
		float xoffset = mousePosX - (windowWidth / 2);

		const float sensitivity = 0.3f;
		xoffset *= sensitivity;

		cameraRotation -= xoffset;

		// Update camera front vector
		camera.rotateY2(-xoffset);

		glutWarpPointer(windowWidth / 2, windowHeight / 2);


		glutTimerFunc(50, gTime, 0);
	}
	glutPostRedisplay();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_water.Load("Models/water2/Plastic Water Bottle Full.3ds");
	model_trap.Load("Models/export/models/nest_LOD3.3ds");
	model_spider.Load("Models/spider/Spider.3ds");
	model_speed.Load("Models/jetpack/jet pack.3ds");
	model_food.Load("Models/bun/bun1.3DS");
	model_player.Load("Models/player/kakashi.3ds");
	model_shield.Load("Models/shield/shiled.3ds");



	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_concrete.Load("Textures/concrete.bmp");
	tex_portal.Load("Textures/portal.bmp");
	tex_exit.Load("Textures/exit.bmp");
	tex_wall.Load("Textures/hedge.bmp");

	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
	loadBMP(&tex1, "Textures/night.bmp", true);

}

void timer(int value) {
	if (value == 0) {
if (gameTimer > 0) {
		gameTimer--;
	} else {
	level1 = false;
	level2 = false;
		lose = true;
	}

	if (powerUpTimer > 0) {
		if (speed) {
			moveSpeed = 6;
		} 
		powerUpTimer--;
	}
	else {
		if (speed) {
			speed = false; 
			moveSpeed = 3;
		}
		else if (shield) {
			shield = false;
		}
	}
	}
	
	glutTimerFunc(1000, timer, 0);

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);
		PlaySound(BACKGROUND_MUSIC_PATH, NULL, SND_FILENAME | SND_ASYNC);

	
	

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(windowWidth, windowHeight);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(Keyboard);
	glutPassiveMotionFunc(mouse_callback);
	glutTimerFunc(100, gTime, 0);
	glutTimerFunc(1000, timer, 0);
	glutTimerFunc(25, update, 0);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutWarpPointer(windowWidth / 2, windowHeight / 2);
	glutSpecialFunc(Special);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	//glutKeyboardFunc(myKeyboard);

	//glutMotionFunc(myMotion);

	//glutMouseFunc(myMouse);

	//glutReshapeFunc(myReshape);

	//myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	Obstacle o;
	o.x = 12 * wallSize - mazeSize / 2.0;
	o.y = 0;
	o.z = 12 * wallSize - mazeSize / 2.0;
	Obstacles.push_back(o);

	Obstacle o1;
	o1.x = 3 * wallSize - mazeSize / 2.0;
	o1.y = 0;
	o1.z = 2 * wallSize - mazeSize / 2.0;
	Obstacles.push_back(o1);

	Collectible c;
	c.x = 12 * wallSize - mazeSize / 2.0;
	c.y = 0;
	c.z = 4 * wallSize - mazeSize / 2.0;
	c.c = false;
	Collectibles.push_back(c);

	Collectible c1;
	c1.x = 4 * wallSize - mazeSize / 2.0;
	c1.y = 0;
	c1.z = 5 * wallSize - mazeSize / 2.0;
	c1.c = false;
	Collectibles.push_back(c1);

	Collectible2 c2;
	c2.x = 8 * wallSize - mazeSize / 2.0;
	c2.y = 0;
	c2.z = 12* wallSize - mazeSize / 2.0;
	c2.c = false;
	Collectibles2.push_back(c2);

	Collectible2 c3;
	c3.x = 6 * wallSize - mazeSize / 2.0;
	c3.y = 0;
	c3.z = 10 * wallSize - mazeSize / 2.0;
	c3.c = false;
	Collectibles2.push_back(c3);

	Powerup p;
	p.x = 3 * wallSize - mazeSize / 2.0;
	p.y = 0;
	p.z = 9.5 * wallSize - mazeSize / 2.0;
	p.c = false;
	Powerups.push_back(p);

	Powerup p1;
	p1.x = 12 * wallSize - mazeSize / 2.0;
	p1.y = 0;
	p1.z = 0.5 * wallSize - mazeSize / 2.0;
	p1.c = false;
	Powerups.push_back(p1);

	glutMainLoop();
}