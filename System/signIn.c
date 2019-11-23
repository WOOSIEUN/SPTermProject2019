// *Sign In 
// .Bomi
#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#define MCODE 486	//master code 

void set_SIGIO();//exits in startMenu.c 
void sign_In();
void signIn_Check(int,int,int,char*,char*);

int main()
{
	//set_SIGIO();
	initscr();
	sign_In();
	endwin();	
}

void set_SIGIO()
{
        int fd_flags;

        fcntl(0,F_SETOWN,getpid());
        fd_flags = fcntl(0,F_GETFL);
        fcntl(0,F_SETFL,(fd_flags|O_ASYNC));
}

void sign_In(void)
{
	FILE* userData = fopen("../Data/SystemData/userData.txt","a");
	char fullStatus[70];	//ID:PW:Name:isMaster
	char buffer[100];	//buffer to get user input

	char ID[20];
	char PW[20];
	char Name[20];
	int isMaster;

	int index=12; //input start position

//	while(1)
//	{	
		clear();
		//default page
		mvaddstr(0 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
		mvaddstr(1 ,0 ,"|                  Sign In                    |");
		mvaddstr(2 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
		mvaddstr(3 ,0 ,"|   [ Ctrl + c ]   : Return to Main Page      |");
		mvaddstr(4 ,0 ,"|   [    Esc   ]   : Restart your input       |");
		mvaddstr(5 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
			
		//1. ID
		mvaddstr(6 ,0 ,"|             1. Input your ID                |");
		mvaddstr(7 ,0 ,"|  [  engligh + num ,   5  < length < 10   ]  |");
		mvaddstr(8 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

		mvaddstr(index,0 ,"           ID :");
		signIn_Check(5,10,index,buffer,"ID");
		strcpy(ID,buffer);
		index +=2;
		//remove warning
		mvprintw(9,0,"                                                        ");
		
		//2. PW
		mvaddstr(6 ,0 ,"|             2. Input your PW                |");
		mvaddstr(7 ,0 ,"|  [  engligh + num ,   5  < length < 10   ]  |");
		mvaddstr(8 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

		mvaddstr(index,0 ,"           PW :");
		signIn_Check(5,10,index,buffer,"PW");
		strcpy(PW,buffer);
		index +=2;
		//remove warning
		mvprintw(9,0,"                                                        ");


		//3. NAME
		mvaddstr(6 ,0 ,"|           3. Input your Name                |");
		mvaddstr(7 ,0 ,"|      [  engligh  ,   1< length < 10   ]     |");
		mvaddstr(8 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

		mvaddstr	(index,0 ,"         NAME :");
		signIn_Check(1,10,index,buffer,"NAME");
		strcpy(Name,buffer);
		index +=2;
		//remove warning
		mvprintw(9,0,"                                                        ");

		mvprintw(7,0,"                                                        ");
		//4. isMaster
		mvaddstr(6 ,0 ,"|           4. Are you MASTER?                |");
		mvaddstr(7 ,0 ,"|                  [ y/n ]                    |");
		mvaddstr(8 ,0 ,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
		mvaddstr(index,0 ,"      MASTER? :");
		refresh();

		while(1)
		{
			char c;
			mvscanw(index,15,"%c",&c);

			if(c == 'y')//Master -> get Master Code
			{
				int mcode;

				mvaddstr(index+1,0,"   MASTER CODE:");
				refresh();
				mvscanw(index+1,15,"%d",&mcode);

				if(mcode != MCODE)//if user input wrong code -> return to main page
				{
					mvaddstr(index+3,0,"      !!       YOUR MCODE IS WRONG. RETURN TO MAIN PAGE . . .");
					refresh();
					sleep(5);

					return;
				}

				isMaster = 1;
				break;
			}
			else if(c == 'n')
			{
				isMaster =0;
				break;
			}
		
			mvprintw(9,0,"   !!    YOUR ANSWER SHOULD BE 'y' OR 'n' !!  TRY AGAIN  ");

		}

	
		sprintf(fullStatus,"%s:%s:%s:%d",ID,PW,Name,isMaster);
		fprintf(userData,"%s\n",fullStatus);

		mvprintw(22,0,"                 YOUR SIGN IN IS DONE. TRY LOGIN ");
		mvprintw(23,0,"                             -> RETURN TO MAIN PAGE. . . . .");
		refresh();
	
		sleep(5);
//	}
	
	fclose(userData);
		
}

void signIn_Check(int minLen,int maxLen,int index,char* buffer,char* label)
{
	while(1)
	{
		refresh();

                mvscanw(index,15,"%s",buffer);

                //length check
                if(strlen(buffer) <minLen || strlen(buffer) >maxLen)
                {
                        mvprintw(9,0,"   !!   YOUR %s SHOULD BE IN %d~%d LENGTH !!  TRY AGAIN  ",label,minLen,maxLen);
                        //remove input
                        mvprintw(index,15,"                                                      ");
                }
                else
                        break;
	}
}


