/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
////////////////////////////////////////////////////////////////
//  graphics - Advanced C Lab Library
//  this library is based on ACLlib, another open source graphics lib, but has been greatly changed
//  Modified  2019 by Zheng Yusheng 
////////////////////////////////////////////////////////////////

/*
For Dev C++, these lib files need to be added into linker options.
Be sure to change the leading folders as your installation.
"C:/Program Files/Dev-Cpp/MinGW32/lib/libwinmm.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libmsimg32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libkernel32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libuser32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libgdi32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libole32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/liboleaut32.a"
"C:/Program Files/Dev-Cpp/MinGW32/lib/libuuid.a"
*/

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#ifdef _UNICODE
#undef _UNICODE
#endif
#ifdef UNICODE
#undef UNICODE
#endif

#ifdef THINK_C
typedef int bool;
#else
#  ifdef TRUE
#    ifndef bool
#      define bool int
#    endif
#  else
#    ifdef bool
#      define FALSE 0
#      define TRUE 1
#    else
typedef enum { FALSE, TRUE } bool;
#    endif
#  endif
#endif

#include <Windows.h>

#define BLACK			RGB(0, 0, 0)
#define RED				RGB(255, 0, 0)
#define GREEN			RGB(0, 255, 0)
#define BLUE			RGB(0, 0, 255)
#define CYAN			RGB(0, 255, 255)
#define MAGENTA			RGB(255, 0, 255)
#define YELLOW			RGB(255, 255, 0)
#define WHITE			RGB(255, 255, 255)
#define GRAY			RGB(180, 180, 180)

#define EMPTY				0xffffffff
#define DEFAULT				-1
#define DEFAULT_SCREENSIZEW	640
#define DEFAULT_SCREENSIZEH	480

typedef enum
{
	PEN_STYLE_SOLID,
	PEN_STYLE_DASH,			/* -------  */
	PEN_STYLE_DOT,			/* .......  */
	PEN_STYLE_DASHDOT,		/* _._._._  */
	PEN_STYLE_DASHDOTDOT,	/* _.._.._  */
	PEN_STYLE_NULL
} ACL_Pen_Style;

typedef enum
{
	BRUSH_STYLE_SOLID = -1,
	BRUSH_STYLE_HORIZONTAL,		/* ----- */
	BRUSH_STYLE_VERTICAL,		/* ||||| */
	BRUSH_STYLE_FDIAGONAL,		/* \\\\\ */
	BRUSH_STYLE_BDIAGONAL,		/* ///// */
	BRUSH_STYLE_CROSS,			/* +++++ */
	BRUSH_STYLE_DIAGCROSS,		/* xxxxx */
	BRUSH_STYLE_NULL
} ACL_Brush_Style;

typedef enum
{
	NO_BUTTON = 0,
	LEFT_BUTTON,
	MIDDLE_BUTTON,
	RIGHT_BUTTON
} ACL_Mouse_Button;

typedef enum
{
	BUTTON_DOWN,
	BUTTON_DOUBLECLICK,
	BUTTON_UP,
	ROLL_UP,
	ROLL_DOWN,
	MOUSEMOVE
} ACL_Mouse_Event;

typedef enum
{
	KEY_DOWN,
	KEY_UP
} ACL_Keyboard_Event;

typedef struct
{
	HBITMAP hbitmap;
	int width;
	int height;
} Image,*IMAGE;

//typedef enum
//{
//	TM_NO = 0x00,
//	TM_COLOR = 0x01,
//	TM_ALPHA = 0x02
//} ACL_TransparentMode;

typedef COLORREF Color;
typedef char* string;

typedef void(*KeyboardEventCallback) (int key, int event);
typedef void(*CharEventCallback) (char c);
typedef void(*MouseEventCallback) (int x, int y, int button, int event);
typedef void(*TimerEventCallback) (int timerID);

typedef void(*MenuSelectCallback)();

#ifdef __cplusplus
extern "C" {
#endif

	//the main function provided for user
	int main(void);

	// the main function to create window
	void initWindow(const char *wndName, int x, int y, int width, int height);
	int msgBox(const char title[], const char text[], int flag);

	//window size;
	void resetWindowSize(int x, int y, int width, int height);
	int getWindowWidth();
	int getWindowHeight();

	//the number of pixels per inch alongeach of the coordinate
	double GetXResolution(void);
	double GetYResolution(void);

	/*pixels to inches*/
	double ScaleXInches(int x);
	double ScaleYInches(int y);

	//screen size
	int GetFullScreenWidth(void);
	int GetFullScreenHeight(void);

	//some components
	//menu
	void createMyMenu();

	/* function: insertMenu
	 * useage: insertMenu(string menuName,  MenuSelectCallback func);
	* ------------------------------------------------
	* call in the style "pop_up_menu_name/second_menu_name " to create a second menu;
	* space is ignored; you need to call createMyMenu() first
	* you can only insert a popup menu as "pop_up_menu_ name/"
	*/
	bool insertMenuByName(string name, MenuSelectCallback func);

	//register callback functions
	void registerKeyboardEvent(KeyboardEventCallback callback);
	void registerCharEvent(CharEventCallback callback);
	void registerMouseEvent(MouseEventCallback callback);
	void registerTimerEvent(TimerEventCallback callback);

	void cancelKeyboardEvent();
	void cancelCharEvent();
	void cancelMouseEvent();
	void cancelTimerEvent();

	//timer
	void startTimer(int timerID, int timeinterval);
	void cancelTimer(int timerID);

	// Sound
	void Sound(const char * SoundName);
	int loadSound(const char *fileName, int *pSound);
	void playSound(int sid, int repeat);
	void stopSound(int sid);

	// Paint
	void beginPaint();
	void endPaint();
	void clearDevice(void);

	//the total state of the window paint;
	int SaveGraphicsState();
	void RestoreGraphicsState(int id);
	
	//create color: from 0-255
	Color createColor(int red, int green, int blue);

	// Pen
	void setPenColor(Color color);
	void setPenSize(int width);
	void setPenStyle(ACL_Pen_Style style);

	int GetPenSize(void);
	Color GetPenColor(void);
	ACL_Pen_Style GetPenStyle();

	// Brush
	void setBrushColor(Color color);
	void setBrushStyle(ACL_Brush_Style style);

	Color GetBrushColor(void);
	ACL_Brush_Style GetBrushStyle(void);

	// Text
	void setTextColor(Color color);
	void setTextBkColor(Color color);
	void setTextSize(int size);
	void setTextFont(const char *pFontName);

	Color getTextColor();
	Color GetTextBkColor();
	int GetTextSize();

	void drawTextBox(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, string text, unsigned int format);
	void DrawTextString(string text);
	void DisplayText(int x, int y, string textstring);

	int getTextWidth(string text);
	int getTextHeight(string text);

	void setCaretSize(int w, int h);
	void setCaretPos(int x, int y);
	void showCaret();
	void hideCaret();

	// Pixel
	Color getPixelColor(int x, int y);

	// the Point
	int GetCurrentX(void);
	int GetCurrentY(void);
	void moveTo(int x, int y);
	void moveRel(int dx, int dy);

	// Lines and Curves
	void arc(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, \
		int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc);
	void line(int x0, int y0, int x1, int y1);
	void lineTo(int nXEnd, int nYEnd);
	void lineRel(int dx, int dy);
	void polyBezier(const POINT *lppt, int cPoints);
	void polyLine(const POINT *lppt, int cPoints);
	
	// Filled Shapes
	void chrod(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, \
		int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2);
	void ellipse(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);
	void pie(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, \
		int nXRadial1, int nYRadial1, int nXRadial2, int nYRadial2);
	void polygon(const POINT *lpPoints, int nCount);
	void rectangle(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect);
	void roundrect(int nLeftRect, int nTopRect, int nRightRect, int nBottomRect, \
		int nWidth, int nHeight);

	// Image
	void loadImage(string pImageFileName, IMAGE pImage);
	void freeImage(IMAGE pImage);

	void putImage(IMAGE pImage, int x, int y);
	void putImageScale(IMAGE pImage, int x, int y, int width, int height);
	void putImageTransparent(IMAGE pImage, int x, int y, int width, int height, Color bkColor);


	void Pause(double seconds);

	void initConsole(void);

	void ExitGraphics(void);

#ifdef __cplusplus
}
#endif

#endif
