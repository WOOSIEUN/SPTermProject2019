#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#define UP 119		//[w] = up
#define DOWN 115	//[s] = down
#define ENTER 13	//[enter] = enter

void set_SIGIO();
void start_Screen();
void set_SIGIO();
void log_In();
void sign_In();
void start_Handler(int sn);


int main()
{
	set_SIGIO();
	start_Screen();
}

//----------------set sigio signal
void set_SIGIO()
{
	int fd_flags;

	fcntl(0,F_SETOWN,getpid());
	fd_flags = fcntl(0,F_GETFL);
	fcntl(0,F_SETFL,(fd_flags|O_ASYNC));
}

//----------------print start menu
void start_Screen()
{
	int input;

	noecho();	//set noecho mode
	signal(SIGIO,start_Handler);//set sigio handler

	//print main screen
	initscr();
	clear();
	mvaddstr(2 ,18, "+-+-+-+-+-+-+-+-+-+-+");
	mvaddstr(3 ,18, "  M o  y e o  R a    ");
	mvaddstr(4 ,18, "   S C H E D U L E R ");
	mvaddstr(5 ,18, " +-+-+-+-+-+-+-+-+-+-+");

	mvaddstr(7 ,23, "   ^    :   w   ");
	mvaddstr(8 ,23, " < v >  : a s d ");
	mvaddstr(9 ,23, " select : enter ");
	
	mvaddstr(15,20, "Login");
	mvaddstr(16,20, "Sign in");
	mvaddstr(17,20, "Exit");

      	mvaddstr(15,18, ">");           //default curser = login

	refresh();

	while(1)
		pause();
	
}

void start_Handler(int signum)
{
	int  input;
	int x,y,x_before;

	getyx(curscr,x,y);

	input = getchar();	//get input 

	switch(input)
	{
	case 119://--Up [w]
		x_before = x;	//save before x for erase 
		if(--x == 14)
		       	x=17;	
		mvaddstr(x_before,y-1," ");
		mvaddstr(x,y-1,">");
		refresh();
		break;
	case 115://--Down [s]
		x_before = x;
		if(++x == 18)
			x=15;
		mvaddstr(x_before,y-1," ");
                mvaddstr(x,y-1,">");
                refresh();
		break;
	case 13://--Enter
		switch(x)
		{
		case 15://--> Log in
			log_In();
			break;
		case 16://--> Sign in
			sign_In();
			break;
		case 17://--> Exit
			endwin();
			exit(1);
			break;
		}	
		break;

	}
	

}

void log_In(void)
{
	clear();
	mvaddstr(10,10,"Log in");
	refresh();
	
}

void sign_In(void)
{
	clear();
	mvaddstr(10,10,"Sign in");
	refresh();
}
