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

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#define CINTERFACE

#ifdef _UNICODE
#undef _UNICODE
#endif
#ifdef UNICODE
#undef UNICODE
#endif

#include "graphics.h"

#include <windows.h>
#include <olectl.h>
#include <stdio.h>
#include<time.h>

#ifdef _MSC_VER
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"msimg32.lib")
#endif

#ifdef _DEBUG
#define ACL_ASSERT(_Expression,errStr) (void)( (!!(_Expression)) || (acl_error(errStr),0) )
#else
#define ACL_ASSERT(flag,errStr) ((void)0)
#endif

#define ACL_ASSERT_HWND ACL_ASSERT(g_hWnd!=0, \
	TEXT("You should call function \"initWindow(...)\" befor use function \"" __FUNCTION__ "\"") )
#define ACL_ASSERT_BEGIN_PAINT ACL_ASSERT(g_hmemdc!=0, \
	TEXT("You should call function \"beginPaint()\" befor use function \"" __FUNCTION__ "\"") )

#define MAX_MENU_NUM 50
#define MAX_WINDOW_NAME 256
#define MENUID_START 300
#define MAX_G_STATE_NUM 10

//the struct for saveing the graphics state;
typedef struct {

	Color g_penColor;
	int g_penWidth;
	int g_penStyle;
	int pen_cx;
	int pen_cy;

	Color g_brushColor;
	int g_brushStyle;

	int g_textSize;
	Color g_textColor;
	Color g_textBkColor;

	int g_caretHeight;
	int g_caretWidth;
	int g_caretX;
	int g_caretY;
	bool CaretSize_seted;

} graphStateStruct;

//private variables
/*-----------------------------------------------*/
static const char g_wndClassName[] = TEXT("ACL_WND_CLASS");
static const char g_libName[] = TEXT("ACLLIB");

static string menuName[MAX_MENU_NUM];
static void* menuID[MAX_MENU_NUM];
static int menu_count = 0;

static HINSTANCE g_hInstance;

static HWND g_hWnd = NULL;
static HDC g_hmemdc = NULL;
static HBITMAP g_hbitmap = NULL;
static RECT clientRect = { 0, 0, DEFAULT_SCREENSIZEW,DEFAULT_SCREENSIZEH };
static HMENU hmenu = NULL;

static HFONT g_font = NULL;
static char g_fontName[MAX_WINDOW_NAME] = TEXT("宋体");
static int g_textSize = 13;
static Color g_textColor = BLACK;
static Color g_textBkColor = WHITE;

static int alpha_h = 0;
static int alpha_w = 0;

static int soundID_count;

static graphStateStruct g_stateArray[MAX_G_STATE_NUM];
static int g_stateCount = 0;

static HPEN g_pen = NULL;
static Color g_penColor = BLACK;
static int g_penWidth = 1;
static int g_penStyle = PEN_STYLE_SOLID;
static int pen_cx;
static int pen_cy;

static HBRUSH g_brush = NULL;
static Color g_brushColor = BLACK;
static int g_brushStyle = BRUSH_STYLE_SOLID;

static int g_caretHeight = 12;
static int g_caretWidth = 6;
static int g_caretX = 0;
static int g_caretY = 0;

static KeyboardEventCallback g_keyboard = NULL;
static MouseEventCallback g_mouse = NULL;
static TimerEventCallback g_timer = NULL;
static CharEventCallback g_char = NULL;

static bool CaretSize_seted = FALSE;

//private functions declare
/*------------------------------------------------------*/
//for update the window
static void updatePen();
static void updateBrush();
static void updateFont();

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*--------------------------------------*/
//simple error process
void acl_error(char *errStr)
{
	MessageBox(g_hWnd, errStr, g_libName, MB_ICONERROR);
	exit(0);
}

//winmain: the entrance of the library
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	MSG          msg;
	WNDCLASSA     wndclass;

	g_hInstance = hInstance;
	g_hWnd = NULL;
	g_keyboard = NULL;
	g_mouse = NULL;
	g_timer = NULL;

	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = g_wndClassName;

	if (!RegisterClass(&wndclass))
	{
		MessageBoxA(NULL, TEXT("Register Class fail!"), g_libName, MB_ICONERROR);
		return 0;
	}

	main();

	ACL_ASSERT(g_hWnd, "You must call \"initWindow(...)\" in Main()");

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

//the main process function
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		HDC hdc;
		hdc = GetDC(hwnd);
		g_hbitmap = CreateCompatibleBitmap(
			hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		g_hmemdc = CreateCompatibleDC(hdc);
		SelectObject(g_hmemdc, g_hbitmap);
		BitBlt(g_hmemdc,
			0, 0,
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),
			g_hmemdc,
			0, 0,
			WHITENESS);
		DeleteDC(g_hmemdc);
		ReleaseDC(hwnd, hdc);

		CreateCaret(hwnd, 0, g_caretWidth, g_caretHeight);
		GetClientRect(g_hWnd, &clientRect);
		g_caretX = getWindowWidth();
		g_caretY = getWindowHeight();
		SetCaretPos(g_caretX, g_caretY);

		break;
	}
	case WM_ERASEBKGND:
		break;
	case WM_PAINT:
	{
		HDC hdc;
		PAINTSTRUCT ps;
		RECT rect;

		hdc = BeginPaint(hwnd, &ps);
		g_hmemdc = CreateCompatibleDC(hdc);
		SelectObject(g_hmemdc, g_hbitmap);
		GetClientRect(hwnd, &rect);
		BitBlt(hdc, 0, 0, rect.right - rect.left,
			rect.bottom - rect.top, g_hmemdc, 0, 0, SRCCOPY);
		alpha_h = getTextHeight("l");
		alpha_w = getTextWidth("a");
		DeleteDC(g_hmemdc);
		g_hmemdc = 0;
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_COMMAND:
		((MenuSelectCallback)menuID[LOWORD(wParam)])();
		break;
	case WM_CHAR:
		if (g_char != NULL)
			g_char((char)wParam);
		break;

	case WM_SIZE:
		GetClientRect(g_hWnd, &clientRect);
		break;

	case WM_KEYDOWN:
		if (g_keyboard != NULL)
			g_keyboard((int)wParam, KEY_DOWN);
		break;

	case WM_KEYUP:
		if (g_keyboard != NULL)
			g_keyboard((int)wParam, KEY_UP);
		break;

	case WM_LBUTTONDOWN:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), LEFT_BUTTON, BUTTON_DOWN);
		break;

	case WM_LBUTTONUP:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), LEFT_BUTTON, BUTTON_UP);
		break;

	case WM_LBUTTONDBLCLK:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), LEFT_BUTTON, BUTTON_DOUBLECLICK);
		break;

	case WM_MBUTTONDOWN:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MIDDLE_BUTTON, BUTTON_DOWN);
		break;

	case WM_MBUTTONUP:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MIDDLE_BUTTON, BUTTON_UP);
		break;

	case WM_MBUTTONDBLCLK:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MIDDLE_BUTTON, BUTTON_DOUBLECLICK);
		break;

	case WM_RBUTTONDOWN:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), RIGHT_BUTTON, BUTTON_DOWN);
		break;

	case WM_RBUTTONUP:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), RIGHT_BUTTON, BUTTON_UP);
		break;

	case WM_RBUTTONDBLCLK:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), RIGHT_BUTTON, BUTTON_DOUBLECLICK);
		break;

	case WM_MOUSEMOVE:
		if (g_mouse != NULL)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MOUSEMOVE, MOUSEMOVE);
		break;

	case WM_MOUSEWHEEL:
		if (g_mouse == NULL)
			break;
		if (HIWORD(wParam) == 120)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MIDDLE_BUTTON, ROLL_UP);
		else if (HIWORD(wParam) == 65416)
			g_mouse((int)LOWORD(lParam), (int)HIWORD(lParam), MIDDLE_BUTTON, ROLL_DOWN);
		break;

	case WM_TIMER:
		if (g_timer != NULL)
			g_timer(wParam);
		break;

	case WM_DESTROY:
		DeleteObject(g_hbitmap);
		DestroyMenu(hmenu);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

//
void initWindow(const char *wndName, int x, int y, int width, int height)
{
	RECT rect;

	ACL_ASSERT(!g_hWnd, "Don't call initWindow twice");

	if (x == DEFAULT || y == DEFAULT)
		x = y = CW_USEDEFAULT;

	g_hWnd = CreateWindow(
		g_wndClassName, wndName,
		WS_OVERLAPPEDWINDOW,
		x, y,
		width, height,
		NULL, hmenu, 0, NULL);

	if (!g_hWnd)
	{
		MessageBox(NULL, "Fail to create window", g_libName, MB_ICONERROR);
		exit(0);
	}
	GetClientRect(g_hWnd, &rect);
	width += width - (rect.right - rect.left);
	height += height - (rect.bottom - rect.top);
	SetWindowPos(g_hWnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);

	ShowWindow(g_hWnd, 1);
	UpdateWindow(g_hWnd);
}

void resetWindowSize(int x, int y, int width, int height) {
	clientRect.left = x;
	clientRect.top = y;
	clientRect.right = x + width;
	clientRect.bottom = y + height;
	SetWindowPos(g_hWnd, HWND_TOP, x, y, width, height, SWP_SHOWWINDOW);
}

//must called before insertMenu
void createMyMenu() {
	if (g_hWnd)
		acl_error("Call before Init window!");
	hmenu = CreateMenu();
	if(!hmenu)
		acl_error("can not create menu!");
}

static int findMenuName(string name) {
	int i;
	for (i = 0; i < menu_count; ++i)
		if (strcmp(name, menuName[i]) == 0)
			return i;
	return -1;
}

/* function: insertMenu
 * useage: insertMenu(string menuName,  MenuSelectCallback func);
 * ------------------------------------------------
 * call in the style "pop_up_menu_name/second_menu_name " to create a second menu;
 * space is ignored;
 * you can only insert a popup menu as "pop_up_menu_ name/"
 */
bool insertMenuByName(string name, MenuSelectCallback func) {
	if (g_hWnd) {
		acl_error("Call before Init window!");
	}
	//remove space
	char buffer[50];
	int p1 = 0, p2 = 0;
	while (name[p1])
		if (!isspace(name[p1]))
			buffer[p2++] = name[p1++];
		else p1++;
	buffer[p2] = 0;
	name = buffer;
	if (strlen(name) == 0)
		return FALSE;

	string name1;
	int id;
	HMENU pmenu = hmenu;
	name1 = strchr(name, '/');
	if (name1 == NULL)
		return FALSE;
	*name1 = 0;

	if ((id = findMenuName(name)) == -1) {
		menuName[menu_count++] = (string)malloc(sizeof(char)*(strlen(name) + 3));
		strcpy(menuName[menu_count - 1], name);
		HMENU pop= CreatePopupMenu();
		menuID[menu_count - 1] = pop;
		AppendMenu(hmenu, MF_POPUP, (UINT_PTR)pop, name);
		pmenu = pop;
	}
	else {
		pmenu = (HMENU)menuID[id];
	}

	name = name1 + 1;
	if (strlen(name) == 0)
		return TRUE;
	menuName[menu_count++] = (string)malloc(sizeof(char)*(strlen(name) + 3));
	menuName[menu_count - 1][0] = 0;
	menuID[menu_count - 1] = func;
	AppendMenu(pmenu, MF_STRING, menu_count - 1, name);
	return TRUE;
}

void initConsole(void)
{
	AllocConsole();
	freopen("CONIN$", "r+t", stdin);
	freopen("CONOUT$", "w+t", stdout);
}

void ExitGraphics(void)
{
	exit(0);
}

void SetWindowTitle(string title)
{	
	SetWindowText(g_hWnd, title);
}

int msgBox(const char title[], const char text[], int flag)
{
	ACL_ASSERT_HWND;
	return MessageBox(g_hWnd, text, title, flag);
}

//create color
Color createColor(int red, int green, int blue) {
	return RGB(red, green, blue);
}

//
void beginPaint()
{
	HDC hdc;

	ACL_ASSERT_HWND;

	hdc = GetDC(g_hWnd);
	g_hmemdc = CreateCompatibleDC(hdc);
	SelectObject(g_hmemdc, g_hbitmap);

	updatePen();
	updateBrush();
	updateFont();
	setTextColor(g_textColor);
	setTextBkColor(g_textBkColor);
}

void endPaint()
{
	DeleteDC(g_hmemdc);
	g_hmemdc = 0;
	InvalidateRect(g_hWnd, 0, 0);

	DeleteObject(g_pen);
	DeleteObject(g_brush);
	DeleteObject(g_font);
	g_pen = NULL;
	g_brush = NULL;
	g_font = NULL;
}

void clearDevice(void)
{
	ACL_ASSERT_BEGIN_PAINT;
	BitBlt(
		g_hmemdc,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		g_hmemdc,
		0, 0,
		WHITENESS);
}

//pen and brush
/*------------------------------------------------------------*/
static void updatePen()
{
	if (g_pen)DeleteObject(g_pen);
	if (g_penColor == EMPTY)
		g_pen = (HPEN)GetStockObject(NULL_PEN);
	else
		g_pen = CreatePen(g_penStyle, g_penWidth, g_penColor);
	SelectObject(g_hmemdc, g_pen);
}

static void updateBrush()
{
	if (g_brush)DeleteObject(g_brush);
	if (g_brushColor == EMPTY)
	{
		g_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	else
	{
		if (g_brushStyle == BRUSH_STYLE_SOLID)
			g_brush = CreateSolidBrush(g_brushColor);
		else
			g_brush = CreateHatchBrush(g_brushStyle, g_brushColor);
	}
	SelectObject(g_hmemdc, g_brush);
}

void setPenColor(Color newColor)
{
	ACL_ASSERT_BEGIN_PAINT;
	g_penColor = newColor;
	updatePen();
}

void setPenSize(int width)
{
	ACL_ASSERT_BEGIN_PAINT;
	g_penWidth = width;
	updatePen();
	alpha_h = getTextHeight("l");
	alpha_w = getTextWidth("a");
}

void setPenStyle(ACL_Pen_Style newStyle)
{
	ACL_ASSERT_BEGIN_PAINT;

	switch (newStyle)
	{
	case PEN_STYLE_SOLID:
		g_penStyle = PS_SOLID; break;
	case PEN_STYLE_DASH:
		g_penStyle = PS_DASH; break;
	case PEN_STYLE_DOT:
		g_penStyle = PS_DOT; break;
	case PEN_STYLE_DASHDOT:
		g_penStyle = PS_DASHDOT; break;
	case PEN_STYLE_DASHDOTDOT:
		g_penStyle = PS_DASHDOTDOT; break;
	case PEN_STYLE_NULL:
		g_penStyle = -1;
		setPenColor(EMPTY);
		return;
	default:
		break;
	}
	updatePen();
}

ACL_Pen_Style GetPenStyle() {
	return g_penStyle;
}

int GetPenSize(void) {
	return g_penWidth;
}

Color GetPenColor(void) {
	return g_penColor;
}

void setBrushColor(Color newColor)
{
	ACL_ASSERT_BEGIN_PAINT;
	g_brushColor = newColor;
	updateBrush();
}

void setBrushStyle(ACL_Brush_Style newStyle)
{
	ACL_ASSERT_BEGIN_PAINT;

	switch (newStyle)
	{
	case BRUSH_STYLE_SOLID:
		g_brushStyle = BRUSH_STYLE_SOLID; break;
	case BRUSH_STYLE_HORIZONTAL:
		g_brushStyle = HS_HORIZONTAL; break;
	case BRUSH_STYLE_VERTICAL:
		g_brushStyle = HS_VERTICAL; break;
	case BRUSH_STYLE_FDIAGONAL:
		g_brushStyle = HS_FDIAGONAL; break;
	case BRUSH_STYLE_BDIAGONAL:
		g_brushStyle = HS_BDIAGONAL; break;
	case BRUSH_STYLE_CROSS:
		g_brushStyle = HS_CROSS; break;
	case BRUSH_STYLE_DIAGCROSS:
		g_brushStyle = HS_DIAGCROSS; break;
	case BRUSH_STYLE_NULL:
		g_brushStyle = BRUSH_STYLE_SOLID;
		setBrushColor(EMPTY);
		return;
	default:
		break;
	}
	updateBrush();
}

Color GetBrushColor(void) {
	return g_brushColor;
}

ACL_Brush_Style GetBrushStyle(void) {
	return g_brushStyle;
}

//the total state of the window paint; save and restore
/*-------------------------------------------------------*/
int SaveGraphicsState() {
	graphStateStruct g_state;
	g_state.CaretSize_seted = CaretSize_seted;
	g_state.g_brushColor = g_brushColor;
	g_state.g_brushStyle = g_brushStyle;
	g_state.g_caretHeight = g_caretHeight;
	g_state.g_caretWidth = g_caretWidth;
	g_state.g_caretX = g_caretX;
	g_state.g_caretY = g_caretY;
	g_state.g_penColor = g_penColor;
	g_state.g_penStyle = g_penStyle;
	g_state.g_penWidth = g_penWidth;
	g_state.g_textBkColor = g_textBkColor;
	g_state.g_textColor = g_textColor;
	g_state.g_textSize = g_textSize;
	g_state.pen_cx = pen_cx;
	g_state.pen_cy = pen_cy;
	if (g_stateCount == MAX_G_STATE_NUM) {
		g_stateArray[g_stateCount-1] = g_state;
	}else
		g_stateArray[g_stateCount++] = g_state;
	return g_stateCount - 1;
}

void RestoreGraphicsState(int id) {
	graphStateStruct g_state;
	if (id < -1 || id >= g_stateCount ||g_stateCount==0) {
		acl_error("graphic state id out of range!");
		return;
	}
	else if (id == -1)	g_state = g_stateArray[g_stateCount - 1];
	else  g_state = g_stateArray[id];
	CaretSize_seted = g_state.CaretSize_seted;
	g_brushColor = g_state.g_brushColor;
	g_brushStyle = g_state.g_brushStyle;
	g_caretHeight = g_state.g_caretHeight;
	g_caretWidth = g_state.g_caretWidth;
	g_caretX = g_state.g_caretX;
	g_caretY = g_state.g_caretY;
	g_penColor = g_state.g_penColor;
	g_penStyle = g_state.g_penStyle;
	g_penWidth = g_state.g_penWidth;
	g_textBkColor = g_state.g_textBkColor;
	g_textColor = g_state.g_textColor;
	g_textSize = g_state.g_textSize;

	updatePen();
	updateBrush();
	updateFont();
	setTextColor(g_textColor);
	setTextBkColor(g_textBkColor);
	moveTo(g_state.pen_cx, g_state.pen_cy);
	if (id == -1) g_stateCount--;
	else {
		int i = id;
		for (; i < g_stateCount - 1; ++i)
			g_stateArray[i] = g_stateArray[i + 1];
		g_stateCount--;
	}
}

//text 1: show text part
/*----------------------------------------------------------*/
/*    DT_CENTER：指定文本水平居中显示。
    DT_VCENTER：指定文本垂直居中显示。该标记仅仅在单行文本输出时有效，所以它必须与DT_SINGLELINE结合使用。
    DT_SINGLELINE：单行显示文本，回车和换行符都不断行。
	*/
void drawTextBox(int x1, int y1, int x2, int y2, string text, unsigned int format) {
	ACL_ASSERT_BEGIN_PAINT;
	RECT box;
	SetRect(&box, x1, y1, x2, y2);
	SetBkMode(g_hmemdc, TRANSPARENT);
	DrawText(g_hmemdc, text, strlen(text), &box, format);
	SetBkMode(g_hmemdc, OPAQUE);
}

void DisplayText(int x, int y, string textstring)
{
	ACL_ASSERT_BEGIN_PAINT;
	SetBkMode(g_hmemdc, TRANSPARENT);
	TextOut(g_hmemdc, x, y, textstring, strlen(textstring));
	SetBkMode(g_hmemdc, OPAQUE);
}

void DrawTextString(string text)
{
	DisplayText(pen_cx, pen_cy, text);
	moveTo(pen_cx + getTextWidth(text), pen_cy);
}

//text 2: set up text property part
/*--------------------------------------*/
static void updateFont()
{
	if (g_font)DeleteObject(g_font);
	g_font = CreateFont(
		g_textSize,
		0,
		0, 0, 700, 0, 0, 0, 0, 0, 0, 0, 0, g_fontName);
	SelectObject(g_hmemdc, g_font);
}

void setTextColor(Color color)
{
	ACL_ASSERT_BEGIN_PAINT;
	ACL_ASSERT(color != EMPTY, "text color can not be EMPTY");
	g_textColor = color;
	SetTextColor(g_hmemdc, color);
}

void setTextBkColor(Color color)
{
	ACL_ASSERT_BEGIN_PAINT;
	g_textBkColor = color;
	if (color == EMPTY)
		SetBkMode(g_hmemdc, TRANSPARENT);
	else
	{
		SetBkMode(g_hmemdc, OPAQUE);
		SetBkColor(g_hmemdc, color);
	}
}

void setTextSize(int size)
{
	ACL_ASSERT_BEGIN_PAINT;
	g_textSize = size;
	updateFont();
}

void setTextFont(const char *pfn)
{
	size_t len;
	ACL_ASSERT_BEGIN_PAINT;
	len = strlen(pfn);
	strcpy(g_fontName, pfn);
	updateFont();
}

//text 3: get text property
/*-----------------------*/
int getTextWidth(string text) {
	if (g_hmemdc == 0) {
		return alpha_w*strlen(text);
	}
	SIZE text_size;
	GetTextExtentPointA(g_hmemdc, text, strlen(text), &text_size);
	return text_size.cx;
}

Color getTextColor() {
	return g_textColor;
}

Color GetTextBkColor() {
	return g_textBkColor;
}


int GetTextSize() {
	return g_textSize;
}

int getTextHeight(string text) {
	if (g_hmemdc == 0) {
		return alpha_h;
	}
	SIZE text_size;
	GetTextExtentPointA(g_hmemdc, text, strlen(text), &text_size);
	return text_size.cy;
}

//text 4: caret functions
/*------------------------------------------------------------*/
void setCaretSize(int w, int h)
{
	DestroyCaret();
	CreateCaret(g_hWnd, 0, w, h);
	SetCaretPos(g_caretX, g_caretY);
	CaretSize_seted = TRUE;
}

void setCaretPos(int x, int y)
{
	if (!CaretSize_seted) {
		setCaretSize(getTextWidth("l"), getTextHeight("a"));
		showCaret();
	}
	g_caretX = x;
	g_caretY = y;
	SetCaretPos(g_caretX, g_caretY);
}

void showCaret()
{
	//setCaretSize(getTextWidth("a"), getTextHeight("a"));
	ShowCaret(g_hWnd);
}

void hideCaret()
{
	HideCaret(g_hWnd);
}

//grapichics functions: part1 get and set property
/*---------------------------------------------------------------*/

Color getPixelColor(int x, int y)
{
	ACL_ASSERT_BEGIN_PAINT;
	return GetPixel(g_hmemdc, x, y);
}

int getWindowWidth(void)
{	
	//ACL_ASSERT(g_hWnd, "You must call \"initWindow(...)\" in Main()");
	return clientRect.right;
	//return g_wndWidth;
}

int getWindowHeight(void)
{	
	//ACL_ASSERT(g_hWnd, "You must call \"initWindow(...)\" in Main()");
	return clientRect.bottom;
	//return g_wndHeight;
}

int GetFullScreenWidth(void)
{
	HWND desktop;
	RECT bounds;

	desktop = GetDesktopWindow();
	GetWindowRect(desktop, &bounds);
	return bounds.right-bounds.left;
}

int GetFullScreenHeight(void)
{
	HWND desktop;
	RECT bounds;

	desktop = GetDesktopWindow();
	GetWindowRect(desktop, &bounds);
	return  bounds.bottom - bounds.top;
}

int GetCurrentX(void)
{
	//POINT point;
	//ACL_ASSERT_BEGIN_PAINT;
	//GetCurrentPositionEx(g_hmemdc, &point);
	//return (int) point.x;
	return pen_cx;
}

int GetCurrentY(void)
{
	//POINT point;
	//ACL_ASSERT_BEGIN_PAINT;
	//GetCurrentPositionEx(g_hmemdc, &point);
	//return (int) point.y;
	return pen_cy;
}

void moveTo(int x, int y)
{
	ACL_ASSERT_BEGIN_PAINT;
	MoveToEx(g_hmemdc, x, y, NULL);
	pen_cx = x;
	pen_cy = y;
}

void moveRel(int dx, int dy)
{
	ACL_ASSERT_BEGIN_PAINT;
	//GetCurrentPositionEx(g_hmemdc, &point);
	MoveToEx(g_hmemdc, (int)pen_cx + dx, (int)pen_cy + dy, NULL);
	pen_cx += dx;
	pen_cy += dy;
}

//grapichics functions: part2 Lines and Curves
/*---------------------------------------------------------------*/
void arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	ACL_ASSERT_BEGIN_PAINT;
	Arc(g_hmemdc, x1, y1, x2, y2, x3, y3, x4, y4);
	pen_cx = x4;
	pen_cy = y4;
}

void line(int x0, int y0, int x1, int y1)
{
	//POINT point;	
	ACL_ASSERT_BEGIN_PAINT;
	//GetCurrentPositionEx(g_hmemdc, &point);
	MoveToEx(g_hmemdc, x0, y0, NULL);
	LineTo(g_hmemdc, x1, y1);
	pen_cx = x1;
	pen_cy = y1;
	//MoveToEx(g_hmemdc,point.x,point.y,NULL);
}

void lineTo(int x, int y)
{
	ACL_ASSERT_BEGIN_PAINT;
	LineTo(g_hmemdc, x, y);
	pen_cx = x;
	pen_cy = y;
}

void lineRel(int dx, int dy)
{
	//POINT point;
	ACL_ASSERT_BEGIN_PAINT;
	//GetCurrentPositionEx(g_hmemdc, &point);
	LineTo(g_hmemdc, (int)pen_cx + dx, (int)pen_cy + dy);
	pen_cx += dx;
	pen_cy += dy;
}

void polyBezier(const POINT *lppt, int cPoints)
{
	ACL_ASSERT_BEGIN_PAINT;
	PolyBezier(g_hmemdc, lppt, cPoints);
	pen_cx = lppt[cPoints - 1].x;
	pen_cy = lppt[cPoints - 1].y;
}

void polyLine(const  POINT *lppt, int cPoints)
{
	ACL_ASSERT_BEGIN_PAINT;
	Polyline(g_hmemdc, lppt, cPoints);
	pen_cx = lppt[cPoints - 1].x;
	pen_cy = lppt[cPoints - 1].y;
}

// grapichics functions: part3 Filled Shapes: without move the pen
/*------------------------------------------------------*/
void chrod(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	ACL_ASSERT_BEGIN_PAINT;
	Chord(g_hmemdc, x1, y1, x2, y2, x3, y3, x4, y4);
}

void ellipse(int left, int top, int right, int bottom)
{
	ACL_ASSERT_BEGIN_PAINT;
	Ellipse(g_hmemdc, left, top, right, bottom);
}

void pie(int left, int top, int right, int bottom, int xr1, int yr1, int xr2, int yr2)
{
	ACL_ASSERT_BEGIN_PAINT;
	Pie(g_hmemdc, left, top, right, bottom, xr1, yr1, xr2, yr2);
}

void polygon(const POINT *apt, int cpt)
{
	ACL_ASSERT_BEGIN_PAINT;
	Polygon(g_hmemdc, apt, cpt);
}

void rectangle(int left, int top, int right, int bottom)
{
	ACL_ASSERT_BEGIN_PAINT;
	Rectangle(g_hmemdc, left, top, right, bottom);
}

void roundrect(int left, int top, int right, int bottom, int width, int height)
{
	ACL_ASSERT_BEGIN_PAINT;
	RoundRect(g_hmemdc, left, top, right, bottom, width, height);
}

//image functions
/*---------------------------------------*/
void putImage(IMAGE pImage, int x, int y)
{
	HDC hbitmapdc;
	ACL_ASSERT_BEGIN_PAINT;
	hbitmapdc = CreateCompatibleDC(g_hmemdc);
	SelectObject(hbitmapdc, pImage->hbitmap);
	BitBlt(g_hmemdc, x, y, pImage->width, pImage->height, hbitmapdc, 0, 0, SRCCOPY);
	DeleteDC(hbitmapdc);
}

void putImageScale(IMAGE pImage, int x, int y, int width, int height)
{
	HDC hbitmapdc;
	ACL_ASSERT_BEGIN_PAINT;
	hbitmapdc = CreateCompatibleDC(g_hmemdc);
	SelectObject(hbitmapdc, pImage->hbitmap);
	if (width == -1)width = pImage->width;
	if (height == -1)height = pImage->height;
	SetStretchBltMode(g_hmemdc, COLORONCOLOR);
	StretchBlt(g_hmemdc, x, y, width, height, hbitmapdc, 0, 0, pImage->width, pImage->height, SRCCOPY);
	DeleteDC(hbitmapdc);
}

void putImageTransparent(IMAGE pImage, int x, int y, int width, int height, Color bkColor)
{
	HDC hbitmapdc;
	ACL_ASSERT_BEGIN_PAINT;
	hbitmapdc = CreateCompatibleDC(g_hmemdc);
	SelectObject(hbitmapdc, pImage->hbitmap);
	if (width == -1)width = pImage->width;
	if (height == -1)height = pImage->height;
	//SetStretchBltMode(g_hmemdc,COLORONCOLOR);
	TransparentBlt(g_hmemdc, x, y, width, height, hbitmapdc, 0, 0, pImage->width, pImage->height, bkColor);
	DeleteDC(hbitmapdc);
}

void loadImage( string image, IMAGE mapbuf)
{
	HDC hmapdc;
	IPicture *ipicture;
	IStream *istream;
	DWORD filesize = 0, bytes;
	OLE_XSIZE_HIMETRIC width;
	OLE_YSIZE_HIMETRIC height;
	HANDLE file = NULL;
	HGLOBAL global = NULL;
	LPVOID data = NULL;

	ACL_ASSERT_HWND;

	file = CreateFileA(image, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		acl_error("Fail to load image, File not exist");
	filesize = GetFileSize(file, NULL);

	global = GlobalAlloc(GMEM_MOVEABLE, filesize);
	data = GlobalLock(global);
	ReadFile(file, data, filesize, &bytes, NULL);
	GlobalUnlock(global);
	CreateStreamOnHGlobal(global, TRUE, &istream);

	OleLoadPicture(istream, filesize, TRUE, &IID_IPicture, (LPVOID*)&ipicture);
	ipicture->lpVtbl->get_Width(ipicture, &width);
	ipicture->lpVtbl->get_Height(ipicture, &height);

	mapbuf->width = (int)(width / 26.45833333333);
	mapbuf->height = (int)(height / 26.45833333333);

	hmapdc = CreateCompatibleDC(GetDC(g_hWnd));
	if (mapbuf->hbitmap != NULL)
		DeleteObject(mapbuf->hbitmap);
	mapbuf->hbitmap = CreateCompatibleBitmap(GetDC(g_hWnd), mapbuf->width, mapbuf->height);
	SelectObject(hmapdc, mapbuf->hbitmap);

	ipicture->lpVtbl->Render(ipicture, hmapdc, 0, 0, mapbuf->width, mapbuf->height, 0, height, width, -height, NULL);
	ipicture->lpVtbl->Release(ipicture);
	istream->lpVtbl->Release(istream);

	DeleteDC(hmapdc);
	GlobalFree(global);
	CloseHandle(file);
}

void freeImage(IMAGE mapbuf)
{
	if (mapbuf->hbitmap) return;
	DeleteObject(mapbuf->hbitmap);
	mapbuf->hbitmap = NULL;
}

//call back functions
/*------------------------------------------*/
void registerKeyboardEvent(KeyboardEventCallback callback)
{
	g_keyboard = callback;
}

void registerCharEvent(CharEventCallback callback)
{
	g_char = callback;
}

void registerMouseEvent(MouseEventCallback callback)
{
	g_mouse = callback;
}

void registerTimerEvent(TimerEventCallback callback)
{
	g_timer = callback;
}

void cancelKeyboardEvent()
{
	g_keyboard = NULL;
}

void cancelCharEvent()
{
	g_char = NULL;
}

void cancelMouseEvent()
{
	g_mouse = NULL;
}

void cancelTimerEvent()
{
	g_timer = NULL;
}

//timer functions
/*-----------------------------------------------*/
void startTimer(int id, int timeinterval)
{	
	ACL_ASSERT(g_hWnd, "You must call \"initWindow(...)\" in Main()");
	SetTimer(g_hWnd, id, timeinterval, NULL);
}

void cancelTimer(int id)
{
	KillTimer(g_hWnd, id);
}

void Pause(double seconds)
{
	double finish;
	finish = (double)clock() / CLK_TCK + seconds;
	while (((double)clock() / CLK_TCK) < finish);
}
//sound functions
/*--------------------------------------------------------------*/
void Sound(const char * SoundFileName)
{
	PlaySound(SoundFileName, NULL, SND_FILENAME | SND_ASYNC);
}

int loadSound(const char *fileName, int *pSound)
{
	char *cmdStr;
	int len = strlen(fileName) * sizeof(char);
	len += 64;
	cmdStr = (char*)malloc(len);
	sprintf(cmdStr, "open \"%s\" type mpegvideo alias S%d", fileName, soundID_count);
	*pSound = soundID_count;
	++soundID_count;
	if (mciSendStringA(cmdStr, NULL, 0, NULL) != 0) {
		--soundID_count;
		acl_error("loading sound fail!");
		*pSound = 0;
	}
	free(cmdStr);
	return *pSound;
}

void playSound(int sid, int repeat)
{
	char cmdStr[32];
	stopSound(sid);
	if (repeat)
		sprintf(cmdStr, "play S%d from 0 repeat", sid);
	else
		sprintf(cmdStr, "play S%d from 0", sid);
	if (mciSendStringA(cmdStr, NULL, 0, NULL) != 0) {
		acl_error("play sound fail!");
	}
}

void stopSound(int sid)
{
	char cmdStr[32];
	sprintf(cmdStr, "stop S%d", sid);
	if (mciSendStringA(cmdStr, NULL, 0, NULL) != 0) {
		acl_error("stop sound fail!");
	}
}

double GetXResolution(void)
{
	HWND desktop;
	HDC dc;
	int xdpi;

	//if (initialized) return (xResolution);
	desktop = GetDesktopWindow();
	dc = GetDC(desktop);
	xdpi = GetDeviceCaps(dc, LOGPIXELSX);
	ReleaseDC(desktop, dc);
	return (xdpi);
}

double GetYResolution(void)
{
	HWND desktop;
	HDC dc;
	int ydpi;

	//if (initialized) return (yResolution);
	desktop = GetDesktopWindow();
	dc = GetDC(desktop);
	ydpi = GetDeviceCaps(dc, LOGPIXELSY);
	ReleaseDC(desktop, dc);
	return (ydpi);
}

double ScaleXInches(int x) /*x coordinate from pixels to inches*/
{
	return (double)x / GetXResolution();
}

double ScaleYInches(int y)/*y coordinate from pixels to inches*/
{
	return (double)y / GetYResolution();
}