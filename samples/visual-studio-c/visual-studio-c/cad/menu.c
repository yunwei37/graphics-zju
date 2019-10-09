#include <stdio.h>
#include "engine.h"
#include "cad.h"
#include <stdlib.h>
#include <math.h>

//
extern OBJECT tb1, tb2;

extern OBJECTS toolbox;


//menu func
/*------------------------------------------------------------*/
// the about menu op

static void about() {
	msgBox("about", "this is a demo for cad \nusing the unnamed guilib", 1);
}

static void open_f() {
	msgBox("about", "open", 1);
}

static void close_f() {
	msgBox("about", "close", 1);
}

static void new_f() {
	msgBox("new file", "do you want to save the file?", 3);
	for (int i = 0; i < layer_count; ++i)
		deleteObjSet(layers[i]);
	layer_count = 0;
	layers[layer_count++] = createObjSet();
	layer_now = 0;
}

static void save_f() {
	msgBox("about", "save", 1);
}

static void exit_f() {
	msgBox("new file", "do you want to save the file?", 3);
	exit(0);
}

static void document_h() {
	msgBox("about", "document", 1);
}

//set the paint retangle
static void set1() {
	shapeid = 1;
	changeTextBox(tb1, "shape:\nrectangle", FALSE);
}

//set the paint ellipse
static void set2() {
	shapeid = 2;
	changeTextBox(tb1, "shape:\nellipse", FALSE);
}

static void set3() {
	shapeid = 3;
	changeTextBox(tb1, "shape:\npolyline", FALSE);
}

static void layer_up() {
	msgBox("about", "layer", 1);
}

static void layer_down() {
	msgBox("about", "layer", 1);
}

static void layer_new() {
	msgBox("about", "layer", 1);
}

static void layer_delete() {
	msgBox("about", "layer", 1);
}

static void button1(OBJECT obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		break;
	}
}

void cadmenu() {
	insertMenuByName("file/open", open_f);
	insertMenuByName("file/close", close_f);
	insertMenuByName("file/save", save_f);
	insertMenuByName("file/new", new_f);
	insertMenuByName("file/exit", exit_f);

	insertMenuByName("edit/choose", open_f);
	insertMenuByName("edit/copy", close_f);
	insertMenuByName("edit/paste", save_f);
	insertMenuByName("edit/delete", new_f);
	insertMenuByName("edit/cancel", exit_f);

	insertMenuByName("shape/rectangle", set1);
	insertMenuByName("shape/ellipse", set2);
	insertMenuByName("shape/polyline", set3);
	insertMenuByName("shape/polyline", set3);

	insertMenuByName("layer/down", layer_down);
	insertMenuByName("layer/up", layer_up);
	insertMenuByName("layer/new", layer_new);
	insertMenuByName("layer/delete", layer_delete);
	insertMenuByName("layer/showAllLayers", layer_delete);

	insertMenuByName("help/document", document_h);
	insertMenuByName("help/about", about);
}

void cadcomponents() {

	RECTANGLE r0 = createRect(3, 2, 90, 85, WHITE);
	OBJECT obj0 = createObj(RECT_TYPE, NULL, r0, NULL, 0);
	setTextBox(obj0, "draw state", 30, FALSE);

	//TextBox: show current type
	RECTANGLE r1 = createRect(5, 20, 30, 80, WHITE);
	OBJECT obj1 = createObj(RECT_TYPE, NULL, r1, NULL, 0);
	tb1 = setTextBox(obj1, "shape:\nrectangle", 30, FALSE);

	//TextBox: show current layer
	RECTANGLE r2 = createRect(5, 50, 30, 80, WHITE);
	OBJECT obj2 = createObj(RECT_TYPE, NULL, r2, NULL, 0);
	tb2 = setTextBox(obj2, "layer:\nlayer1", 30, FALSE);

	//button
	//RECTANGLE r3 = createRect(10, 80, 30, 30, RED);
	//OBJECT obj3 = createObj(RECT_TYPE, NULL, r3, button1, MOUSECLICK_EVE);

}