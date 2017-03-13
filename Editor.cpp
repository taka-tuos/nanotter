#include "Editor.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

Editor::Editor(WINDOW *uwin)
{
	x=0;y=0;mode='i',win=uwin;
	status = "Normal Mode";

	/* Initializes buffer and appends line to
		prevent seg. faults */
	buff = new Buffer();
	buff->appendLine("");
	getmaxyx(win,row,col);
	
	updateStatus();
}

void Editor::updateStatus()
{
	switch(mode)
	{
	case 'n':
		// Normal mode
		status = "Normal Mode";
		break;
	case 'i':
		// Insert mode
		status = "Insert Mode";
		break;
	case 'x':
		// Exiting
		status = "Exiting";
		break;
	}
	status += "\tCOL: " + tos(x) + "\tROW: " + tos(y);
}

string Editor::tos(int i)
{
	stringstream ss;
	ss << i;
	return ss.str();
}

void Editor::handleInput(int c)
{
	switch(c)
	{
	case KEY_LEFT:
		moveLeft();
		return;
	case KEY_RIGHT:
		moveRight();
		return;
	case KEY_UP:
		moveUp();
		return;
	case KEY_DOWN:
		moveDown();
		return;
	}
	switch(mode)
	{
	case 'n':
		switch(c)
		{
		case 'x':
			// Press 'x' to exit
			mode = 'x';
			break;
		case 'i':
			// Press 'i' to enter insert mode
			mode = 'i';
			break;
		}
		break;
	case 'i':
		switch(c)
		{
		case 27:
			// The Escape/Alt key
			mode = 'x';
			saveFile();
			break;
		case 127:
		case KEY_BACKSPACE:
			// The Backspace key
			if(x == 0 && y > 0)
			{
				x = buff->lines[y-1].length();
				// Bring the line down
				buff->lines[y-1] += buff->lines[y];
				// Delete the current line
				deleteLine();
				moveUp();
			}
			else
			{
				// Removes a character
				if(x != 0) buff->lines[y].erase(--x, 1);
			}
			break;
		case KEY_DC:
			// The Delete key
			if(x == buff->lines[y].length() && y != buff->lines.size() - 1)
			{
				// Bring the line down
				buff->lines[y] += buff->lines[y+1];
				// Delete the line
				deleteLine(y+1);
			}
			else
			{
				buff->lines[y].erase(x, 1);
			}
			break;
		case KEY_ENTER:
		case 10:
			// The Enter key
			// Bring the rest of the line down
			if(x < buff->lines[y].length())
			{
				// Put the rest of the line on a new line
				buff->insertLine(buff->lines[y].substr(x, buff->lines[y].length() - x), y + 1);
				// Remove that part of the line
				buff->lines[y].erase(x, buff->lines[y].length() - x);
			}
			else
			{
				buff->insertLine("", y+1);
			}
			x = 0;
			moveDown();
			break;
		case KEY_BTAB:
		case KEY_CTAB:
		case KEY_STAB:
		case KEY_CATAB:
		case 9:
			// The Tab key
			buff->lines[y].insert(x, 4, ' ');
			x += 4;
			break;
		default:
			// Any other character
			buff->lines[y].insert(x, 1, char(c));
			int oldsubx = 0;
			if(c >= 0x00 && c <= 0x7f) subx = 0;
			if(c >= 0xc2 && c <= 0xdf) subx = 1;
			if(c >= 0xe0 && c <= 0xef) subx = 2;
			if(c >= 0xf0 && c <= 0xf7) subx = 3;
			if(c >= 0xf8 && c <= 0xfb) subx = 4;
			if(c >= 0xfc && c <= 0xfd) subx = 5;
			x++;
			break;
		}
		break;
	}
}

void Editor::moveLeft()
{
	if(x-1 >= 0)
	{
		x--;
		wmove(win, y, x);
	}
}

void Editor::moveRight()
{
	if(x+1 < col && x+1 <= buff->lines[y].length())
	{
		x++;
		wmove(win, y, x);
	}
}

void Editor::moveUp()
{
	if(y-1 >= 0)
		y--;
	if(x >= buff->lines[y].length())
		x = buff->lines[y].length();
	wmove(win, y, x);
}

void Editor::moveDown()
{
	if(y+1 < row-1 && y+1 < buff->lines.size())
		y++;
	if(x >= buff->lines[y].length())
		x = buff->lines[y].length();
	wmove(win, y, x);
}

void Editor::printBuff()
{
	for(int i=0; i<row-1; i++)
	{
		if(i >= buff->lines.size())
		{
			wmove(win, i, 0);
			wclrtoeol(win);
		}
		else
		{
			mvwprintw(win, i, 0, buff->lines[i].c_str());
		}
		wclrtoeol(win);
	}
	wmove(win, y, x);
}

void Editor::printStatusLine()
{
	wattron(win, A_REVERSE);
	mvwprintw(win, row-1, 0, status.c_str());
	wclrtoeol(win);
	wattroff(win, A_REVERSE);
}

void Editor::deleteLine()
{
	buff->removeLine(y);
}

void Editor::deleteLine(int i)
{
	buff->removeLine(i);
}

void Editor::saveFile()
{
	stringstream f;
	
	for(int i=0; i<buff->lines.size(); i++)
	{
		f << buff->lines[i] << endl;
	}
	
	saved = f.str();
}
