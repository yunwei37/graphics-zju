#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

static int ox, oy;

//OBJECT tb1,tb2;

char shapename[40]="rectange";
char layernow[30]="layer1";
char penstylename[30] = "solid";
char brushstylename[30] = "solid";

// rects, ellips;
static OBJECTS layers[20];
static OBJECTS layerbutton;
static layer_count = 0;
static layer_now = 0;

OBJECTS colortable[3];
int colorindex[3] = {25,25,25};

static OBJECTS toolbox;

typedef struct polylineS {
	POINT p[30];
	int pcount;
	ACL_Pen_Style s;
	Color c;
} *POLYLINE;

Color pencolor=BLACK;
Color brushcolor=WHITE;

ACL_Brush_Style brushstyle=BRUSH_STYLE_SOLID;
ACL_Pen_Style penstyle=PEN_STYLE_SOLID;

//1 rect; 2 ellipse; 3 polyline; 4 text
int shapeid = 1;

bool iscreate = TRUE;

OBJECT chosen_obj = NULL;

//选中标志
void paint_chosen(OBJECT obj) {
	SaveGraphicsState();
	setPenSize(1);
	setPenColor(BLACK);
	setPenStyle(PEN_STYLE_DASHDOT);
	RECTANGLE r = STRUCTURE(RECTANGLE, obj);
	line(r->posX-5, r->posY-5, r->posX + r->width+5, r->posY-5);
	line(r->posX-5, r->posY-5, r->posX-5, r->posY+r->height+5);
	line(r->posX + r->width+5, r->posY-5, r->posX + r->width+5, r->posY + r->height+5);
	line(r->posX-5 , r->posY + r->height+5, r->posX + r->width+5, r->posY + r->height+5);
	RestoreGraphicsState(-1);
}

//paint the poline
static void paint_polyline(OBJECT obj) {
	POLYLINE p = STRUCTURE(POLYLINE, obj);
	setPenColor(p->c);
	setPenStyle(p->s);
	polyLine(p->p, p->pcount);
}

static void updatetaxtbox(OBJECT obj) {
	STRUCTURE(RECTANGLE, obj)->width = getTextWidth(SOURCE(textboxStruct*, obj)->buffer)+5;
}

//layout
static void paint_layout(OBJECT obj) {
	SaveGraphicsState();
	setPenSize(1);
	setPenColor(BLACK);
	setPenStyle(PEN_STYLE_SOLID);
	line(0, 30, getWindowWidth(), 30);
	line(0, 200, 100, 200);
	line(100, 30, 100, getWindowHeight());
	line(100, getWindowHeight() - 30, getWindowWidth(), getWindowHeight() - 30);
	DisplayText(105, getWindowHeight() - 25, shapename);
	DisplayText(205, getWindowHeight() - 25, layernow);
	DisplayText(305, getWindowHeight() - 25, penstylename);
	DisplayText(405, getWindowHeight() - 25, brushstylename);
	DisplayText(5, 35, "layers:");
	DisplayText(5, 205, "color table:");
	setBrushColor(brushcolor);
	rectangle(505, getWindowHeight() - 25, 535, getWindowHeight() - 5);
	setPenColor(GRAY);
	line(100, getWindowHeight() / 2, getWindowWidth(), getWindowHeight() / 2);
	line(getWindowWidth() / 2, 30, getWindowWidth() / 2, getWindowHeight()-30);

	RestoreGraphicsState(-1);
}

int chooseColor(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	int i, index;
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		i = 0;
		while(i<3&&(index=getObjIndex(colortable[i],obj))==-1) ++i;
		if (i == 3)	msgBox("error", "error", 1);
		colorindex[i] = index;
		pencolor = brushcolor = createColor(colorindex[0]*10, colorindex[1]*10, colorindex[2]*10);
	}
	return 1;
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
	static paintMethod p;
	//static int ctrl_flag = FALSE;
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		setCalls(MOUSEMOVE_EVE, obj);
		setCalls(CHAR_EVE, obj);
		p = obj->paint;
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
		addpaintMethod(obj, p);
		p = NULL;
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

//menu func
/*------------------------------------------------------------*/
// the about menu op
static void about() {
	msgBox("about", "a demo for cad", 1);
}

static void open_f() {
	initConsole();
	printf("input the file name:");
	char buffer[50];
	scanf("%s", buffer);
	//FILE *f = fopen(buffer, "r");
}

static void close_f() {
	int result=msgBox("close", "do you want to save the file?", 1);
}

static void exit_f() {
	exit(0);
}

static void new_f() {
	for (int i = 0; i < layer_count; ++i)
		deleteObjSet(layers[i]);
	deleteObjSet(layerbutton);
	layer_count = 0;
	layers[layer_count++] = createObjSet();
	layer_now = 0;
	layerbutton = createObjSet();
}

static void save_f() {
	//msgBox("about", "save", 1);
}

static void document_h() {
	//msgBox("about", "document", 1);
}
/*---------------------------------------------------------*/
//set the paint retangle
static void set1() {
	shapeid = 1;
	strcpy(shapename, "rectangle");
}

//set the paint ellipse
static void set2() {
	shapeid = 2;
	strcpy(shapename, "ellipse");
}

//set the paint polyline
static void set3() {
	shapeid = 3;
	strcpy(shapename, "polyline");
}

static void set4() {
	shapeid = 4;
	strcpy(shapename, "text");
}

static void set5() {
	shapeid = 5;
	strcpy(shapename, "null");
}

/*----------------------------------------------------------------------*/
static void p_solid() {
	penstyle = PEN_STYLE_SOLID;
	strcpy(penstylename, "solid");
}

static void p_dash() {
	penstyle = PEN_STYLE_DASH;
	strcpy(penstylename, "dash");
}

static void p_dot() {
	penstyle = PEN_STYLE_DOT;
	strcpy(penstylename, "dot");
}

static void p_dashdot() {
	penstyle = PEN_STYLE_DASHDOT;
	strcpy(penstylename, "dashdot");
}

static void p_null() {
	penstyle = PEN_STYLE_NULL;
	strcpy(penstylename, "null");
}
/*----------------------------------------------------------------------*/
static void b_solid() {
	brushstyle = BRUSH_STYLE_SOLID;
	strcpy(brushstylename, "solid");
}

static void b_hor() {
	brushstyle = BRUSH_STYLE_HORIZONTAL;
	strcpy(brushstylename, "horizontal");
}

static void b_cross() {
	brushstyle = BRUSH_STYLE_CROSS;
	strcpy(brushstylename, "cross");
}

static void b_null() {
	brushstyle = BRUSH_STYLE_NULL;
	strcpy(brushstylename, "null");
}
/*-------------------------------------------------------------------------*/
static void layer_up() {
	if(layer_now<layer_count-1)
		layer_now++;
	sprintf(layernow, "layer%d", layer_now + 1);
}

static void layer_down() {
	if (layer_now >0)
		layer_now--;
	sprintf(layernow, "layer%d", layer_now + 1);
}

static void layer_new() {
	if(layer_count<20)
		layers[layer_count] = createObjSet();
	RECTANGLE r1 = createRect(5, 55 + layer_count*25, 20, 80, WHITE);
	OBJECT obj2 = createObj(RECT_TYPE, NULL, r1, NULL, 0);
	addSetObj(layerbutton, obj2);
	layer_count++;
	char buffer[20];
	sprintf(buffer, "layer%d", layer_count);
	setTextBox(obj2, buffer, 30, FALSE);
}

static void layer_delete() {
	msgBox("about", "layer", 1);
}

/*-------------------------------------------------------------------*/
//the default mouse event process
static void MouseEventProcess(int x, int y, int button, int event) {
	RECTANGLE r;
	ELLIPSE e;
	POLYLINE p;
	static bool textcrt = FALSE;
	static OBJECT textobj;
	switch (event) {
	case BUTTON_DOWN:
		ox = x;
		oy = y;
		if (shapeid == 1) {
			r = createRect(x, y, 0,0, brushcolor);
			r->style = brushstyle;
			OBJECT obj = createObj(RECT_TYPE, NULL, r, create_rectangle, MOUSECLICK_EVE | MOUSEMOVE_EVE);
			addSetObj(layers[layer_now], obj);
		}
		else if (shapeid == 2) {
			e = createEllipse(x, y, 0, 0, brushcolor);
			e->style = brushstyle;
			OBJECT obj = createObj(ELLI_TYPE, NULL, e, create_rectangle, MOUSECLICK_EVE | MOUSEMOVE_EVE);
			addSetObj(layers[layer_now], obj);
		}
		else if (shapeid == 3) {
			if (button == LEFT_BUTTON) {
				p = (POLYLINE)malloc(sizeof(struct polylineS));
				p->pcount = 2;
				p->p[0].x = x;
				p->p[0].y = y;
				p->c = pencolor;
				p->s = penstyle;
				OBJECT obj = createObj(OTHER_TYPE, paint_polyline, p, polyline_drawer, MOUSECLICK_EVE | MOUSEMOVE_EVE);
				addSetObj(layers[layer_now], obj);
			}
		}
		else if (shapeid == 4) {
			if (!textcrt) {
				r = createRect(x, y, getTextHeight("l")+2, getTextWidth("w") + 2, WHITE);
				r->style = BRUSH_STYLE_SOLID;
				OBJECT obj = createObj(RECT_TYPE, NULL, r, NULL, 0);
				setTextBox(obj, "", 30, TRUE);
				setUpdate(obj, updatetaxtbox);
				//showCaret();
				addSetObj(layers[layer_now], obj);
				textcrt = TRUE;
				textobj = obj;
			}
			else {
				changeTextBox(textobj, SOURCE(textboxStruct*, textobj)->buffer, FALSE);
				cancelUpdate(textobj);
				changeObjectOp(textobj, move);
				setCalls(MOUSECLICK_EVE, textobj);
				textcrt = FALSE;
				textobj = NULL;
			}

		}
	}
 }

static void button1(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		set1();
	}
}

static void button2(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		set2();
	}
}

static void button3(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		set4();
	}
}

static void button4(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		set3();
	}
}

static void paint_button4(OBJECT obj) {
	RECTANGLE r = STRUCTURE(RECTANGLE, obj);
	line(r->posX, r->posY, r->posX + r->width / 2, r->posY+r->height);
	line(r->posX+ r->width / 2 , r->posY + r->height, r->posX + r->width / 2, r->posY);
	line(r->posX + r->width / 2, r->posY, r->posX + r->width, r->posY + r->height);
}

void setup() {
	setDefaultMouseEvent(MouseEventProcess);
	
	insertMenuByName("file/open", open_f);
	insertMenuByName("file/close", close_f);
	insertMenuByName("file/save", save_f);
	insertMenuByName("file/new", new_f);
	insertMenuByName("file/exit", exit_f);

	insertMenuByName("edit/select", open_f);
	insertMenuByName("edit/create", open_f);
	insertMenuByName("edit/copy", close_f);
	insertMenuByName("edit/paste", save_f);
	insertMenuByName("edit/delete", new_f);
	insertMenuByName("edit/cancel", exit_f);

	insertMenuByName("shape/rectangle", set1);
	insertMenuByName("shape/ellipse", set2);
	insertMenuByName("shape/polyline", set3);
	insertMenuByName("shape/text", set4);
	//insertMenuByName("shape/round", set5);
	
	insertMenuByName("penstyle/solid", p_solid);
	insertMenuByName("penstyle/dash", p_dash);
	insertMenuByName("penstyle/dot", p_dot);
	insertMenuByName("penstyle/dashdot", p_dashdot);
	insertMenuByName("penstyle/NULL", p_null);

	insertMenuByName("brushstyle/solid", b_solid);
	insertMenuByName("brushstyle/horizontal", b_hor);
	insertMenuByName("brushstyle/cross", b_cross);
	insertMenuByName("brushstyle/NULL", b_null);

	insertMenuByName("layer/down", layer_down);
	insertMenuByName("layer/up", layer_up);
	insertMenuByName("layer/new", layer_new);
	insertMenuByName("layer/delete", layer_delete);

	insertMenuByName("help/document", document_h);
	insertMenuByName("help/about",about);
	
	//drawer
	OBJECT obj1 = createDrawers(paint_layout,NULL);

	layers[layer_count++] = createObjSet();

	RECTANGLE r1 = createRect(5, 55, 20, 80, WHITE);
	OBJECT obj2 = createObj(RECT_TYPE, NULL, r1, NULL, 0);
	setTextBox(obj2, "layer1", 30, FALSE);
	//addpaintMethod(obj2, paint_chosen);

	//create color table
	int i;
	for (i = 0; i < 3; ++i)
		colortable[i]=createObjSet();
	//red
	for (i = 0; i < 25; ++i) {
		RECTANGLE tb = createRect(5, 225+8*i, 9,30, createColor(i * 10,0,0));
		tb->hasframe = FALSE;
		OBJECT ob = createObj(RECT_TYPE, NULL, tb, chooseColor, MOUSECLICK_EVE);
		addSetObj(colortable[0],ob);
	}
	for (i = 0; i < 25; ++i) {
		RECTANGLE tb = createRect(35, 225 + 8 * i, 9, 30, createColor(0, i*10, 0));
		tb->hasframe = FALSE;
		OBJECT ob = createObj(RECT_TYPE, NULL, tb, chooseColor, MOUSECLICK_EVE);
		addSetObj(colortable[1],ob);
	}
	for (i = 0; i < 25; ++i) {
		RECTANGLE tb = createRect(65, 225 + 8 * i, 9, 30, createColor(0, 0,i*10 ));
		tb->hasframe = FALSE;
		OBJECT ob = createObj(RECT_TYPE, NULL, tb, chooseColor, MOUSECLICK_EVE);
		addSetObj(colortable[2],ob);
	}
	
	layerbutton= createObjSet();

	//toolbox
	toolbox = createObjSet();
	
	RECTANGLE tb1 = createRect(5,5,20, 25,WHITE);
	OBJECT ob1 = createObj(RECT_TYPE, NULL, tb1, button1, MOUSECLICK_EVE);
	addSetObj(toolbox, ob1);
	
	ELLIPSE tb2 = createEllipse(35, 5, 20, 25, WHITE);
	OBJECT ob2 = createObj(ELLI_TYPE, NULL, tb2, button2, MOUSECLICK_EVE);
	addSetObj(toolbox, ob2);

	RECTANGLE tb3 = createRect(65, 5, 20, 25, WHITE);
	OBJECT ob3 = createObj(RECT_TYPE, NULL, tb3, button3, MOUSECLICK_EVE);
	setTextBox(ob3, "Aa", 30, FALSE);

	RECTANGLE tb4 = createRect(95, 5, 20, 25, WHITE);
	tb4->hasframe = FALSE;
	OBJECT ob4 = createObj(RECT_TYPE, NULL, tb4, button4, MOUSECLICK_EVE);
	addpaintMethod(ob4, paint_button4);
}

