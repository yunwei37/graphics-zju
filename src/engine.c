#include "engine.h"
#include "graphics.h"
#include<time.h>
#include <stdio.h>
#include <math.h>

#define TIME_EVE 256
#define DELETE_OBJ 511

//the struct of the window
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

//the struct source of the image
typedef struct {
	string name;
	IMAGE i;
} imageStruct;

//window obj
static windowObjs windowObj;

//future for speed up;
bool isMove = FALSE;
bool isAdd = FALSE;

//the global time: ues gettime function to get it
//count in seconds, when the program is start the timer is set to 0;
static double timer = 0;

//the ms between the screen is fresh or objects been update
static int frequency = 30;

//the list of calls
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
static int TextBoxop(Object* obj, int x, int y, int button, int Event, int eventKind);
static textboxStruct* createTBS(string name, int maxChar, bool editable);
static void paintImage(OBJECT obj);
static void paintTextBox(OBJECT obj);
static void delObjCall(OBJECT obj);

//public functions
/*--------------------------------------------------------------------*/
/* part1: obj and obj sets */
//create an obj
OBJECT createObj(int objType, paintMethod paint, void *structure, operation op, int EventFlag) {
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

//delete an obj
void deleteObj(OBJECT obj) {
	delObjCall(obj);
	freeObj(obj);
}

//create an obj set
OBJECTS createObjSet() {
	OBJECTS objs = (OBJECTS)malloc(sizeof(objectSet));
	objs->num = 0;
	objs->maxSize = 10;
	objs->objs = (OBJECT*)malloc(sizeof(OBJECT) * 10);
	return objs;
}

// add obj to the obj set
void addSetObj(OBJECTS objset, OBJECT obj) {
	if (objset->maxSize == objset->num) { //copy and double the space: like cpp vector
		OBJECT *t = objset->objs;
		objset->objs = (OBJECT*)malloc(sizeof(OBJECT) * 2 * objset->maxSize);
		int i;
		for (i = 0; i < objset->num; ++i)
			objset->objs[i] = t[i];
		free(t);
		objset->maxSize *= 2;
	}
	objset->objs[objset->num++] = obj;
}

//delete an obj set and all the obj in it
void deleteObjSet(OBJECTS objset) {
	int i;
	for (i = 0; i < objset->num; ++i)
		deleteObj(objset->objs[i]);
	free(objset->objs);
	free(objset);
}

//get the obj in ith pos
OBJECT getObjInSet(OBJECTS objset, int i) {
	if (i < 0 || i >= objset->num) {
		return NULL;
	}
	return objset->objs[i];
}

int getObjIndex(OBJECTS objset, OBJECT obj) {
	int i;
	for (i = 0; i < objset->num; ++i)
		if (objset->objs[i] == obj)
			break;
	if (i == objset->num)	return -1;
	return i;
}

//delete an obj in an objset
void deleteObjInSet(OBJECTS objset, OBJECT obj) {
	int i= getObjIndex(objset,obj);
	if (i == -1)	return;
	deleteObj(obj);
	for (++i; i < objset->num; ++i)
		objset->objs[i - 1] = objset->objs[i];
	objset->num--;
}

/*------------------------------------------------------------------------*/
/* part2: create some components or set the obj some propertys */

//create an image obj with operation and position
OBJECT createImageObj(string name, int x, int y, int height, int width, operation op) {
	RECTANGLE r = createRect(x, y, height, width, WHITE);
	OBJECT obj = createObj(RECT_TYPE, paintImage, r, op, CHAR_EVE);
	imageStruct* i = (imageStruct*)malloc(sizeof(imageStruct));
	i->name = name;
	i->i = malloc(sizeof(Image));
	addsource(obj, i);
	return obj;
}

//after call setTextBox function to change the obj into a text box,
//you can change the text string or the editable flag
void changeTextBox(OBJECT textBox, string text, bool editable) {
	strcpy(SOURCE(textboxStruct*, textBox)->buffer, text);
	if (!editable && SOURCE(textboxStruct*, textBox)->editable) {
		SOURCE(textboxStruct*, textBox)->editable = FALSE;
		if (textBox->op && SOURCE(textboxStruct*, textBox)->OriOp);
			textBox->op=SOURCE(textboxStruct*, textBox)->OriOp;
		SOURCE(textboxStruct*, textBox)->OriOp = NULL;		
		releaseCalls(CHAR_EVE, textBox);
	}
	else if (editable && !SOURCE(textboxStruct*, textBox)->editable) {
		SOURCE(textboxStruct*, textBox)->editable = TRUE;
		SOURCE(textboxStruct*, textBox)->OriOp = textBox->op;
		textBox->op = TextBoxop;
		setCalls(CHAR_EVE, textBox);
	}
}

//change an obj into a text box:
//the default operation of the obj is save in the text box struct and can be recover
//using changeTextBox function
OBJECT setTextBox(OBJECT obj, string name, int maxChar, bool editable) {
	if (editable)
		setCalls(CHAR_EVE, obj);
	textboxStruct* tbs = createTBS(name, maxChar, editable);
	obj->paint = paintTextBox;
	addsource(obj, tbs);
	if (editable) {
		tbs->OriOp = obj->op;
		obj->op = TextBoxop;
	}
	else {
		tbs->OriOp = NULL;
	}
	return obj;
}

//create unvisible drawers to draw some lines or curves
//if you want, any thing can be draw in this function
OBJECT createDrawers(paintMethod paint, void *structure) {
	OBJECT obj = createObj(OTHER_TYPE, paint, structure, NULL, 0);
	return obj;
}

//whether the two objects is overlaping each other
//support ellips type and rectangle type currently
bool isOverlap(OBJECT obj1, OBJECT obj2) {
	double cx2, cy2, cx1, cy1, r;//center of one ellipse
	OBJECT t;
	switch ((int)obj1->flag & ALL_TYPE | (int)obj2->flag & ALL_TYPE) {
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
		r = sqrt((cx1 - cx2)*(cx1 - cx2) + (cy1 - cy2)*(cy1 - cy2));
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

/*--------------------------------------------------------------*/
//part3: register callback functions or add add some additional source to an obj

//set default events: is call after all object event is sent
void setDefaultKeyboardEvent(KeyboardEventCallback callback) {
	windowObj.globalKeyEvent = callback;
}

void setDefaultCharEvent(CharEventCallback callback) {
	windowObj.globalCharEvent = callback;
}

void setDefaultMouseEvent(MouseEventCallback callback) {
	windowObj.globalMouseEvent = callback;
}

void setDefaultTimerEvent(TimerEventCallback callback) {
	windowObj.globalTimerEvent = callback;
}

// add source to an obj
void addsource(OBJECT obj, void* source) {
	obj->source = source;
}

// set the obj not displayed
void setNotDisplay(OBJECT obj) {
	Delete_list_node(obj->node);
}

// set the obj displayed
void setDisplay(OBJECT obj) {
	obj->node = Insert_list_node(windowObj.L, obj);
}

//set update method
void setUpdate(OBJECT obj, updateMethod update) {
	obj->update = update;
	setCalls(TIME_EVE, obj);
}

//cancel update method
void cancelUpdate(OBJECT obj) {
	obj->update = NULL;
	releaseCalls(TIME_EVE, obj);
}

//add paint method
void addpaintMethod(OBJECT obj, paintMethod paint) {
	obj->paint = paint;
}

//cancel paint method
void cancelpaintMethod(OBJECT obj) {
	obj->paint = NULL;
}

void changeObjectOp(OBJECT obj, operation op) {
	obj->op = op;
}

//set call to and obj:
void setCalls(int callType, OBJECT obj) {
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
//cancel calls of an obj
void releaseCalls(int callType, OBJECT obj) {
	if (callType & CHAR_EVE) {
		obj->flag |= CHAR_EVE;
	}
	if (callType & MOUSEMOVE_EVE) {
		obj->flag |= MOUSEMOVE_EVE;
	}
	if (callType & MOUSECLICK_EVE) {
		obj->flag |= MOUSEDOUBLECLICK_EVE | MOUSEDOWN_EVE | MOUSEUP_EVE;
	}
	if (callType & KEY_EVE) {
		obj->flag |= KEY_EVE;
	}
	if (callType & TIME_EVE) {
		obj->flag |= TIME_EVE;
	}
}

//set the update frequency
void setFrequency(int ms) {
	cancelTimer(0);
	frequency = ms;
	startTimer(0, ms);
}

//get the global time or reset 

double getTime() {
	return timer;
}

void resetTime() {
	timer = 0;
}

/*------------------------------------------------------------------*/
//part4: create shape structures

//create rectangle shape
RECTANGLE createRect(int x, int y, int height, int width, Color c) {
	RECTANGLE rect = (RECTANGLE)malloc(sizeof(rectS));
	rect->posX = x;
	rect->posY = y;
	rect->width = width;
	rect->height = height;
	rect->color = c;
	rect->style = BRUSH_STYLE_SOLID; //deafult style is solid;
	rect->hasframe = TRUE;
	return rect;
}

//create ellipse shape
ELLIPSE createEllipse(int x, int y, int height, int width, Color c) {
	ELLIPSE rect = (ELLIPSE)malloc(sizeof(ellipseS));
	rect->posX = x;
	rect->posY = y;
	rect->width = width;
	rect->height = height;
	rect->color = c;
	rect->style = BRUSH_STYLE_SOLID;
	rect->hasframe = TRUE;
	return rect;
}

//create round shape
ELLIPSE createRound(int x, int y, int r, Color c) {
	ELLIPSE rect = (ELLIPSE)malloc(sizeof(ellipseS));
	rect->posX = x;
	rect->posY = y;
	rect->width = 2*r;
	rect->height = 2*r;
	rect->color = c;
	rect->style = BRUSH_STYLE_SOLID;
	rect->hasframe = TRUE;
	return rect;
}

//create Polygon shape
POLYGON createPolygon(POINT *points, Color c, int pointNum) {
	POLYGON p = (POLYGON)malloc(sizeof(polygonS));
	p->color = c;
	p->pointCount = pointNum;
	p->points = (POINT*)malloc(sizeof(POINT)*pointNum);
	p->style = BRUSH_STYLE_SOLID;
	int i;
	for (i = 0; i < pointNum; ++i)
		p->points[i] = points[i];
	p->hasframe = TRUE;
	return p;
}

//part5: slot and signal;
/*----------------------------------------------------------------------------*/

//create a slot
SLOT createSlot(OBJECT obj, void(*receiver)(OBJECT obj, void* message)){
	SLOT s = (SLOT)malloc(sizeof(slotS));
	s->obj = obj;
	s->receiver = receiver;
	return s;
}

//create a signal
SIGNAL createSignal(OBJECT obj) {
	SIGNAL s = (SIGNAL)malloc(sizeof(signalS));
	s->obj = obj;
	s->L = createEmptyList();
	return s;
}

//connect a slot and a signal
void connectSS(SLOT slot, SIGNAL signal) {
	if (signal == NULL || slot == NULL) return;
	Insert_list_node(signal->L, slot);
}

//send a signal
void sentSignal(SIGNAL signal, void* message) {
	if (signal == NULL) return;
	List p;
	for (p = signal->L->Next; p->Next != NULL; p = p->Next) {
		if (((SLOT)(p->Data))->receiver != NULL)
			(((SLOT)(p->Data))->receiver)(((SLOT)(p->Data))->obj, message);
	}
}

//disconnect the slot and signal
void disconnectSS(SLOT slot, SIGNAL signal) {
	if (signal == NULL || slot==NULL) return;
	List node;
	while (node = Find_node(signal->L, slot))
		Delete_list_node(node);
}

//delete the slot: must call disconnect first;
void deleteSlot(SLOT slot) {
	if (slot == NULL) return;
	free(slot);
}

//delete the signal: must call disconnect first;
void deleteSignal(SIGNAL signal) {
	if (signal == NULL) return;
	delete_list(signal->L);
	free(signal);
}

//private functions
/*--------------------------------------------------------------------------*/

//free the space of an obj
static void freeObj(OBJECT obj) {
	if (obj->structure != NULL)
		free(obj->structure);
	//if (obj->source != NULL)
	//	free(obj->source);
	free(obj);
}

//paint the obj;
//change the Brush_Style or the color only if needed to speed up the program
//support the three types currently
static void paintObj(OBJECT obj) {
	static Color c,pc;
	static ACL_Brush_Style b;//static : record the current state of brush color and style
	ACL_Pen_Style ps;
	if ((int)obj->flag & ALL_TYPE) {
		switch ((int)obj->flag & ALL_TYPE) {
		case RECT_TYPE:
			ps = GetPenStyle();
			pc = GetPenColor();
			if (((RECTANGLE)(obj->structure))->hasframe) {
				setPenStyle(PEN_STYLE_SOLID);
				setPenColor(BLACK);
			}else
				setPenStyle(PEN_STYLE_NULL);
			if (c != ((RECTANGLE)(obj->structure))->color) //change if necessary
				setBrushColor(c = ((RECTANGLE)(obj->structure))->color);
			if (b != ((RECTANGLE)(obj->structure))->style)
				setBrushStyle(b = ((RECTANGLE)(obj->structure))->style);
			rectangle(((RECTANGLE)(obj->structure))->posX, ((RECTANGLE)(obj->structure))->posY,
				((RECTANGLE)(obj->structure))->posX + ((RECTANGLE)(obj->structure))->width,
				((RECTANGLE)(obj->structure))->posY + ((RECTANGLE)(obj->structure))->height);
			setPenStyle(ps);
			setPenColor(pc);
			break;
		case ELLI_TYPE:
			ps = GetPenStyle();
			pc = GetPenColor();
			if (((ELLIPSE)(obj->structure))->hasframe) {
				setPenStyle(PEN_STYLE_SOLID);
				setPenColor(BLACK);
			}
			else
				setPenStyle(PEN_STYLE_NULL);
			if (c != ((ELLIPSE)(obj->structure))->color)
				setBrushColor(c = ((ELLIPSE)(obj->structure))->color);
			if (b != ((ELLIPSE)(obj->structure))->style)
				setBrushStyle(b = ((ELLIPSE)(obj->structure))->style);
			ellipse(((ELLIPSE)(obj->structure))->posX, ((ELLIPSE)(obj->structure))->posY,
				((ELLIPSE)(obj->structure))->posX + ((ELLIPSE)(obj->structure))->width,
				((ELLIPSE)(obj->structure))->posY + ((ELLIPSE)(obj->structure))->height);
			setPenStyle(ps);
			setPenColor(pc);
			break;
		case POLY_TYPE:
			ps = GetPenStyle();
			pc = GetPenColor();
			if (((POLYGON)(obj->structure))->hasframe) {
				setPenStyle(PEN_STYLE_SOLID);
				setPenColor(BLACK);
			}
			else
				setPenStyle(PEN_STYLE_NULL);
			if (c != ((POLYGON)(obj->structure))->color)
				setBrushColor(c = ((POLYGON)(obj->structure))->color);
			if (b != ((POLYGON)(obj->structure))->style)
				setBrushStyle(b = ((POLYGON)(obj->structure))->style);
			polygon(((POLYGON)(obj->structure))->points, ((POLYGON)(obj->structure))->pointCount);
			setPenStyle(ps);
			setPenColor(pc);
			break;
		case OTHER_TYPE:
			break;
		}
	}
	if (obj->paint)
		(obj->paint)(obj);
}

//send event to the register obj in the event List
//if return number is 1, it catch the event signal and will not pass it to the next obj;
//the object is registed by setCalls function, and cancel call by releaseCalls function
static int sendEventProcess(List L, int x, int y, int buttom, int event, int eventKind) {
	List p, d;
	for (p = L->Next; p->Next != NULL;) {
		d = p;
		p = p->Next;
		if (eventKind & ((OBJECT)d->Data)->flag) {
			((OBJECT)d->Data)->flag &= ~eventKind;
			Delete_list_node(d);
			continue;
		}
		else if (((OBJECT)d->Data)->op != NULL)
			if ((((OBJECT)d->Data)->op)((OBJECT)d->Data, x, y, buttom, event, eventKind)) return 1;
	}
	return 0;
}

//paint all the objects in the screen 
static void paintScreen() {
	List p;
	for (p = windowObj.L->Next; p->Next != NULL; p = p->Next);
	for (p = p->Before; p->Before != NULL; p = p->Before)
		paintObj((OBJECT)p->Data);
}

//paint image: used as paint method in image objects
static void paintImage(OBJECT obj) {
	static bool init = FALSE;
	if (!init) {
 		loadImage(SOURCE(imageStruct*, obj)->name, SOURCE(imageStruct*, obj)->i);
		init = TRUE;
	}
	putImageScale(SOURCE(imageStruct*, obj)->i, STRUCTURE(RECTANGLE, obj)->posX, STRUCTURE(RECTANGLE, obj)->posY, STRUCTURE(RECTANGLE, obj)->width, STRUCTURE(RECTANGLE, obj)->height);
}

//create an textboxStruct for the text box obj
static textboxStruct* createTBS(string name, int maxChar, bool editable) {
	textboxStruct* tbs = (textboxStruct*)malloc(sizeof(textboxStruct));
	string buffer = malloc(sizeof(char)*maxChar);
	strcpy(buffer, name);
	tbs->buffer = buffer;
	tbs->length = maxChar;
	tbs->editable = editable;
	tbs->OriOp = NULL;
	return tbs;
}

//used as paint method in text box objects
static void paintTextBox(OBJECT obj) {
	drawTextBox(STRUCTURE(RECTANGLE, obj)->posX, STRUCTURE(RECTANGLE, obj)->posY,
		STRUCTURE(RECTANGLE, obj)->posX + STRUCTURE(RECTANGLE, obj)->width,
		STRUCTURE(RECTANGLE, obj)->posY + STRUCTURE(RECTANGLE, obj)->height, SOURCE(textboxStruct*, obj)->buffer, DT_CENTER);
}

//used as operation method in text box objects
//if the box can be edit, it get the char form input and show it
static int TextBoxop(Object* obj, int x, int y, int button, int Event, int eventKind) {
	int l;
	switch (eventKind) {
	case CHAR_EVE:
		l = strlen(SOURCE(textboxStruct*, obj)->buffer);
		if (l == SOURCE(textboxStruct*, obj)->length - 1)
			return 0;
		SOURCE(textboxStruct*, obj)->buffer[l] = button;
		SOURCE(textboxStruct*, obj)->buffer[l + 1] = 0;
	}
	if (SOURCE(textboxStruct*, obj)->OriOp)
		(SOURCE(textboxStruct*, obj)->OriOp)(obj, x, y, button, Event, eventKind);
	return 0;
}

//call back functions of KeyboardEvent, call by graphis library
//it send the event to sendEventProcess function
static void KeyboardEventProcess(int key, int event)/*每当产生键盘消息，都要执行*/
{
	if (sendEventProcess(keyCalls, 0, 0, key, event, KEY_EVE)) return;
	if (windowObj.globalKeyEvent)
		(windowObj.globalKeyEvent)(key, event);
}

//call back functions of  CharEvent, call by graphis library
//it send the event to sendEventProcess function
static void CharEventProcess(char c)
{
	if (sendEventProcess(charCalls, 0, 0, c, 0, CHAR_EVE)) return;
	if (windowObj.globalCharEvent)
		(windowObj.globalCharEvent)(c);
}

//check whether the click is in the box
static bool inBox(int x0, int y0, RECTANGLE r)
{
	return (x0 >= r->posX - 2 && x0 <= r->posX + r->width + 2 && y0 >= r->posY - 2 && y0 <= r->posY + r->height + 2);
}

//check whether the click is in the ellipse
static bool inEllip(int x0, int y0, ELLIPSE e)
{
	double a = e->width / 2.0 + 1, b = e->height / 2.0 + 1;
	double x = e->posX + a, y = e->posY + b;
	return ((x0 - x)*(x0 - x) / a / a + (y0 - y)*(y0 - y) / b / b <= 1);
}

//need implement
static bool inPoly(int x0, int y0, POLYGON p) {
	return TRUE;
}

//check whether the click is in the shape
static int checkObjSendEve(List L, int x, int y, int buttom, int event, int eventKind) {
	List p = L->Next, d;
	OBJECT obj;
	RECTANGLE r;
	ELLIPSE e;
	while (p->Next != NULL) {
		obj = ((OBJECT)p->Data);
			d = p;
			p = p->Next;
		if (eventKind & obj->flag) {
			Delete_list_node(d);
			obj->flag &= ~eventKind;
			continue;
		}
		if (obj->structure == NULL)
			continue;
		switch (((OBJECT)d->Data)->flag & ALL_TYPE) {
		case RECT_TYPE:
			r = ((RECTANGLE)(obj->structure));
			if (obj->op != NULL && inBox(x, y, r))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		case ELLI_TYPE:
			e = ((ELLIPSE)(obj->structure));
			if (obj->op != NULL && inBox(x, y, e))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		case POLY_TYPE:
			if (obj->op != NULL && inPoly(x, y, ((POLYGON)(obj->structure))))
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		case OTHER_TYPE:
			if (obj->op != NULL)
				if ((obj->op)(obj, x, y, buttom, event, eventKind))	return 1;
			break;
		}
	}
	return 0;
}

//delete objct now; call by deleteObj
//delete all the calls of the obj
static void delObjCall(OBJECT obj) {
	List node;
	while(node = Find_node(windowObj.L, obj))
		Delete_list_node(node);
	while(node = Find_node(charCalls, obj))
		Delete_list_node(node);
	while(node = Find_node(keyCalls, obj))
		Delete_list_node(node);
	while(node = Find_node(MouseMoveCalls, obj))
		Delete_list_node(node);
	while(node = Find_node(MouseClickCalls, obj))
		Delete_list_node(node);
	while (node = Find_node(timerCalls, obj))
		Delete_list_node(node);
}

//call back functions of  CharEvent, call by graphis library
//for mouse move event, call sendEventProcess;
//others; call checkObjSendEve to check whether the click is in the shape
static void MouseEventProcess(int x, int y, int button, int event)
{
	switch (event) {
	case BUTTON_DOWN:
		if (checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEDOWN_EVE)) return;
		break;
	case BUTTON_DOUBLECLICK:
		if (checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEDOUBLECLICK_EVE)) return;
		break;
	case BUTTON_UP:
		if (checkObjSendEve(MouseClickCalls, x, y, button, event, MOUSEUP_EVE)) return;
		break;
	case MOUSEMOVE:
		if (sendEventProcess(MouseMoveCalls, x, y, button, event, MOUSEMOVE_EVE)) return;
		break;
	}
	if (windowObj.globalMouseEvent)
		windowObj.globalMouseEvent(x, y, button, event);
}

//chect the update event of objects and repaint the screen
static void checkUpdate() {

	List p, d;
	for (p = timerCalls->Next; p->Next != NULL;) {
			d = p;
			p = p->Next;
		if (TIME_EVE & ((OBJECT)d->Data)->flag) {
			Delete_list_node(d);
			continue;
		}
		else if (((OBJECT)d->Data)->update != NULL)
			(((OBJECT)d->Data)->update)((OBJECT)d->Data);
	}
	beginPaint();
	clearDevice();
	paintScreen();
	endPaint();
}

//call back functions of  CharEvent, call by graphis library
//it update the global time and repaint the screen
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

//main: init the event lists and regist the call back function
int main()
{
	windowObj.L = createEmptyList();
	charCalls = createEmptyList();
	keyCalls = createEmptyList();
	timerCalls = createEmptyList();
	MouseMoveCalls = createEmptyList();
	MouseClickCalls = createEmptyList();

	registerKeyboardEvent(KeyboardEventProcess);/*注册键盘消息回调函数*/
	registerCharEvent(CharEventProcess);/*注册字符消息回调函数*/
	registerMouseEvent(MouseEventProcess);/*注册鼠标消息回调函数*/
	registerTimerEvent(TimerEventProcess);/*注册定时器消息回调函数*/

	//create menu for insert:
	createMyMenu();

	setup();//for the user to init

	//init window using default size
	initWindow("Window", DEFAULT, DEFAULT, 640, 480);

	//start the screen paint
	startTimer(0, frequency);

	return 0;
}
