#include "nanotter.h"
#include <json/json.h>
#include <ncurses.h>
#include <twitcurl.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

extern twitCurl g_twitterObj_Stream;
extern twitCurl g_twitterObj;

extern WINDOW *g_winTimeLine;
extern char *g_StreamBuffer;

string g_strConsumerKey       = "Brbukpk7aHle3rIbSlpTvvleU";
string g_strConsumerSecret    = "PcoimOIhAhwHVK3SZ0NlSifX8PLjBcLT90FnKaiL5vgzVHG1hK";
string g_strAccessTokenKey    = "nil";
string g_strAccessTokenSecret = "nil";

string g_strRequestTokenUrl;

int apiStreamingCallback(char* ptr, size_t size, size_t nmemb, void* data)
{
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
