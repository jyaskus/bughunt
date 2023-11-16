
#ifndef _SHIPS_H
#define _SHIPS_H

#include "csprite.h"
#include "cguns.h"

typedef enum
{
	LOC_HULL,
	LOC_ARMOR,
	LOC_SHIELD
} DMG_LOC;

typedef struct 
{
	int Hull;
	int Armor;
	int Shield;
} dmgType;

typedef struct
{
	int energy; // current
	int regen; // 1 every x cycles
	int max;
	int delay; // current counter
} energyType;

class CShip : public CSprite
{
public:
	CGuns *weapon;


	void setupShip(int nShield, int nArmor, int nHull);

	// true if power enough to fire gun
	void recharge();
	bool canFire();
	int getGunDelay();
	// returns true if gun is loaded
	bool isLoaded();

	int getAmmo();
    void setGun(CGuns *newGun, int GunNum);

	int getCur(DMG_LOC hitLoc);	// returns current value
	int getMax(DMG_LOC hitLoc);	// returns Max value

	int getHP();				// returns total hitpoints
	int getMaxHP();				// max hit points total

	// return false if ship destroyed in the process
	bool applyDmg(DAMAGE damage, int amount);

	void addSpeed(int newSpeed);	// adds to ships speed, up to max speed
	void setMaxSpeed(int newMax);
	void setSpeed(int newSpeed);
	 int getSpeed();
	 int getMaxSpeed();

	 // override these inherited functions
	 int getMoveX();
	 int getMoveY();
	 POINT getMoveXY();

	bool checkBounds(int xMin,int yMin, int xMax,int yMax);
	void doMove();
//	void setFacing(DIRS newFacing);
	void turn(int LEFTorRIGHT);	

	int getSolar();
	int getSolarMax();
	int getGun();

	// returns false if unable to use that amount of energy
	bool useSolar(int amount); 
	void regenSolar();
	int getShields();

	void regenShield();

	// constructor
	CShip(int x, int y, int width, int height, LPCSTR filename, int newSpeed, int newMaxSpd,int Energy, int eRecharge, CGuns *Weapon, int GunNum, int Shields,int Armor, int Hull);

	// destructor
	~CShip();

private:
	dmgType current;
	dmgType Max;

	energyType Solar;       // Energy 
	energyType Shield;

	int speed,max_speed;

	int gunNum;
	int gunDelay;
	int gunRecharge;
	bool loaded;

	int upgrade_slots; // upgrade slot class ... name, effect, etc
	int gun_slots;		// number of gun slots available
};

#endif