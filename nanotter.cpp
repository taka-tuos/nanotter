#include <iostream>
#include <fstream>
#include <string>
#include <twitcurl.h>
#include <ncurses.h>
#include <json/json.h>
#include <locale.h>
#include <pthread.h>

#include "nanotter.h"

using namespace std;

twitCurl g_twitterObj_Stream;
twitCurl g_twitterObj;

extern string g_strConsumerKey;
extern string g_strConsumerSecret;
extern string g_strAccessTokenKey;
extern string g_strAccessTokenSecret;

extern string g_strRequestTokenUrl;

char *g_StreamBuffer = NULL;

WINDOW *g_winTimeLine;

pthread_mutex_t g_objMutex;
pthread_t g_objTid1;

int g_intTerminalRow,g_intTerminalCol;

void executeCommand(WINDOW *win, const char *cmd)
{
	if(!strcmp(cmd,"t") || !strcmp(cmd,"tweet")) {
		g_twitterObj.statusUpdate(tuiFloatMessageBox("ついーとしよう","ツイート内容を入力してね",1));
	}
	
	if(!strcmp(cmd,"h") || !strcmp(cmd,"help")) {
		tuiFloatMessageBox("コマンド一覧","help : この画面を表示\ntweet : ついーと",0);
	}
}

void* threadUserInterface(void* pParam)
{
	WINDOW *win = (WINDOW *)pParam;
	
	while(1) {
		wclear(win);
		wmove(win,0,0);
		waddstr(win,">");
		wrefresh(win);
		char cmd[512];
		wgetstr(win,cmd);
		executeCommand(win,cmd);
	}
}

int main() {
	// 画面初期化
	setlocale(LC_ALL, "");
	
	initscr();
	getmaxyx(stdscr,g_intTerminalRow,g_intTerminalCol);
	resize_term(g_intTerminalRow,g_intTerminalCol);
	scrollok(stdscr, FALSE);
	
	start_color();
	for(int i = 0; i < 8; i++) {
		init_pair(i, i, COLOR_BLACK);
	}
	
	// PINとかそのへんの処理
	apiGetTokenKey();
	
	// TLを流す
	g_winTimeLine = newwin(g_intTerminalRow - 2, g_intTerminalCol, 1, 0);
	scrollok(g_winTimeLine, TRUE);
	
	mvaddstr(g_intTerminalRow - 1,g_intTerminalCol - strlen(VERSION_NANOTTER) - 1,VERSION_NANOTTER);
	refresh();
	
	pthread_create(&g_objTid1, NULL, threadUserInterface, 
		newwin(1, g_intTerminalCol, 0, 0));
	
	
	g_twitterObj_Stream.timelineHomeGetStream(apiStreamingCallback);
	
	endwin();
	
	return 0;
}
