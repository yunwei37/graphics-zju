#include "graphics.h"
#include <stdio.h>

Image img;

void paint(int id) {
    beginPaint();
    putImageScale(&img,0,0,getWindowWidth(),getWindowHeight());
    endPaint();
}

int main()
{
    initWindow("Image",DEFAULT,DEFAULT,640,480);

	loadImage("lena.jpg",&img);
	/*
	char a[30];
	sprintf(a, "%d", GetFullScreenHeight());
	msgBox("h", a, 1);
	sprintf(a, "%d", GetFullScreenWidth());
	msgBox("h", a, 1);
	*/
	startTimer(1, 30);
	registerTimerEvent(paint);

    return 0;
}
