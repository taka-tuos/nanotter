#include <iostream>
#include <fstream>
#include <string>
#include <twitcurl.h>
#include <ncurses.h>
#include <json/json.h>
#include <locale.h>
#include <pthread.h>

#include "Editor.h"

#define VERSION_NANOTTER "nanotter alpha-0.1"

using namespace std;

twitCurl g_twitterObj_Stream;
twitCurl g_twitterObj;

string g_strConsumerKey       = "Brbukpk7aHle3rIbSlpTvvleU";
string g_strConsumerSecret    = "PcoimOIhAhwHVK3SZ0NlSifX8PLjBcLT90FnKaiL5vgzVHG1hK";
string g_strAccessTokenKey    = "nil";
string g_strAccessTokenSecret = "nil";

string g_strRequestTokenUrl;

char *g_StreamBuffer = NULL;

WINDOW *g_winTimeLine;

pthread_mutex_t g_objMutex;
pthread_t g_objTid1;

int g_intTerminalRow,g_intTerminalCol;

char *tuiFloatMessageBox(const char *title, const char *text, int reqinput)
{
	WINDOW *win = newwin(g_intTerminalRow - 6, g_intTerminalCol - 6, 3, 3);
	WINDOW *wid = newwin(g_intTerminalRow - 8, g_intTerminalCol - 8, 4, 4);
	
	wborder(win,'|','|','-','-','+','+','+','+');
	
	mvwaddstr(win,0,3,title);
	mvwaddstr(wid,1,0,text);
	
	mvwaddstr(wid,1,0,text);
	
	char *req = NULL;
	
	if(!reqinput) {
		mvwaddstr(wid,g_intTerminalRow - 8 - 2,(g_intTerminalCol - 8)/2-strlen("[ PRESS ENTER ]")/2,"[ PRESS ENTER ]");
		wrefresh(win);
		wrefresh(wid);
		int ch = wgetch(wid);
		while(ch != KEY_ENTER && ch != 0x0a) ch = wgetch(wid);
	} else {
		WINDOW *boxp = newwin(12, g_intTerminalCol - 10, g_intTerminalRow - 17, 5);
		wborder(boxp,'|','|','-','-','+','+','+','+');
		WINDOW *box = newwin(10, g_intTerminalCol - 12, g_intTerminalRow - 16, 6);
		
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
	
	wrefresh(g_winTimeLine);
	
	delwin(win);
	delwin(wid);
	
	wrefresh(g_winTimeLine);
	
	return req;
}

char *tuiFullMessageBox(const char *title, const char *text, int reqinput)
{
	WINDOW *win = newwin(g_intTerminalRow, g_intTerminalCol, 0, 0);
	
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
	
	wrefresh(g_winTimeLine);
	
	return req;
}

void strWriteStringColorTimeLine(int f, const char *s)
{
	wattrset(g_winTimeLine,COLOR_PAIR(f));
	waddstr(g_winTimeLine,s);
}

void strWriteDataTimeLine(const char *str)
{
	struct json_object *obj = json_tokener_parse(str);
	
	struct json_object *text,*user,*name,*screen_name;
	json_object_object_get_ex(obj,"text",&text);
	if(text) {
		char *json_text,*json_name,*json_screen_name;
		
		json_text = (char *)json_object_get_string(text);
		
		json_object_object_get_ex(obj,"user",&user);
		
		json_object_object_get_ex(user,"name",&name);
		json_name = (char *)json_object_get_string(name);
		
		json_object_object_get_ex(user,"screen_name",&screen_name);
		
		json_screen_name = (char *)json_object_get_string(screen_name);
		
		strWriteStringColorTimeLine(COLOR_GREEN,json_name);
		strWriteStringColorTimeLine(COLOR_WHITE,(char *)" ");
		strWriteStringColorTimeLine(COLOR_CYAN,(char *)"@");
		strWriteStringColorTimeLine(COLOR_CYAN,json_screen_name);
		strWriteStringColorTimeLine(COLOR_WHITE,(char *)"\n");
		strWriteStringColorTimeLine(COLOR_WHITE,json_text);
		strWriteStringColorTimeLine(COLOR_WHITE,(char *)"\n");
		strWriteStringColorTimeLine(COLOR_WHITE,(char *)"\n");
		
		wrefresh(g_winTimeLine);
		refresh();
	}
}

int apiStreamingCallback(char* ptr, size_t size, size_t nmemb, void* data) {
	if (size * nmemb == 0)
		return 0;

	size_t realsize = size * nmemb;
	char str[32768];
	
	memcpy(str, ptr, realsize);
	
	if(realsize == 2) return realsize;
	
	str[realsize] = '\0';
	
	int cr = str[realsize - 2];
	int lf = str[realsize - 1];
	int len = 0;
	
	if(g_StreamBuffer) len = strlen(g_StreamBuffer);
	
	g_StreamBuffer = (char *)realloc(g_StreamBuffer, len + realsize + 1);
	
	memcpy(g_StreamBuffer + len, str, realsize + 1);
	
	if(cr == 0x0d && lf == 0x0a)
	{
		strWriteDataTimeLine(g_StreamBuffer);
		free(g_StreamBuffer);
		g_StreamBuffer = NULL;
	}

	return realsize;
}

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

void apiGetTokenKey(void)
{
	string token,tokensecret;
	
	g_twitterObj.getOAuth().setConsumerKey(g_strConsumerKey);
	g_twitterObj.getOAuth().setConsumerSecret(g_strConsumerSecret);
	
	ifstream tokenfile(".nanotter");
	
	if(tokenfile.fail()) {
		g_twitterObj.oAuthRequestToken(g_strRequestTokenUrl);
		bool isValidPIN = false;
		while(!isValidPIN) {
			string text = "このURLにアクセスしてPINコードを入力してください。\n" + 
						g_strRequestTokenUrl +
						"\n認証できない場合時間をおいてお試しください。";
			
			string pincode = tuiFullMessageBox("PINコード認証",text.c_str(),1);
			
			g_twitterObj.getOAuth().setOAuthPin(pincode);
			
			g_twitterObj.oAuthAccessToken();
			
			string resp;
			
			g_twitterObj.getLastWebResponse(resp);
			
			isValidPIN = (strncmp(resp.c_str(),"oauth",5) == 0);
			
			if(isValidPIN) {
				ofstream newtoken(".nanotter");
				
				g_twitterObj.getOAuth().getOAuthTokenKey(token);
				g_twitterObj.getOAuth().getOAuthTokenSecret(tokensecret);
				
				newtoken << token << endl;
				newtoken << tokensecret << endl;
				
				tuiFloatMessageBox("メッセージ","認証成功",0);
			} else {
				tuiFloatMessageBox("エラー","認証失敗",0);
			}
		}
	} else {
		tokenfile >> token;
		tokenfile >> tokensecret;
		
		g_twitterObj.getOAuth().setOAuthTokenKey(token);
		g_twitterObj.getOAuth().setOAuthTokenSecret(tokensecret);
	}
	
	g_twitterObj_Stream = *g_twitterObj.clone();
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
