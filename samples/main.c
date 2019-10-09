
#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

#define RAD(x) ((x)/360.0*2*3.1415926535)

void op(Object* obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		//msgBox("hello","hello",1);
		((RECTANGLE)(obj->structure))->color = createColor(rand() % 256, rand() % 256, rand() % 256);
	}
}

void update(Object* obj) {
	//double time = getTime();
	//time = time -(int)time + (int)time % 6;
	//((RECTANGLE)(obj->structure))->posY+=time*time;
	int d = rand() % 2 == 0 ? 1 : -1;
	((RECTANGLE)(obj->structure))->posY += d*rand() % 3;
	d = rand() % 2 == 0 ? 1 : -1;
	((RECTANGLE)(obj->structure))->posX += d*rand() % 3;
	if (((RECTANGLE)(obj->structure))->posY > getWindowHeight() || ((RECTANGLE)(obj->structure))->posY < 0) {
		((RECTANGLE)(obj->structure))->posY = getWindowHeight() / 2;
	}
	if (((RECTANGLE)(obj->structure))->posX > getWindowWidth()|| ((RECTANGLE)(obj->structure))->posX < 0) {
		((RECTANGLE)(obj->structure))->posX = getWindowWidth() / 2;
	}
}

void setup() { 
	int i;
	for (i = 0; i < 5; ++i) {
		RECTANGLE r = createRect( 300, 200, 20, 20, createColor(rand()%256,rand()%256,rand()%256 ));
		OBJECT obj = createObj(RECT_TYPE, NULL, r, op, MOUSECLICK_EVE);
		setUpdate(obj,update);
	}
}

