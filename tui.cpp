#include "nanotter.h"
#include <ncurses.h>

#include "Editor.h"

extern int g_intTerminalRow,g_intTerminalCol;
extern WINDOW *g_winTimeLine;

char *tuiFloatMessageBox(const char *title, const char *text, int reqinput)
{
	WINDOW *win = newwin(g_intTerminalRow - 6, g_intTerminalCol - 6, 3, 3);
	WINDOW *wid = newwin(g_intTerminalRow - 8, g_intTerminalCol - 8, 4, 4);
	
	overlay(g_winTimeLine,win);
	
	wborder(win,'|','|','-','-','+','+','+','+');
	
	mvwaddstr(win,0,3,title);
	mvwaddstr(wid,1,0,text);
	
	mvwaddstr(wid,1,0,text);
	
	int tline = 1;
	char *ts = (char *)text;
	for(;*ts;ts++) if(*ts == 0x0a) tline++;
	
	char *req = NULL;
	
	if(!reqinput) {
		mvwaddstr(wid,g_intTerminalRow - 8 - 2,(g_intTerminalCol - 8)/2-strlen("[ PRESS ENTER ]")/2,"[ PRESS ENTER ]");
		wrefresh(win);
		wrefresh(wid);
		int ch = wgetch(wid);
		while(ch != KEY_ENTER && ch != 0x0a) ch = wgetch(wid);
	} else {
		WINDOW *boxp = newwin(g_intTerminalRow - 8 - tline - 2, g_intTerminalCol - 10, 4 + tline + 1, 5);
		wborder(boxp,'|','|','-','-','+','+','+','+');
		WINDOW *box = newwin(g_intTerminalRow - 8 - tline - 4, g_intTerminalCol - 12, 4 + tline + 2, 6);
		
		mvwaddstr(wid,g_intTerminalRow + 8 - 17,(g_intTerminalCol - 8)/2-strlen("[ Escで確定 ]")/2,"[ Escで確定 ]");
		
		wmove(box,0,0);
		
		wrefresh(win);
		wrefresh(wid);
		wrefresh(boxp);
		wrefresh(box);
		
		Editor ed(box);
		
		noecho();
		cbreak();
		keypad(box, true);
		
		while(ed.getMode() != 'x') {
			ed.updateStatus();
			ed.printStatusLine();
			ed.printBuff();
			int input = wgetch(box);
			ed.handleInput(input);
			wrefresh(box);
		}
		
		echo();
		nocbreak();
		keypad(box, false);
		
		req = (char *)malloc(4096);
		strcpy(req, ed.saved.c_str());
		
		wclear(boxp);
		wclear(box);
		wrefresh(boxp);
		wrefresh(box);
		
		delwin(box);
		delwin(boxp);
	}
	
	wclear(win);
	wclear(wid);
	wrefresh(win);
	wrefresh(wid);
	
	delwin(win);
	delwin(wid);
	
	waddch(g_winTimeLine,' ');
	waddch(g_winTimeLine,0x08);
	wrefresh(g_winTimeLine);
	refresh();
	
	return req;
}

char *tuiFullMessageBox(const char *title, const char *text, int reqinput)
{
	WINDOW *win = newwin(g_intTerminalRow, g_intTerminalCol, 0, 0);
	
	overlay(g_winTimeLine,win);
	
	keypad(win,true);
	
	mvwaddstr(win,0,0,title);
	mvwaddstr(win,2,0,text);
	
	char *req = NULL;
	
	if(!reqinput) {
		mvwaddstr(win,g_intTerminalRow - 2,g_intTerminalCol/2-strlen("[ PRESS ENTER ]")/2,"[ PRESS ENTER ]");
		wrefresh(win);
		int ch = wgetch(win);
		while(ch != KEY_ENTER && ch != 0x0a) ch = wgetch(win);
	} else {
		mvwaddch(win,g_intTerminalRow - 2, 1, '[');
		mvwaddch(win,g_intTerminalRow - 2, g_intTerminalCol - 1, ']');
		wmove(win,g_intTerminalRow - 2, 2);
		wrefresh(win);
		req = (char *)malloc(512);
		wgetstr(win,req);
	}
	
	wclear(win);
	wrefresh(win);
	delwin(win);
	
	waddch(g_winTimeLine,' ');
	waddch(g_winTimeLine,0x08);
	wrefresh(g_winTimeLine);
	refresh();
	
	return req;
}
