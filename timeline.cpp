#include "nanotter.h"
#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

extern WINDOW *g_winTimeLine;

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
