#include "cdebris.h"

void CPlanet::setMap(int newmap)
{
	map = newmap;
};
int CPlanet::getMap()
{
	return map;
};


void CPlanet::doMove()
{
	// do orbit
	// increase angle as well, as some objects will be rotating
	setAngle(getAngle() + rotation);
};

CPlanet::CPlanet(int X,int Y,int Width,int Height,LPCSTR Filename,int max)
		:CSprite(X,Y,Width,Height,Filename)
{
	setupAnim(max,EAST,IMMOBILE_SPRITES_PER_ROW);

	// choose random frame
	setFrame(rand() % max);	 // 0 - 9 (10)

	// speed of  rotation
	rotation = rand() % 9 -4; 
	setAngle(rotation);
}


void CDebris::doMove()
{
	CSprite::doMove();

	// increase angle as well, as some objects will be rotating
	setAngle(getAngle() + rotation);
};

void CDebris::randomSpawn()
{
	// choose random frame
	setFrame(rand() % getLastFrame());	// 0-14 (15)

	// randomize it all
	rotation = rand() % 6 -2; // speed of  rotation
	setAngle(rotation);

	speed = (rand() % 4) + 1; // 1-4	

	POINT nMax;
	nMax.x = max.x - getWidth()-1;
	nMax.y = max.y - getHeight()-1;

	// and random border to space from
	// with random heading and speed
	switch((rand() % 4) + 1)
	{
	case 1: // heading west-ish
		{
			setMoveXY(-speed,(rand() % 3) -1 );
			setXY(nMax.x,rand() % nMax.y);
			setAngle(180);
			if (getMoveY() < 0)
				addAngle(+45); // nw
			if (getMoveY() > 0)
				addAngle(-45); // sw
		};break;
	case 2: // heading east-ish
		{
			setMoveXY(speed,(rand() % 3) -1 );
			setXY(0,rand() % nMax.y);			
			setAngle(0);
			if (getMoveY() < 0)
				addAngle(-45); // ne
			if (getMoveY() > 0)
				addAngle(+45); // se

		};break;
	case 3: // heading north-ish
		{
			setMoveXY((rand() % 3) -1,-speed);
			setXY(rand() % nMax.x,nMax.y);			
			setAngle(270);
			if (getMoveX() < 0)
				addAngle(-45); // ne
			if (getMoveX() > 0)
				addAngle(+45); // nw
		};break; 
	case 4: // heading south-ish
		{
			setMoveXY((rand() % 3) -1,speed);
			setXY(rand() % nMax.x,0);			
			setAngle(90);
			if (getMoveX() < 0)
				addAngle(+45); // sw 
			if (getMoveX() > 0)
				addAngle(-45); // se
		};break;
	}; // end SWITCH
}

void CDebris::checkExit()
{
	POINT loc = getAnchor();

	// if it exits the game map ... respawn it somewhere else
	if (( loc.x < 0) || (loc.x > 2048) || (loc.y < 0) || (loc.y > 2048))
	{
		sDelay++;

		if (sDelay > 30)
		{
			randomSpawn();
			sDelay = 0;
		}
	}
		// border collision occured;
}


CDebris::CDebris(int Width,int Height,LPCSTR Filename,int Frames, int max_x, int max_y)	
	:CSprite(rand() % max_x,rand() % max_y,Width,Height,Filename)
{
	setupAnim(Frames,EAST,DEBRIS_SPRITES_PER_ROW);

	// this will keep the sprites within gameworld or respawn them
	setBorder(0,0,max_x,max_y);
	max.x = max_x;
	max.y = max_y;

	sDelay = 0;

	// random spawn in with random heading as well
	randomSpawn();
}
