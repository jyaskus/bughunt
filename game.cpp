// Beginning Game Programming, Second Edition
// DynamicScroll program

// for File I/O
//#include <iostream> 
//#include <fstream>

#include "game.h"

// #define LINUX_OS = true

#ifdef LINUX_OS
#include "SDL.h" 
#endif

/* THESE WILL BE THE IN-GAME OPTIONS */
// GAME DIFFICULTY - "limiter" = max # of badies to spawn on ya at once
int DIFF_LEVEL = 5; // AVG
// SCREEN_WIDTH
// SCREEN_HEIGHT	< 2000 :D
// FULLSCREEN	0/1
// MAX	 10
// AVG	  6
// NOVICE 3
// EASY    2

// counters for score
int MAX_SYTEM =0;
int MAX_PLANET =0;


int ScrollX=0, ScrollY=0;			//current scroll position
int SpeedX, SpeedY;					//scroll speed
LPDIRECT3DSURFACE9 scrollbuffer;	//scroll buffer
LPDIRECT3DSURFACE9 space_tiles,surface_tiles[NUM_MAPS_PLANET],system_tiles[NUM_MAPS_SYSTEM];			//source image(s) containing tiles
bool gamestart = true;

/* music class */
CWMp3* mp3 = CreateCWMp3();


long start;							//timing variable

// keep track of which state the game is in
GAME_STATE gameView = SPACE;

// TILE MAP Data
int MAPDATA[MAPWIDTH][MAPHEIGHT];
int SYSTEM_MAPDATA[MAPWIDTH][MAPHEIGHT];

// track players progress -- and mobs, etc --
bool clearedArea[NUM_MAPS_SYSTEM][NUM_MAPS_PLANET];

struct debrisType
{
	bool space[DEBRIS_AMT_SPACE];
	bool system[DEBRIS_AMT_SYSTEM];
	bool surface[DEBRIS_AMT_PLANET];
} hitDebris;

struct planetType
{
	bool space[IMMOBILE_AMT_SPACE];
	bool system[IMMOBILE_AMT_SYSTEM];
	bool surface[IMMOBILE_AMT_PLANET];
} hitPlanet[NUM_MAPS_PLANET];

float COLL_RADIUS = 0.40; // 80% of mass

// FLAG if player is at a dock-able object (planet,sun,etc)
// will be -1 if not
int DOCKED_AT = -1;

struct dType
{
	int planet;
	int system;
	int space;
};
dType docked = {-1,-1};
dType CURR_MAP = {0,0,0};

// SPRITE handler
LPD3DXSPRITE sprite_handler;
float scalex=1.0f,scaley=1.0f; // used for sprite scaling
float ScaleX=1.0f,ScaleY=1.0f; // used for sprite scaling

// static objects for the guns/ammo
CGuns		*Guns[MAX_GUNS];
CProjectile	*Ammo[MAX_AMMO];
int Alt_Weapon = 3; // missiles, 4 is torpedo

// FOR AI guns and such
CGuns		*Guns_AI[MAX_GUNS_AI];
CProjectile	*Ammo_AI[MAX_GUNS_AI];

// max # of projectiles to track
int cntBullets=0; // projectiles in motion
int bulletsDrawn=0;
int rCode=0;
int enemyBullets=0;
int bulletsHit=0;

// enemy
const int NUM_DRONE_SHIPS=3;
const int NUM_ENEMIES=10;
CShip *enemy[NUM_ENEMIES];
CShip *boss; // only one at a time
CSprite *beeks;
int deaths=0; // number of times boss has died , starts with frame #0 (grey)
const int lives = 8; // 8 colors/frames

// Players ship
CShip	*player;
CSprite *shield;

// keep track of enter/exit locations
// pt is xy location, number is object collides with last, dir is facing
struct LAST_LOC
{
	POINT pt;		// xy location
	int number;		// object # collided with
	DIRS direction; // direction you were facing
	POINT scroll;   // scrollx and scrolly at entrance xy
};

struct LAST_COLL
{
	int space;  // numbers reflect # of last object collided with in that view
	int system;
	int surface;
};
LAST_COLL lastColl = {0,0,0};
LAST_COLL currColl = {0,0,0};
LAST_COLL lastPlanetColl = {0,0,0};
LAST_COLL currPlanetColl = {0,0,0};

// used to save previous locations and entrance locations
LAST_LOC enterPLANET,enterSYSTEM,enterSPACE;

struct lastType
{
	LAST_LOC space;
	LAST_LOC system;
	LAST_LOC planet;
} prev;

// health bars
LPDIRECT3DSURFACE9 color_bars;
LPDIRECT3DSURFACE9 sun;
LPD3DXFONT dxfont;

CSprite *yellowBmp,*redBmp;

// random space debris
// Debris class to handle the moving debris
CDebris *debris_space[DEBRIS_AMT_SPACE];
CDebris *debris_system[DEBRIS_AMT_SYSTEM];
CDebris *debris_planet[DEBRIS_AMT_PLANET];

CPlanet *immobile_space[IMMOBILE_AMT_SPACE];
CPlanet *immobile_system[NUM_MAPS_SYSTEM][IMMOBILE_AMT_SYSTEM];
CPlanet *immobile_planet[NUM_MAPS_PLANET][IMMOBILE_AMT_PLANET];

// collision stuff
double vPx;
double vPy;
double vJx;
double vJy;
//  ---

const int MAX_EXPLOSIONS = 30;
CSprite *explosion[MAX_EXPLOSIONS];
bool activeEXP[MAX_EXPLOSIONS];

const int RIGHT = 1;
const int LEFT  = 0; 

// timer to delay key presses
bool KEY_PRESSED=false;

//-------------------------------------------------------//
//-----------------   GAME INIT -------------------------//
//initializes the game
int Game_Init(HWND hwnd)
{
	HRESULT result;

	Init_DirectInput(hwnd);
	Init_Keyboard(hwnd);


    //create sprite handler object
    result = D3DXCreateSprite(d3ddev, &sprite_handler);
    if (result != D3D_OK)
	 return 0;

	// set all explosions to false
	for (int i=0; i < MAX_EXPLOSIONS; i++)
		activeEXP[i] = false;

	// set all map data to (1) ... SPACE/PLANET (for now)
	for (int row=0; row < MAPHEIGHT;row++)
		for (int column=0; column < MAPWIDTH; column++)
			MAPDATA[column][row] = 1; // all will use sprite #1

	
	// system map data
	int cnt = 0;
	for (int row=0; row < MAPHEIGHT;row++)
		for (int column=0; column < MAPWIDTH; column++)
			SYSTEM_MAPDATA[column][row] = cnt++; // so it draws the bmp using tiles


	// load the font
	D3DXCreateFont( d3ddev, 14, 0, FW_NORMAL, 1, false, DEFAULT_CHARSET, 
					OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
					"Arial Black", &dxfont);

	// setup the guns and ammo for the ships
	Init_Guns();

	//load the tile images
	
	space_tiles = LoadSurface(TEXT("./Media/Images/space_00.png"), D3DCOLOR_XRGB(0,0,0));
	assert(space_tiles);

	//load the tile images	
	for (int p=0; p < NUM_MAPS_PLANET; p++)
	{
		char myText[100];
		sprintf(myText,"./Media/Images/planet_0%d.png",p);

		surface_tiles[p] = LoadSurface((LPCSTR) myText, D3DCOLOR_XRGB(0,0,0));
		assert(surface_tiles[p]);
	}

	//load the tile images
	for (int s=0; s < NUM_MAPS_SYSTEM; s++)
	{
		char myText[100];
		sprintf(myText,"./Media/Images/system_0%d.png",s);

		system_tiles[s] = LoadSurface((LPCSTR) myText, D3DCOLOR_XRGB(0,0,0));
		assert(system_tiles[s]);
	}

	//create the scroll buffer surface in memory, slightly bigger 
    //than the screen
	result = d3ddev->CreateOffscreenPlainSurface(
		SCROLLBUFFERWIDTH, SCROLLBUFFERHEIGHT, 
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
		&scrollbuffer, 
		NULL);

	// test bitmap
	// will draw under objects that are collided with ... as a test
	yellowBmp = new CSprite(0,0,120,100,TEXT("./Media/Images/test1.bmp"));	
	assert(yellowBmp);

	redBmp = new CSprite(0,0,120,100,TEXT("./Media/Images/test2.png"));	
	assert(redBmp);

    //player
	// now using GAME WORLD coordinates
	// starting ship  Max Speed 8, Solar Energy 20, Recharge Delay 50, BLASTERS, shields, armor, hull (hit points)
	player = new CShip(16,16,128,128,TEXT("./Media/Images/ship.png"),0,8,20,25,Guns[0],0,3,2,1);
	player->setupAnim(4,SOUTH,4); // sprites per anim, initially heading, sprites per row on sheet	
	player->setSTATE(ACTIVE);	
	player->setBorder(0,0,GAMEWORLDWIDTH - 64,GAMEWORLDHEIGHT - 64); // set border to game window
	assert(player);

	shield = new CSprite(50,50,128,128,TEXT("./Media/Images/shield.png"));
	shield->setupAnim(4,EAST,4);
	shield->setSTATE(ACTIVE);
	assert(shield);

	// spawn/create the Enemy SHIPs & BOSS
	createDrones();

	// randomly initializes the debris
	createDebris();

	// load the static objects (planets)
	loadMaps();



	// SCORE
	// SYSTEMS CLEARED
	int cntPl=0,cntSys=0, temp=0;
	for (int s=0; s < NUM_MAPS_SYSTEM; s++)
	{
		temp=cntPl;
		for (int p=0; p < NUM_MAPS_SYSTEM; p++)
			if (clearedArea[s][p] == false)
				cntPl++;

		if (temp != cntPl)
		{
			cntSys++;
			temp = cntPl;
		}
	}				

	MAX_SYTEM  =cntSys;
	MAX_PLANET =cntPl; // <- this is THE SCORE (max)

	// Status bars for players
	color_bars = LoadSurface(TEXT("./Media/Images/bars.bmp"), D3DCOLOR_XRGB(255,0,255));
	assert(color_bars);

	// setup the enter/exit objects
	// when entering a planet view, start at the bottom of map - looking north
	enterPLANET.pt.x = 1;//(GAMEWORLDWIDTH-(player->getWidth())/2);
	enterPLANET.pt.y = 1;//GAMEWORLDHEIGHT - player->getHeight();
	enterPLANET.direction = SE;//NORTH;
	enterPLANET.scroll.x = 0;//getScrollX(enterPLANET.pt.x);
	enterPLANET.scroll.y = 0;//getScrollY(enterPLANET.pt.y);

	enterSYSTEM.pt.x = 1;//(GAMEWORLDWIDTH-(player->getWidth())/2);
	enterSYSTEM.pt.y = 1;//GAMEWORLDHEIGHT - player->getHeight();
	enterSYSTEM.direction = SE;//NORTH;
	enterSYSTEM.scroll.x = 0;//getScrollX(enterSYSTEM.pt.x);
	enterSYSTEM.scroll.y = 0;//getScrollY(enterSYSTEM.pt.y);

	enterSPACE.pt.x = 150;//(GAMEWORLDWIDTH-(player->getWidth())/2);
	enterSPACE.pt.y = 150;//GAMEWORLDHEIGHT - player->getHeight();
	enterSPACE.direction = SE;//NORTH;
	enterSPACE.scroll.x = 0;//getScrollX(enterSPACE.pt.x);
	enterSPACE.scroll.y = 0;//getScrollY(enterSPACE.pt.y);

	start = GetTickCount();

	mp3->OpenFile(".\\Media\\Sound\\Music\\Plunder_the_Galaxy.mp3",800, 0, 0);
	mp3->Play();

	return 1;
}


// -------------------------------------------------- //
// ------------ GAME RUN ---------------------------- //

//the main game loop
void Game_Run(HWND hwnd)
{
    //make sure the Direct3D device is valid
    if (d3ddev == NULL)
        return;

	//poll DirectInput devices
	Poll_Keyboard();

    //check for escape key (to exit program)
    if (Key_Down(DIK_ESCAPE))
        PostMessage(hwnd, WM_DESTROY, 0, 0);

	if (gamestart)
	{
		mp3->Stop();
		gamestart = false;
	}

	int temp=0;

    //keep the game running at a steady frame rate
    if (GetTickCount() - start >= 30)
    {
        //reset timing
        start = GetTickCount();

		// check for player pressing a key
		Check_Keys();

		// set player ship scale based on view level
		switch(gameView)
		{
		case SPACE: 
			{
				scalex=0.5f;
				scaley=0.5f;
			}; 
			break;
		case SYSTEM: 
			{
				scalex=0.75f;
				scaley=0.75f;
			}; 
			break;
		case PLANET: 
			{
				scalex=1.0f;
				scaley=1.0f;
			}; 
			break;
		}

	// RECOVER
		// regen solar energy & shields & guns
		player->recharge();

		// recharge power on enemy ships, so they can continue shooting at you
		for (int i=0; i < NUM_ENEMIES; i++)
			if ((enemy[i]->getSTATE() != DEAD) && (enemy[i]->getSTATE() != INACTIVE))
				enemy[i]->recharge();

		if ( (boss->getSTATE() != DEAD) && (boss->getSTATE() != INACTIVE) )
			boss->recharge();

	// MOVE
       //move the sprite
        player->doMove();

		// Bullet movement
		moveBullets();

		// process Enemy Actions (FSM)
		// and MOVE enemy ships
		EnemyAI();
		BossAI();

		// move Debris in current Game View
		moveDebris();

	// CHECK MOVES
		// Player - check if left screen or not
		// make sure ship doesnt leave game coordinates
		if (player->checkBounds(0,0,GAMEWORLDWIDTH,GAMEWORLDHEIGHT))
			player->setSpeed(0);// border collision occured ... now what? player ship is stopped
		
		//update the scrolling view
		UpdateScrollPosition();

	//COLLISIONS
		// AI bullets vs Player
		bullet_AI_Collision(player);

		// player bullets vs AI
		for (int i=0;i < NUM_ENEMIES; i++)
			if ((enemy[i]->getSTATE() != DEAD) && (enemy[i]->getSTATE() != INACTIVE))
				bulletCollision(enemy[i]);

		// player bullets vs Boss
		if ( (boss->getSTATE() != DEAD) && (boss->getSTATE() != INACTIVE) )
			bulletCollision(boss);

		// check for player vs. debris collision ... and determine the results
		checkDebris();

		// if player collides with a planet - enter planet mode
		temp = planetCollision(player); // object # collided with (what about 0?!)

		// track current and last object collided with		
		if (gameView == SPACE)	
		{
			lastPlanetColl.space   = currPlanetColl.space; // resets to (0) if no collision 
			currPlanetColl.space   = temp; // resets to (0) if no collision 
		}
		if (gameView == SYSTEM)
		{
			lastPlanetColl.system = currPlanetColl.system;
			currPlanetColl.system  = temp;
		}
		if (gameView == PLANET)
		{
			lastPlanetColl.surface = currPlanetColl.surface;
			currPlanetColl.surface = temp; 
		}		

		// Will hold the current object player is docked at
		// DOCKED_AT = temp;
		DOCKED_AT = -1;

		// OK- if you are docked at a planet -- you can hit the + key to "zoom in"
		// check to make sure its a valid DOCK target
		switch(gameView)
		{
			case SPACE: 
				{
					// DOCKED_AT only holds the [i] last collision w/ object
					// lets get the frame # from the given sprite					
					// find the resulting frame # which represents which type of system it will be					

					// make sure its a valid #
					if (( temp < IMMOBILE_AMT_SPACE) && (temp >= 0))
					{
						// this should tell us the map # associated, (-1) if none
						if (immobile_space[temp]->getMap() > -1)
							DOCKED_AT = immobile_space[temp]->getMap();						
					}
				}; 
				break;
			case SYSTEM: 
				{
					// make sure its a valid #
					if (( temp < IMMOBILE_AMT_SYSTEM) && (temp >= 0))
					{
						if (immobile_system[docked.system][temp]->getMap() > -1)
							DOCKED_AT = immobile_system[docked.system][temp]->getMap();
					}
				}; 
				break;
			case PLANET: 
				{
				}; 
				break;
		};
		

		// check for debris leaving the screen
		respawnDebris();

	// ANIMATE
		// change shield color
		shield->setFrame(shield->getLastFrame() - player->getShields());

        //animate player sprite
//		player->nextFrame(); // all ships animate on doMove() - this includes enemies and player

		// animate explosions
		animExp();

		// bullets?
		

		//start rendering
		if (d3ddev->BeginScene())
		{			

	        //erase the entire background
		    d3ddev->ColorFill(backbuffer, NULL, D3DCOLOR_XRGB(0,0,0));

            //start sprite handler
            sprite_handler->Begin(D3DXSPRITE_ALPHABLEND);

			//draw tiles onto the scroll buffer	
			switch (gameView)
			{
				case SPACE: DrawBG(space_tiles); break;
				case SYSTEM:
					{
						int c = docked.system;
						if ((c > NUM_MAPS_SYSTEM) || (c < 0))
							c = 0;
						DrawBG(system_tiles[c]);
					}; break;
				case PLANET:
					{
						int c = docked.planet;
						if ((c > NUM_MAPS_PLANET) || (c < 0))
							c = 0;
						DrawBG(surface_tiles[c]);
					}; break;
			}

			// SCORE
			// SYSTEMS CLEARED
			int cntPl=0,cntSys=0, temp=0;
			for (int s=0; s < NUM_MAPS_SYSTEM; s++)
			{
				temp=cntPl;
				for (int p=0; p < NUM_MAPS_SYSTEM; p++)
					if (clearedArea[s][p] == false)
						cntPl++;

				if (temp != cntPl)
				{
					cntSys++;
					temp = cntPl;
				}
			}				

			// SCORE BOARD
			drawText("Shots",  50, 0);
			drawText("Enemy"  ,150, 0);
			drawText("Hits"   ,250, 0);			
			drawNumbers(cntBullets,   50,30);
			drawNumbers(enemyBullets,150,30);
			drawNumbers(bulletsHit,  250,30);

		    drawText("Unsafe  ", 650,15);
			drawText("Max  ",    800,40);
			drawText("Systems ", 750,0);			  
			drawText("Planets ", 850,0);			
			drawNumbers(cntSys,750,28);			
			drawNumbers(cntPl, 850,28);
			drawNumbers(MAX_SYTEM, 750,50);						  
			drawNumbers(MAX_PLANET,850,50);

//			drawText("X axis", 0,700);			  
//			drawText("Y axis",50,700);			
//			drawNumbers(player->getX(),0,740);			
//			drawNumbers(player->getY(),50,740);			

//			drawText("Boss Stage",150,700);			
//			drawNumbers(deaths,150,740);			
//			drawNumbers(boss->getFrame(),200,740);

			// draw the STATIC objects (not moving; planets and such)
			drawPlanets();

			//draw the space debris sprites (clouds, asteroids, etc)
			drawDebris();

			// this should draw all the player's bullets that are active
			drawBullets();

			// draw the health bars for enemy / player ships
			drawBars();

			//draw shield under the player sprite 			
			DrawSprite( player->getX()-ScrollX,player->getY()-ScrollY, 
						shield->getWidth(),shield->getHeight(),
						scalex, scaley, player->getAngle(), 
						D3DCOLOR_XRGB(255,255,255), 
						shield->getImage(), 
						&shield->getRect(), 
						d3ddev, true);

			//draw the player sprite 
			DrawSprite( player->getX()-ScrollX,player->getY()-ScrollY, 
						player->getWidth(),player->getHeight(),
						scalex, scaley, player->getAngle(), 
						D3DCOLOR_XRGB(255,255,255), 
						player->getImage(), 
						&player->getRect(), 
						d3ddev, true);

			
			for (int i=0;i < NUM_ENEMIES; i++)
				if ((enemy[i]->getSTATE() != DEAD) && (enemy[i]->getSTATE() != INACTIVE))
					if (enemy[i]->getHP() > 0)// check for ZOMBIES :P
					{
						DrawSprite( enemy[i]->getX()-ScrollX,enemy[i]->getY()-ScrollY, 
									enemy[i]->getWidth(),enemy[i]->getHeight(),
									scalex, scaley, enemy[i]->getAngle(), 
									D3DCOLOR_XRGB(255,255,255), 
									enemy[i]->getImage(), 
									&enemy[i]->getRect(), 
									d3ddev, true);				
					}
					else
						enemy[i]->setSTATE(DEAD);	

			if ((boss->getSTATE() != DEAD) && (boss->getSTATE() != INACTIVE))
				if (boss->getHP() > 0)// check for ZOMBIES :P
				{
					DrawSprite( boss->getX()-ScrollX,boss->getY()-ScrollY, 
								boss->getWidth(),boss->getHeight(),
								scalex, scaley, boss->getAngle(), 
								D3DCOLOR_XRGB(255,255,255), 
								boss->getImage(), 
								&boss->getRect(), 
								d3ddev, true);	

					// Draw Beeks over-top
					DrawSprite( boss->getX()-ScrollX,boss->getY()-ScrollY, 
								beeks->getWidth(),beeks->getHeight(),
								scalex, scaley, boss->getAngle(), 
								D3DCOLOR_XRGB(255,255,255), 
								beeks->getImage(), 
								&beeks->getRect(), 
								d3ddev, true);				
				}
				else
					boss->setSTATE(DEAD);	

			// draw the Explosions (if any)
			drawExp();


// DEBUG --
			/*
			//info on the enemy sprites
			for (int i=0;i < NUM_ENEMIES; i++)
			{

				int x = (i*50)+60;
				int y = SCREEN_HEIGHT - 30;
				switch( enemy[i]->getSTATE() )
				{
					case FLEEING  : drawText(TEXT("Flee"),x,y); break;
					case ACTIVE   : drawText(TEXT("Actv"),x,y); break;
					case SEARCHING: drawText(TEXT("Srch"),x,y); break;
					case ATTACKING: drawText(TEXT("Attk"),x,y); break;					
					case DEAD     : drawText(TEXT("Dead"),x,y); break;
					case DYING    : drawText(TEXT("Dyng"),x,y); break;
					case INACTIVE : drawText(TEXT("Inact"),x,y); break;
				};
				// what is the guns range
				int gunRange = Ammo[enemy[i]->getAmmo()]->getRange() * scalex;

				y -= 25;if(i==0) drawText("Rng",0,y);		drawNumbers(gunRange,x,y);
				y -= 25;if(i==0) drawText("Pwr",0,y);		drawNumbers(enemy[i]->getSolar(),x,y);
				y -= 25;if(i==0) drawText("Dly",0,y);		drawNumbers(enemy[i]->getGunDelay(),x,y);
				y -= 25;if(i==0) drawText("Rld",0,y);		drawNumbers(enemy[i]->weapon->getReload(),x,y);
				y -= 25;if(i==0) drawText("nrg",0,y);		drawNumbers(enemy[i]->weapon->getEnergy(),x,y);
				y -= 25;if(i==0) drawText("HP",0,y);		drawNumbers(enemy[i]->getHP(),x,y);
				y -= 25;if(i==0) drawText("Max",0,y);		drawNumbers(enemy[i]->getMaxHP(),x,y);
			}
			*/
// -- DEBUG

			//stop drawing
            sprite_handler->End();
   
			//stop rendering
			d3ddev->EndScene();
		}
	}

    //display the back buffer on the screen
    d3ddev->Present(NULL, NULL, NULL, NULL);
}

// -------------------------------------- //
// ---------- GAME END ------------------ //
//frees memory and cleans up before the game ends
void Game_End(HWND hwnd)
{
	Kill_Keyboard();
	Kill_Mouse();
	dinput->Release();

    if (sprite_handler != NULL)
        sprite_handler->Release();
}

// -------------------------------------- //
// ------------ Functions --------------- //


void drawText(LPCSTR outText, int screenX, int screenY)
{
	RECT textbox;
	textbox.left   = screenX;
	textbox.top    = screenY;
	textbox.right  = screenX + (strlen((LPCSTR) &outText)*32);
	textbox.bottom = screenY + 32; // 90x30 box ok?

	dxfont->DrawTextA(  NULL,
						(LPCSTR)outText,
						strlen((LPCSTR) outText),
						&textbox,
						DT_RIGHT,
						D3DCOLOR_ARGB(255, 120, 255, 120)
						);
};

// used for debug purposes, writing numbers to screen
// draw numbers to screen
void drawNumbers(int number, int screenX, int screenY)
{
	static char outText[10];
	_itoa_s(number, outText, 10);

	RECT textbox;
	textbox.left   = screenX;
	textbox.top    = screenY;
	textbox.right  = screenX + 90;
	textbox.bottom = screenY + 30; // 90x30 box ok?

	dxfont->DrawTextA(  NULL,
						(LPCSTR)&outText,
						strlen((LPCSTR) &outText),
						&textbox,
						DT_RIGHT,
						D3DCOLOR_ARGB(255, 120, 255, 120)
						);
};


// function to scale, rotate and draw a sprite - all in one!!
// draws sprite in the position (x,y) with scaling factor (sx,sy), 
// with rotation of angle-degrees and with trasparency of "alpha" (0xFF = fully opaque). 
// You can also specify a subrect to draw (rc).
void DrawSprite(float x, float y, int width, int height, float sx, float sy, float angle, unsigned char alpha, LPDIRECT3DTEXTURE9 sprite, RECT *rc, IDirect3DDevice9* pd3dDevice, bool FixAngle)
{
	D3DSURFACE_DESC desc;
	sprite->GetLevelDesc(0, &desc);

	// angles are messed up now, got to add 90 before drawing
	int tAngle = angle;
	if (FixAngle)
	{
		tAngle += 90;
		if (tAngle < 0)
			tAngle = 360 + tAngle;
		if (tAngle >= 360)
			tAngle = tAngle - 360;
	}

	const int halfWidth = width/2; //desc.Width >> 1;
	const int halfHeight = height/2; //desc.Height >> 1;

	D3DXMATRIX rotate, trasl, scale, result;

	D3DXMatrixIdentity(&result);
	D3DXMatrixTranslation(&trasl, -halfWidth, -halfHeight, 0);
	D3DXMatrixMultiply(&result, &result, &trasl);
	D3DXMatrixScaling(&scale, sx, sy, 1.0f);
	D3DXMatrixMultiply(&result, &result, &scale);
	D3DXMatrixRotationZ(&rotate, D3DXToRadian(tAngle));
	D3DXMatrixMultiply(&result, &result, &rotate);
	D3DXMatrixTranslation(&trasl, +halfWidth, +halfHeight, 0);
	D3DXMatrixMultiply(&result, &result, &trasl);
	D3DXMatrixTranslation(&trasl, x, y, 0);
	D3DXMatrixMultiply(&result, &result, &trasl);

	sprite_handler->SetTransform(&result);
	sprite_handler->Draw(sprite, rc, NULL, NULL, (0xFFFFFF) | (alpha << 24));
}

// checks to see if player is trying to move with keys
void Check_Keys()
{
	if (KEY_DOWN(VK_DOWN))
	{
		if (KEY_PRESSED)
			return;
		// slowing down
		player->addSpeed(-2);
		KEY_PRESSED=true;		
	}
	else if (KEY_DOWN(VK_UP))
	{
		if (KEY_PRESSED)
			return;
		// increment current speed
		player->addSpeed(1);
		KEY_PRESSED=true;
		player->useSolar(1); // burst uses some energy
	}
	else if (KEY_DOWN(VK_RIGHT))	
	{
		if (KEY_PRESSED)
			return;

		player->turn(RIGHT);

		KEY_PRESSED=true;
	}	
	else if (KEY_DOWN(VK_LEFT))
	{
		if (KEY_PRESSED)
			return;

		player->turn(LEFT); // left

		KEY_PRESSED=true;
	}
	else if (KEY_DOWN(VK_ADD) || KEY_DOWN(VK_OEM_PLUS))
	{ // zoom in
		if (KEY_PRESSED == true)
			return;

		switch (gameView)
		{
		case SPACE: 
			{ // space -> system
				if (DOCKED_AT > -1)				
				{
					// check if any enemies in this system 
					// if all planets cleared, then its safe
					bool systemSafe=true;
					int pcnt=0;
					for (int p=0; p < NUM_MAPS_PLANET; p++)
						if ( clearedArea[DOCKED_AT][p] == false)
						{
							pcnt++;
							systemSafe=false;
						}
					// respawn aliens (1 per planet)
					// pcnt = planets w enemies
					// can have up to 10 planets
					// max of 10 enemies
					if (! systemSafe)
						respawnDrones(pcnt);
					else
						{
							boss->setSTATE(INACTIVE);
							deaths=0;
						}

					// keep track of what system # you are entering
					docked.system = DOCKED_AT;
					CURR_MAP.system = DOCKED_AT;

					// save current location
					prev.space.direction = player->getFacing();
					prev.space.pt.x = player->getX();
					prev.space.pt.y = player->getY();

					// enter system
					gameView=SYSTEM;
					
					// set speed,location,etc ...
					SpeedX=0; 
					SpeedY=0;

					player->setSpeed(0);			
					player->setPT(enterSYSTEM.pt);
					player->setFacing(enterSYSTEM.direction);
					ScrollX = enterSYSTEM.scroll.x;
					ScrollY = enterSYSTEM.scroll.y;

					mp3->OpenFile(".\\Media\\Sound\\teleport_activate.mp3",800, 0, 0);
					mp3->Play();
				}
			
			};
			break;
		case SYSTEM:
			{ // SYSTEM -> PLANET
				if (DOCKED_AT > -1)
				{
					// check if any enemies in this system 
					// if all planets cleared, then its safe
					if ( clearedArea[CURR_MAP.system][DOCKED_AT] == false)
						{
							// planet NOT Safe
							// max of 10 enemies
							respawnDrones(8);
							deaths=0;
							respawnBoss();
						}
					else
						{
							boss->setSTATE(INACTIVE);
							deaths=0;
						}

					// keep track of what system # you are entering
					docked.planet = DOCKED_AT;
					CURR_MAP.planet = DOCKED_AT;

					// save current location
					prev.system.direction = player->getFacing();
					prev.system.pt.x = player->getX();
					prev.system.pt.y = player->getY();

					// enter planet
					gameView=PLANET;						
					SpeedX=0; 
					SpeedY=0;
					player->setSpeed(0);

					// move to starting location
					player->setPT(enterPLANET.pt);
					player->setFacing(enterPLANET.direction);
					ScrollX = enterPLANET.scroll.x; // map scroll values for given xy starting location
					ScrollY = enterPLANET.scroll.y;

					mp3->OpenFile(".\\Media\\Sound\\teleport_activate.mp3",800, 0, 0);
					mp3->Play();
				}
			};	break;
		 case PLANET: break; // do nothing, its already PLANET
		}
		KEY_PRESSED=true;
	}
	else if (KEY_DOWN(VK_SUBTRACT) || KEY_DOWN(VK_OEM_MINUS) || (player->getSTATE() == DEAD) )
	{
		if (KEY_PRESSED == true)
			return;

		// --- NOTE --- //
		// the "Escape" button :D
		// the "-" key  ///
		// heals u back up

	//	LOC_HULL,
	//	LOC_ARMOR,
	//	LOC_SHIELD
		switch (gameView)
		{
			case SPACE:
				{
					// this is the "panic" button 
					// so if you FLY off the map or some crazy thing -- 
					// hit "-" and .. teleports u back					
					POINT dest = { 105, 105 }; // GAMEWORLDWIDTH HEIGHT
					player->setPT(dest);
					ScrollX=0;ScrollY=0;

					player->setupShip(player->getMax(LOC_SHIELD),player->getMax(LOC_ARMOR),player->getMax(LOC_HULL));
					player->setSTATE(ACTIVE);
					player->useSolar(20);

					mp3->Stop();
					mp3->OpenFile(".\\Media\\Sound\\teleport_activate.mp3",800, 0, 0);
					mp3->Play();
				}; break;

			case SYSTEM:
				{	// SYSTEM -> SPACE view					
					gameView=SPACE;	

					// un-spawn the drones
					respawnDrones(0); // sets them all to inactive
					boss->setSTATE(INACTIVE);
					deaths=0; // reset boss

					// move to starting location
					player->setPT(prev.space.pt);
					player->setFacing(prev.space.direction);
					ScrollX = getScrollX(prev.space.pt.x); 
					ScrollY = getScrollY(prev.space.pt.y);
					SpeedX= 0; 
					SpeedY=0;
					player->setSpeed(0);
					KEY_PRESSED=true;

					player->setupShip(player->getMax(LOC_SHIELD),player->getMax(LOC_ARMOR),player->getMax(LOC_HULL));
					player->setSTATE(ACTIVE);

					mp3->Stop();
					mp3->OpenFile(".\\Media\\Sound\\teleport_activate.mp3",800, 0, 0);
					mp3->Play();

				};break;
			case PLANET:
				{   // PLANET -> SYSTEM view
					gameView=SYSTEM;   

					// check if any enemies in this system 
					// if all planets cleared, then its safe
					int pcnt=0; // number of unsafe planets
					for (int p=0; p < NUM_MAPS_PLANET; p++)
						if ( clearedArea[DOCKED_AT][p] == false)
							pcnt++;
					if (pcnt > DIFF_LEVEL)
						pcnt = DIFF_LEVEL; 

					// respawn aliens (1 per planet)
					respawnDrones(pcnt); // NONE if all planets safe
					boss->setSTATE(INACTIVE);
					deaths=0; // reset boss

					player->setupShip(player->getMax(LOC_SHIELD),player->getMax(LOC_ARMOR),player->getMax(LOC_HULL));
					player->setSTATE(ACTIVE);

					// move to starting location
					player->setPT(prev.system.pt);
					player->setFacing(prev.system.direction);
					ScrollX = getScrollX(prev.system.pt.x); 
					ScrollY = getScrollY(prev.system.pt.y);
					SpeedX= 0; 
					SpeedY=0;
					player->setSpeed(0);
					KEY_PRESSED=true;

					mp3->Stop();
					mp3->OpenFile(".\\Media\\Sound\\teleport_activate.mp3",800, 0, 0);
					mp3->Play();
				};break; 
		}
	}
	else if (KEY_DOWN(VK_SPACE) || KEY_DOWN(VK_LBUTTON))
	{
		if (KEY_PRESSED == true)
			return;

		// fire the weapon
		if ( player->canFire() )
		{			
			LPCSTR filename;
			// ok got enough power to fire the gun
			switch(player->getGun())
			{			
				case  4: filename=".\\Media\\Sound\\plasma.mp3";  break;// torpedo
				case  3: filename=".\\Media\\Sound\\missile.mp3"; break;// missile
				case  2: filename=".\\Media\\Sound\\cannon.mp3";  break;// cannon
				case  1: filename=".\\Media\\Sound\\Laser.mp3";   break;// laser
				default: filename=".\\Media\\Sound\\blaster.mp3"; break;// blaster
			}	
			mp3->Stop();
			mp3->OpenFile(filename,800, 0, 0);
			mp3->Play();

			// start bullets out front at a safe distance 
			rCode = Ammo[player->getAmmo()]->spawnBullet(player->getAnchor(),player->getFacing());
			if (rCode > -1)
				cntBullets++;
		}
		else
		{
			// SOUND
			// make a sound that tells player it couldnt fire because you are out of power!!!
		}	

		KEY_PRESSED=true;
	}
	else if ( KEY_DOWN(VK_RBUTTON) || KEY_DOWN(VK_LCONTROL) || KEY_DOWN(VK_RCONTROL) )
	{
		if (KEY_PRESSED == true)
			return;

		int current = player->getGun(); //
		
		// left/right control
		// fires missile(s)
		if ( player->isLoaded() )
		{
			LPCSTR filename;
			// ok got enough power to fire the gun
			switch(Alt_Weapon)
			{
				case  4: filename=".\\Media\\Sound\\plasma.mp3";  break;// torpedo
				default: filename=".\\Media\\Sound\\missile.mp3"; break;// blaster
			}	
			mp3->Stop();
			mp3->OpenFile(filename,800, 0, 0);
			mp3->Play();


			// start bullets out front at a safe distance 
			POINT destPT = player->getAnchor();//, player->getWidth(),player->getHeight(),player->getFacing());
			rCode= Ammo[Alt_Weapon]->spawnBullet(destPT,player->getFacing()); // missiles=3, torpedoes=4
			if (rCode > -1)
				cntBullets++;

			KEY_PRESSED=true;
		}
	}
	else if (KEY_DOWN('1')) // 1 key
	{
		if (KEY_PRESSED == true)
			return;

		mp3->Stop();
		mp3->OpenFile(".\\Media\\Sound\\gun_change01.mp3",400, 0, 0);
		mp3->Play();
			
		player->setGun(Guns[0],0);
		KEY_PRESSED=true;
	}
	else if (KEY_DOWN('2')) // 2 key
	{
		if (KEY_PRESSED == true)
			return;

		mp3->Stop();
		mp3->OpenFile(".\\Media\\Sound\\gun_change02.mp3",400, 0, 0);
		mp3->Play();
		player->setGun(Guns[1],1);
		KEY_PRESSED=true;
	}
	else if (KEY_DOWN('3')) // 3 key
	{
		if (KEY_PRESSED == true)
			return;
		mp3->OpenFile(".\\Media\\Sound\\gun_change03.mp3",800, 0, 0);
		mp3->Play();
		player->setGun(Guns[2],2);
		KEY_PRESSED=true;
	}
	else if (KEY_DOWN('4')) // 4 key
	{
		if (KEY_PRESSED == true)
			return;
		mp3->OpenFile(".\\Media\\Sound\\gun_change04.mp3",800, 0, 0);
		mp3->Play();
		Alt_Weapon = 3;
		KEY_PRESSED=true;
	}
	else if (KEY_DOWN('5')) // 5 key
	{
		if (KEY_PRESSED == true)
			return;
		mp3->OpenFile(".\\Media\\Sound\\gun_change04.mp3",800, 0, 0);
		mp3->Play();
		Alt_Weapon = 4;
		KEY_PRESSED=true;
	}
	else
		KEY_PRESSED=false;
		// if none of the are pressed, we can press keys again
}

bool canScroll(DIRS direction,int speed)
{
	//ScrollX and ScrollY
	int testX=ScrollX;
	int testY=ScrollY;

	switch(direction)
	{
		case NORTH:  testY-=speed;		break;
		case SOUTH:  testY+=speed;		break;
		case EAST:   testX+=speed;		break;
		case WEST:   testX-=speed;		break;
		case NE:     testX+=speed;testY-=speed; break;
		case NW:     testX-=speed;testY-=speed; break;
		case SE:	 testX+=speed;testY+=speed; break;
		case SW:	 testX-=speed;testY+=speed; break;
	}

	return ( min_max(testX,0,GAMEWORLDWIDTH - SCREEN_WIDTH) && min_max(testY,0,GAMEWORLDHEIGHT - SCREEN_HEIGHT) );
}

//This function updates the scrolling position and speed
void UpdateScrollPosition()
{
	bool doScrollLeft  =  ((player->getX() - ScrollX) <= (SCREEN_WIDTH  * 0.25) && ((player->getFacing() == WEST)  || (player->getFacing() == NW) || (player->getFacing() == SW)) ); // WEST	
	bool doScrollRight =  ((player->getX() - ScrollX) >= (SCREEN_WIDTH  * 0.75) && ((player->getFacing() == EAST)  || (player->getFacing() == NE) || (player->getFacing() == SE)) ); // EAST
	bool doScrollUp    =  ((player->getY() - ScrollY) <= (SCREEN_HEIGHT * 0.25) && ((player->getFacing() == NORTH) || (player->getFacing() == NW) || (player->getFacing() == NE)) );   // NORTH
	bool doScrollDown  =  ((player->getY() - ScrollY) >= (SCREEN_HEIGHT * 0.75) && ((player->getFacing() == SOUTH) || (player->getFacing() == SE) || (player->getFacing() == SW)) );   // SOUTH

	if ((doScrollLeft || doScrollRight || doScrollUp || doScrollDown) == false)
		return;

	// exit if cant scroll
	if (! canScroll(player->getFacing(),player->getSpeed()) )
		return;

//	POINT   tPT = { ScrollX, ScrollY };
//	POINT newPT = calcXY(player->getAngle(),player->getSpeed(),tPT);

//	SpeedX = newPT.x - ScrollX; // east (10-8 -> +2 pix) west (6,8=-2)
//	SpeedY = newPT.y - ScrollY; // north (6-8 = -2) south (8-6) = +2

	SpeedX = 0; SpeedY=0;
	switch(player->getFacing())
	{		// based on direction we are heading, scroll the background - if possible
			case NORTH: SpeedY -= player->getSpeed(); break;
			case SOUTH: SpeedY += player->getSpeed(); break;
			case EAST:  SpeedX += player->getSpeed(); break;
			case WEST:  SpeedX -= player->getSpeed(); break;
			case NE:    SpeedX += player->getSpeed(); 
						SpeedY -= player->getSpeed(); break;
			case NW:    SpeedX -= player->getSpeed(); 
						SpeedY -= player->getSpeed(); break;
			case SE:	SpeedX += player->getSpeed(); 
						SpeedY += player->getSpeed(); break;
			case SW:	SpeedX -= player->getSpeed(); 
						SpeedY += player->getSpeed(); break;
	}

	// N/S movement
	if ((ScrollY + SpeedY) < 0)
	{
		ScrollY = 0;
		SpeedY  = 0;
	}
	else if ((ScrollY + SpeedY) > (GAMEWORLDHEIGHT - SCREEN_HEIGHT))
	{
		ScrollY = GAMEWORLDHEIGHT - SCREEN_HEIGHT;
		SpeedY = 0;
	}
	else
		ScrollY += SpeedY;		// OK to scroll Y

	// E/W movement
	if ((ScrollX + SpeedX) < 0)
	{
		ScrollX = 0;
		SpeedX  = 0;
	}
	else if ( (ScrollX + SpeedX) > (GAMEWORLDWIDTH - SCREEN_WIDTH))
	{
		ScrollX = GAMEWORLDWIDTH - SCREEN_WIDTH;
		SpeedX = 0;
	}
	else
		ScrollX += SpeedX;

	// now see what direction we are facing
	if ( (SpeedX < 0) && (SpeedY < 0) ) // NW
		player->setFacing(NW);
	if ((SpeedX > 0) && (SpeedY < 0)) // NE
		player->setFacing(NE);
	if ((SpeedX < 0) && (SpeedY > 0)) // SW
		player->setFacing(SW);
	if ((SpeedX > 0) && (SpeedY > 0)) // SE
		player->setFacing(SE);
	if ((SpeedX == 0) && (SpeedY > 0)) // SOUTH
		player->setFacing(SOUTH);
	if ((SpeedX == 0) && (SpeedY < 0)) // NORTH
		player->setFacing(NORTH);
	if ((SpeedX > 0) && (SpeedY == 0)) // EAST
		player->setFacing(EAST);
	if ((SpeedX < 0) && (SpeedY == 0)) // WEST
		player->setFacing(WEST);

}

//This function does the real work of drawing a single tile from the 
//source image onto the tile scroll buffer. Parameters provide much 
//flexibility.
void DrawTile(  //LPDIRECT3DSURFACE9 *source,	// source surface image
				int tilenum,				// tile #
				int width,					// tile width
				int height,					// tile height
				int columns,				// columns of tiles
				LPDIRECT3DSURFACE9 dest,	// destination surface
				int destx,					// destination x
				int desty)					// destination y
{    
    //create a RECT to describe the source image
    RECT r1;
    r1.left = (tilenum % columns) * width;
    r1.top = (tilenum / columns) * height;
    r1.right = r1.left + width;
    r1.bottom = r1.top + height;
    
    //set destination rect
	RECT r2 = {destx,desty,destx+width,desty+height};

    //draw the tile 
	switch (gameView)
	{
		case SPACE: d3ddev->StretchRect(space_tiles,  &r1, dest, &r2, D3DTEXF_NONE); break;
		case SYSTEM:d3ddev->StretchRect(system_tiles[0], &r1, dest, &r2, D3DTEXF_NONE); break;
		case PLANET:d3ddev->StretchRect(surface_tiles[0],&r1, dest, &r2, D3DTEXF_NONE); break;
	}
}

//This function fills the tilebuffer with tiles representing
//the current scroll display based on scrollx/scrolly.
void DrawTiles()
{ 
	switch (gameView)
	{
		case SPACE: DrawBG(space_tiles); break;
		case SYSTEM:DrawBG(system_tiles[0]); break;
		case PLANET:DrawBG(surface_tiles[0]); break;
	}
	return;
/*
    int tilex, tiley;
    int columns, rows;
    int x, y;
    int tilenum;

    //calculate starting tile position
    tilex = ScrollX / TILEWIDTH;
    tiley = ScrollY / TILEHEIGHT;
    
    //calculate the number of columns and rows
    columns = WINDOWWIDTH / TILEWIDTH;
    rows = WINDOWHEIGHT / TILEHEIGHT;
    
    //draw tiles onto the scroll buffer surface
    for (y=0; y<=rows; y++)
	{
        for (x=0; x<=columns; x++)
		{
			// default
			tilenum = MAPDATA[tilex + x][tiley + y];
			
			//retrieve the tile number from this position            
			if (gameView == SYSTEM)
				tilenum = SYSTEM_MAPDATA[tilex + x][tiley + y];

			DrawTile(tilenum,TILEWIDTH,TILEHEIGHT,16,scrollbuffer, x*TILEWIDTH,y*TILEHEIGHT);
		}
	}
*/
}

//This function draws the portion of the scroll buffer onto the back buffer
//according to the current "partial tile" scroll position.
void DrawScrollWindow()
{
    //calculate the partial sub-tile lines to draw using modulus
    int partialx = ScrollX % TILEWIDTH;
    int partialy = ScrollY % TILEHEIGHT;
    
    //set dimensions of the source image as a rectangle
	RECT r1 = {partialx,partialy,partialx+WINDOWWIDTH,partialy+WINDOWHEIGHT};
        
    //set the destination rectangle
    //This line draws the virtual scroll buffer to the screen exactly as is,
    //without scaling the image to fit the screen. If your screen does not
    //divide evenly with the tiles, then you may want to scale the scroll
    //buffer to fill the entire screen. It's better to use a resolution that
    //divides evenly with your tile size.

    //use this line for scaled display
	//RECT r2 = {0, 0, WINDOWWIDTH-1, WINDOWHEIGHT-1};  
    
    //use this line for non-scaled display
    RECT r2 = {0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1};

    //draw the "partial tile" scroll window onto the back buffer
    d3ddev->StretchRect(scrollbuffer, &r1, backbuffer, &r2, D3DTEXF_NONE);
}

// draw bar at location; using current, max # and color
void drawBar(int x, int y,int curr,int max, int color)
{
//		BARS
//		grey		0
//		blue		1
//		green		2
//		orange		3
//		blue		4
//		red			5
//		yellow		6

	int width  = 64;
	int height = 6;

	// 0 is grey bar
	// source
	RECT r1;
	r1.left   = 0;
	r1.right  = width;
	r1.top    = 0;
	r1.bottom = height;

	// dest
	RECT r2;
	r2.left = x;
	r2.top  = y;
	r2.right = x + (width*scalex);
	r2.bottom = y + (height*scaley);

	d3ddev->StretchRect(color_bars, &r1, backbuffer, &r2, D3DTEXF_NONE);

	// draw bar % percentage
	// change width to match %
	r1.left = (width * color);
	r1.right = r1.left + width;
	if (curr < max)
	{
		// only draw a portion of the rect if not equal
		r1.right = r1.left + (width * curr / max);
		r2.right = r2.left + (width * scalex * curr / max);
	}
	d3ddev->StretchRect(color_bars, &r1, backbuffer, &r2, D3DTEXF_NONE);
}

// returns t/f is given # is with the range 
bool min_max(int num, int min,int max)
{
	return ((num <= max) && (num >= min));
}


void drawPlanets()
{
	CPlanet *planet;

	// check for debris leaving the screen
	switch(gameView)
	{
		case SPACE:
			{
				for (int i=0; i < IMMOBILE_AMT_SPACE; i++)
				{
					planet = immobile_space[i];	
					POINT dest = planet->getPT();
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord


					// draw the sprite if it would appear on screen
					if ( (((dest.x + planet->getWidth()) > 0) || (dest.y + planet->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if  ( (hitPlanet[0].space[i] == true) && (planet->getMap() > -1) )							
						{
							DrawSprite( dest.x, dest.y,
										yellowBmp->getWidth(),yellowBmp->getHeight(),
										ScaleX, ScaleY, 
										planet->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										yellowBmp->getImage(), 
										NULL, //&yellowBmp->getRect(), 
										d3ddev, false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(planet->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									planet->getWidth(),planet->getHeight(),
									ScaleX, ScaleY, 
									planet->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									planet->getImage(), 
									&planet->getRect(), 
									d3ddev, false);
					}
				}
			};break;
		case SYSTEM:
			{
				for (int i=0; i < IMMOBILE_AMT_SYSTEM; i++)
				{
					planet = immobile_system[docked.system][i];	

					POINT dest = planet->getPT();
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord

					// draw the sprite if it would appear on screen
					if ( (((dest.x + planet->getWidth()) > 0) || (dest.y + planet->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if ((hitPlanet[docked.system].system[i] == true) && (planet->getMap() > -1))
						{
							DrawSprite( dest.x, dest.y,
										yellowBmp->getWidth(),yellowBmp->getHeight(),
										ScaleX, ScaleY, 
										planet->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										yellowBmp->getImage(), 
										NULL, //&yellowBmp->getRect(), 
										d3ddev,false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(planet->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									planet->getWidth(),planet->getHeight(),
									ScaleX, ScaleY, 
									planet->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									planet->getImage(), 
									&planet->getRect(), 
									d3ddev,false);
					}
				}
			};break;
		case PLANET:
			{
				for (int i=0; i < IMMOBILE_AMT_PLANET; i++)
				{
					planet = immobile_planet[docked.planet][i];	

					POINT dest = planet->getPT();
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord

					// draw the sprite if it would appear on screen
					if ( (((dest.x + planet->getWidth()) > 0) || (dest.y + planet->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if ((hitPlanet[docked.planet].surface[i] == true) && (planet->getMap() > -1))
						{
							DrawSprite( dest.x, dest.y,
										yellowBmp->getWidth(),yellowBmp->getHeight(),
										ScaleX, ScaleY, 
										planet->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										yellowBmp->getImage(), 
										NULL, //&yellowBmp->getRect(), 
										d3ddev, false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(planet->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									planet->getWidth(),planet->getHeight(),
									ScaleX, ScaleY, planet->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									planet->getImage(), 
									&planet->getRect(), 
									d3ddev, false);
					}
				};
			};break;
	} // end case ...
};
//draw the debris sprites
void drawDebris()
{
	CDebris *debris;

	// check for debris leaving the screen
	switch(gameView)
	{
		case SPACE:
			{
				for (int i=0; i < DEBRIS_AMT_SPACE; i++)
				{
					debris = debris_space[i];	
					POINT dest = {debris->getX(),debris->getY()};
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord

					// draw the sprite if it would appear on screen
					if ( (((dest.x + debris->getWidth()) > 0) || (dest.y + debris->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if (hitDebris.space[i] == true)
						{
							DrawSprite( dest.x, dest.y,
										redBmp->getWidth(),redBmp->getHeight(),
										scalex, scaley, 
										debris->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										redBmp->getImage(), 
										NULL,  //&debris->getRect(), 
										d3ddev,false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(debris->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									debris->getWidth(),debris->getHeight(),
									scalex, scaley, 
									debris->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									debris->getImage(), 
									&debris->getRect(), 
									d3ddev,false);
					}
				}
			};break;
		case SYSTEM:
			{
				for (int i=0; i < DEBRIS_AMT_SYSTEM; i++)
				{
					debris = debris_system[i];	

					POINT dest = {debris->getX(),debris->getY()};
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord

					// draw the sprite if it would appear on screen
					if ( (((dest.x + debris->getWidth()) > 0) || (dest.y + debris->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if (hitDebris.system[i] == true)
						{
							DrawSprite( dest.x, dest.y,
										redBmp->getWidth(),redBmp->getHeight(),
										scalex, scaley, 
										debris->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										redBmp->getImage(), 
										NULL,  //&debris->getRect(), 
										d3ddev,false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(debris->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									debris->getWidth(),debris->getHeight(),
									scalex, scaley, 
									debris->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									debris->getImage(), 
									&debris->getRect(), 
									d3ddev,false);
					}
				}
			};break;
		case PLANET:
			{
				for (int i=0; i < DEBRIS_AMT_PLANET; i++)
				{
					debris = debris_planet[i];	

					POINT dest = {debris->getX(),debris->getY()};
					dest.x -= ScrollX; // since they use game world coords
					dest.y -= ScrollY; // subtract the offset values to get screen coord

					// draw the sprite if it would appear on screen
					if ( (((dest.x + debris->getWidth()) > 0) || (dest.y + debris->getHeight() >= 0)) && (( dest.x < SCREEN_WIDTH) || (dest.y  < SCREEN_HEIGHT)) )
					{
						if (hitDebris.surface[i] == true)
						{
							DrawSprite( dest.x, dest.y,
										redBmp->getWidth(),redBmp->getHeight(),
										scalex, scaley, 
										debris->getAngle(), 
										D3DCOLOR_XRGB(255,255,255),
										yellowBmp->getImage(), 
										NULL,  //&debris->getRect(), 
										d3ddev,false);
//							drawNumbers(i,dest.x,dest.y-10);
//							drawNumbers(debris->getFrame(),dest.x+50,dest.y-10);
						}

						DrawSprite( dest.x, dest.y,
									debris->getWidth(),debris->getHeight(),
									scalex, scaley, debris->getAngle(), 
									D3DCOLOR_XRGB(255,255,255),
									debris->getImage(), 
									&debris->getRect(), 
									d3ddev,false);
					}
				};
			};break;
	} // end case ...
}


// determine which direction to turn to face target
DIRS getDirection(POINT sourcePT, POINT targetPT)
{
	int a = 0; // centered horizontally
	// left to right
	if (sourcePT.x < targetPT.x) // left
		a = -1;
	if (sourcePT.x > targetPT.x) // right
		a = 1;

	int b = 0; // centered vertically
	// top to bottom
	if (sourcePT.y < targetPT.y) // above
		b = -1;
	if (sourcePT.y > targetPT.y) // below
		b = 1;

	switch(a)
	{
		case -1:
			{
				if (b == -1)
					{ return NW; }
				else if (b == 1)
					{ return SW; }
				else
					return WEST;
			}; break;
		case 1:
			{
				if (b == -1)
					{ return NE; }
				else if (b == 1)
					{ return SE; }
				else
					return EAST;
			}; break;
		case 0:
			{
				if (b == -1)
					{ return NORTH; }
				else
					{ return SOUTH; }
			};break;
	}

	return EAST; // default catch all
}

// returns distance in pixels
float findDistance(POINT pt1,POINT pt2)
{
	float distance;
	float	dx = pt1.x - pt2.x; 
	float	dy = pt1.y - pt2.y;
	distance = sqrt(dx*dx + dy*dy);
	return distance;
}

// test for collisions
int SpriteCollision(CSprite *sprite1, CSprite *sprite2)
{
    RECT rect1;
    rect1.left   = sprite1->getX() + 1;
    rect1.top    = sprite1->getY() + 1;
    rect1.right  = sprite1->getX() + sprite1->getWidth()  -2;
    rect1.bottom = sprite1->getY() + sprite1->getHeight() -2;

    RECT rect2;
    rect2.left   = sprite2->getX() + 1;
    rect2.top    = sprite2->getY() + 1;
    rect2.right  = sprite2->getX() + sprite2->getWidth()  -2;
    rect2.bottom = sprite2->getY() + sprite2->getHeight() -2;

    RECT dest;
    return IntersectRect(&dest, &rect1, &rect2);
}

// test for collisions
bool rectCollision(RECT r1, RECT r2)
{

return ( r1.left   < r2.right  &&
		 r1.top    < r2.bottom &&
		 r1.right  > r2.left   &&		 
		 r1.bottom > r2.top    );
}

bool circleCollision(POINT a,int radius1, POINT b, int radius2)
{
	int dx = b.x - a.x;
	int dy = b.y - a.y;
	int radii = radius1 + radius2;
	//return ( ((b.x - a.x)^2 + (b.y - a.y)^2) < (radius1 + radius2) );
	return ( (dx*dx)+(dy*dy) < (radii * radii) );
};

int planetCollision(CSprite *target)
{
//	RECT r1,r2;

	CPlanet *planet;

	int i;

	// see if player has collided with any debris
	switch(gameView)
	{
		case SPACE:
		{
			for (i=0; i < IMMOBILE_AMT_SPACE; i++)
			{
				planet = immobile_space[i];

				POINT targetPT = target->getAnchor();
				POINT planetPT = planet->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = planet->getHeight();
				if (planet->getWidth() < planet->getHeight())
					temp = planet->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,planetPT,radiusD))
				{
					hitPlanet[0].space[i] = true;
					return i;	
				}
				else
					hitPlanet[0].space[i] = false;					
			}
		}; break;
		case SYSTEM:
		{
			for (i=0; i < IMMOBILE_AMT_SYSTEM; i++)
			{
				planet = immobile_system[docked.system][i];

				POINT targetPT = target->getAnchor();
				POINT planetPT = planet->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = planet->getHeight();
				if (planet->getWidth() < planet->getHeight())
					temp = planet->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,planetPT,radiusD))
				{
					hitPlanet[docked.system].system[i] = true;
					return i;	
				}
				else
					hitPlanet[docked.system].system[i] = false;
			}
		}; break;
		case PLANET:
		{
			for (i=0; i < IMMOBILE_AMT_PLANET; i++)
			{
				planet = immobile_planet[docked.planet][i];

				POINT targetPT = target->getAnchor();
				POINT planetPT = planet->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = planet->getHeight();
				if (planet->getWidth() < planet->getHeight())
					temp = planet->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,planetPT,radiusD))
				{
					hitPlanet[docked.planet].surface[i] = true;
					return i;	
				}
				else
					hitPlanet[docked.planet].surface[i] = false;
			}
		};break;
	}

	// if no collisions (return -1)
	return -1;
} // end planet collision testing

int debrisCollision( CSprite *target)
{
//	RECT r1,r2;

	CDebris *debris;

	int i;
	// see if player has collided with any debris
	switch(gameView)
	{
		case SPACE:
		{			
			for (i=0; i < DEBRIS_AMT_SPACE; i++)
			{
				debris = debris_space[i];

				POINT targetPT = target->getAnchor();
				POINT debrisPT = debris->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = debris->getHeight();
				if (debris->getWidth() < debris->getHeight())
					temp = debris->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,debrisPT,radiusD))
				{
					hitDebris.space[i] = true;
					return i;	
				}
				else
					hitDebris.space[i] = false;
			}
		}; break;
		case SYSTEM:
		{
			for (i=0; i < DEBRIS_AMT_SYSTEM; i++)
			{
				debris = debris_system[i];

				POINT targetPT = target->getAnchor();
				POINT debrisPT = debris->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = debris->getHeight();
				if (debris->getWidth() < debris->getHeight())
					temp = debris->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,debrisPT,radiusD))
				{
					hitDebris.system[i] = true;
					return i;	
				}
				else 
					hitDebris.system[i] = false;
			}
		}; break;
		case PLANET:
		{
			for (i=0; i < DEBRIS_AMT_PLANET; i++)
			{
				debris = debris_planet[i];

				POINT targetPT = target->getAnchor();
				POINT debrisPT = debris->getAnchor();

				int temp = target->getHeight();
				if (target->getWidth() < target->getHeight())
					temp = target->getWidth();
				int radiusT = temp * COLL_RADIUS * scalex; // should cover most of the sprite in its radius ... 96/2=48-4=44

				temp = debris->getHeight();
				if (debris->getWidth() < debris->getHeight())
					temp = debris->getWidth();				
				int radiusD = temp * COLL_RADIUS; // should cover most of the sprite in its radius ... 120/2=60-

				// use circle collision
				if (circleCollision(targetPT,radiusT,debrisPT,radiusD))
				{
					hitDebris.surface[i] = true;
					return i;	
				}
				else
					hitDebris.surface[i] = false;
			}
		};break;
	}

	// if no collisions
	return -1;
} // end debris collision testing


// checks if debris has left screen and respawns it randomly
// only used for the space view
void respawnDebris()
{
	// it checks for objects leaving the game map
	// and respawns them with new image
	// heading etc ... from one of the 4 map borders
	for (int i=0; i < DEBRIS_AMT_SPACE; i++)
		debris_space[i]->checkExit();
	for (int i=0; i < DEBRIS_AMT_SYSTEM; i++)
		debris_system[i]->checkExit();
	for (int i=0; i < DEBRIS_AMT_PLANET; i++)
		debris_planet[i]->checkExit();
}


// calculates new velocity and heading
void collision2Ds(double m1, double m2,
                 double x1, double y1, double x2, double y2) //,
//                 double& vx1, double& vy1, double& vx2, double& vy2)
{
// globals
//double vPx; vx1
//double vPy; vy1
//double vJx; vx2
//double vJy; vy2

       double  m21,dvx2,a,x21,y21,vx21,vy21,fy21,sign;

       m21=m2/m1;
       x21=x2-x1;
       y21=y2-y1;
       vx21=vJx-vPx;
       vy21=vJy-vPy;

//     *** return old velocities if balls are not approaching ***
       if ( (vx21*x21 + vy21*y21) >= 0) return;

       fy21=1.0E-12*fabs(y21);                            
       if ( fabs(x21)<fy21 ) {  
                   if (x21<0) { sign=-1; } else { sign=1;}  
                   x21=fy21*sign; 
        } 

//     ***  update velocities ***
       a=y21/x21;
       dvx2= -2*(vx21 +a*vy21)/((1+a*a)*(1+m21)) ;
       vJx=vJx+dvx2;
       vJy=vJy+a*dvx2;
       vPx=vPx-m21*dvx2;
       vPy=vPy-a*m21*dvx2;
}

// translates a given x location to get ScrollX value
int getScrollX(int x)
{
	int temp = 0;
	if (x < (SCREEN_WIDTH*0.75))
		return 0;

	temp = x - (SCREEN_WIDTH /2); // 1000 - 480 = 520 scrollx
	if ( temp > (GAMEWORLDWIDTH - SCREEN_WIDTH))
		temp = GAMEWORLDWIDTH - SCREEN_WIDTH;

	return temp;
};

// translates a given y location to get ScrollY value
int getScrollY(int y)
{
	int temp = 0;
	if (y < (SCREEN_HEIGHT*0.75))
		return 0;

	temp = y - (SCREEN_HEIGHT /2); // 1000 - 480 = 520 scrollx
	if ( temp > (GAMEWORLDHEIGHT - SCREEN_HEIGHT))
		temp = GAMEWORLDHEIGHT - SCREEN_HEIGHT;

	return temp;
};

void loadMaps()
{
	int cnt=0;
	char buffer[256];
	char line[255];	
	char filename[100];
	LPCSTR Filename;

	// setup cleared - by default all areas are cleared, until maps are loaded
	for (int s=0;s < NUM_MAPS_SYSTEM; s++)
		for (int p=0;p<NUM_MAPS_PLANET;p++)
			clearedArea[s][p] = true;

// ------------------------------------------------------------------------
	for (int map=0; map < NUM_MAPS_PLANET; map++)
	{
		sprintf(filename,"./Media/static_surface_0%d.txt",map);

		// load map data
		ifstream mapFile(filename);

		// read in data from map file
		assert(! mapFile.fail());

		// BITMAP SOURCE
		Filename = TEXT("./Media/Images/static_surface.png");

		cnt=0;
		while ((! mapFile.eof()) && (cnt < IMMOBILE_AMT_PLANET))
		{
			// read in the map data		
			mapFile.getline(line, sizeof(buffer), ','); // X
			int x = atoi(line);			
			mapFile.getline(line, sizeof(buffer), ','); // Y
			int y = atoi(line);			
			mapFile.getline(line, sizeof(buffer), ','); // FRAME #
			int frame = atoi(line);		
			mapFile.getline(line, sizeof(buffer), ','); // ANGLE
			int rotation = atoi(line);		
			mapFile.getline(line, sizeof(buffer), '\n'); // grab the end of line
			int Map = atoi(line);					// rotation speed (if any)

			immobile_planet[map][cnt] = new CPlanet(x,y,120,110,Filename,IMMOBILE_AMT_PLANET);		
			immobile_planet[map][cnt]->setFrame(frame-1);
			immobile_planet[map][cnt]->setFacing(NORTH);
			immobile_planet[map][cnt]->setMap(Map);
			immobile_planet[map][cnt]->setAngle(rotation);

			hitPlanet[map].surface[cnt] = false;			

			assert(immobile_planet[map][cnt]);		

			cnt++;
		}
		mapFile.close();
	}
// ------------------------------------------------------------------------
	ifstream spaceFile(TEXT("./Media/static_space_00.txt"));
	assert(! spaceFile.fail());

	Filename = TEXT("./Media/Images/static_space.png");

	cnt=0;
	while ((! spaceFile.eof()) && (cnt < IMMOBILE_AMT_SPACE))
	{
		// read in the map data		
		spaceFile.getline(line, sizeof(buffer), ','); // X
		int x = atoi(line);			
		spaceFile.getline(line, sizeof(buffer), ','); // Y
		int y = atoi(line);			
		spaceFile.getline(line, sizeof(buffer), ','); // FRAME #
		int frame = atoi(line);		
		spaceFile.getline(line, sizeof(buffer), ','); // ANGLE
		int rotation = atoi(line);		
		spaceFile.getline(line, sizeof(buffer), '\n'); // grab the end of line
        int map = atoi(line);					// rotation speed (if any)

		immobile_space[cnt] = new CPlanet(x,y,120,110,Filename,IMMOBILE_SPRITES_SPACE);		
		immobile_space[cnt]->setFrame(frame-1);
		immobile_space[cnt]->setAngle(rotation);
		immobile_space[cnt]->setMap(map);
		hitPlanet[0].space[cnt] = false;
		assert(immobile_space[cnt]);

		cnt++;
	}
	spaceFile.close();
// ------------------------------------------------------------------------
	for (int map=0; map < NUM_MAPS_SYSTEM; map++)
	{
		sprintf(filename,"./Media/static_system_0%d.txt",map);

		ifstream sysFile(filename);
		// read in data from map file
		assert(! sysFile.fail());

		// BITMAP SOURCE
		Filename = TEXT("./Media/Images/static_system.png");

		cnt=0;
		while ((! sysFile.eof()) && (cnt < IMMOBILE_AMT_SYSTEM))
		{
			// read in the map data		
			sysFile.getline(line, sizeof(buffer), ','); // X
			int x = atoi(line);			
			sysFile.getline(line, sizeof(buffer), ','); // Y
			int y = atoi(line);			
			sysFile.getline(line, sizeof(buffer), ','); // FRAME #
			int frame = atoi(line);		
			sysFile.getline(line, sizeof(buffer), ','); // ANGLE
			int rotation = atoi(line);		
			sysFile.getline(line, sizeof(buffer), '\n'); // grab the end of line
			int Map = atoi(line);					// rotation speed (if any)

			// Map > -1 , cnt = Planet # // indicate enemy occupied areas
			if (Map > -1)
				clearedArea[map][cnt] = false; // there will be enemies there now

			immobile_system[map][cnt] = new CPlanet(x,y,120,110,Filename,IMMOBILE_AMT_SYSTEM);		
			immobile_system[map][cnt]->setFrame(frame-1); // 0 is the first frame!
			immobile_system[map][cnt]->setAngle(rotation);		
			immobile_system[map][cnt]->setMap(Map);

			assert(immobile_system[map][cnt]);

			hitPlanet[map].system[cnt] = false; 
			cnt++;
		}
		sysFile.close();
	}
};

void moveBullets()
{
	for (int a=0; a < MAX_AMMO; a++)
		Ammo[a]->doMoves(scalex); // only moves the active ones
	for (int a=0; a < MAX_GUNS_AI; a++)
		Ammo_AI[a]->doMoves(scalex); // only moves the active ones
};

// draw the active bullet sprites
void drawBullets()
{
	int cnt = 0;

	// PC bullets
	for (int a=0; a < MAX_AMMO; a++)
		for (int i=0; i < MAX_BULLETS; i++)
			if (Ammo[a]->Bullets[i].isActive == true)
			{	//draw the bullet sprite 
				cnt++; // keep track of number of bullets drawn
				DrawSprite( Ammo[a]->Bullets[i].currPT.x-ScrollX,
							Ammo[a]->Bullets[i].currPT.y-ScrollY, 
							Ammo[a]->getWidth(), Ammo[a]->getHeight(),
							scalex, scaley, 
							Ammo[a]->Bullets[i].angle, 
							D3DCOLOR_XRGB(255,255,255), 
							Ammo[a]->getImage(), 
							& Ammo[a]->getRect(), 
							d3ddev, true);
			}
	// AI Bullets
	for (int a=0; a < MAX_GUNS_AI; a++)
		for (int i=0; i < MAX_BULLETS; i++)
			if (Ammo_AI[a]->Bullets[i].isActive == true)
			{	//draw the bullet sprite 
				cnt++; // keep track of number of bullets drawn
				DrawSprite( Ammo_AI[a]->Bullets[i].currPT.x-ScrollX,
							Ammo_AI[a]->Bullets[i].currPT.y-ScrollY, 
							Ammo_AI[a]->getWidth(), Ammo_AI[a]->getHeight(),
							scalex, scaley, 
							Ammo_AI[a]->Bullets[i].angle, 
							D3DCOLOR_XRGB(255,255,255), 
							Ammo_AI[a]->getImage(), 
							& Ammo_AI[a]->getRect(), 
							d3ddev, true);

			}
	bulletsDrawn = cnt;
};

// draw the active bullet sprites
void bulletCollision(CShip *target)
{
	int cnt = 0;
	bool hit = false;
	int hitCnt = 0;

	for (int a=0; a < MAX_AMMO; a++)
		for (int i=0; i < MAX_BULLETS; i++)
			if (Ammo[a]->Bullets[i].isActive == true)
				{
					POINT  pt1 = Ammo[a]->Bullets[i].currPT;
					int halfW = Ammo[a]->getWidth()  / 2;
					int halfH = Ammo[a]->getHeight() / 2;

					POINT center;
					center.x = pt1.x + halfW;
					center.y = pt1.y + halfH;
					int radius1 = halfW;

					// players location
					POINT pt2 = target->getAnchor();
					int radius2 = target->getWidth() * 0.4f;				

					if ( circleCollision(center,radius1,pt2,radius2) )
					{
						// this bullet has collided
						hitCnt++;
						Ammo[a]->Bullets[i].isActive = false;

						DAMAGE dType = Ammo[a]->getDmgType();
						int dmg	     = Ammo[a]->getDmg();

						if (target->applyDmg(dType,dmg))
						{
							// spawn new explosion
							newExp(target->getAnchor());

							// target is DEAD !!!
							target->setSTATE(DEAD);						

							// BOOM SOUND
						}
					}
				}
	bulletsHit += hitCnt; 
};

// checks AI bullets for collision - most likely only with player sprite
void bullet_AI_Collision(CShip *target)
{
	int cnt = 0;
	bool hit = false;
	int hitCnt = 0;

	for (int a=0; a < MAX_GUNS_AI; a++)
		for (int i=0; i < MAX_BULLETS; i++)
			if (Ammo_AI[a]->Bullets[i].isActive == true)
				{
					POINT  pt1 = Ammo_AI[a]->Bullets[i].currPT;
					int halfW = Ammo_AI[a]->getWidth()  / 2;
					int halfH = Ammo_AI[a]->getHeight() / 2;

					POINT center;
					center.x = pt1.x + halfW;
					center.y = pt1.y + halfH;
					int radius1 = halfW;

					// players location
					POINT pt2 = target->getAnchor();
					int radius2 = target->getWidth() * 0.4f;				

					if ( circleCollision(center,radius1,pt2,radius2) )
					{
						// this bullet has collided
						hitCnt++;
						Ammo_AI[a]->Bullets[i].isActive = false;

						DAMAGE dType = Ammo_AI[a]->getDmgType();
						int dmg	     = Ammo_AI[a]->getDmg();

						if (target->applyDmg(dType,dmg))
						{
							// spawn new explosion
							newExp(target->getAnchor());

							// target is DEAD !!!
							target->setSTATE(DEAD);						

							// BOOM SOUND
						}
					}
				}
	bulletsHit += hitCnt; 
};

void Init_Guns()
{
	// [5]
	Ammo[0] = new CProjectile(1,ENERGY,  14,RANGE_MED,  TEXT("./Media/Images/proj_blaster.png") ,30,30); // 24
	Ammo[1] = new CProjectile(2,ENERGY,  18,RANGE_LONG, TEXT("./Media/Images/proj_laser.bmp")   ,32,32); // 25
	Ammo[2] = new CProjectile(3,KINETIC, 12,RANGE_SHORT,TEXT("./Media/Images/cannon.png")       ,44,44); // 35
	Ammo[3] = new CProjectile(2,EXPLODE, 12,RANGE_LONG ,TEXT("./Media/Images/missile.png")      ,32,32); // 25
	Ammo[4] = new CProjectile(4,ENERGY,  10,RANGE_SHORT,TEXT("./Media/Images/proj_torpedo.bmp") ,32,32); // 25
	Ammo[3]->setupAnim(4,NORTH,4); // animated rocket frames

	for (int i=0;i < MAX_AMMO; i++)
		assert(Ammo[i]); // make sure all load successfully

	//                     reload cycl energy per shot
	Guns[0] = new CGuns(0, DELAY_SHORT,	1,TEXT("Blaster"));		 // blaster
	Guns[1] = new CGuns(1, DELAY_SHORT, 2,TEXT("Laser"));		 // laser
	Guns[2] = new CGuns(2, DELAY_MED,	3,TEXT("Cannon"));		 // cannon
	Guns[3] = new CGuns(3, DELAY_LONG,	5,TEXT("Missile"));		 // missile
	Guns[4] = new CGuns(4, DELAY_LONG,	5,TEXT("Torpedo"));		 // photon torpedo
	for (int i=0;i < MAX_GUNS; i++)
		assert(Guns[i]); // make sure all load successfully

	//const int MAX_AI_GUNS;
	Ammo_AI[0] = new CProjectile(1,KINETIC,  9,RANGE_SHORT,TEXT("./Media/Images/mantabullet.png")  ,32,32); // 25
	Ammo_AI[1] = new CProjectile(2,KINETIC, 12,RANGE_LONG ,TEXT("./Media/Images/mantabullet.png")  ,32,32); // 25
	Guns_AI[0] = new CGuns(1, DELAY_MED, 2,TEXT("manta bullet")); // AI blasters
	Guns_AI[1] = new CGuns(2, DELAY_MED, 2,TEXT("boss bullet"));  // AI blasters x 3
	for (int i=0;i < MAX_GUNS_AI; i++)
	{
		assert(Guns_AI[i]); // make sure all load successfully
		assert(Ammo_AI[i]); // make sure all load successfully
	}

}

// performs basic FSM and then moves enemy ship
void EnemyAI()
{
	int GUN_RANGE, distance;
	DIRS chaseDIR, fleeDIR;

	// Enemy AI //
	for (int i=0; i < NUM_ENEMIES; i++)
		if (enemy[i]->getSTATE() != INACTIVE) // ignore the enemy if its INACTIVE
		{
			GUN_RANGE = Ammo[ enemy[i]->getAmmo() ]->getRange() * scalex; // 75% range in SYSTEM, 50% range in SPACE

			distance  = findDistance(enemy[i]->getAnchor(),player->getAnchor());
			chaseDIR  = getDirection(player->getAnchor()  ,enemy[i]->getAnchor()); // used to CHASE
			fleeDIR   = getDirection(enemy[i]->getAnchor(),player->getAnchor());   // used to FLEE

			// always move away if super close 
			if (distance <= TOO_CLOSE)
			{
				enemy[i]->setSpeed(0);
				if (distance <= (TOO_CLOSE/2))
				{
					enemy[i]->setSTATE(FLEEING); // way too close, get away
					enemy[i]->addSpeed(2);
				}
			} 

			switch ( enemy[i]->getSTATE() )
			{
			case FLEEING:
				{
					// flee until at a safe distance away
					if (distance > (2*TOO_CLOSE))
					{ // no longer need to flee
						enemy[i]->setSTATE(ACTIVE);
					}
					else
					{	// FLEE						
						enemy[i]->addSpeed(1); // move towards MAX speed (caps at max)

						// face player
						enemy[i]->setFacing(fleeDIR);

						if ((rand() % 100) < 10) // 10% chance for random 45 degree turns
							enemy[i]->turn(rand() % 2);
					}

				}; break;
			case ACTIVE:		// Moving around normally
				{
					if ((rand() % 100) < 10) // 10% chance for random 45 degree turns
						enemy[i]->turn(rand() % 2);

					// random +/- speed
					enemy[i]->addSpeed(1); 
					// caps at MAX speed, so no harm in this

					// change state if target visible
					if (distance <= SIGHT_RANGE)
						enemy[i]->setSTATE(SEARCHING); // sees player on radar

					if (distance <= GUN_RANGE)
						enemy[i]->setSTATE(ATTACKING);
				}; break;
			case SEARCHING:
				{   // HEAD towards player - AI "Sees" the player on radar
					enemy[i]->addSpeed(1); // move towards MAX speed (caps at max)

					// face player
					enemy[i]->setFacing(chaseDIR);					

					// change state if target visible
					if (distance > SIGHT_RANGE)
						enemy[i]->setSTATE(ACTIVE); // sees player on radar
					if (distance <= GUN_RANGE)
						enemy[i]->setSTATE(ATTACKING);
				}; break;
			case ATTACKING:		// Attacking player
				{
					// make sure the target is still in range
					if (distance > GUN_RANGE)
					{
						// unable to fire ... move first
						enemy[i]->setSTATE(ACTIVE);

						// this should make him head towards player
						enemy[i]->setFacing(chaseDIR);
						enemy[i]->addSpeed(2);
					}
					else
					{   // close enough to fire
						// see if ship has enough power to shoot, returns true if able to fire
						// also checks if the gun is reloaded or not
						if ( enemy[i]->canFire())
						{	
							// try to match players speed
							//enemy[i]->setSpeed(player->getSpeed()); 
							enemy[i]->setSpeed(0);  // stop moving and fire gun

							// face player
							enemy[i]->setFacing(chaseDIR);
							
							Ammo_AI[0]->spawnBullet(enemy[i]->getAnchor(),enemy[i]->getFacing());						
							enemyBullets++;

							// SOUND :: enemies guns firing at your
						}
						else
						{
							int dir=rand()%2, times=(rand()%3+1);
							for (int i=0; i < times;i++) 
								enemy[i]->turn(dir);
							//45-135 degree change one direction and move							
							enemy[i]->addSpeed(4);
						}
					} // distance 
				}; break; // CASE ..
			} // end of SWITCH()

			// perform the actual MOVE for enemy
			enemy[i]->doMove(); 

			// check boundaries
			if ( enemy[i]->checkBounds(0,0,GAMEWORLDWIDTH,GAMEWORLDHEIGHT) )           // BOUNDARIES CHECK
			{
				// warp back to top left corner
				enemy[i]->setX(300 + (rand() % 200)); // spawn point
				enemy[i]->setY(100 + (rand() % 200));
				enemy[i]->setFacing(SOUTH);
				enemy[i]->setSpeed(1);
			}
		} // END ... Enemy AI code //

}; // end EnemyAI function //

// performs basic FSM and then moves enemy ship
void BossAI()
{
	// Thw ANIM will be beaks added on top --

	// died
	if (boss->getSTATE() == DEAD)
	{
		deaths++;
		if (deaths < lives)
		{ // respawn, next color
			boss->setSTATE(ACTIVE);
			// create new boss
			respawnBoss();

			boss->setFrame(deaths);
			return;
		}
		else		// did boss just die? if so, make sure the planet is now "safe"
		{
			if (gameView == PLANET)
				if ( clearedArea[CURR_MAP.system][CURR_MAP.planet] == false)
					 clearedArea[CURR_MAP.system][CURR_MAP.planet] = true;
			return; // either way, exit
		}
	}

	// not activated
	if (boss->getSTATE() == INACTIVE)
		return;

// Boss AI //
	int GUN_RANGE=RANGE_LONG, distance, SIGHT = RANGE_LONG;
	DIRS chaseDIR, fleeDIR;

	GUN_RANGE = Ammo[ boss->getAmmo() ]->getRange() * scalex; // 75% range in SYSTEM, 50% range in SPACE
	distance  = findDistance(boss->getAnchor(),player->getAnchor());
	chaseDIR  = getDirection(player->getAnchor()  ,boss->getAnchor()); // used to CHASE
	fleeDIR   = getDirection(boss->getAnchor(),player->getAnchor());   // used to FLEE

	// always move away if super close 
	if (distance <= 300)
	{
		boss->setSTATE(FLEEING); // way too close, get away
		boss->setSpeed(2);
	};
	if (distance <= 200)
	{
		boss->setSTATE(ATTACKING); // way too close, just attack
		boss->setSpeed(0);
	} ;

	switch ( boss->getSTATE() )
	{
	case FLEEING:
		{
			// flee until at a safe distance away
			if (distance > 400)
			{ // no longer need to flee
				boss->setSTATE(ACTIVE);
			}
			else
			{	// FLEE						
				boss->addSpeed(1); // move towards MAX speed (caps at max)

				// face player
				boss->setFacing(fleeDIR);

				if ((rand() % 100) < 1) // 10% chance for random 45 degree turns
					boss->turn(rand() % 2);
			}

		}; break;
	case ACTIVE:		// Moving around normally
		{
			if ((rand() % 100) < 1) // 10% chance for random 45 degree turn
				boss->turn(rand() % 2);

			// random +/- speed
			boss->addSpeed(1); 
			// caps at MAX speed, so no harm in this

			// change state if target visible
			if (distance <= 1000)
				boss->setSTATE(SEARCHING); // sees player on radar

			if (distance <= RANGE_LONG)
				boss->setSTATE(ATTACKING);
		}; break;
	case SEARCHING:
		{   // HEAD towards player - AI "Sees" the player on radar
			boss->addSpeed(1); // move towards MAX speed (caps at max)

			// face player
			boss->setFacing(chaseDIR);					

			// change state if target visible
			if (distance > SIGHT_RANGE)
				boss->setSTATE(ACTIVE); // sees player on radar
			if (distance <= GUN_RANGE)
				boss->setSTATE(ATTACKING);
		}; break;
	case ATTACKING:		// Attacking player
		{
			// make sure the target is still in range
			if (distance > RANGE_LONG)
			{
				// unable to fire ... move first
				boss->setSTATE(ACTIVE);

				// this should make him head towards player
				boss->setFacing(chaseDIR);
				boss->addSpeed(2);
			}
			else
			{   // close enough to fire
				// see if ship has enough power to shoot, returns true if able to fire
				// also checks if the gun is reloaded or not
				if ( boss->canFire() )
				{	
					// try to match players speed
					//boss->setSpeed(player->getSpeed()); 
					boss->setSpeed(0);  // stop moving and fire gun

					// face player
					boss->setFacing(chaseDIR);

					DIRS dir1,dir2=chaseDIR,dir3;
					switch(boss->getFacing())
					{
						case NORTH	: dir1=NW;    dir3=NE;   break; // dir1 = left, dir3=right
						case NE		: dir1=NORTH; dir3=EAST; break;
						case NW		: dir1=WEST;  dir3=NORTH;break;
						case EAST	: dir1=NW;	  dir3=SE;   break;
						case WEST	: dir1=SW;    dir3=NW;   break;
						case SOUTH	: dir1=SW;    dir3=SE;   break;
						case SE 	: dir1=EAST;  dir3=SOUTH;break;
						case SW		: dir1=SOUTH; dir3=WEST; break;
					};

					POINT pt1,pt2=boss->getAnchor(),pt3;
					pt1=pt2;
					pt3=pt2;
					switch(chaseDIR)
					{
						case NE:
						case SW:
							{
								pt1.x -= 64; 
								pt3.x += 64;

								pt1.y -= 64; 
								pt3.y += 64;
							}; break;

						case NW:
						case SE:
							{
								pt1.x += 64; 
								pt3.x -= 64;

								pt1.y += 64; 
								pt3.y -= 64;
							}; break;

						case WEST:
						case EAST:
							{
								pt1.x -= 64; 
								pt3.x += 64;
							}; break;

						case NORTH:
						case SOUTH:
							{
								pt1.y -= 64; 
								pt3.y += 64;
							}; break;
					}
						

					// fire (3) bullet towards player
					Ammo_AI[1]->spawnBullet( pt1, dir3 ); // towards right
					Ammo_AI[1]->spawnBullet( pt2, dir2);  // ahead		
					Ammo_AI[1]->spawnBullet( pt3, dir1);  // towards left
					enemyBullets+=3;
					// SOUND :: enemies guns firing at your
				}
/*				else
				{
					int dir=rand()%2, times=(rand()%3+1);
					for (int i=0; i < times;i++) 
						boss->turn(dir);
					//45-135 degree change one direction and move							
					boss->addSpeed(2);
				}*/
			} // distance 
		}; break; // CASE ..
	} // end of SWITCH()

		// perform the actual MOVE for enemy
		boss->doMove(); // which doesnt animate the body
		beeks->nextFrame();// animate the beeks tho

		// check boundaries
		if (boss->getX() < 0)
		{
			boss->setX(64);
			boss->setSpeed(0);
		};
		if (boss->getX() > (GAMEWORLDWIDTH-128))
		{
			boss->setX(0); // ha ha -- GAMEWORLDWIDTH-128);
			boss->setSpeed(0);
		};
		if (boss->getY() < 0)
		{
			boss->setY(1);
			boss->setSpeed(0);
		};
		if (boss->getY() > (GAMEWORLDHEIGHT-128))
		{
			boss->setY(0); // GAMEWORLDHEIGHT-128);
			boss->setSpeed(0);
		};
}; // end EnemyAI function //

void moveDebris()
{
	// move Debris (current view only)
	switch(gameView)
	{
		case SPACE:
			{
			for (int i=0; i < DEBRIS_AMT_SPACE; i++)
				debris_space[i]->doMove(); // moves and rotates
			}; break;
		case SYSTEM:
			{
			for (int i=0; i < DEBRIS_AMT_SYSTEM; i++)
				debris_system[i]->doMove(); // rotates only
			}; break;
		case PLANET:
			{
			for (int i=0; i < DEBRIS_AMT_PLANET; i++)
				debris_planet[i]->doMove(); // static
			}; break;
	}
}

// check for collisions with moving debris ... and applies the effect there of
void checkDebris()
{
	//track recent collision
	if (gameView == SPACE)	lastColl.space   = currColl.space;
	if (gameView == SYSTEM)	lastColl.system  = currColl.system;
	if (gameView == PLANET)	lastColl.surface = currColl.surface;

	// check for player vs. debris collisions		
	int temp = debrisCollision(player);
	if (gameView == SPACE)	currColl.space  = temp; // resets to (0) if no collision
	if (gameView == SYSTEM)	currColl.system = temp;
	if (gameView == PLANET)	currColl.surface = temp;

	// ignore mid-air collisions on planets (clouds dont hurt)
	if (temp > -1)
		switch(gameView)
		{
			case PLANET:
				{
					lastColl.surface = temp;
				};	break; // ignore collisions on planets 			
			case SYSTEM:
				{
					//if its a new collision, apply damage
					if (lastColl.system != temp)
						player->applyDmg(ENERGY,1);

					// save collision
					lastColl.system = temp;
				}; break;
			case SPACE:
				{
					//if its a new collision, apply damage
					if (lastColl.system != temp)
						player->applyDmg(ENERGY,1);

					// save collision
					lastColl.system =  temp;
				}; break;
		}
		// we can apply physics for collision if want to -- got the formula already --
		// sure, if shields deflect a large collision - you bounce away			
}

void createDebris()
{
	// load the moving objects (asteroids and such)
	for (int i=0;i < DEBRIS_AMT_SPACE;i++)
	{
		debris_space[i] = new CDebris(120,110,TEXT("./Media/Images/debris_space.png"),DEBRIS_SPRITES_SPACE,GAMEWORLDWIDTH,GAMEWORLDHEIGHT);		
		hitDebris.space[i] = false;
		assert(debris_space[i]);
	}
	for (int i=0;i < DEBRIS_AMT_SYSTEM;i++)
	{
		debris_system[i] = new CDebris(120,110,TEXT("./Media/Images/debris_system.png"),DEBRIS_SPRITES_SYSTEM,GAMEWORLDWIDTH,GAMEWORLDHEIGHT);		
		hitDebris.system[i] = false;
		assert(debris_system[i]);
	}
	for (int i=0;i < DEBRIS_AMT_PLANET;i++)
	{
		debris_planet[i] = new CDebris(120,110,TEXT("./Media/Images/debris_surface.png"),DEBRIS_SPRITES_PLANET,GAMEWORLDWIDTH,GAMEWORLDHEIGHT);		
		hitDebris.surface[i] = false;
		assert(debris_planet[i]);
	}
}

void drawBars()
{
	// draw enemy health bars?
	// scale the bar per the gameView
	int sx = 32 / scalex;
	int sy =  6 * scaley;
	POINT dest;
	POINT curr;

	// Draw Players Health Bars
	if ((player->getSTATE() != DYING) && (player->getSTATE() != DEAD))
	{
		dest = player->getAnchor();
		dest.x -= ScrollX;  // + sx
		dest.y -= ScrollY;
		dest.y += (player->getHeight()/2) * scaley;

		drawBar(dest.x,dest.y, player->getCur(LOC_SHIELD), player->getMax(LOC_SHIELD), 1);      // SHIELDS
		dest.y += sy;
		drawBar(dest.x,dest.y, player->getCur(LOC_ARMOR) , player->getMax(LOC_ARMOR),  2);      // ARMOR
		dest.y += sy;
		drawBar(dest.x,dest.y, player->getCur(LOC_HULL)  , player->getMax(LOC_HULL),   5);      // HULL
		dest.y += sy;
		drawBar(dest.x,dest.y, player->getSolar()        , player->getSolarMax(),      6);      // solar power
		dest.y += sy;
		if (player->isLoaded())																	// gun reload
			drawBar(dest.x,dest.y, 1,1,3); 
		else
			drawBar(dest.x,dest.y, player->getGunDelay()     , player->weapon->getReload(),3); 
	}

	for (int i=0; i < NUM_ENEMIES; i++)
		if ( (enemy[i]->getSTATE() != DEAD) && (enemy[i]->getSTATE() != INACTIVE) )
			{ 
				curr = enemy[i]->getAnchor();
				curr.x -= ScrollX; // convert to window coord (not game coord)
				curr.y -= ScrollY;

				dest.x = enemy[i]->getX() - ScrollX;
				dest.y = enemy[i]->getY() - ScrollY + ((enemy[i]->getHeight() / 2) *scaley);

				// on-screen ?
				if ((curr.x >= 0) && (curr.x <= SCREEN_WIDTH) && (curr.y >= 0) && (curr.y <= SCREEN_HEIGHT))
				{ // should be visible on screen - so draw health bars
					drawBar(dest.x,dest.y, enemy[i]->getHP(),      enemy[i]->getMaxHP(),			5); 
					dest.y += sy;
					drawBar(dest.x,dest.y, enemy[i]->getSolar(),   enemy[i]->getSolarMax(),			6); // Power
					dest.y += sy;
					if ( enemy[i]->isLoaded() )
						drawBar(dest.x,dest.y, 1, 1, 3); 
					else
						drawBar(dest.x,dest.y, enemy[i]->getGunDelay(),enemy[i]->weapon->getReload(),	3); 
				}
			}
}

void DrawBG(IDirect3DSurface9 *pSource)
{
	RECT r1 = { ScrollX, ScrollY, ScrollX+SCREEN_WIDTH, ScrollY+SCREEN_HEIGHT };
	RECT r2 = { 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1 }; 
	
	d3ddev->StretchRect(pSource,  &r1, backbuffer, &r2, D3DTEXF_NONE);
};

void animExp()
{
	for (int i=0;i < MAX_EXPLOSIONS;i++)
		if (activeEXP[i] == true)
			if (explosion[i]->nextFrame() == 1)
			{ // animation complete
				activeEXP[i] = false;

				delete explosion[i]; 
				// does that destroy it safely ?
			}
}
// 
void newExp(POINT dest)
{
	// find the next available one to use
	int i=0;
	while ((activeEXP[i] == true) && (i < MAX_EXPLOSIONS))
		i++;

	if (i >= MAX_EXPLOSIONS)
		return; // quit

	// make it ACTIVE duh
	activeEXP[i] = true;

	int Height=120;
	int Width =120;
	int Frames=12;
	LPCSTR Filename = TEXT("./Media/Images/exp_red.png");
	explosion[i] = new CSprite(dest.x,dest.y,Height, Width, Filename);
	explosion[i]->setupAnim(Frames,EAST,4);
	explosion[i]->setSTATE(ACTIVE);

	mp3->OpenFile(".\\Media\\Sound\\explosion.mp3",800, 0, 0);
	mp3->Play();

  //const int MAX_EXPLOSIONS = 30;
  //CSprite *explosion[MAX_EXPLOSIONS];
  //bool activeEXP[MAX_EXPLOSIONS];
}

void drawExp()
{
	for (int i=0;i < MAX_EXPLOSIONS;i++)
		if (activeEXP[i] == true)
			DrawSprite( explosion[i]->getX()-ScrollX,explosion[i]->getY()-ScrollY, 
						explosion[i]->getWidth(),explosion[i]->getHeight(),
						scalex, scaley, explosion[i]->getAngle(), 
						D3DCOLOR_XRGB(255,255,255), 
						explosion[i]->getImage(), 
						&explosion[i]->getRect(), 
						d3ddev,false);
}


// sets up random enemy drones
void createDrones()
{
	for (int i=0; i < NUM_ENEMIES; i++)
	{
		int aiGun = 0;// Guns[5] = AI Blasters
		POINT dest = {rand()%2000+24,rand()%2000+24};
		enemy[i] = new CShip(dest.x,dest.y,64,64,TEXT("./Media/Images/manta.png"),2+(rand()%4),2+(rand()%4),10,30,Guns_AI[aiGun],aiGun,0,rand()%4+1,rand()%2 +1);  
		enemy[i]->setupAnim(3,NORTH,3); // sprites per anim, initially heading, sprites per row on sheet	
		enemy[i]->setFrame(0);
		enemy[i]->setSTATE(INACTIVE);	
		enemy[i]->setBorder(0,0,GAMEWORLDWIDTH - 16,GAMEWORLDHEIGHT - 16); // set border to game window
		assert(enemy[i]);
	}

	int Shield = 0;
	int Armor  = rand()%6 +5; // 6-10
	int Hull   = rand()%5 +8; // 8-12
	int Speed  = 2;
	int bossGun = 1;// BOSS GUN 
	POINT dest = {rand()%800+1200,rand()%800+1200};
	
	boss = new CShip(900,900,256,256,TEXT("./Media/Images/bens-jellyboss.png"),0,Speed,40,40,Guns_AI[bossGun],bossGun,Shield,Armor,Hull);  
	boss->setupAnim(8,EAST,4); // sprites per anim, initially heading, sprites per row on sheet	
	boss->setDelay(6);
	boss->setSTATE(ACTIVE);	
	boss->setBorder(0,0,GAMEWORLDWIDTH - 16,GAMEWORLDHEIGHT - 16); // set border to game window

	boss->setFrame(0);
	boss->setLastFrame(1);
	boss->setAnimate(false);

	beeks = new CSprite(900,900,256,256,"./Media/Images/jelly-beeks.png");
	beeks->setupAnim(16,EAST,4);
	beeks->setDelay(5);
	beeks->setFrame(0);

	assert(boss);
}

void respawnDrones(int MAX = NUM_ENEMIES)
{
	// call this when you enter a new system or map

	if (MAX < NUM_ENEMIES)
		for (int i=MAX; i < NUM_ENEMIES; i++)
			enemy[i]->setSTATE(INACTIVE); // be sure the others are ignored then

	for (int i=0; i < MAX; i++)
	{
		POINT dest = {rand()%1200+800,rand()%1200+800};
		int Shield = 0;
		int Armor  = rand()%4 +1;   // 1-4
		int Hull   = rand()%2 +1; // 1-2
		int Speed  = 9 - (Armor + Hull);
		int Angle  = rand() % 360;

		enemy[i]->setFrame(0);
		enemy[i]->setSTATE(ACTIVE);	
		enemy[i]->setPT(dest);
		enemy[i]->setMaxSpeed(Speed);
		enemy[i]->setSpeed(Speed);
		enemy[i]->setAngle(Angle);
		enemy[i]->setupShip(Shield,Armor,Hull);
	}
}

void respawnBoss()
{
//spawns boss on far side of map
//when boss is destroyed this zone becomes clear
//	areacleared[CURR_MAP.system][CURR_MAP.planet] = true;
		POINT dest = {rand()%1200+800,rand()%1200+800};
		int Shield = 0;
		int Armor  = rand()%6 + 5; // 5-10
		int Hull   = rand()%5 + 8; // 8-12
		int Speed  = 2;
		int Angle  = rand() % 360;

		boss->setFrame(deaths);
		boss->setDelay(16);
		boss->setPT(dest);		
		boss->setSpeed(Speed);				
		boss->setAngle(Angle);
		boss->setSTATE(ACTIVE);	
		boss->setupShip(Shield,Armor,Hull);
};

POINT offsetPT(POINT pt1, int Width, int Height,DIRS Facing)
{
		POINT dPT = pt1;
		int halfW = Width  / 2 * scalex;
		int halfH = Height / 2 * scaley;
		 DIRS dir = Facing;

		if ( (dir == NORTH) || (dir == NE) || (dir == NW))	dPT.y -= halfH;
		if ( (dir == SOUTH) || (dir == SE) || (dir == SW))	dPT.y += halfH;
		if ( (dir == WEST)  || (dir == SW) || (dir == NW))	dPT.x -= halfW;
		if ( (dir == EAST)  || (dir == SE) || (dir == NE))	dPT.x += halfW;

		return dPT;
}