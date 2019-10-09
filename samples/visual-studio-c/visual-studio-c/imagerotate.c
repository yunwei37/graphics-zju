// a very simple demo
//show a image and let it rotate 

#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

void update(Object* obj) {

	double time = getTime();
	
	//¼«×ø±ê£º¦È= r ;

	double d = time * 10 ;
	double angle = time * 10;

	STRUCTURE(RECTANGLE, obj)->posY = d * sin(angle)+ getWindowHeight() / 2;//walk
	
	STRUCTURE(RECTANGLE, obj)->posX = d * cos(angle)+ getWindowWidth() / 2;
	//delete it if it get out of window
	if (STRUCTURE(RECTANGLE, obj)->posY > getWindowHeight() || STRUCTURE(RECTANGLE, obj)->posY < 0) {
		deleteObj(obj);
	}
	else if (STRUCTURE(RECTANGLE, obj)->posX > getWindowWidth() || STRUCTURE(RECTANGLE, obj)->posX < 0) {
		deleteObj(obj);
	}
}

void setup() {
	int i;
	for (i = 0; i < 1; ++i) {//create block objects
		OBJECT obj = createImageObj("1.jpg", getWindowWidth()/2, getWindowHeight()/2, 50, 50,NULL);
		setUpdate(obj, update);
	}
}