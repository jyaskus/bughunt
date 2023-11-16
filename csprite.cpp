// CSprite class

#include "csprite.h"


// --------------------------//
// ------- CSPRITE ----------//

LPCSTR CSprite::getFilename()
{
	return filename;
};

int CSprite::nextFrame()
{ 
	if (++animcount > animdelay)
		animcount=0;
	else
		return 0; // use anim delay to slow down animations

	// increments current frame and checks for max
	if (++curframe > lastframe)
	{
		curframe=0;
		return 1;
	}
	else
		return 0;
}
int CSprite::getFrame()
{
	return curframe; 
}
void CSprite::setFrame(int newFrame)
{
	curframe=newFrame; 
}
void CSprite::setLastFrame(int newLastFrame)
{
	lastframe = newLastFrame; 
}
int CSprite::getLastFrame()
{
	return lastframe; 
}

void CSprite::setDelay(int newDelay)
{
	animdelay = newDelay; 
}
int CSprite::getDelay()
{
	return animdelay;
}


void CSprite::setXY(int newX, int newY)
{
	x = newX; 
	y = newY; 
}	

void CSprite::setMoveXY(int moveX, int moveY)
{ 
	movex = moveX;
	movey = moveY;
}

int CSprite::getX()
{
	return x; 
}
int CSprite::getY()
{
	return y; 
}

int CSprite::getMoveY()
{
	return movey; 
}
void CSprite::setMoveY(int newY)
{
	movey = newY; 
}

int CSprite::getMoveX()
{
	return movex; 
}
void CSprite::setMoveX(int newX)
{
	movex = newX;	
}

void CSprite::setX(int newX)
{
	x = newX;	
}
void CSprite::addX(int addtoX)
{
	x += addtoX; 

}

void CSprite::setY(int newY)
{
	y = newY;	
}
void CSprite::addY(int addtoY)
{
	y += addtoY; 
}

int CSprite::getHeight()
{
	return height; 
}
int CSprite::getWidth()
{
	return width;	
}

LPDIRECT3DTEXTURE9 CSprite::getImage()
{
	return _image;
}

// returns a rect pointing to the sprites frame 
RECT CSprite::getRect()
{
	/*	EAST	= 1,
		NORTH	= 2,
		NE		= 3,
		NW		= 4,
		SOUTH	= 5,
		SE		= 6,
		SW		= 7,
		WEST	= 8		
	*/

	RECT srcRect;

	// will return the RECT point to current sprite, within sprite sheet		
	srcRect.left   = (curframe % columns) * width;
	srcRect.top    = (curframe / columns) * height;
	srcRect.right  = srcRect.left + width;
	srcRect.bottom = srcRect.top  + height;
	
	return srcRect;
}


// returns a collision rect, using GAMEWORLD coordinates
RECT CSprite::collisionRect(int ScrollX,int ScrollY)
{
	POINT loc;
	loc.x = x - ScrollX;
	loc.y = y - ScrollY;

	RECT r1;
	r1.left		= loc.x + 1;
	r1.top		= loc.y + 1;
	r1.right    = loc.x + width  -2;
	r1.bottom	= loc.y + height -2;

	return r1;
}; 

// returns a collision rect, using GAMEWORLD coordinates (this one is scaled down)
RECT CSprite::scaledRect(int ScrollX,int ScrollY, float scaleX, float scaleY)
{
	POINT scaled;
	scaled.x = (int)((width/2)  * scaleX); // the gets the x amount to add/subtract from center point	
	scaled.y = (int)((height/2) * scaleY); // the gets the x amount to add/subtract from center point

	POINT center;
	center.x = x - ScrollX + (width  / 2) -1; // since first pixel is counted (0)
	center.y = y - ScrollY + (height / 2) -1;

	RECT r1;
	r1.left		= center.x - scaled.x;
	r1.top		= center.y - scaled.y;
	r1.right    = center.x + scaled.x;
	r1.bottom	= center.y + scaled.y;

	return r1;
}; 


DIRS CSprite::getFacing()
{
	return facingDir; 
}
void CSprite::setFacing(DIRS newFacing)
{
	facingDir = newFacing; 

	// set new angle to match
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
/*	switch(facingDir)
	{
	case NORTH: angle = 0;   break;
	case SOUTH: angle = 180; break;
	case EAST:  angle = 90;  break;
	case WEST:  angle = 270; break;
	case NE:    angle = 45;  break;
	case NW:    angle = 315; break;
	case SE:    angle = 135; break;
	case SW:    angle = 225; break;
	}		*/

}; 

bool CSprite::isMoving()
{
	return moving; 
}

void CSprite::setMoving(bool newState)
{
	moving = newState;	
}

POINT CSprite::getPT()
{
	POINT tPT;
	tPT.x = x;
	tPT.y = y;

	return tPT;
}

void CSprite::setPT(POINT tPT)
{
	x = tPT.x;
	y = tPT.y;
}

void CSprite::setNewDIR(int newDir)
{
	switch(newDir)
	{
		case  1: movey=-2;				facingDir = NORTH;	break;	// NORTH
		case  2: movey= 2;				facingDir = SOUTH;	break;	// SOUTH
		case  3: movex= 2;				facingDir = EAST;	break;	// EAST
		case  4: movex=-2;				facingDir = WEST;	break;	// WEST
		case  5: movex= 2;	movey=-1;	facingDir = NE;		break;	// NE
		case  6: movex=-2;	movey=-1;	facingDir = NW;		break;	// NW
		case  7: movex= 2;	movey= 1;	facingDir = SE;		break;	// SE
		case  8: movex=-2;	movey= 1;	facingDir = SW;		break;	// SW
	};
}

// defines the screen walkable/collision border for this sprite
void CSprite::setBorder(int Left, int Right, int Top, int Bottom)
{
	border.left		= Left;
	border.right	= Right;
	border.top		= Top;
	border.bottom	= Bottom;
}

// returns TRUE if sprite is within its defined border, FALSE otherwise
bool CSprite::inBorder()
{
	// see if it is inside the border boundaries
	POINT anchor = getAnchor();
	if ((anchor.x < border.left) || (anchor.x > border.right) || (anchor.y < border.top)  || (anchor.y  > border.bottom))
		return false;

	return true;
}

bool CSprite::checkBorder()
{
	bool fixed = false;

	// ( (x <= border.right) && (x >= border.left) && (y <= border.bottom) && (y >= border.top) );
	if (x < border.left)
	{
		x = border.left;
		fixed = true;
	}
	else if (x > border.right)
	{
		x = border.right;
		fixed = true;
	}

	if (y < border.top)
	{
		y = border.top;
		fixed = true;
	}
	else if (y > border.bottom)
	{
		y = border.bottom;
		fixed = true;
	};
	
	return fixed;
}

void CSprite::setSTATE(STATEtype newS)
{
	STATE = newS;
}

STATEtype CSprite::getSTATE()
{
	return STATE;
}

POINT CSprite::getAnchor()
{
	POINT tPT;

	// returns center point
	tPT.x = (x -1) + (width  * 0.5);
	tPT.y = (y -1) + (height * 0.5);

	return tPT;
}

void CSprite::doMove()
{
	x += movex;
	y += movey;
};

void CSprite::setAnimate(bool value)
{
	doANIM = value;	
};

void CSprite::setupAnim(int numFrames, DIRS newDir, int perRow)
{
	curframe = 0;
	lastframe = numFrames-1; // first frame should be (0)
	frames_per_anim = numFrames;
	facingDir = newDir;
	animdelay = 2;
	animcount = 0;
    columns = perRow; // sprite per row in sprite sheet
}

// constructor for new sprites ... curFrame will always be (1) .. scale and rotation not currently implemented
// and this is for non-animated sprites
CSprite::CSprite(int X, int Y, int Width, int Height, LPCSTR Filename)
{
	x = X;
	y = Y;
	width = Width;
	height = Height;

	// initialize these, but not currently in use since this is non-animated sprite
	curframe = 0;
	lastframe = 1;
	animdelay = 3;
	animcount = 0;
	movex = 0;
	movey = 0;

	columns = 1;
	frames_per_anim = 1;

	setSTATE(INACTIVE);

	doANIM= true;

	// default
	setFacing(EAST); // sets angle to 0 as well, EAST=1

	// save for later
	filename = Filename;

	// Load the sprite file
	LoadImage(Filename);		
}

// destructor -- the default one should release all memory allocated for this sprite
 CSprite::~CSprite(void)
{
	// release the sprite sheet
	_image->Release();		
} 


void CSprite::LoadImage(LPCSTR Filename)
{
    _image = LoadTexture(Filename, D3DCOLOR_XRGB(255,0,255));
    assert(_image);        
}

int CSprite::getAngle()
{ return angle; }

void CSprite::addAngle(int newA)
{
	setAngle(angle + newA);
}

void CSprite::setAngle(int newA)
{
	angle = newA;
	if ((angle >= 360) || (angle < 0))
		angle = 360 - angle;

	if ((angle >= 360) || (angle < 0))
		angle = 0;
}

POINT calcXY(int degree, float distance, POINT startPT)
{
	POINT endPT;

	// formula assumes objects xy is {0,0} on a graph
	endPT.x = (find_x(degree) * distance) + startPT.x;
	endPT.y = (find_y(degree) * distance) + startPT.y;

	return endPT;
};
double find_x(int degree)
{
	// convert degree to radian first
	return cos(degree * (PI / 180));
};
double find_y(int degree)
{
	// convert degree to radian first
	return sin(degree * (PI / 180));
};

POINT find_OrbitXY(int degree, float distance, POINT startPT)
{
	POINT endPT;

	// formula assumes objects xy is {0,0} on a graph
	endPT.x = (find_x(degree) * distance) + startPT.x;
	endPT.y = (find_y(degree) * distance) + startPT.y;

	return endPT;
};
