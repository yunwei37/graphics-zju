#include <stdio.h>
#include "engine.h"
#include "cad.h"
#include <stdlib.h>
#include <math.h>

static int ox, oy;

// rects, ellips;
OBJECTS layers[20];
int layer_count;
int layer_now;

//1 rect; 2 ellipse; 3 polyline; 
int shapeid;

OBJECT chosen_obj;

//选中标志
void paint_chosen(OBJECT obj) {
	RECTANGLE r = STRUCTURE(RECTANGLE, obj);
	line(r->posX-5, r->posY-5, r->posX + r->width+5, r->posY-5);
	line(r->posX-5, r->posY-5, r->posX-5, r->posY+r->height+5);
	line(r->posX + r->width+5, r->posY-5, r->posX + r->width+5, r->posY + r->height+5);
	line(r->posX-5 , r->posY + r->height+5, r->posX + r->width+5, r->posY + r->height+5);
}

//paint the poline
static void paint_polyline(OBJECT obj) {
	POLYLINE p = STRUCTURE(POLYLINE, obj);
	polyLine(p->p, p->pcount);
}

int move(OBJECT obj, int x, int y, int button, int Event, int eventKind);

//the operation for create rectangle
int create_rectangle(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEMOVE_EVE:
		if (y - STRUCTURE(RECTANGLE, obj)->posY > 0) {
			STRUCTURE(RECTANGLE, obj)->posY = oy;
			STRUCTURE(RECTANGLE, obj)->height = y - oy ;
		}
		else {
			STRUCTURE(RECTANGLE, obj)->height +=  STRUCTURE(RECTANGLE, obj)->posY - y;
			STRUCTURE(RECTANGLE, obj)->posY = y;
		}
		if (x - STRUCTURE(RECTANGLE, obj)->posX > 0) {
			STRUCTURE(RECTANGLE, obj)->posX = ox;
			STRUCTURE(RECTANGLE, obj)->width = x-ox;
		}
		else {
			STRUCTURE(RECTANGLE, obj)->width += STRUCTURE(RECTANGLE, obj)->posX - x;
			STRUCTURE(RECTANGLE, obj)->posX = x;
		}
		break;
	case MOUSEUP_EVE:
		releaseCalls(MOUSEMOVE_EVE, obj);
		changeObjectOp(obj, move);
		break;
	}
	return 1;
}

//the operation to draw the polyline
int polyline_drawer(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	POLYLINE p = STRUCTURE(POLYLINE, obj);
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		if (button == RIGHT_BUTTON) {
			releaseCalls( MOUSEMOVE_EVE | MOUSECLICK_EVE, obj);
		}
		else if (button == LEFT_BUTTON) {
			p->p[p->pcount-1].x = x;
			p->p[p->pcount-1].y = y;
			p->pcount++;
			p->p[p->pcount - 1].x = x;
			p->p[p->pcount - 1].y = y;
		}
		break;
	case MOUSEMOVE_EVE:
		p->p[p->pcount-1].x = x;
		p->p[p->pcount-1].y = y;
		break;
	case MOUSEUP_EVE:
		//releaseCalls(MOUSEMOVE_EVE, obj);
		//changeObjectOp(obj, move);
		break;
	}
	return 1;
}

//the function for move the ellips and rectangle
int move(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	static int omx, omy;
	static int button_flag;
	//static int ctrl_flag = FALSE;
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		setCalls(MOUSEMOVE_EVE, obj);
		setCalls(CHAR_EVE, obj);
		addpaintMethod(obj,paint_chosen);
		omx = x; 
		omy = y;
		ox = STRUCTURE(RECTANGLE, obj)->posX;
		oy = STRUCTURE(RECTANGLE, obj)->posY;
		button_flag = button;
		chosen_obj = obj;
		break;
	case MOUSEMOVE_EVE:
		if (button_flag == LEFT_BUTTON) {
			STRUCTURE(RECTANGLE, obj)->posX += x - omx;
			STRUCTURE(RECTANGLE, obj)->posY += y - omy;
		}
		else if (button_flag == RIGHT_BUTTON) {
			if (y - STRUCTURE(RECTANGLE, obj)->posY > 0) {
				STRUCTURE(RECTANGLE, obj)->posY = oy;
				STRUCTURE(RECTANGLE, obj)->height = y - oy;
			}
			else {
				STRUCTURE(RECTANGLE, obj)->height += STRUCTURE(RECTANGLE, obj)->posY - y;
				STRUCTURE(RECTANGLE, obj)->posY = y;
			}
			if (x - STRUCTURE(RECTANGLE, obj)->posX > 0) {
				STRUCTURE(RECTANGLE, obj)->posX = ox;
				STRUCTURE(RECTANGLE, obj)->width = x - ox;
			}
			else {
				STRUCTURE(RECTANGLE, obj)->width += STRUCTURE(RECTANGLE, obj)->posX - x;
				STRUCTURE(RECTANGLE, obj)->posX = x;
			}
		}
		omx = x;
		omy = y;
		break;
	case MOUSEUP_EVE:
		releaseCalls(MOUSEMOVE_EVE| CHAR_EVE, obj);
		cancelpaintMethod(obj);
		chosen_obj = NULL;
		break;
	case CHAR_EVE:
		switch (button) {
		case '\b':
			chosen_obj = NULL;
			cancelpaintMethod(obj);
			//releaseCalls(CHAR_EVE, obj);
			if (TYPEFLAG(obj) == RECT_TYPE)
				deleteObjInSet(layers[layer_now], obj);
			else if (TYPEFLAG(obj) == ELLI_TYPE)
				deleteObjInSet(layers[layer_now], obj);
		}
		break;
	}
	return 1;
}

/*-------------------------------------------------------------------*/
//the default mouse event process to create objects
static void MouseEventProcess(int x, int y, int button, int event) {
	RECTANGLE r;
	ELLIPSE e;
	POLYLINE p;
	switch (event) {
	case BUTTON_DOWN:
		ox = x;
		oy = y;
		if (shapeid == 1) {
			r = createRect(x, y, 0,0, createColor(rand() % 256, rand() % 256, rand() % 256));
			OBJECT obj = createObj(RECT_TYPE, NULL, r, create_rectangle, MOUSECLICK_EVE | MOUSEMOVE_EVE);
			addSetObj(layers[layer_now], obj);
		}
		else if (shapeid == 2) {
			e = createEllipse(x, y, 0, 0, createColor(rand() % 256, rand() % 256, rand() % 256));
			OBJECT obj = createObj(ELLI_TYPE, NULL, e, create_rectangle, MOUSECLICK_EVE | MOUSEMOVE_EVE);
			addSetObj(layers[layer_now], obj);
		}
		else if (shapeid == 3) {
			if (button == LEFT_BUTTON) {
				p = (POLYLINE)malloc(sizeof(struct polylineS));
				p->pcount = 2;
				p->p[0].x = x;
				p->p[0].y = y;
				OBJECT obj = createObj(OTHER_TYPE, paint_polyline, p, polyline_drawer, MOUSECLICK_EVE | MOUSEMOVE_EVE);
				addSetObj(layers[layer_now], obj);
			}
		}
	}
 }


void setup() {
	setDefaultMouseEvent(MouseEventProcess);
	
	cadmenu();

	cadcomponents();

	layers[layer_count++] = createObjSet();

}

