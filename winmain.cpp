// Beginning Game Programming, Second Edition
// winmain.cpp - Windows framework source code file

#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <stdio.h>
#include "dxgraphics.h"
#include "game.h"
#include "D3DGUI.h"
#include "Timers.h"
#include "libwmp3.h"


// DirectX gui object.
CD3DGUISystem *g_gui = NULL;
CD3DGUISystem *g_gui_options = NULL;

// Direct3D object and device.
LPDIRECT3D9 g_D3D = NULL;
LPDIRECT3DDEVICE9 g_D3DDevice = NULL;

// Mouse state information.
bool LMBDown = false;
int mouseX = 0, mouseY = 0;

// ids for our GUI controls.
#define STATIC_ID_1  1
#define STATIC_ID_2  2
#define BUTTON_ID_1  3
#define BUTTON_ID_2  4
#define BUTTON_ID_3  5
#define BUTTON_ID_4  6
#define BUTTON_VIDEO_800  7
#define BUTTON_VIDEO_1024 8
#define BUTTON_VIDEO_1280 9
#define BUTTON_IMP 10
#define BUTTON_VHARD 11
#define BUTTON_HARD 12
#define BUTTON_AVG 13
#define BUTTON_ROOKIE 14
#define BUTTON_EASY 15

// current menu state
int CurrentMenuState = MENU_STATE::RunGame;//MENU_STATE::Menu;//MENU_STATE::Splash1;		//default to first splash

// Font ids used to create font objects.
int arialID = -1;
int timesID = -1;
int arialBlackID = -1;

// FPS DirectX font object.
LPD3DXFONT g_Font = NULL;

// FPS RECT used to position the font.
RECT g_FontPosition = {0, 0, 0, 0};

//timer
Win32Timer timer;
double times;
unsigned long startTick;

//function prototypes
bool InitializeObjects();
bool AddMenuButtons();
void GUICallback(int id, int state);
void RenderScene();
void CheckScreen();
bool InitializeD3D(HWND hWnd, bool fullscreen);

bool CheckScreenEnabled = true;
bool isPlaying = false;



//window event callback function
LRESULT WINAPI WinProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            //release the Direct3D device
            if (d3ddev != NULL) 
                d3ddev->Release();

            //release the Direct3D object
            if (d3d != NULL)
                d3d->Release();

            //call the "front-end" shutdown function
            Game_End(hWnd);
            //tell Windows to kill this program
            PostQuitMessage(0);
            return 0;
			break;

		case WM_KEYUP:
			 if(wParam == VK_ESCAPE && CurrentMenuState == MENU_STATE::Splash1)
			 {
				 g_gui->AddBackdrop(".\\Media\\Images\\splash2.bmp");
				 CurrentMenuState = MENU_STATE::Splash2;
				 timer.Stop();
				 timer.Start();
			 }
			 else if(wParam == VK_ESCAPE  && CurrentMenuState == MENU_STATE::Splash2)
			 {
				 g_gui->AddBackdrop(".\\Media\\Images\\backdrop.jpg");
				 CurrentMenuState = MENU_STATE::Menu;
				 AddMenuButtons();
				 timer.Stop();
			 }
			 //else if(wParam == VK_ESCAPE && CurrentMenuState == MENU_STATE::Menu) PostQuitMessage(0);
			 else if(wParam == VK_ESCAPE  && CurrentMenuState == MENU_STATE::Menu)
			 {
				g_gui->Shutdown();
				CurrentMenuState = MENU_STATE::RunGame;
			 }


		case WM_LBUTTONDOWN:
            LMBDown = true;
            break;

         case WM_LBUTTONUP:
            LMBDown = false;
            break;

         case WM_MOUSEMOVE:
            mouseX = LOWORD (lParam);
            mouseY = HIWORD (lParam);
            break;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
}


//helper function to set up the window properties
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    //create the window class structure
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX); 

    //fill the struct with info
    //wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.style		 = CS_CLASSDC;
    wc.lpfnWndProc   = (WNDPROC)WinProc;
    wc.cbClsExtra	 = 0L;
    wc.cbWndExtra	 = 0L;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = APPTITLE;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    //set up the window with the class info
    return RegisterClassEx(&wc);
}


//entry point for a Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow)
{
	MSG msg;
    HWND hWnd;

	// register the class
	MyRegisterClass(hInstance);

    //set up the screen in windowed or fullscreen mode?
    DWORD style;
    if (FULLSCREEN)
        style = WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP;
    else
        style = WS_OVERLAPPED;

    //create a new window
    hWnd = CreateWindow(
       APPTITLE,              //window class
       APPTITLE,              //title bar
       style,                 //window style
       //CW_USEDEFAULT,         //x position of window
       //CW_USEDEFAULT,         //y position of window
       0,
	   0,
	   SCREEN_WIDTH,          //width of the window
       SCREEN_HEIGHT,         //height of the window
       GetDesktopWindow(),    //parent window
       NULL,                  //menu
       hInstance,             //application instance
       NULL);                 //window parameters

    //was there an error creating the window?
    if (!hWnd)
      return FALSE;

	//init menu objects
	if(InitializeD3D(hWnd, false))

    //display the window
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
	
    if (!Init_Direct3D(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN))
        return 0;

	//initialize the game
    if (!Game_Init(hWnd))
    {
        MessageBox(hWnd, "Error initializing the game", "Error", MB_OK);
        return 0;
    }

	if(!InitializeObjects()) return false;

	//SetCursor(LoadCursor(NULL, IDC_ARROW));
	ShowCursor(TRUE);

		//Play menu music

	
    // main message loop
    int done = 0;
	while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
	    {
            //look for quit message
            if (msg.message == WM_QUIT)
                done = 1;

            //decode and pass messages on to WndProc
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
	    }
        else
            //process game loop (else prevents running after window is closed)
			if(CurrentMenuState == MENU_STATE::RunGame)
				Game_Run(hWnd);
			else
			{
				if(CheckScreenEnabled)
					CheckScreen();

				RenderScene();
			}
    }

	return msg.wParam;
}


bool InitializeObjects()
{
   // Set the image states.
	g_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
	g_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);
	g_D3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

   // Create the GUI system.
   g_gui = new CD3DGUISystem(g_D3DDevice, SCREEN_WIDTH, SCREEN_HEIGHT);
   if(!g_gui) return false;

   // Add background image.
   if(!g_gui->AddBackdrop(".\\Media\\Images\\splash1.bmp")) return false;

   // Add two types of fonts to the system.
   if(!g_gui->CreateFont("Arial", 18, &arialID)) return false;
   if(!g_gui->CreateFont("Times New Roman", 18, &timesID)) return false;

	// OPtions GUI
   g_gui_options = new CD3DGUISystem(g_D3DDevice, SCREEN_WIDTH, SCREEN_HEIGHT);
   if(!g_gui_options) return false;

   // Add background image.
   if(!g_gui_options->AddBackdrop(".\\Media\\Images\\options.png")) return false;

   // Add two types of fonts to the system.
   if(!g_gui_options->CreateFont("Arial Black", 18, &arialBlackID)) return false;

//   if(!g_gui_options->AddButton(BUTTON_VIDEO_1280, 700,550, ".\\Media\\Images\\buttonUp_1280.png",".\\Media\\Images\\buttonOver_1280.png", ".\\Media\\Images\\buttonDown_1280.png"));
//   if(!g_gui_options->AddButton(BUTTON_VIDEO_1024, 700,600, ".\\Media\\Images\\buttonUp_1024.png",".\\Media\\Images\\buttonOver_1024.png", ".\\Media\\Images\\buttonDown_1024.png"));
//   if(!g_gui_options->AddButton(BUTTON_VIDEO_800,  700,650, ".\\Media\\Images\\buttonUp_800.png",".\\Media\\Images\\buttonOver_800.png", ".\\Media\\Images\\buttonDown_800.png"));
//#define BUTTON_VIDEO_800  7
//#define BUTTON_VIDEO_1024 8
//#define BUTTON_VIDEO_1280 9
//   if(!g_gui_options->AddButton(BUTTON_IMP,	  200,375, ".\\Media\\Images\\buttonUp_max.png",".\\Media\\Images\\buttonOver_max.png", ".\\Media\\Images\\buttonDown_max.png"));
//   if(!g_gui_options->AddButton(BUTTON_VHARD, 200,425, ".\\Media\\Images\\buttonUp_vhard.png",".\\Media\\Images\\buttonOver_vhard.png", ".\\Media\\Images\\buttonDown_vhard.png"));
//   if(!g_gui_options->AddButton(BUTTON_HARD,  200,475, ".\\Media\\Images\\buttonUp_hard.png",".\\Media\\Images\\buttonOver_hard.png", ".\\Media\\Images\\buttonDown_hard.png"));
//   if(!g_gui_options->AddButton(BUTTON_AVG,	  200,525, ".\\Media\\Images\\buttonUp.png",".\\Media\\Images\\buttonOver.png", ".\\Media\\Images\\buttonDown.png"));
//   if(!g_gui_options->AddButton(BUTTON_ROOKIE,200,575, ".\\Media\\Images\\buttonUp.png",".\\Media\\Images\\buttonOver.png", ".\\Media\\Images\\buttonDown.png"));
//   if(!g_gui_options->AddButton(BUTTON_EASY,  200,625, ".\\Media\\Images\\buttonUp.png",".\\Media\\Images\\buttonOver.png", ".\\Media\\Images\\buttonDown.png"));
//#define BUTTON_IMP 10
//#define BUTTON_VHARD 11
//#define BUTTON_HARD 12
//#define BUTTON_AVG 13
//#define BUTTON_ROOKIE 14
//#define BUTTON_EASY 15
   if(!g_gui_options->AddButton(BUTTON_ID_4, 25,SCREEN_HEIGHT-75, ".\\Media\\Images\\quitUp.png",".\\Media\\Images\\quitOver.png", ".\\Media\\Images\\quitDown.png"));

	//g_D3DDevice->ShowCursor(TRUE);

	timer.Start(0.0);

   return true;
}

bool AddMenuButtons()
{
   // Add 3 buttons to the GUI.
   if(!g_gui->AddButton(BUTTON_ID_1, ((SCREEN_WIDTH / 2) - 64), 200, ".\\Media\\Images\\startUp.png",
      ".\\Media\\Images\\StartOver.png", ".\\Media\\Images\\startDown.png"));

   if(!g_gui->AddButton(BUTTON_ID_2, ((SCREEN_WIDTH / 2) - 64), 232, ".\\Media\\Images\\loadUp.png",
      ".\\Media\\Images\\loadOver.png", ".\\Media\\Images\\loadDown.png"));

   if(!g_gui->AddButton(BUTTON_ID_3, ((SCREEN_WIDTH / 2) - 64), 264, ".\\Media\\Images\\optionsUp.png",
      ".\\Media\\Images\\optionsOver.png", ".\\Media\\Images\\optionsDown.png"));

   if(!g_gui->AddButton(BUTTON_ID_4, ((SCREEN_WIDTH / 2) - 64), 296, ".\\Media\\Images\\quitUp.png",
      ".\\Media\\Images\\quitOver.png", ".\\Media\\Images\\quitDown.png"));

   return false;
}

void GUICallback(int id, int state)
{
	if(state == UGP_BUTTON_DOWN)
	{
		switch(id)
		{
			case BUTTON_ID_1:	//START
				CurrentMenuState = MENU_STATE::RunGame;
				break;
			case BUTTON_ID_2:	
				// DO NOTHING
				break;
			case BUTTON_ID_3:	//OPTIONS MENU
				// Display the options screen
				//CurrentMenuState = MENU_STATE::Options;
				break;
			case BUTTON_ID_4:	//QUIT (if on menu)
				if ( ! (CurrentMenuState == MENU_STATE::Menu))
					CurrentMenuState = MENU_STATE::Menu;
				else
					PostQuitMessage(0);
					// If the quit button is pressed, quit app.
				break;
		}
	}
}

void CheckScreen()
{

	if(timer.GetElapsedTime() > 3)
	{
		if(CurrentMenuState == MENU_STATE::Splash1)
		{
			g_gui->AddBackdrop(".\\Media\\Images\\splash2.bmp");
			CurrentMenuState = MENU_STATE::Splash2;
			timer.Stop();
			timer.Start();
			//startTick = GetTickCount();
		}
		else if(CurrentMenuState == MENU_STATE::Splash2)
		{
			g_gui->AddBackdrop(".\\Media\\Images\\backdrop.jpg");
			CurrentMenuState = MENU_STATE::Menu;
			AddMenuButtons();
			timer.Stop();
			timer.Start();
			//CheckScreenEnabled = false;
		}
		else if(CurrentMenuState == MENU_STATE::Menu)
		{ 
			g_gui->Shutdown();
			CurrentMenuState = MENU_STATE::RunGame;
			timer.Stop();
			CheckScreenEnabled = false;
		}
		else // if(CurrentMenuState == MENU_STATE::RunGame)
		{  // default
			g_gui->Shutdown();
			CurrentMenuState = MENU_STATE::RunGame;
			timer.Stop();
			CheckScreenEnabled = false;
		}
	}
}

bool InitializeD3D(HWND hWnd, bool fullscreen)
{
   D3DDISPLAYMODE displayMode;

   // Create the D3D object.
   g_D3D = Direct3DCreate9(D3D_SDK_VERSION);
   if(g_D3D == NULL) return false;

   // Get the desktop display mode.
   if(FAILED(g_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
      return false;

   // Set up the structure used to create the D3DDevice
   D3DPRESENT_PARAMETERS d3dpp;
   ZeroMemory(&d3dpp, sizeof(d3dpp));

   if (fullscreen)
      {
         d3dpp.Windowed = FALSE;
         d3dpp.BackBufferWidth = SCREEN_WIDTH;
         d3dpp.BackBufferHeight = SCREEN_HEIGHT;
      }
   else
      d3dpp.Windowed = TRUE;
   d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
   d3dpp.BackBufferFormat = displayMode.Format;

   // Create the D3DDevice
   if(FAILED(g_D3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
             D3DCREATE_HARDWARE_VERTEXPROCESSING |
             D3DCREATE_PUREDEVICE, &d3dpp, &g_D3DDevice)))
      {
         return false;
      }

   if(!InitializeObjects()) return false;
	
   return true;
}

void RenderScene()
{
   // Clear the backbuffer.
   g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET,
                      D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

   // Begin the scene.  Start rendering.
   g_D3DDevice->BeginScene();

	//g_D3DDevice->ShowCursor(TRUE);

   // Process and render our gui system.
   if ( CurrentMenuState == MENU_STATE:: Options)
	   ProcessGUI(g_gui_options, LMBDown, mouseX, mouseY, GUICallback);
   else
	   if(CurrentMenuState == MENU_STATE::Menu || CurrentMenuState == MENU_STATE::Splash1 || CurrentMenuState == MENU_STATE::Splash2 || CurrentMenuState == MENU_STATE::Splash3)
		   ProcessGUI(g_gui, LMBDown, mouseX, mouseY, GUICallback);

   // End the scene.  Stop rendering.
   g_D3DDevice->EndScene();

   // Display the scene.
   g_D3DDevice->Present(NULL, NULL, NULL, NULL);
}