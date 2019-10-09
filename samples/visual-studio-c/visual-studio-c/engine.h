
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Windows.h>
#include "doublelist.h"
#include "graphics.h"

#define RECT_TYPE 1
#define ELLI_TYPE 2
#define POLY_TYPE 4
#define OTHER_TYPE 8

//event display
#define CHAR_EVE 16
#define MOUSEMOVE_EVE 32
#define MOUSECLICK_EVE 64
#define KEY_EVE 128

#define ALL_EVE 240
#define MOUSEDOUBLECLICK_EVE 112
#define MOUSEDOWN_EVE 48
#define MOUSEUP_EVE 96

#define STRUCTURE(TYPE,obj) ((TYPE)(obj->structure))
#define SOURCE(TYPE,obj) ((TYPE)(obj->source))

typedef struct objo Object;
typedef Object* OBJECT;
typedef void (*paintMethod)(Object* obj);
//operation : an obj use the function to respond the operation; if the return num is 1, it catch the imformation
typedef int (*operation)(Object* obj,int x,int y,int button,int Event,int eventKind);
typedef void (*updateMethod)(Object* obj);

typedef struct {
	int posX;
	int posY;
	int height;
	int width;
	Color color;
	ACL_Brush_Style style;
} rectS;

typedef struct {
	int posX;
	int posY;
	int height;
	int width;
	Color color;
	ACL_Brush_Style style;
} ellipseS;

typedef struct {
	POINT *points;
	int pointCount;
	Color color;
	ACL_Brush_Style style;
} polygonS;

typedef struct {
	void* data;
	Color color;
	ACL_Brush_Style style;
} otherObj;

//flag 后三位是指明 object type ,中间5位是event type
//
struct objo{
	paintMethod paint;
	updateMethod update;
	void* structure;
	operation op;
	void* source;
	List node;
	int flag;
};

typedef rectS* RECTANGLE;
typedef ellipseS* ELLIPSE;
typedef polygonS* POLYGON;
typedef otherObj* OTHERSHAP;

//you need to call
void setup();

//create or delete object
OBJECT createObj(int objType, paintMethod paint, void *structure, operation op, int callType);
void deleteObj(OBJECT obj);

//create some
OBJECT createImageObj(string name, int x, int y, int height, int width,operation op);
OBJECT createTextBox(int x, int y, int height, int width, int maxChar);

//determine whether two object is overlap
bool isOverlap(OBJECT obj1, OBJECT obj2);

//add source to object;
void addsource(OBJECT obj,void* source);

//register default callback functions
void setDefaultKeyboardEvent(KeyboardEventCallback callback);
void setDefaultCharEvent(CharEventCallback callback);
void setDefaultMouseEvent(MouseEventCallback callback);
void setDefaultTimerEvent(TimerEventCallback callback);

//display obj or not
void setNotDisplay(OBJECT obj);
void setDisplay(OBJECT obj);

//setcalls for object
void setCalls(int callType, OBJECT obj);
void releaseCalls(int callType, OBJECT obj);

//set dispaly frequency
void setFrequency(int ms);

//set update method
//calltime is the time point the function is called; times is the call times of the function
//you can use get time to get the current time; 
void setUpdate(OBJECT obj,updateMethod update);
void cancelUpdate(OBJECT obj, updateMethod update);

//get current time: 0.01s
double getTime();
void resetTime();

//create shape structure
RECTANGLE createRect(int x,int y,int height,int width,Color c);
ELLIPSE createEllipse(int x, int y, int height, int width, Color c);
POLYGON createPolygon(POINT *points, Color c, int pointNum);

#endif