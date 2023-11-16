#ifndef DEBRIS_H
#define DEBRIS_H

#include "csprite.h"

// Constant values
const int NUM_MAPS_SPACE  = 1;
const int NUM_MAPS_SYSTEM = 8; // 00 - 07, aligned with stars # 0 - 7 (1-8)
const int NUM_MAPS_PLANET = 8;

// number of sprites in the sprite sheet
const int DEBRIS_SPRITES_SPACE   = 10; 
const int DEBRIS_SPRITES_SYSTEM  = 10; 
const int DEBRIS_SPRITES_PLANET  = 10; 
const int DEBRIS_SPRITES_PER_ROW = 5;

// number of objects to randomly spawn for each "map" or game view
const int DEBRIS_AMT_SPACE		= 30; 
const int DEBRIS_AMT_SYSTEM		= 30;
const int DEBRIS_AMT_PLANET		= 30;

// for the non moving background objects like planets or such
const int IMMOBILE_SPRITES_SPACE   = 10; 
const int IMMOBILE_SPRITES_SYSTEM  = 15; 
const int IMMOBILE_SPRITES_PLANET  = 10; 
const int IMMOBILE_SPRITES_PER_ROW = 5;

const int IMMOBILE_AMT_SPACE		= 18; 
const int IMMOBILE_AMT_SYSTEM		= 10;
const int IMMOBILE_AMT_PLANET		= 10;

// CDebris the class used for the random, moving objects on each level
class CPlanet : public CSprite
{
public:
	void doMove();
	CPlanet(int X,int Y,int Width,int Height,LPCSTR Filename, int max);
	void setMap(int newmap);
	int getMap();

private:
	int rotation; // speed of rotation
	int degree;   // accumulated rotation, as it orbits a target
	int map;      // number of the map used for this planet (-1) means no map, so no "docking"
	bool orbiting;
	POINT orbitPT;
};

// CDebris the class used for the random, moving objects on each level
class CDebris : public CSprite
{
public:
// bool isMoving()
	void doMove();

	void checkExit();
	void randomSpawn();

	CDebris(int Width,int Height,LPCSTR Filename, int Frames, int max_x, int max_y);

private:
	int rotation; // speed of rotation
	int speed;
	int sDelay; // spawn delay counter
	POINT max;
};

#endif

