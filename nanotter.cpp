#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <twitcurl.h>
#include <ncurses.h>
#include <json/json.h>

using namespace std;

twitCurl g_twitterObj;

string g_strConsumerKey       = "Brbukpk7aHle3rIbSlpTvvleU";
string g_strConsumerSecret    = "PcoimOIhAhwHVK3SZ0NlSifX8PLjBcLT90FnKaiL5vgzVHG1hK";
string g_strAccessTokenKey    = "nil";
string g_strAccessTokenSecret = "nil";

string g_strRequestTokenUrl;

int streaming_callback_sample(char* ptr, size_t size, size_t nmemb, void* data) {
	if (size * nmemb == 0)
		return 0;

	size_t realsize = size * nmemb;
	char str[32768];
	
	memcpy(str, ptr, realsize);
	
	if(realsize == 2) return realsize;
	
	str[strlen(str) - 1] = '\0';
	
	struct json_object *obj = json_tokener_parse(str);
	
	char *tweet_text,*tweeter_name;
	
	struct json_object *val = json_object_object_get(obj,"text");
	if(val) {
		tweet_text = json_object_get_string(val);
		val = json_object_object_get(obj,"user");
		tweeter_name = json_object_get_string(json_object_object_get(val,"name"));
		
		printf("NAME : %s\n%s\n\n",tweeter_name,tweet_text);
	}

	//fprintf(stderr, "RECIEVED: %ld bytes\n", realsize);
	//fprintf(stderr, "[STREAMING API] received -> %s\n", str);

	return realsize;
}

int main() {
	g_twitterObj.getOAuth().setConsumerKey(g_strConsumerKey);
	g_twitterObj.getOAuth().setConsumerSecret(g_strConsumerSecret);
	
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
				
				string token,tokensecret;
				
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
			string token,tokensecret;
			
			tokenfile >> token;
			tokenfile >> tokensecret;
			
			g_twitterObj.getOAuth().setOAuthTokenKey(token);
			g_twitterObj.getOAuth().setOAuthTokenSecret(tokensecret);
		}
	}
	
	g_twitterObj.timelineHomeGetStream(streaming_callback_sample);
	
	//initscr();
	
	/*
	auto start = std::chrono::system_clock::now();
	
	while(1) {
		auto end = std::chrono::system_clock::now();
		auto diff = end - start;
		int etime = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
		
		if(etime > 1000) {
			start = std::chrono::system_clock::now();
			
			g_twitterObj.timelineHomeGet();
			
			string json;
			
			g_twitterObj.getLastWebResponse(json);
			
			//priintw(json);
			
			json = "{\"timeline_array\":" + json + "}";
			
			if(json.length()) {
				//cout << json.c_str() << endl;
				
				struct json_object *jobj_from_string = json_tokener_parse(json.c_str());
				libjson_parse_check_type(jobj_from_string);
			}
		}
	}
	*/
	//endwin();
	
	return 0;
}
