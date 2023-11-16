// CSprite class header

#ifndef CSPRITE_H
#define CSPRITE_H

//windows/directx headers
#include <d3d9.h>
#include <d3dx9.h>
#include <assert.h>

//framework-specific headers
//#include "dxinput.h"
#include "dxgraphics.h"

//macros to read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

// unit logic
// range (in pixels) to check for player as target
#define SIGHT_RANGE 500

typedef enum
{ // matches the order of sprites in sprite sheets now //
	EAST    = 1,
	NORTH	= 2,
	SOUTH	= 3,
	WEST	= 4,
	SW		= 5,
	NW		= 6,
	NE		= 7,
	SE		= 8
} DIRS;
//		case EAST:  setAngle(  0); break;
//		case SE:    setAngle( 45); break;
//		case SOUTH: setAngle( 90); break;
//		case SW:    setAngle(135); break;
//		case WEST:  setAngle(180); break;
//		case NW:    setAngle(225); break;
//		case NORTH: setAngle(270); break;
//		case NE:    setAngle(315); break;

// these are used to define the unit states
typedef enum
{
	INACTIVE	= 0,
	ACTIVE	= 1,		// moving
	ATTACKING	= 3,
	SPAWNING	= 4,
	FLEEING	= 6,
	SEARCHING	= 7,
	DYING     = 8,
	DEAD      = 9
} STATEtype;

#define PI 3.1415926535897932384626433832795


// GLOBALS //
double find_x(int degree);
double find_y(int degree);
POINT find_OrbitXY(int degree, float distance, POINT startPT);
POINT calcXY(int degree, float distance, POINT startPT);


// CUSTOM SPRITE CLASS //
class CSprite
{
public:	
	void setPT(POINT tPT);

	 int nextFrame();			 // increments frame and returns 1 if its reached the end
	 int getFrame();			 // returns current frame
	void setFrame(int newFrame);	 // sets current frame to new value

	void setLastFrame(int newLastFrame); // sets the last frame of animation sequence
	int getLastFrame();				  // returns current "last" frame

	void setDelay(int newDelay);		// sets the delay used for animations
 	 int getDelay();				// returns current value

	void setupAnim(int numFrames, DIRS newDir, int perRow);

	void setXY(int newX, int newY);
     int getX();
	 int getY();
	void setX(int newX);
	void setY(int newY);
	void addX(int addtoX);
	void addY(int addtoY);

	void setMoveXY(int moveX, int moveY);
	void setMoveX(int newX);
	void setMoveY(int newY);
	 int getMoveY();
	 int getMoveX();

	 int getHeight();
	 int getWidth();

	// holds the units sprite sheet
	LPDIRECT3DTEXTURE9 getImage();

	// returns a rect pointing to the sprites current frame 
	RECT getRect();

	// returns a collision rect, using GAMEWORLD coordinates
	RECT collisionRect(int ScrollX,int ScrollY); 
	RECT scaledRect(int ScrollX,int ScrollY, float scaleX, float scaleY);

	DIRS getFacing();					// returns current facing of sprite
	void setFacing(DIRS newFacing);		// sets the facing of sprite
	void setNewDIR(int newDIR);			// sets new facing and movexy at same time, using int

	bool isMoving();					// returns true if sprite is moving
	void setMoving(bool newState);		// sets if sprite is moving
	
	POINT getPT();	// returns current screen xy of sprite as POINT

	void setBorder(int left, int right, int top, int bottom);	// defines the screen walkable/collision border for this sprite
	bool inBorder();											// returns TRUE if sprite is within its defined border, FALSE otherwise
	bool checkBorder();											// checks and corrects xy inside border, returns true if fixed something
	
	void setSTATE(STATEtype newS);			// sets the sprites current STATE 	
	STATEtype getSTATE();							// returns the curren STATE of sprite
	
	POINT getAnchor();	// returns the anchor point, which is in center of the sprite collision rectangle

	 int getAngle();			// current angle used for rotating sprite
	void addAngle(int newA);	// adds to current angle, wrapping around at 360 to 0
	void setAngle(int newA);	// sets new angle


	void doMove();

	LPCSTR getFilename();

	// constructor for new sprites ... curFrame will always be (1) .. scale and rotation not currently implemented
	// and this is for non-animated sprites
	CSprite(int X, int Y, int Width, int Height, LPCSTR Filename);

	// destructor
	~CSprite(void); 

	void setAnimate(bool value);

private:
	int angle;

	STATEtype STATE;

	DIRS facingDir;       // direction sprite is facing, used for choosing correct sprite from sprite sheet
	DIRS lastDir;

	RECT border;

	bool moving;

    int x,y;
	int width,height;

	LPCSTR filename;

    int movex,movey;
    int curframe,lastframe;
    int animdelay,animcount;
	int columns;         // used to grabs sprites from sprite map
	int frames_per_anim; // using sprite sheet, will need this

	LPDIRECT3DTEXTURE9 _image;

	void LoadImage(LPCSTR Filename);

	bool doANIM;
}; // end of CSprite class

#endif
