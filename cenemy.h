
#include "cship.h"

enum ENEMY_STATE
{
	INACTIVE,
	ACTIVE,
	ATTACKING,
	DYING,
	DEAD
};

class CEnemy : public CShip
{
public:
	CGuns *weapon;

	int getAmmo();
	int getCur(DMG_LOC hitLoc);	// returns current value
	int getMax(DMG_LOC hitLoc);	// returns Max value

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
	void turn(int LEFTorRIGHT);

	int getSolar();
	int getSolarMax();

	// returns false if unable to use that amount of energy
	bool useSolar(int amount); 
	void regenSolar();
	int getShields();

	void regenShield();

	// constructor
	CShip(int x, int y, int width, int height, LPCSTR filename, int newSpeed, int newMaxSpd,int Energy, int eRecharge, CGuns *Weapon, int Shields,int Armor, int Hull);

	// destructor
	~CShip();

private:
	dmgType current;
	dmgType Max;

	energyType Solar;       // Energy 
	energyType Shield;

	int speed,max_speed;

	int upgrade_slots; // upgrade slot class ... name, effect, etc
	int gun_slots;		// number of gun slots available
};