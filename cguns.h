#ifndef GUNS_H
#define GUNS_H

#include "csprite.h"

typedef enum
{
	BLASTER = 0,
	LASER	= 1,
	CANNON	= 3,
	MISSILE	= 4,
	TORPEDO	= 5
} GUNS;

typedef enum
{
	ENERGY	= 0,	// Photon Torpedos, Lasers and Blasters -- stopped by shields
	KINETIC	= 1,    // Cannon --- ignores shields
	EXPLODE	= 2     // Missiles --- Ignores shields
} DAMAGE;

typedef enum
{
	RANGE_SHORT	=  400, //pixels
	RANGE_MED	=  800,
	RANGE_LONG  = 1200
} RANGES;

typedef enum
{
	DELAY_SHORT	=   25, // # of cycles to skip between recharges
	DELAY_MED	=   50,
	DELAY_LONG  =   75
} DELAYS; 

const int MAX_BULLETS = 99; // of each type
const int MAX_AMMO    = 5;  
const int MAX_GUNS    = 5;
const int MAX_GUNS_AI = 2; // used for AI stuff

const int TOO_CLOSE   = 100; // stop/try to move away if this close

// ----------------------------------------------------- //
// ------------------ PROJECTILE CLASS ----------------- //
struct bulletType
{
	int traveled; // distance traveled
	DIRS facing;
	int angle;
	bool isActive;
	POINT startPT;
	POINT currPT;
};

class CProjectile  : public CSprite
{
public:
	int getDmg();
 DAMAGE getDmgType();
	int getSpeed();
	int getRange(); //returns the weapons max range

	CProjectile(int dmg, DAMAGE dmgType, int Speed, int Range, LPCSTR Filename, int width,int height);

	// over-ride the default one
	bool doMove(int bullet, float scalex); 
	void doMoves(float scalex); // does all active ones

	// also sets up dirFacing (OVER RIDE from parent class)
	void setupAngle(int newA);

	int spawnBullet(POINT start, DIRS facing);

	bulletType Bullets[MAX_BULLETS]; // one projectile, with many bullets it tracks -- 

private:	
	int dmg;	 // amount
	DAMAGE dmgType; // determines if passes through shields and explosion type

	int speed;   // 10+ pixels/iteration
	int range;   //
};

// ----------------------------------------------------- //
// ----------------- GUNS CLASS ------------------------ //
class CGuns
{
public:

//	CGuns(CProjectile *ammo, int Range,int Reload, int EnergyCost,LPCSTR gunsName);
//	CGuns(int ammoNum, int Range, int Reload, int EnergyCost, LPCSTR gunsName);
	//                  gun max          reload                                                     DMG   bullet  Anim
	//                  #   range        delay	  nrgy dmg name		bullet bmp							type	spd	 frames
//	Guns[0] = new CGuns(0, RANGE_MED , DELAY_MED,	2,	1	"Blaster"	,"./Media/Images/proj_blaster.bmp",	ENERGY	,12	,1);
//	CGuns(int ammoNum, int Range, int Reload, int EnergyCost, int Damage, LPCSTR gunsName, LPCSTR Filename, DAMAGE dType, int Speed, int Frames);
	CGuns(int ammoNum,int Reload, int EnergyCost, LPCSTR gunsName);
    //        ammo,range, reload,energy,Name,bmpFile 

	int getReload();

	// energy cost per shot
	int getEnergy();

	int ammoType();

	LPCSTR getFilename();
	int getDmg();
	DAMAGE getDmgType();

	void Fire(POINT dest, DIRS direction); 

//	CProjectile *Ammo;
private:
	LPCSTR Name;
	LPCSTR bmpFile;
	int frames;

	 int ammo;
  DAMAGE dmgType;

	 int damage;
	 int eCost;  // # per use

	bool loaded;
 	 int reload;  // cycles to reload
	 int delay;
};

#endif