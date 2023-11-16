
#include "cguns.h"

int CProjectile::spawnBullet(POINT start, DIRS direction)
{
	int i=0;
	while ((Bullets[i].isActive==true) && (i < MAX_BULLETS))
		i++;

	// is there room left ?
	if (i < MAX_BULLETS)
	{
		// activate this one
		Bullets[i].isActive = true;
		Bullets[i].startPT  = start;
		Bullets[i].currPT   = start;
		Bullets[i].traveled = 0;
		Bullets[i].facing   = direction;

		switch(direction)
		{
			case NORTH: Bullets[i].angle = 270;  break;
			case NE:    Bullets[i].angle = 315;  break;
			case EAST:  Bullets[i].angle =   0;  break;
			case SE:    Bullets[i].angle =  45; break;
			case SOUTH: Bullets[i].angle =  90; break;
			case SW:    Bullets[i].angle = 135; break;
			case WEST:  Bullets[i].angle = 180; break;
			case NW:    Bullets[i].angle = 225; break;
		}		

		return i; // bullet was fired, return its number
	}
	else
		return -1;	 
		// means no room to fire another bullet ... 
};

// Bullets
int CProjectile::getDmg()
{
	return dmg;
};
DAMAGE CProjectile::getDmgType()
{
	return dmgType;
};
int CProjectile::getSpeed()
{
	return speed;
};
// max range in pixels
int CProjectile::getRange()
{
	return range;
};

bool CProjectile::doMove(int bulletNum, float scalex)
{
	// animate as well 
	if (getLastFrame() > 1)
		nextFrame();

	if (Bullets[bulletNum].traveled <= range ) // 0.75 SYSTEM, 0.5 SPACE
	{
		// find new XY location
		switch(Bullets[bulletNum].facing)
		{		// based on direction we are heading, scroll the background - if possible
				case NORTH: Bullets[bulletNum].currPT.y -= speed; break;
				case SOUTH: Bullets[bulletNum].currPT.y += speed; break;
				case EAST:  Bullets[bulletNum].currPT.x += speed; break;
				case WEST:  Bullets[bulletNum].currPT.x -= speed; break;
				case NE:    Bullets[bulletNum].currPT.x += speed; 
							Bullets[bulletNum].currPT.y -= speed; break;
				case NW:    Bullets[bulletNum].currPT.x -= speed; 
							Bullets[bulletNum].currPT.y -= speed; break;
				case SE:	Bullets[bulletNum].currPT.x += speed; 
							Bullets[bulletNum].currPT.y += speed; break;
				case SW:	Bullets[bulletNum].currPT.x -= speed; 
							Bullets[bulletNum].currPT.y += speed; break;
		}

		//Bullets[bulletNum].currPT = calcXY( Bullets[bulletNum].angle, speed,  Bullets[bulletNum].currPT);		
		Bullets[bulletNum].traveled += speed;

		POINT tPT = Bullets[bulletNum].currPT;
		if ( (tPT.x < 0) || (tPT.y < 0) || (tPT.x > 2048) || (tPT.y > 2048))
			return false; // it left the map

		return true;
	}
	else 
		return false; // bullet is out of range now (stop moving it)
};

void CProjectile::doMoves(float scalex)
{
	bool success=false;

	for (int i=0; i < MAX_BULLETS;i++)
		if (Bullets[i].isActive)
		{
			success = doMove(i,1.0f); // scalex);	
			if ( ! success )
			{
				Bullets[i].isActive = false;
				// bullet is out of range -- it will stop moving
				// but should be set to not active
			}
		}
};


void CProjectile::setupAngle(int newA)
{
	setAngle(newA);

	// set angle as well
	switch(newA)
	{
	case   0:
	case 360: setFacing(NORTH); break;
	case  45: setFacing(NE);	break;
	case  90: setFacing(EAST);	break;
	case 135: setFacing(SE);	break;
	case 180: setFacing(SOUTH); break;
	case 225: setFacing(SW);	break;
	case 270: setFacing(WEST);	break;
	case 315: setFacing(NW);	break;
	};
}

// constructor for projectile class
CProjectile::CProjectile(int Dmg, DAMAGE DmgType, int Speed, int Range, LPCSTR Filename, int width,int height)
: CSprite(50,50,width,height,Filename)
{
	dmg		= Dmg;
	dmgType = DmgType;
	speed	= Speed;
	range	= Range;

	// clear the bullets array,
	for (int i=0; i < MAX_BULLETS; i++)
		Bullets[i].isActive = false;

	// make it active, when its first created
	setSTATE(ACTIVE);
};


// Guns
void CGuns::Fire(POINT dest, DIRS direction)
{
//	if (Ammo[ammo]->spawnBullet(dest,direction))
//	{
//		// bullet fired, consume ammo/power
//	}
//	else
//	{
//		// guns didnt fire
//	}
}; 

LPCSTR CGuns::getFilename()
{
	return bmpFile;
};

int CGuns::getDmg()
{
	return damage;
};
DAMAGE CGuns::getDmgType()
{
	return dmgType;
};



int CGuns::ammoType()
{
	return ammo;
};

int CGuns::getReload()
{

	return reload;
}

// energy cost per shot
int CGuns::getEnergy()
{
	return eCost;
};


	//                  gun max          reload                                                     DMG   bullet  Anim
	//                  #   range        delay	  nrgy dmg name		bullet bmp							type	spd	 frames
//	Guns[0] = new CGuns(0, RANGE_MED , DELAY_MED,	2,	1	"Blaster"	,"./Media/Images/proj_blaster.bmp",	ENERGY	,12	,1);
//CGuns::CGuns(int ammoNum, int Range, int Reload, int EnergyCost, int Damage, LPCSTR gunsName, LPCSTR Filename, DAMAGE dType, int Speed, int Frames)
CGuns::CGuns(int ammoNum, int Reload, int EnergyCost,LPCSTR gunsName)
{
	ammo = ammoNum;
	reload=Reload;		// cycles to reload
	delay=0;
	eCost=EnergyCost;  // # per use
	Name = gunsName;

	// this data will all be static once defined
	loaded=true;
};

