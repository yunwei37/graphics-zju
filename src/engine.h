
#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Windows.h>
#include "doublelist.h"
#include "graphics.h"

//defination
/*--------------------------------------------------------------------------*/
//the defination of object type;
#define RECT_TYPE 1
#define ELLI_TYPE 2
#define POLY_TYPE 4
#define OTHER_TYPE 8
#define ALL_TYPE 15 //the sum of above

//event call flag: used in delete object and processing event;
#define CHAR_EVE 16
#define MOUSEMOVE_EVE 32
#define MOUSECLICK_EVE 64
#define KEY_EVE 128

#define ALL_EVE 240 //the sum of above

//another three kinds of mouse click event
#define MOUSEDOUBLECLICK_EVE 512
#define MOUSEDOWN_EVE 1024
#define MOUSEUP_EVE 2048

//use to get the data in the object
#define STRUCTURE(TYPE,obj) ((TYPE)(obj->structure))
#define SOURCE(TYPE,obj) ((TYPE)(obj->source))
#define STYLE(TYPE,structure) ((TYPE)structure->style)

//use for get the type flag of an obj
#define TYPEFLAG(obj) (obj->flag&ALL_TYPE)

//object type pointer
typedef struct objo Object;
typedef Object* OBJECT;

//three kinds of callback function pointers declare
typedef void (*paintMethod)(Object* obj);//paint 
//operation : an obj use the function to respond the operation; if the return num is 1, it catch the imformation;
//if 0, the signal is pass to another object;
typedef int (*operation)(Object* obj,int x,int y,int button,int Event,int eventKind);//the operation of an object
typedef void (*updateMethod)(Object* obj);//the update method for an object to react with time

//the rectangle structrure
typedef struct {
	int posX;
	int posY;
	int height;
	int width;
	Color color;
	ACL_Brush_Style style;
	bool hasframe;
} rectS;

//the ellipseS structrure: nearly the same as the rectangle structrure
typedef struct {
	int posX;
	int posY;
	int height;
	int width;
	Color color;
	ACL_Brush_Style style;
	bool hasframe;
} ellipseS;

//the polygon structure
typedef struct {
	POINT *points;
	int pointCount;
	Color color;
	ACL_Brush_Style style;
	bool hasframe;
} polygonS;

//the suructure for other obj type
typedef struct {
	void* data;
	Color color;
	ACL_Brush_Style style;
} otherObj;

//the struct of an obj
struct objo{
	paintMethod paint; //the additional paint method
	updateMethod update; //the pointer for update function
	void* structure; //the structure of the obj, like the rectangle struct
	operation op; //the operation pointer
	void* source; //the additional source for some data,like pictures,string, you can define your own data struct
	List node; //the list node for display
	int flag; //flag 后四位是指明 object type ,中间5位是event type
};

//the set of objects
typedef struct {
	OBJECT *objs;
	int num;
	int maxSize;
} objectSet;

//the struct source of the text box
typedef struct {
	int length;
	bool editable;
	string buffer;
	operation OriOp;
} textboxStruct;

//pointer defines
typedef objectSet* OBJECTS;
typedef rectS* RECTANGLE;
typedef ellipseS* ELLIPSE;
typedef polygonS* POLYGON;
typedef otherObj* OTHERSHAP;

//slot define
typedef struct {
	List L;
	OBJECT obj;
} signalS ;
typedef signalS* SIGNAL;

//signal define
typedef struct{
	OBJECT obj;
	void(*receiver)(OBJECT obj, void* message);
} slotS ;
typedef slotS* SLOT;

//api
/*------------------------------------------------------------------------------*/
//you need to call to init your programe
void setup();

//create or delete object
OBJECT createObj(int objType, paintMethod paint, void *structure, operation op, int callType);
void deleteObj(OBJECT obj);

//functions for object set
OBJECTS createObjSet();
void addSetObj(OBJECTS objset, OBJECT obj);
void deleteObjSet(OBJECTS objset);
OBJECT getObjInSet(OBJECTS objset, int i);//get the OBJ at pos i;
int getObjIndex(OBJECTS objset, OBJECT obj);
void deleteObjInSet(OBJECTS objset,OBJECT obj);

//create some components, or add some property to an object
OBJECT createImageObj(string name, int x, int y, int height, int width,operation op);
OBJECT setTextBox(OBJECT obj, string name, int maxChar, bool editable);
void changeTextBox(OBJECT textBox, string text, bool editable);

//create drawers: using to draw some simple shapes without react with operation and will not change
OBJECT createDrawers(paintMethod paint, void *structure);

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
//you can use getTime() to get the current time; 
void setUpdate(OBJECT obj,updateMethod update);
void cancelUpdate(OBJECT obj);

// add additional paint method to an object
void addpaintMethod(OBJECT obj, paintMethod paint);
void cancelpaintMethod(OBJECT obj);

//change the operation function of an object
void changeObjectOp(OBJECT obj, operation op);

//get current time: 0.01s
double getTime();
void resetTime();

//create shape structures
RECTANGLE createRect(int x,int y,int height,int width, Color c);
ELLIPSE createEllipse(int x, int y, int height, int width, Color c);
ELLIPSE createRound(int x, int y, int r, Color c);
POLYGON createPolygon(POINT *points, Color c, int pointNum);

//slot and signal
SLOT createSlot(OBJECT obj, void(*receiver)(OBJECT obj, void* message));
SIGNAL createSignal(OBJECT obj);
void connectSS(SLOT slot,SIGNAL signal);
void sentSignal(SIGNAL signal,void* message);
void disconnectSS(SLOT slot, SIGNAL signal);

void deleteSlot(SLOT slot);
void deleteSignal(SIGNAL signal);
#endif