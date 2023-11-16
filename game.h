// Beginning Game Programming, 2nd Edition
// Chapter 8
// Tiled_Sprite program header file

#ifndef GAME_H
#define GAME_H

//windows/directx headers
#include <d3dx9math.h>
#include <dxerr9.h>
#include <dinput.h>
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "libwmp3.h"

#include <iostream> 
#include <fstream>

using namespace std;

//framework-specific headers
#include "dxgraphics.h"
#include "dxinput.h"

#include "csprite.h"
#include "cship.h"
#include "cguns.h"
#include "cdebris.h"

//application title
#define APPTITLE "SPACE Bug Hunt - alpha"

//screen setup // GLOBALS //
#define FULLSCREEN    0     //0 = windowed, 1 = fullscreen
#define SCREEN_WIDTH  1024  //1280 // 1024 // 800 
#define SCREEN_HEIGHT 768   // 960 // 768  // 600 

#define TILEWIDTH  64
#define TILEHEIGHT 64
#define MAPWIDTH   32 //24 //16
#define MAPHEIGHT  32 //64 //96
#define GAMEWORLDWIDTH  2048 //(TILEWIDTH * MAPWIDTH)
#define GAMEWORLDHEIGHT 2048 // (TILEHEIGHT * MAPHEIGHT)

// scrolling window size
#define WINDOWWIDTH  (SCREEN_WIDTH / TILEWIDTH)   * TILEWIDTH
#define WINDOWHEIGHT (SCREEN_HEIGHT / TILEHEIGHT) * TILEHEIGHT

// scroll buffer size
#define SCROLLBUFFERWIDTH  (SCREEN_WIDTH + TILEWIDTH   * 2)
#define SCROLLBUFFERHEIGHT (SCREEN_HEIGHT + TILEHEIGHT * 2)

//function prototypes
 int Game_Init(HWND);
void Game_Run(HWND);
void Game_End(HWND);

// dynamic scrolling map support functions
void DrawTile(LPDIRECT3DSURFACE9, int,int,int,int, LPDIRECT3DSURFACE9, int, int);
void DrawTiles();
void DrawScrollWindow();
void UpdateScrollPosition();
void loadMaps();		// loads the static and moving object maps

void DrawSprite(float x, float y, int width, int height, float sx, float sy, float angle, unsigned char alpha, LPDIRECT3DTEXTURE9 sprite, RECT *rc, IDirect3DDevice9* pd3dDevice, bool FixAngle);

void DrawSprites();

void Check_Keys();

 DIRS getDirection(POINT sourcePT, POINT targetPT);		// determine which direction to turn to face target
float findDistance(POINT pt1,POINT pt2);				// returns distance in pixels
bool min_max(int num, int min,int max);					// returns t/f is given # is with the range 

void drawNumbers(int number, int screenX, int screenY);
void drawText(LPCSTR outText, int screenX, int screenY);
void drawBar(int x, int y,int curr,int max, int color);	// draw bar at location; using current, max # and color
void drawBars();										// wrapper to draw player / enemy health bars

 int SpriteCollision(CSprite *sprite1, CSprite *sprite2);// test for collisions
bool rectCollision(RECT r1, RECT r2);					
bool circleCollision(POINT a,int radius1, POINT b, int radius2);

void createDebris();									// randomly creates the initial Debris for SPACE/SYSTEM/PLANET views
void respawnDebris();									// checks if debris leaves game space and spawns new debris
void drawPlanets();										// non-moving objects
void drawDebris();										// random moving objects
void moveDebris();
void checkDebris();										// player vs debris collision testing and results of impact

int planetCollision(CSprite *target);					// returns # of junk sprite collided with and reverses movement, if collided
int debrisCollision( CSprite *target);					// updated xy velocity after impact of 2 objects
void collision2Ds(double m1, double m2,
                 double x1, double y1, double x2, double y2);

int getScrollX(int x);
int getScrollY(int y);

void Init_Guns();										// setup the guns and ammo (for all SHIPS)
void moveBullets();										// moves the active bullets
void drawBullets();										// draws the bullet sprites
void bulletCollision(CShip *target);
void bullet_AI_Collision(CShip *target);

void createDrones();									// assigns the initial values to enemy ships
void respawnDrones(int );
void EnemyAI();											// makes AI choices/state changes
void BossAI();
void respawnBoss();

void DrawBG(IDirect3DSurface9  *pSource);


POINT offsetPT(POINT pt1, int Width, int Height,DIRS Facing);

// handle the explosions
void newExp(POINT dest);
void animExp();
void drawExp();

typedef enum
{
	Splash1,
	Splash2,
	Splash3,
	Menu,
	Options,
	RunGame
} MENU_STATE;

typedef enum
{
	SPACE  = 1,
	SYSTEM = 2,
	PLANET = 3
} GAME_STATE;

#endif
