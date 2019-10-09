#include "graphics.h"
#include <stdlib.h>

int flag = 0;
int id;

void paint(int x,int y)
{
	roundrect(x - 50, y - 10, x + 50, y + 10, 10, 10);
	drawTextBox(x-50,y-10,x+50,y+10,"hello world!", DT_CENTER);
}

void mouseEvent(int x,int y,int bt,int event)
{
	if (event == BUTTON_DOWN&&bt == LEFT_BUTTON) {

		flag = 1;
	}
	if (event == BUTTON_UP && bt == LEFT_BUTTON) {

		flag = 0;
	}
	if (bt == MOUSEMOVE && flag) {
		beginPaint();
		clearDevice();
        paint(x,y);
		endPaint();
	}
	else if (bt == RIGHT_BUTTON) {
		//resetWindowSize(300, 300, 1000, 800);
		beginPaint();
		id = SaveGraphicsState();
		setBrushColor(RED);
		paint(x+100, y+100);
		endPaint();
		beginPaint();
		RestoreGraphicsState(id);
		paint(x, y);
		setBrushColor(BLUE);
		endPaint();
	}
}

void printmsg() {
	msgBox("hhh", "hhh", 1);
}

void Soundp() {
	Sound("HelloWin.wav");
}

int main()
{	
	createMyMenu();
	insertMenuByName("hhh/mmm", printmsg);
	insertMenuByName("hhh/sound", Soundp);
    initWindow("",DEFAULT,DEFAULT,800,600);

    registerMouseEvent(mouseEvent);
}
