#include "graphics.h"

#define MLEN 64

int len = 0;
char str[MLEN];

int n=0;

void paint()
{
    beginPaint();
    
    setTextSize(16);
	DrawTextString(str+len);
	++len;
	setCaretPos(GetCurrentX(), GetCurrentY());
    endPaint();
}

void charEvent(char c)
{
    if(len+1>=MLEN)
    {
        memset(str,0,sizeof(str));
        len = 0;
    }
    str[len] = c;
	showCaret();
    paint();
}

void KeyboardEventProcess(int key, int event)/*每当产生键盘消息，都要执行*/
{
	double oldradius;

	switch (event) {
	case KEY_DOWN:
		switch (key) {
		case VK_UP:/*UP*/
			
			break;
		case VK_DOWN:

			break;
		case VK_LEFT:
			beginPaint();
			//DrawTextString(str + len);
			setCaretPos(GetCurrentX()-(++n)*getTextWidth("a"), GetCurrentY());
			endPaint();
			break;
		case VK_RIGHT:
			beginPaint();
			//DrawTextString(str + len);
			setCaretPos(GetCurrentX() - (--n)*getTextWidth("a"), GetCurrentY());
			endPaint();
			break;
		}
		break;
	case KEY_UP:
		break;
	}
}

int main()
{
    initWindow("",DEFAULT,DEFAULT,800,600);
    registerCharEvent(charEvent);
	registerKeyboardEvent(KeyboardEventProcess);/*注册键盘消息回调函数*/
	//showCaret();
    return 0;
}
