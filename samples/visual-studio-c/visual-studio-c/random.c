// a very simple demo
//generate random blocks and move them randomly

#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

//change the color of the block when click
int op(Object* obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		//msgBox("hello","hello",1);
		STRUCTURE(RECTANGLE, obj)->color = createColor(rand() % 256, rand() % 256, rand() % 256); //change color
	}
	return 1;
}

void update(Object* obj) {
	int d = rand() % 2 == 0 ? 1 : -1;//determine the position
	STRUCTURE(RECTANGLE, obj)->posY += d*rand() % 5;//walk
	d = rand() % 2 == 0 ? 1 : -1;
	STRUCTURE(RECTANGLE, obj)->posX += d*rand() % 5;
	//delete it if it get out of window
	if (STRUCTURE(RECTANGLE, obj)->posY > getWindowHeight() || STRUCTURE(RECTANGLE, obj)->posY < 0) {
		deleteObj(obj);
	}
	else if (STRUCTURE(RECTANGLE, obj)->posX > getWindowWidth()|| STRUCTURE(RECTANGLE, obj)->posX < 0) {
		deleteObj(obj);
	}
}

void setup() { 
	int i;
	for (i = 0; i < 1000; ++i) {//create block objects
		RECTANGLE r = createRect(getWindowWidth() / 2, getWindowHeight() / 2, 20, 20, createColor(rand() % 256, rand() % 256, rand() % 256));
		OBJECT obj = createObj(RECT_TYPE, NULL, r, op, MOUSECLICK_EVE);
		setUpdate(obj,update);
	}
}

