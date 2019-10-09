#include "engine.h"
#include "graphics.h"
#include<time.h>
#include <stdio.h>
#include <math.h>

#define TIME_EVE 256
#define DELETE_OBJ 511

typedef struct {
	int height;
	int width;
	int objCount;
	List L;
	KeyboardEventCallback globalKeyEvent;
	CharEventCallback globalCharEvent;
	MouseEventCallback globalMouseEvent;
	TimerEventCallback globalTimerEvent;
	paintMethod paint;
} windowObjs;

static windowObjs windowObj;

bool isMove = FALSE;
bool isAdd = FALSE;

static double timer = 0;
static int frequency = 30;

List charCalls;
List keyCalls;
List timerCalls;
List MouseMoveCalls;
List MouseClickCalls;

//private function declare
/*----------------------------------------------------------------*/
static void freeObj(OBJECT obj);
static void paintObj(OBJECT obj);
static void checkUpdate();
static bool inEllip(int x0, int y0, ELLIPSE e);
static bool inBox(int x0, int y0, RECTANGLE r);
static bool inPoly(int x0, int y0, POLYGON p);
static int checkObjSendEve(List L, int x, int y, int buttom, int event, int eventKind);
static int sendEventProcess(List L, int x, int y, int buttom, int event, int eventKind);

//public functions
/*--------------------------------------------------------------------*/

OBJECT createObj(int objType, paintMethod paint, void *structure ,operation op, int EventFlag) {
	OBJECT obj = (OBJECT)malloc(sizeof(Object));
	obj->op = op;
	obj->source = NULL;
	obj->structure = structure;
	obj->paint = paint;
	obj->flag = objType;
	obj->update = NULL;
	obj->node = Insert_list_node(windowObj.L, obj);
	setCalls(EventFlag, obj);
	return obj;
}

void deleteObj(OBJECT obj) {
	releaseCalls(DELETE_OBJ, obj);
	freeObj(obj);
}

void addsource(OBJECT obj,void* source) {
	obj->source = source;
}

void setNotDisplay(OBJECT obj) {
	Delete_list_node(obj->node);
}

void setDisplay(OBJECT obj) {
	obj->node = Insert_list_node(windowObj.L, obj);
}

static void paintImage(OBJECT obj) {
	putImageScale(obj->source, STRUCTURE(RECTANGLE, obj)->posX, STRUCTURE(RECTANGLE, obj)->posY, STRUCTURE(RECTANGLE, obj)->width, STRUCTURE(RECTANGLE, obj)->height);
}

//create some components

OBJECT createImageObj(string name, int x, int y, int height, int width, operation op) {
	Image i;
	loadImage(name, i);
	RECTANGLE r = createRect(x, y, height, width, WHITE);
	OBJECT obj = createObj(RECT_TYPE, paintImage, r, op, CHAR_EVE);
	addsource(obj,i);
	return obj;
}

static void paintTextBox(OBJECT obj) {
	drawTextBox(STRUCTURE(RECTANGLE, obj)->posX, STRUCTURE(RECTANGLE, obj)->posY,
		STRUCTURE(RECTANGLE, obj)->posX + STRUCTURE(RECTANGLE, obj)->width,
		STRUCTURE(RECTANGLE, obj)->posY + STRUCTURE(RECTANGLE, obj)->height, obj->source, DT_CENTER);
}

static int TextBoxop(Object* obj, int x, int y, int button, int Event, int eventKind) {
		int l;	
	switch (eventKind) {
	case CHAR_EVE:
		l = strlen((string)(obj->source));
		((string)(obj->source))[l] = button;
		((string)(obj->source))[l + 1] = 0;
	}
	return 0;
}

OBJECT createTextBox(int x, int y, int height, int width,int maxChar) {
	RECTANGLE r = createRect(x, y,height,width ,WHITE );
	OBJECT obj = createObj(RECT_TYPE, paintTextBox, r, TextBoxop, CHAR_EVE);
	addsource(obj, malloc(sizeof(char)*maxChar + 5));
	return obj;
}

bool isOverlap(OBJECT obj1, OBJECT obj2) {
	double cx2, cy2, cx1, cy1,r;//center of one ellipse
	OBJECT t;
	switch ((int)obj1->flag & 7 | (int)obj2->flag & 7) {
	case 1:// both retangles have no vertex in another one
		return (inBox(STRUCTURE(RECTANGLE, obj1)->posX, STRUCTURE(RECTANGLE, obj1)->posY, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(STRUCTURE(RECTANGLE, obj1)->posX + STRUCTURE(RECTANGLE, obj1)->width, STRUCTURE(RECTANGLE, obj1)->posY + STRUCTURE(RECTANGLE, obj1)->height, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(STRUCTURE(RECTANGLE, obj1)->posX + STRUCTURE(RECTANGLE, obj1)->width, STRUCTURE(RECTANGLE, obj1)->posY, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(STRUCTURE(RECTANGLE, obj1)->posX, STRUCTURE(RECTANGLE, obj1)->posY + STRUCTURE(RECTANGLE, obj1)->height, STRUCTURE(RECTANGLE, obj2)) ||

			inBox(STRUCTURE(RECTANGLE, obj2)->posX, STRUCTURE(RECTANGLE, obj2)->posY, STRUCTURE(RECTANGLE, obj1)) ||
			inBox(STRUCTURE(RECTANGLE, obj2)->posX + STRUCTURE(RECTANGLE, obj2)->width, STRUCTURE(RECTANGLE, obj2)->posY + STRUCTURE(RECTANGLE, obj2)->height, STRUCTURE(RECTANGLE, obj1)) ||
			inBox(STRUCTURE(RECTANGLE, obj2)->posX + STRUCTURE(RECTANGLE, obj2)->width, STRUCTURE(RECTANGLE, obj2)->posY, STRUCTURE(RECTANGLE, obj1)) ||
			inBox(STRUCTURE(RECTANGLE, obj2)->posX, STRUCTURE(RECTANGLE, obj2)->posY + STRUCTURE(RECTANGLE, obj2)->height, STRUCTURE(RECTANGLE, obj1))
			);
	case 2:// the border point in the line from one ellipse center to another center is not in another shape
		cx2 = STRUCTURE(ELLIPSE, obj2)->posX + STRUCTURE(ELLIPSE, obj2)->width / 2.0;
		cy2 = STRUCTURE(ELLIPSE, obj2)->posY + STRUCTURE(ELLIPSE, obj2)->height / 2.0;
		cx1 = STRUCTURE(ELLIPSE, obj1)->posX + STRUCTURE(ELLIPSE, obj1)->width / 2.0;
		cy1 = STRUCTURE(ELLIPSE, obj1)->posY + STRUCTURE(ELLIPSE, obj1)->height / 2.0;
		r = sqrt((cx1-cx2)*(cx1 - cx2)+ (cy1 - cy2)*(cy1 - cy2));
		return inEllip(cx1 + (cx2 - cx1) / r * STRUCTURE(ELLIPSE, obj1)->width / 2.0, cy1 + (cy2 - cy1) / r * STRUCTURE(ELLIPSE, obj1)->height / 2.0, STRUCTURE(ELLIPSE, obj2));
	case 3:
		if ((int)obj2->flag & 7 == ELLI_TYPE) {
			t = obj2;
			obj2 = obj1;
			obj1 = t;
		}
		cx1 = STRUCTURE(ELLIPSE, obj1)->posX + STRUCTURE(ELLIPSE, obj1)->width / 2.0;
		cy1 = STRUCTURE(ELLIPSE, obj1)->posY + STRUCTURE(ELLIPSE, obj1)->height / 2.0;
		return (inBox(STRUCTURE(ELLIPSE, obj1)->posX, cy1, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(STRUCTURE(ELLIPSE, obj1)->posX + STRUCTURE(ELLIPSE, obj1)->width, cy1, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(cx1, STRUCTURE(ELLIPSE, obj1)->posY, STRUCTURE(RECTANGLE, obj2)) ||
			inBox(cx1, STRUCTURE(ELLIPSE, obj1)->posY + STRUCTURE(ELLIPSE, obj1)->height, STRUCTURE(RECTANGLE, obj2)) ||

			inEllip(STRUCTURE(RECTANGLE, obj2)->posX, STRUCTURE(RECTANGLE, obj2)->posY, STRUCTURE(ELLIPSE, obj1)) ||
			inEllip(STRUCTURE(RECTANGLE, obj2)->posX + STRUCTURE(RECTANGLE, obj2)->width, STRUCTURE(RECTANGLE, obj2)->posY + STRUCTURE(RECTANGLE, obj2)->height, STRUCTURE(ELLIPSE, obj1)) ||
			inEllip(STRUCTURE(RECTANGLE, obj2)->posX + STRUCTURE(RECTANGLE, obj2)->width, STRUCTURE(RECTANGLE, obj2)->posY, STRUCTURE(ELLIPSE, obj1)) ||
			inEllip(STRUCTURE(RECTANGLE, obj2)->posX, STRUCTURE(RECTANGLE, obj2)->posY + STRUCTURE(RECTANGLE, obj2)->height, STRUCTURE(ELLIPSE, obj1))
			);
	case 4:
	case 5:
	case 6:
		break;
	}
	return FALSE;
}

//register callback functions
void setDefaultKeyboardEvent(KeyboardEventCallback callback) {
	windowObj.globalKeyEvent = callback;
}

void setDefaultCharEvent(CharEventCallback callback){
	windowObj.globalCharEvent = callback;
}
void setDefaultMouseEvent(MouseEventCallback callback) {
	windowObj.globalMouseEvent = callback;
}

void setDefaultTimerEvent(TimerEventCallback callback) {
	windowObj.globalTimerEvent = callback;
}

//set update method
void setUpdate(OBJECT obj,updateMethod update) {
	obj->update = update;
	setCalls(TIME_EVE, obj);
}

void cancelUpdate(OBJECT obj, updateMethod update) {
	obj->update = NULL;
	releaseCalls(TIME_EVE, obj);
}

void setCalls(int callType, OBJECT obj) {
	if (callType & CHAR_EVE) {
		Insert_list_node(charCalls, obj);
	}
	if (callType & MOUSEMOVE_EVE) {
		Insert_list_node(MouseMoveCalls, obj);
	}
	if (callType & MOUSECLICK_EVE) {
		Insert_list_node(MouseClickCalls, obj);
	}
	if (callType & KEY_EVE) {
		Insert_list_node(keyCalls, obj);
	}
	if (callType & CHAR_EVE) {
		Insert_list_node(charCalls, obj);
	}
	if (callType & TIME_EVE) {
		Insert_list_node(timerCalls, obj);
	}
}

//懒惰删除保证时间复杂度
void releaseCalls(int callType, OBJECT obj) {
	if (callType & DELETE_OBJ) {
		List node;
		if(node= Find_node(windowObj.L, obj))
			Delete_list_node( node );
		if (node = Find_node(charCalls, obj))
			Delete_list_node(node);
		if (node = Find_node(keyCalls, obj))
			Delete_list_node(node);
		if (node = Find_node(MouseMoveCalls, obj))
			Delete_list_node(node);
		if (node = Find_node(MouseClickCalls, obj))
			Delete_list_node(node);
		return;
	}
	if (callType & CHAR_EVE) {
		obj->flag |= CHAR_EVE;
	}
	if (callType & MOUSEMOVE_EVE) {
		obj->flag |= MOUSEMOVE_EVE;
	}
	if (callType & MOUSECLICK_EVE) {
		obj->flag |= MOUSECLICK_EVE;
	}
	if (callType & KEY_EVE) {
		obj->flag |= KEY_EVE;
	}
	if (callType & TIME_EVE) {
		obj->flag |= TIME_EVE;
	}
}

void setFrequency(int ms) {
	cancelTimer(0);
	frequency = ms;
	startTimer(0, ms);
}

RECTANGLE createRect(int x, int y, int height, int width, Color c) {
	RECTANGLE rect = (RECTANGLE)malloc(sizeof(rectS));
	rect->posX = x;
	rect->posY = y;
	rect->width = width;
	rect->height = height;
	rect->color = c;
	return rect;
}

ELLIPSE createEllipse(int x, int y, int height, int width, Color c) {
	ELLIPSE rect = (ELLIPSE)malloc(sizeof(ellipseS));
	rect->posX = x;
	rect->posY = y;
	rect->width = width;
	rect->height = height;
	rect->color = c;
	return rect;
}

POLYGON createPolygon(POINT *points, Color c,int pointNum) {
	POLYGON p = (POLYGON)malloc(sizeof(polygonS));
	p->color = c;
	p->pointCount = pointNum;
	p->points = (POINT*)malloc(sizeof(POINT)*pointNum);
	int i;
	for (i = 0; i < pointNum; ++i)
		p->points[i] = points[i];
	return p;
}

double getTime() {
	return timer;
}

void resetTime() {
	timer=0;
}

//private functions
/*-------------------------------------------*/
static void freeObj(OBJECT obj) {
	if (obj->structure != NULL)
		free(obj->structure);
	if (obj->source != NULL)
		free(obj->source);
	free(obj);
}

static void paintObj(OBJECT obj) {
	static Color c;
	if ((int)obj->flag & 7) {
		switch ((int)obj->flag & 7) {
		case RECT_TYPE:
			if (c != ((RECTANGLE)(obj->structure))->color) 
				setBrushColor(c = ((RECTANGLE)(obj->structure))->color);	
			rectangle(((RECTANGLE)(obj->structure))->posX, ((RECTANGLE)(obj->structure))->posY,
				((RECTANGLE)(obj->structure))->posX + ((RECTANGLE)(obj->structure))->height,
				((RECTANGLE)(obj->structure))->posY + ((RECTANGLE)(obj->structure))->height);
			break;
		case ELLI_TYPE:
			if (c != ((ELLIPSE)(obj->structure))->color) 				
				setBrushColor(c = ((ELLIPSE)(obj->structure))->color);
			ellipse(((ELLIPSE)(obj->structure))->posX, ((ELLIPSE)(obj->structure))->posY,
				((ELLIPSE)(obj->structure))->posX + ((ELLIPSE)(obj->structure))->height,
				((ELLIPSE)(obj->structure))->posY + ((ELLIPSE)(obj->structure))->height);
			break;
		case POLY_TYPE:
			if (c != ((POLYGON)(obj->structure))->color)
				setBrushColor(c = ((POLYGON)(obj->structure))->color);
			polygon(((POLYGON)(obj->structure))->points, ((POLYGON)(obj->structure))->pointCount);
			break;
		case OTHER_TYPE:
			break;
		}
	}
	if(obj->paint)
		(obj->paint)(obj);
}

static int sendEventProcess(List L,int x,int y,int buttom,int event, int eventKind) {
	List p,d;
	for (p = L->Next; p->Next!= NULL;) {
		if (eventKind & ((OBJECT)p->Data)->flag) {
			d = p;
			p = p->Next;
			Delete_list_node(d);
			continue;
		}else if(((OBJECT)p->Data)->op!=NULL)
		if((((OBJECT)p->Data)->op)((OBJECT)p->Data, x, y, buttom, event,eventKind)) return 1;
		p = p->Next;
	}
	return 0;
}

static void paintScreen() {
	List p;
	for (p = windowObj.L->Next; p->Next != NULL; p = p->Next) {
		paintObj((OBJECT)p->Data);
	}
}

static void KeyboardEventProcess(int key, int event)/*每当产生键盘消息，都要执行*/
{	
	if(sendEventProcess(keyCalls, 0, 0, key, event, KEY_EVE)) return;
	if (windowObj.globalKeyEvent)
		(windowObj.globalKeyEvent)(key, event);
}

static void CharEventProcess(char c)
{
	if(sendEventProcess(charCalls, 0, 0, c, 0, CHAR_EVE)) return;
	if (windowObj.globalCharEvent)
		(windowObj.globalCharEvent)(c);
}

static bool inBox(int x0, int y0, RECTANGLE r)
{	
	return ( x0 >= r->posX && x0 <= r->posX+r->width && y0 >= r->posY && y0 <= r->posY+r->height );
}

static bool inEllip(int x0, int y0, ELLIPSE e)
{	
	double a = e->width / 2.0, b = e->height / 2.0;
	double x = e->posX + a, y = e->posY + b;
	return ( (x0-x)*(x0 - x)/a/a + (y0 - y)*(y0 - y)/b/b <= 1 );
}

static bool inPoly(int x0,int y0,POLYGON p) {

}

static int checkObjSendEve(List L, int x, int y, int buttom, int event, int eventKind) {
	List p,d;
	OBJECT obj;
	RECTANGLE r;
	ELLIPSE e;
	for (p = L->Next; p->Next != NULL;) {
		obj = ((OBJECT)p->Data);
		if (eventKind & obj->flag) {
			d = p;
			p = p->Next;
			Delete_list_node(d);
			continue;
		}
		if (obj->structure == NULL)
			continue;
		switch (((OBJECT)p->Data)->flag & 7) {
		case RECT_TYPE:
			r = ((RECTANGLE)(obj->structure));
			if (obj->op != NULL && inBox(x, y, r))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		case ELLI_TYPE:
			e = ((ELLIPSE)(obj->structure));
			if (obj->op != NULL && inEllip(x, y,e))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		case POLY_TYPE:
			if (obj->op != NULL && inPoly(x,y,((POLYGON)(obj->structure))))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		}
		p = p->Next;
	}
	return 0;
}

static void MouseEventProcess(int x, int y, int button, int event)
{
	switch (event) {
	case BUTTON_DOWN:
		if(checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEDOWN_EVE)) return;
		break;
	case BUTTON_DOUBLECLICK:
		if(checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEDOUBLECLICK_EVE)) return;
		break;
	case BUTTON_UP:
		if(checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEUP_EVE)) return;
		break;
	case MOUSEMOVE:
		if(checkObjSendEve(MouseMoveCalls, x, y, button, event, MOUSEMOVE_EVE)) return;
		break;
	}
	if (windowObj.globalMouseEvent)
		windowObj.globalMouseEvent(x, y, button, event);
}

static void checkUpdate() {

	List p,d;
	for (p = timerCalls->Next; p->Next != NULL;) {
		if (TIME_EVE & ((OBJECT)p->Data)->flag) {
			d = p;
			p = p->Next;
			Delete_list_node(d);
			continue;
		}
		else if (((OBJECT)p->Data)->update != NULL)
			(((OBJECT)p->Data)->update)((OBJECT)p->Data);
		p = p->Next;
	}
	beginPaint();
	clearDevice();
	paintScreen();
	endPaint();
}

static void TimerEventProcess(int timerID)
{	
	switch (timerID) {
	case 0: 
		timer += (double)frequency / 1000;
		checkUpdate();
		break;
	default:
		break;
	}
}

int main()
{
	windowObj.L= createEmptyList();
	charCalls=createEmptyList();
	keyCalls = createEmptyList();
	timerCalls= createEmptyList();
	MouseMoveCalls = createEmptyList();
	MouseClickCalls=createEmptyList();

	registerKeyboardEvent(KeyboardEventProcess);/*注册键盘消息回调函数*/
	registerCharEvent(CharEventProcess);/*注册字符消息回调函数*/
	registerMouseEvent(MouseEventProcess);/*注册鼠标消息回调函数*/
	registerTimerEvent(TimerEventProcess);/*注册定时器消息回调函数*/

	createMyMenu();

	setup();

	initWindow("Window", DEFAULT, DEFAULT, 640, 480);
	startTimer(0, frequency);
	//startTimer(1, 10);

	return 0;
}
