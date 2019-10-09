// a very simple demo
//test of slot and signal;

#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

SIGNAL signal;
SIGNAL signal1;

//change the color of the block when click the center button
int op1(Object* obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		//msgBox("hello","hello",1);
		sentSignal(signal, &(STRUCTURE(RECTANGLE, obj)->color));//send color messgae
		STRUCTURE(RECTANGLE, obj)->color = createColor(rand() % 256, rand() % 256, rand() % 256); //change color
	}
	return 1;
}

int op3(Object* obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		//msgBox("hello","hello",1);
		sentSignal(signal1, &(STRUCTURE(RECTANGLE, obj)->color));//send color messgae
		STRUCTURE(RECTANGLE, obj)->color = createColor(rand() % 256, rand() % 256, rand() % 256); //change color
	}
	return 1;
}

// disconnect the signal;
int op2(Object* obj, int x, int y, int button, int Event, int eventKind) {
	switch (eventKind) {
	case MOUSEDOWN_EVE:
		//msgBox("hello","hello",1);
		if (button == LEFT_BUTTON) {
			disconnectSS(obj->source, signal);//disconnect
			disconnectSS(obj->source, signal1);//disconnect
		}
		else {
			//disconnect and delete the object and slot
			disconnectSS(obj->source, signal);
			disconnectSS(obj->source, signal1);
			deleteSlot(obj->source);
			deleteObj(obj);
			//deleteSignal(signal);
			//signal = NULL;
		}
	}
	return 1;
}

//receiver color message and change the obj color
void receiver(OBJECT obj, void* message) {
	STRUCTURE(RECTANGLE, obj)->color = *((Color*)message);
}

void setup() { 
	
	//center button: when click, send its color as a signal to change the other object's color; and change color  
	RECTANGLE r = createRect(getWindowWidth() / 2, getWindowHeight() / 2, 20, 20, createColor(rand() % 256, rand() % 256, rand() % 256));
	OBJECT obj = createObj(RECT_TYPE, NULL, r, op1, MOUSECLICK_EVE);
	
	signal = createSignal(obj);
	
	RECTANGLE r1 = createRect(0, 0, 20, 20, createColor(rand() % 256, rand() % 256, rand() % 256));
	OBJECT obj2 = createObj(RECT_TYPE, NULL, r1, op3, MOUSECLICK_EVE);

	signal1 = createSignal(obj);

	for (int i = 0; i < 30; ++i) {
		//generate ellipse randomly
		ELLIPSE e = createEllipse(rand() % getWindowWidth(), rand() % getWindowHeight(), 20, 20, createColor(rand() % 256, rand() % 256, rand() % 256));
		OBJECT obj1 = createObj(ELLI_TYPE, NULL, e, op2, MOUSECLICK_EVE);

		//create a slot and connect it with other
		SLOT slot1 = createSlot(obj1, receiver);
		connectSS(slot1, signal);
		connectSS(slot1, signal1);

		//save the slot inside the obj source
		addsource(obj1, slot1);
	}
	
}

