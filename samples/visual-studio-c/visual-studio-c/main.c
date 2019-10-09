
#include <graphics.h>

#include <stdio.h>

#include "graphics.h"

#include <stdlib.h>
#include <math.h>

#define RAD(x) ((x)/360.0*2*3.1415926535)

int h = 0;
int m = 0;
int s = 0;

void paint()
{
	int ox = getWidth() / 2;
	int oy = getHeight() / 2;

	int hl = 46;
	int ml = 74;
	int sl = 120;

	int i;

	beginPaint();

	clearDevice();

	// circle
	setPenWidth(2);
	setPenColor(BLACK);
	setBrushColor(WHITE);
	ellipse(getWidth()/12, getHeight()/12, getWidth() / 12*11, getHeight() / 12*11);

	// label
	setPenWidth(1);
	setPenColor(BLACK);
	for (i = 0; i < 12; ++i)
	{
		moveTo(ox + 115 * sin(RAD(180 - i * 30)), oy + 115 * cos(RAD(180 - i * 30)));
		lineTo(ox + 125 * sin(RAD(180 - i * 30)), oy + 125 * cos(RAD(180 - i * 30)));
	}

	// hour
	setPenWidth(8);
	setPenColor(BLACK);
	moveTo(ox, oy);
	lineTo(ox + hl * sin(RAD(180 - h * 30)), oy + hl * cos(RAD(180 - h * 30)));

	// minute
	setPenWidth(4);
	setPenColor(GREEN);
	moveTo(ox, oy);
	lineTo(ox + ml * sin(RAD(180 - m * 6)), oy + ml * cos(RAD(180 - m * 6)));

	// second
	setPenWidth(2);
	setPenColor(RED);
	moveTo(ox, oy);
	lineTo(ox + sl * sin(RAD(180 - s * 6)), oy + sl * cos(RAD(180 - s * 6)));

	endPaint();
	//Sound("HelloWin.wav");
}

void timerEvent(int tid)
{
	++s;
	if (s == 60)
	{
		s = 0;
		++m;
	}
	if (m == 60)
	{
		m = 0;
		++h;
	}
	if (h == 12)
		h = 0;
	paint();
}

int main()
{
	initWindow("Clock", DEFAULT, DEFAULT, 300, 300);

	registerTimerEvent(timerEvent);

	setCaretPos(getWidth() / 2, getHeight() / 2);
	showCaret();

	startTimer(0, 1000);
	//startTimer(0,100);
	//startTimer(0,10);
	//startTimer(0,1);

	//playSound("HelloWin.wav");

	return 0;
}