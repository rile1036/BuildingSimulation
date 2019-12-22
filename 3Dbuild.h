//Frank Fodera
//3DHouse.
//This is the header file for the main .cpp file which will create a 3D rendered house.

// 헤더 파일
#include <stdio.h>          // Standard C/C++ Input-Output
#include <math.h>           // Math Functions
#include <windows.h>        // Standard Header For MSWindows Applications
#include "glut.h"			// The GL Utility Toolkit (GLUT) Header

// The Following Directive Fixes The Problem With Extra Console Window
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

//Pre-defined variables
#define PI_OVER_180  0.0174532925f	//This is to convert radians to degrees

//Created Structs
typedef struct						// Create A Structure
{
	GLubyte	*imageData;				// Image Data (Up To 32 Bits)
	GLuint	bpp;					// Image Color Depth In Bits Per Pixel.
	GLuint	width;					// Image Width
	GLuint	height;					// Image Height
	GLuint	texID;					// Texture ID Used To Select A Texture
} TextureImage;						// Structure Name		

typedef struct _VERTEX {			// The Vertex Struct
	float x, y, z;					// The x y and z coordinates for a vertex used to put into the world
	float u, v;						// Used when mapping a texture to the polygon
} VERTEX;							// Structure name

typedef struct _POLYGON {			// A Polygon struct
	VERTEX vertex[4];				// A polygon consists of 4 vertices
} POLYGON;							// Structure name

typedef struct _SECTOR {			// The Secture Struct
	int numpolygons;				// This reads from the text file the number of polygons
	int* texture;					// Reads the texture that we map to that polygon
	POLYGON* polygon;				// Takes in a polygon whose coordinates are in the text file
} SECTOR;							// Structure Name

// Global Variables
bool    g_gamemode;					// GLUT GameMode ON/OFF
bool    g_fullscreen;				// Fullscreen Mode ON/OFF (When g_gamemode Is OFF)
bool    g_key[255];					// Lookup Table For Key's State
bool	DOOR_IS_OPEN	= false;	// Used to Determine if the door is open
bool	ENTER_WAS_PRESSED = false;	// 엔터키 상태 값
bool	LIGHT_ON = false;			// 조명 활성화 상태 값
GLfloat g_xpos			= 0.0f;		// 캐릭터가 존재하는 x값
GLfloat g_zpos			= 0.0f;		// 캐릭터가 존재하는 z값
GLfloat g_ypos			= 0.45f;	// 캐릭터가 존재하는 y값
GLfloat g_xspeed		= 0.0f;	    // x축 회전 속도
GLfloat g_yspeed		= 0.0f;	    // y축 회전 속도
GLfloat	g_z				= 0.0f;	    // 스크린의 깊이값 
GLfloat g_yrot			= 0.0f;		// 캐릭터 방향 각도
GLfloat speed_XYZ		= 0.015f;	// 캐릭터 움직임의 속도
GLfloat speed_UDLR		= 0.50f;	// 시점 방향 속도 
GLfloat g_lookupdown	= 0.0f;		// 시점 방향
GLfloat GlobalAmbient[] = { 1.0, 1.0, 1.0, 1.0 }; // 전역 조명 모델의 주변광 설정
GLuint	g_filter;					// Which Filter To Use 
GLuint	base;						// Font Display List
SECTOR  g_sector1;					// Our Model Goes Here:
TextureImage textures[30];			// Storage For 10 Textures
GLUquadricObj *my_shape[3];
int window_width, window_height, screen_height, screen_width; // The values for setting up the window

//Function Protocols
bool setup_textures(char *name);
GLvoid build_font(GLvoid);										// Build Our Font Display List
void game_function();											// Our Game Function. Check The User Input And Performs The Rendering
GLvoid gl_print(GLint x, GLint y, const char *string, ...);		// Where The Printing Happens
bool init();													// The function to initially set up everything
void keyboard(unsigned char key, int x, int y);					// Our Keyboard Handler (Normal Keys)
void readstr(FILE* f, char* string);							// Reads from a file
void render(void);												// Renders all the objects and text
void reshape(int w, int h);										// Our Reshaping Handler (Required Even In Fullscreen-Only Modes)
void setup_world();												// Reads all of the stuff from the text document and puts it into an array
void special_keys(int a_keys, int x, int y);					// Called when special keys are pressed
void special_keys_up(int key, int x, int y);					// Called when a key is unpressed
bool no_collision();												// Used to check if there is a collision with a wall
bool Load_TGA(TextureImage *texture, char *filename);			// Loads A TGA File Into Memory
void object(float width,float height,GLuint texid);				// Draw Object Using Requested Width, Height And Texture
void texture_object(int polygonNum);								// Textures an object that we read in earlier taking an index for the array