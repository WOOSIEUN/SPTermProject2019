#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>

#define ESC 0x1b

void write_content();
void write_title();
void write_admit();
void write_end_time();
void write_start_time();
void write_date();
void screen_fix_info();

int main(){
	initscr();
	clear();
	screen_fix_info();
	write_date();
	getch();
	endwin();
	return 0;
}

void write_content()
{
	move(15,25);
	char content[2000];
	scanw("%s",content);
	getch();
	refresh();
}

void write_title()
{
	move(13,25);
	char title[40];
	scanw("%s",title);
	//getch();
	refresh();

	write_content();

}

void write_admit()
{
	move(11,25);
	char admit[5];
	scanw("%s",admit);
	//getch();
	refresh();

	write_title();
}

void write_end_time()
{
	move(9,25);
	char end_time[5];
	scanw("%s",end_time);
	//getch();
	refresh();

	write_admit();
}

void write_start_time()
{
	move(7,25);
	char start_time[5];
	scanw("%s",start_time);
	//getch();
	refresh();

	write_end_time();
}

void write_date()
{
	move(5,25);
	char data[10];
	scanw("%s",data);
	//printw("%s\n",data);
	refresh();
	write_start_time();
}

void screen_fix_info()
{
	move(5,10);
	addstr("Date");
	move(5,20);
	addstr(":");

	move(7,10);
	addstr("Start time");
	move(7,20);
	addstr(":");

	move(9,10);
	addstr("End time");
	move(9,20);
	addstr(":");

	move(11,10);
	addstr("Admit");
	move(11,20);
	addstr(":");

	move(13,10);
	addstr("Title");
	move(13,20);
	addstr(":");

	move(15,10);
	addstr("write");
	move(15,20);
	addstr(":");

	refresh();

}
