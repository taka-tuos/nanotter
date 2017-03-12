#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <twitcurl.h>
#include <ncurses.h>
#include <json/json.h>
#include <locale.h>
#include <pthread.h>

using namespace std;

twitCurl g_twitterObj_Stream;
twitCurl g_twitterObj;

string g_strConsumerKey       = "Brbukpk7aHle3rIbSlpTvvleU";
string g_strConsumerSecret    = "PcoimOIhAhwHVK3SZ0NlSifX8PLjBcLT90FnKaiL5vgzVHG1hK";
string g_strAccessTokenKey    = "nil";
string g_strAccessTokenSecret = "nil";

string g_strRequestTokenUrl;

WINDOW *g_winTimeLine;

pthread_mutex_t g_objMutex;
pthread_t g_objTid1;

int g_intTerminalRow,g_intTerminalCol;

int g_StreamFlag = 0;

#define attrcol(f,b) (COLOR_PAIR((((f)<<3)|b)))

void WriteStringColorTL(int f, char *s)
{
	wattrset(g_winTimeLine,COLOR_PAIR(f));
	waddstr(g_winTimeLine,s);
}

int APIStreamingCallback(char* ptr, size_t size, size_t nmemb, void* data) {
	if (size * nmemb == 0)
		return 0;

	size_t realsize = size * nmemb;
	char str[32768];
	
	memcpy(str, ptr, realsize);
	
	if(realsize == 2) return realsize;
	
	str[strlen(str) - 1] = '\0';
	
	struct json_object *obj = json_tokener_parse(str);
	
	struct json_object *val = json_object_object_get(obj,"text");
	if(val) {
		char *text,*name,*screen_name;
		
		text = json_object_get_string(val);
		val = json_object_object_get(obj,"user");
		name = json_object_get_string(json_object_object_get(val,"name"));
		screen_name = json_object_get_string(json_object_object_get(val,"screen_name"));
		
		WriteStringColorTL(COLOR_GREEN,name);
		WriteStringColorTL(COLOR_WHITE," ");
		WriteStringColorTL(COLOR_CYAN,"@");
		WriteStringColorTL(COLOR_CYAN,screen_name);
		WriteStringColorTL(COLOR_WHITE,"\n");
		WriteStringColorTL(COLOR_WHITE,text);
		WriteStringColorTL(COLOR_WHITE,"\n");
		WriteStringColorTL(COLOR_WHITE,"\n");
		
		wrefresh(g_winTimeLine);
		refresh();
	}

	return realsize;
}

int wkbhit(WINDOW *win)
{
	int ch = wgetch(win);

	if (ch != ERR) {
		ungetch(ch);
		return 1;
	} else {
		return 0;
	}
}

void ExecCommnad(WINDOW *win, int ch)
{
	if(ch == 't') {
		mvwaddstr(win,0,0,"ツイートモード\n");
		char tweet[512];
		wmove(win,1,1);
		waddch(win,' ');
		waddch(win,'\b');
		wrefresh(win);
		wgetstr(win,tweet);
		g_twitterObj.statusUpdate(tweet);
		wclear(win);
		mvwaddstr(win,0,0,"t:ツイートモード\n");
		waddstr(win,":");
		wrefresh(win);
	}
}

void* CommandThread(void* pParam)
{
	WINDOW *win = (WINDOW *)pParam;
	
	mvwaddstr(win,0,0,"t:ツイート\n");
	waddstr(win,":");
	
	while(1) {
		wmove(win,1,1);
		wrefresh(win);
		if(wkbhit(win)) ExecCommnad(win, wgetch(win));
	}
}

int main() {
	g_twitterObj.getOAuth().setConsumerKey(g_strConsumerKey);
	g_twitterObj.getOAuth().setConsumerSecret(g_strConsumerSecret);
	
	string token,tokensecret;
	
	// PINとかそのへんの処理
	{
		ifstream tokenfile(".nanotter");
		
		if(tokenfile.fail()) {
			ofstream newtoken(".nanotter");
			g_twitterObj.oAuthRequestToken(g_strRequestTokenUrl);
			bool isValidPIN = false;
			while(!isValidPIN) {
				cout << "このURLにアクセスしてPINコードを入力してください" + g_strRequestTokenUrl << endl;
				cout << "PINコード : ";
				
				string PIN;
				
				cin >> PIN;
				
				cout << endl;
				
				g_twitterObj.getOAuth().setOAuthPin(PIN);
				
				isValidPIN = g_twitterObj.oAuthAccessToken();
				
				if(isValidPIN) {
					g_twitterObj.getOAuth().getOAuthTokenKey(token);
					g_twitterObj.getOAuth().getOAuthTokenSecret(tokensecret);
					
					newtoken << token << endl;
					newtoken << tokensecret << endl;
					
					cout << "認証成功" << endl;
				} else {
					cout << "認証失敗" << endl;
				}
			}
		} else {
			tokenfile >> token;
			tokenfile >> tokensecret;
			
			g_twitterObj.getOAuth().setOAuthTokenKey(token);
			g_twitterObj.getOAuth().setOAuthTokenSecret(tokensecret);
		}
	}
	
	g_twitterObj_Stream.getOAuth().setConsumerKey(g_strConsumerKey);
	g_twitterObj_Stream.getOAuth().setConsumerSecret(g_strConsumerSecret);
	g_twitterObj_Stream.getOAuth().setOAuthTokenKey(token);
	g_twitterObj_Stream.getOAuth().setOAuthTokenSecret(tokensecret);
	
	// TLを流す
	setlocale(LC_ALL, "");
	
	initscr();
	getmaxyx(stdscr,g_intTerminalRow,g_intTerminalCol);
	
	start_color();
	for(int i = 0; i < 8; i++) {
		init_pair(i, i, COLOR_BLACK);
	}
	
	g_winTimeLine = newwin(g_intTerminalRow - 3, g_intTerminalCol, 1, 0);
	scrollok(g_winTimeLine, TRUE);
	
	mvaddstr(0,0,"--- nanotter alpha-0.1 ---\n");
	refresh();
	
	//WriteStringColorTL(COLOR_CYAN,"ようこそnanotterへ！\n");
	//wrefresh(g_winTimeLine);
	
	pthread_create(&g_objTid1, NULL, CommandThread, 
		newwin(2, g_intTerminalCol, g_intTerminalRow - 2, 0));
	
	g_twitterObj_Stream.timelineHomeGetStream(APIStreamingCallback);
	
	endwin();
	
	return 0;
}
