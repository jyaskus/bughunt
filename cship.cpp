
#include "cship.h"

bool CShip::isLoaded()
{
	return (loaded);
}

bool CShip::canFire()
{
	// see if ship has enough power to shoot
	if (loaded == false)
		return false;
	
	if (weapon->getEnergy() > getSolar())
		return false;

	// else got enough power and solar

	// use energy, unload gun
	useSolar(weapon->getEnergy());
	loaded = false;

	return true;
};

int CShip::getHP()
{
	return (current.Hull + current.Armor + current.Shield);
}
int CShip::getMaxHP()
{
	return (Max.Hull + Max.Armor + Max.Shield);
}

// returns current value
int CShip::getCur(DMG_LOC hitLoc)
{
	int temp=0;
	switch(hitLoc)
	{
	case LOC_HULL:		temp=current.Hull;			break;
	case LOC_ARMOR:		temp=current.Armor;			break;
	case LOC_SHIELD: 	temp=current.Shield;		break;
	};
	return temp;
}	

POINT CShip::getMoveXY()
{
	POINT loc = {0,0};

	switch(getFacing())
	{		// based on direction we are heading, scroll the background - if possible
		case NORTH: loc.y-=speed; break;
		case SOUTH: loc.y+=speed; break;
		case EAST:  loc.x+=speed; break;
		case WEST:  loc.x-=speed; break;
		case NE:    loc.x+=speed; loc.y-=speed; break;
		case NW:    loc.x-=speed; loc.y-=speed; break;
		case SE:	loc.x+=speed; loc.y+=speed; break;
		case SW:	loc.x-=speed; loc.y+=speed; break;
	}

	return loc;
}
int CShip::getMoveX()
{
	POINT loc = getMoveXY();
	return loc.x;
};
int CShip::getMoveY()
{
	POINT loc = getMoveXY();
	return loc.y;
};


// returns Max value
int CShip::getMax(DMG_LOC hitLoc)	
{
	int temp=0;
	switch(hitLoc)
	{
	case LOC_HULL:		temp=Max.Hull;			break;
	case LOC_ARMOR:		temp=Max.Armor;			break;
	case LOC_SHIELD: 	temp=Max.Shield;		break;
	};
	return temp;
}


int CShip::getShields()
{
	return current.Shield; //Shield.energy;
};

// return false if ship destroyed in the process
bool CShip::applyDmg(DAMAGE dType, int dmg)
{
// shields absorb dmg but drains power

	// Apply to shields first
	// cuts in half ENERGY damage

// SHIELD BUFFER
/*
	if (current.Shield > 0)
	{
		int buffer = Solar.energy;
		if (dType == ENERGY)
			buffer * 2;	
		// energy recharges the shields?
		
		if ( dmg > buffer )
		{
			dmg -= buffer;
			Solar.energy = 0;
		}
		else
		{	// absorbed
			if (dType == ENERGY)
				Solar.energy -= (dmg/2); // absorbed
			else
				Solar.energy -= dmg;     // absorbed

			if (Solar.energy < 0)
				Solar.energy =0;

			return false; // still alive
		}
	*/
// SHIELDS
		// damage got through Buffer
		if (current.Shield > 0)
		{
			current.Shield -= dmg;
			if (current.Shield < 0)
				current.Shield = 0;
			return false; // not dead
		}

	// DMG still coming
// ARMOR
	// cuts in half KINETIC damage
//	if ( dType == KINETIC )
//		dmg = dmg / 2;

	if (dmg > current.Armor)
	{
		dmg -= current.Armor;
		current.Armor = 0;
	}
	else
	{ // absorbed by armor
		current.Armor -= dmg;
		return false; // still alive, armor saved ya
	}

// HULL is taking damage
	// some creatures more than 1 hull?
	if (dmg >= current.Hull)
	{
		// ship destroyed
		current.Shield=0;
		Solar.energy  =0;
		Shield.energy =0;
		current.Armor =0;
		current.Hull  =0;

		setSTATE(DEAD);

		return true;
	}
	else
		current.Hull -= dmg;

	return false;
};

void CShip::setMaxSpeed(int newMax)
{ max_speed = newMax; }

void CShip::setSpeed(int newSpeed)
{ 
	if (newSpeed <= max_speed)
		speed = newSpeed;
	else
		speed = max_speed;

	if (speed < 0)
		speed = 0;

	setDelay(max_speed - speed + 1); // what is this for ?
}

void CShip::addSpeed(int newSpeed)
{ 
	setSpeed(speed + newSpeed);
}


int CShip::getSpeed()
{ return speed; }

int CShip::getMaxSpeed()
{ return max_speed; }


bool CShip::checkBounds(int xMin,int yMin, int xMax,int yMax)
{
	// checks to make sure the sprite is still on screen
	// fixes it and returns 1 if otherwise ... and stops ship
	bool fixed=false;
	int ranDir = rand() % 2; // 0 or 1 left/right

	RECT r1;

	r1.left = getX();
	r1.top  = getY();
	r1.bottom = r1.top + getHeight();
	r1.right  = r1.left + getWidth();

	if (r1.left < xMin) // lower
	{
		setX(xMin);
		fixed=true;
	}
	else if (r1.right > xMax) // upper
	{
		setX(xMax - getWidth() );
		fixed=true;
	}

	if (r1.top < yMin) // lower
	{
		setY( yMin);
		fixed=true;
	}
	else if (r1.bottom > yMax) // upper
	{
		setY( yMax - getHeight() );
		fixed=true;
	}

	return fixed;
}

void CShip::doMove()
{
	// animate, if necessary
	if (getLastFrame() > 1)
		nextFrame();

	POINT move = getMoveXY();
	setMoveXY(move.x,move.y);
	addX(move.x);
	addY(move.y);
//	x += move.x;
//	y += move.yy;

	/*
	POINT oldPT = getPT();
	setPT( calcXY( getAngle(), getSpeed(),  getPT() ) );
	setMoveX(getPT().x - oldPT.x);
	setMoveY(getPT().y - oldPT.y);
	*/
}

void CShip::setGun(CGuns *newGun,int number)
{
	weapon = newGun;
	gunNum = number;
}

int CShip::getAmmo()
{
	return weapon->ammoType();
};
int CShip::getGun()
{
	return gunNum;
};

// constructor
CShip::CShip(int x, int y, int width, int height, LPCSTR filename, int newSpeed, int newMaxSpd, int Energy, int eRecharge, CGuns *Weapon, int GunNum, int Shields,int Armor, int Hull)
	  :CSprite(x,y,width,height,filename)
{	// call the sprites constructor

	// ships hp and such	
	Max.Armor	= Armor;
	Max.Hull	= Hull;
	Max.Shield	= Shields;

	// set current values to the max values
	current.Armor	= Armor;
	current.Hull	= Hull;
	current.Shield	= Shields;

	Shield.energy	= Shields;
	Shield.max		= Shields;
	Shield.delay	= 0;
	Shield.regen	= 50;

	// setup the ships speed info
	setMaxSpeed(newMaxSpd);
	setSpeed(newSpeed);	

	// starter frigate [20] solar energy
	Solar.energy	= Energy;
	Solar.max		= Energy;
	Solar.regen		= eRecharge; // 1 every 50 cycles
	Solar.delay		= 0;

	weapon = Weapon; //  blasters

	gunNum          = GunNum;
	gunDelay		= 0;
	gunRecharge		= weapon->getReload();
	loaded			= true;
};

// destructor
CShip::~CShip()
{

};

int CShip::getSolar()
{
	return Solar.energy;
};

int CShip::getSolarMax()
{
	return Solar.max;
};

// returns false if unable to use that amount of energy
bool CShip::useSolar(int amount)
{
	return ((Solar.energy-=amount) > 0);
}; 

/*
void CShip::setFacing(DIRS newFacing)
{
	CSprite::setFacing(newFacing); 

	// set new angle to match
	// as ships are using 0 degrees equal to EAST
	switch(newFacing)
	{
		case EAST:  setAngle(  0); break;
		case SE:    setAngle( 45); break;
		case SOUTH: setAngle( 90); break;
		case SW:    setAngle(135); break;
		case WEST:  setAngle(180); break;
		case NW:    setAngle(225); break;
		case NORTH: setAngle(270); break;
		case NE:    setAngle(315); break;
	}		
}
*/

// LEFT=0 RIGHT=1
void CShip::turn(int LEFTorRIGHT ) 
{
	// 0 = LEFT, 1 = RIGHT
	if ( LEFTorRIGHT == 0 ) // LEFT
		switch(getFacing())
		{
			case NORTH: setFacing(NW);    break; // 45 degree increments
			case SOUTH: setFacing(SE);    break; 
			case EAST:  setFacing(NE);    break; 
			case WEST:  setFacing(SW);    break;  
			case NE:    setFacing(NORTH); break;
			case NW:    setFacing(WEST);  break;
			case SE:    setFacing(EAST);  break;
			case SW:    setFacing(SOUTH); break;
		}
	else
		switch(getFacing()) // RIGHT
		{
			case NORTH: setFacing(NE);    break; 
			case SOUTH: setFacing(SW);    break; 
			case EAST:  setFacing(SE);    break; 
			case WEST:  setFacing(NW);    break;  
			case NE:    setFacing(EAST);  break;
			case NW:    setFacing(NORTH); break;
			case SE:    setFacing(SOUTH); break;
			case SW:    setFacing(WEST);  break;
		}

	doMove();
}
void CShip::setupShip(int nShield, int nArmor, int nHull)
{
	current.Shield = nShield;
	current.Armor  = nArmor;
	current.Hull  = nHull;

	Max.Shield = nShield;
	Max.Armor  = nArmor;
	Max.Hull   = nHull;
}

// regens solar energy if delay of cycles has passed
void CShip::regenSolar()
{
	Solar.delay++;

	if (Solar.delay >= Solar.regen)
	{ // recover
		Solar.energy++;
		Solar.delay=0;
	}

	if (Solar.energy > Solar.max)
		Solar.energy = Solar.max;
};

// regens solar energy if delay of cycles has passed
void CShip::regenShield()
{
	Shield.delay++;

	if (Shield.delay >= Shield.regen)
	{ // recover
		current.Shield++;
		Shield.energy++;
		Shield.delay=0;
	}

	if (Shield.energy > Shield.max)
		Shield.energy = Shield.max;	
	if (current.Shield > Max.Shield)
		current.Shield = Max.Shield;
};


void CShip::recharge()
{
	regenSolar();
	regenShield();

	if (loaded)
		return;

	//else
	gunDelay++;

	if (gunDelay >= gunRecharge)
	{
		loaded = true;
		gunDelay = 0;
	}
}

int CShip::getGunDelay()
{
	return gunDelay;
};
