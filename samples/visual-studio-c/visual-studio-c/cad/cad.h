#pragma once
#ifndef CAD_H_
#define CAD_H_

#include <stdio.h>
#include "engine.h"

#include <stdlib.h>
#include <math.h>

// rects, ellips;
extern OBJECTS layers[20];
extern int layer_count = 0;
extern int layer_now = 0;

typedef struct polylineS {
	POINT p[30];
	int pcount;
} *POLYLINE;


//1 rect; 2 ellipse; 3 polyline; 
extern int shapeid = 1;

extern OBJECT chosen_obj = NULL;

void cadcomponents();
void cadmenu();

#endif // !CAD_H_
