/*
   Demo Name:  Direct3D Graphical User Interface
      Author:  Allen Sherrod
     Chapter:  Ch 5
*/


#ifndef _UGP_D3D_GUI_H_
#define _UGP_D3D_GUI_H_

// Types of controls we support.
#define UGP_GUI_STATICTEXT 1
#define UGP_GUI_BUTTON     2
#define UGP_GUI_BACKDROP   3

// Mouse button states.
#define UGP_BUTTON_UP      1
#define UGP_BUTTON_OVER    2
#define UGP_BUTTON_DOWN    3

// A structure for our custom vertex type
struct stGUIVertex
{
    float x, y, z, rhw;
    unsigned long color;
    float tu, tv;
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_GUI (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)


struct stGUIControl
{
   // Control type, id, and color.
   int m_type;
   int m_id;
   unsigned long m_color;

   // If text then this is the id of the font it uses, else vertex buffer.
   int m_listID;

   // Starting location for the control.
   float m_xPos, m_yPos;

   // Width and height of the button.
   float m_width, m_height;

   // Text of static text.
   char *m_text;

   // Fullscreen image of the background.
   LPDIRECT3DTEXTURE9 m_backDrop;

   // Button's up, down, and over textures.
   LPDIRECT3DTEXTURE9 m_upTex, m_downTex, m_overTex;
};


class CD3DGUISystem
{
   public:
      CD3DGUISystem(LPDIRECT3DDEVICE9 device, int w, int h);
      ~CD3DGUISystem() { Shutdown(); }

      bool CreateFont(char *fontName, int size, int *fontID);
      bool AddBackdrop(char *fileName);
      bool AddStaticText(int id, char *text, float x, float y, unsigned long color, int fontID);
      bool AddButton(int id, float x, float y, char *up, char *over, char *down);
      void Shutdown();

      LPD3DXFONT GetFont(int id)
      {
         if(id < 0 || id >= m_totalFonts) return NULL;
         return m_fonts[id];
      }

      stGUIControl *GetGUIControl(int id)
      {
         if(id < 0 || id >= m_totalControls) return NULL;
         return &m_controls[id];
      }

      LPDIRECT3DVERTEXBUFFER9 GetVertexBuffer(int id)
      {
         if(id < 0 || id >= m_totalBuffers) return NULL;
         return m_vertexBuffers[id];
      }

      int GetTotalFonts() { return m_totalFonts; }
      int GetTotalControls() { return m_totalControls; }
      int GetTotalBuffers() { return m_totalBuffers; }
      int GetWindowWidth() { return m_windowWidth; }
      int GetWindowHeight() { return m_windowHeight; }
      LPDIRECT3DDEVICE9 GetDevice() { return m_device; }
      stGUIControl *GetBackDrop() { return &m_backDrop; }
      LPDIRECT3DVERTEXBUFFER9 GetBackDropBuffer() { return m_backDropBuffer; }
      bool UseBackDrop() { return m_useBackDrop; }

      void SetWindowSize(int w, int h) { m_windowWidth = w; m_windowHeight = h; }

   private:
      LPDIRECT3DDEVICE9 m_device;
      LPD3DXFONT *m_fonts;
      stGUIControl *m_controls;
      LPDIRECT3DVERTEXBUFFER9 *m_vertexBuffers;
      stGUIControl m_backDrop;
      LPDIRECT3DVERTEXBUFFER9 m_backDropBuffer;

      bool m_useBackDrop;
      int m_totalFonts;
      int m_totalControls;
      int m_totalBuffers;

      int m_windowWidth;
      int m_windowHeight;
};


void ProcessGUI(CD3DGUISystem *gui, bool LMBDown, int mouseX, int mouseY,
                void(*funcPtr)(int id, int state));

#endif