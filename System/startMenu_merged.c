// *Start Menu 
// .Bomi
//---------HEADER FILE
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

//----------DEFINITIONS
#define USER_MAX 100
#define MCODE 486	//master code 

#define QUIT_SIGN "quit"
#define UP 119		//[w] = up
#define DOWN 115	//[s] = down
#define ENTER 13	//[enter] = enter

//-------------------------STRUCTURES
//1. user data structure
typedef struct
{
	char ID[20];
	char PW[20];
	char Name[20];
	int isMaster;
}user;

//--------------------------GLOBAL VARIABLES
//1. startmenu variables
int login_return = 0;
int signin_return = 0;
int choice = 0;
//2. Login
user userData[USER_MAX]; //structure array for userData
int userData_Size;	//size of userData array
int thisUser_Index;	//index of this user in structure array

//---------------------------FUNCTIONS
//1. start menu
void set_SIGIO();
void start_Screen();
void start_Handler(int sn);
void get_userData();
//2. Login
int log_In();
int find_LoginUser(char*);
//3. sign In
int sign_In();
int signIn_Check(int, int, int, char*, char*);
int dup_Check(char*, char* );

int main()
{
	set_SIGIO();	//1. set sigio signal
	get_userData();	//2. make user structure array from userData.txt

	start_Screen(); //3. start screen 
		//-> in this function, login & signin are excecuted.
		// now you can start main page ( schedule )
		//but should do clear();

	//you can earse under this line after merge
	endwin();
}

//	< set sigio signal >
void set_SIGIO()
{
	int fd_flags;

	fcntl(0,F_SETOWN,getpid());
	fd_flags = fcntl(0,F_GETFL);
	fcntl(0,F_SETFL,(fd_flags|O_ASYNC));
}

//	< print start menu >
void start_Screen()
{
	noecho();	//set noecho mode
	
	//print main screen
	initscr();

	while (login_return == 0 && signin_return == 0)
	{
		signal(SIGIO,start_Handler);//set sigio handler
		move(LINES-1,COLS-1);
		
		clear();

		mvaddstr(2 ,18, "+-+-+-+-+-+-+-+-+-+-+");
		mvaddstr(3 ,18, "  M o  y e o  R a    ");
		mvaddstr(4 ,18, "   S C H E D U L E R ");
		mvaddstr(5 ,18, " +-+-+-+-+-+-+-+-+-+-+");

		mvaddstr(7 ,23, "   ^    :   w   ");
		mvaddstr(8 ,23, " < v >  : a s d ");
		mvaddstr(9 ,23, " select : enter ");
	
		if (choice)
		{
			mvaddstr(15, 23, "     LOG IN     ");	// choice (0)  default -> login 
			standout();
			mvaddstr(16, 23, "     SIGN IN    ");	// choice (1)
			standend();
		}
		else
		{
			standout();
			mvaddstr(15, 23, "     LOG IN     ");	// choice (0)  
			standend();
			mvaddstr(16, 23, "     SIGN IN    ");	// choice (1)
		}

		refresh();
		pause();
	}	
}

/*	< Handler for startMenu input >
if UP ,DOWN : change state
if ENTER : execute function 
*/
void start_Handler(int signum)
{
	int  input;

	input = getchar();	//get input 

	switch(input)
	{
	case UP:case DOWN:// [w] [s]
		choice = !choice;
		break;
	case ENTER:// [enter]
		if (choice == 0) //---->login
			login_return = log_In();
		else if (choice == 1)
			signin_return = sign_In();
		break;
	}
	

}


/*	< make userStucture >
 read userData.txt and make userData array
 */
void get_userData()
{
	char dd[10];
	int i = 0;
	FILE* uData = fopen("../Data/SystemData/userData.txt", "r");

	while (feof(uData) == 0)
	{
		fscanf(uData, "%s %s %s %d ", userData[i].ID, userData[i].PW, userData[i].Name, &(userData[i].isMaster));
		i++;
	}

	//save number of all users
	userData_Size = i;

	fclose(uData);
}

/*	< Login >
if return 1 : back to start menu
if return 0 : go next step
*/
int  log_In()
{
	char ID_input[20];
	char PW_input[20];
	int userIndex;
	
	clear();

	mvaddstr(0, 0, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	mvaddstr(1, 0, "                   LOG IN                        ");
	mvaddstr(2, 0, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	mvaddstr(3, 0, "     input 'quit' : Return to Main Menu          ");

	signal(SIGIO, SIG_IGN);//-----set sigio ingnore!!

	//1. ID
	while (1)
	{
		mvaddstr(4, 0, "              1. input your ID                   ");
		mvaddstr(5, 0, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

		mvaddstr(9, 0, "            ID :");
		refresh();
		scanw("%s", ID_input);

		if (strcmp(ID_input, QUIT_SIGN) == 0)//if user typed 'quit' -> return to main menu
		{
			mvaddstr(13, 0, "             RETURN TO START PAGE....");
			refresh();
			sleep(3);

			return 0;
		}
		else if ((userIndex = find_LoginUser(ID_input)) < 0)//if id is not valid
		{
			mvaddstr(6, 0, "  !!   ID IS NOT VALID. TRY AGAIN   !!");
			mvaddstr(9, 16, "                                    ");//remove input
		}
		else
			break;
	}

	//2. PW
	while (1)
	{
		mvaddstr(4, 0, "              2. input your PW                   ");
		mvaddstr(5, 0, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");

		mvaddstr(11, 0, "            PW :");
		refresh();
		scanw("%s", PW_input);

		if (strcmp(PW_input, QUIT_SIGN) == 0)
		{
			mvaddstr(13, 0, "             RETURN TO START PAGE....");
			refresh();
			sleep(3);

			return 0;
		}
		else if (strcmp(userData[userIndex].PW, PW_input) != 0)
		{
			mvaddstr(6, 0, "  !!   PW IS NOT VALID. TRY AGAIN   !!");
			mvaddstr(11, 16, "                                    ");//remove input

		}
		else
			break;
	}


	//3. save user index
	thisUser_Index = userIndex;

	//4. welcome sign ( login successed ) 
	mvaddstr(13, 0, "             LOG IN SUCCESSED");
	mvprintw(14, 0, "           WELCOME %s  . . .", userData[thisUser_Index].Name);
	refresh();
	sleep(5);
	return 1;

}

/*	< find index of inputID >
if id is exist : return index
if id is not exit in array : return -1
*/
int find_LoginUser(char* id)
{
	for (int i = 0; i < userData_Size; i++)
		if (strcmp(id, userData[i].ID) == 0)
			return i;

	return -1;
}

/*	< Sign in >
if return 1 : back to start menu
if return 0 : go next step
*/
int sign_In(void)
{
	FILE* userData_file = fopen("../Data/SystemData/userData.txt", "a");
	char fullStatus[70];	//ID PW Name isMaster
	char buffer[100];	//buffer to get user input

	user newUser;
	int index = 12; //input start position

	signal(SIGIO, SIG_IGN);//-----set sigio ingnore!!

	clear();
	//default page
	mvaddstr(0, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	mvaddstr(1, 0, "|                  Sign In                    |");
	mvaddstr(2, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	mvaddstr(3, 0, "|  if you want to return to Main page,        |");
	mvaddstr(4, 0, "|                    please input 'quit'      |");
	mvaddstr(5, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

	//1. ID
	mvaddstr(6, 0, "|             1. Input your ID                |");
	mvaddstr(7, 0, "|  [  engligh + num ,   5  < length < 10   ]  |");
	mvaddstr(8, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

	mvaddstr(index, 0, "           ID :");
	if (signIn_Check(5, 10, index, buffer, "ID") == 0)
		return 0;
	strcpy(newUser.ID, buffer);
	index += 2;
	//remove warning
	mvprintw(9, 0, "                                                        ");

	//2. PW
	mvaddstr(6, 0, "|             2. Input your PW                |");
	mvaddstr(7, 0, "|  [  engligh + num ,   5  < length < 10   ]  |");
	mvaddstr(8, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

	mvaddstr(index, 0, "           PW :");
	if (signIn_Check(5, 10, index, buffer, "PW") == 0)
		return 0;
	strcpy(newUser.PW, buffer);
	index += 2;
	//remove warning
	mvprintw(9, 0, "                                                        ");


	//3. NAME
	mvaddstr(6, 0, "|           3. Input your Name                |");
	mvaddstr(7, 0, "|      [  engligh  ,   1< length < 10   ]     |");
	mvaddstr(8, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");

	mvaddstr(index, 0, "         NAME :");
	if (signIn_Check(1, 10, index, buffer, "NAME") == 0)
		return 0;
	strcpy(newUser.Name, buffer);
	index += 2;
	//remove warning
	mvprintw(9, 0, "                                                        ");

	mvprintw(7, 0, "                                                        ");
	//4. isMaster
	mvaddstr(6, 0, "|           4. Are you MASTER?                |");
	mvaddstr(7, 0, "|                  [ y/n ]                    |");
	mvaddstr(8, 0, "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+");
	mvaddstr(index, 0, "      MASTER? :");
	refresh();

	while (1)
	{
		char c;
		mvscanw(index, 15, "%c", &c);

		if (c == 'y')//Master -> get Master Code
		{
			int mcode;

			mvaddstr(index + 1, 0, "   MASTER CODE:");
			refresh();
			mvscanw(index + 1, 15, "%d", &mcode);

			if (mcode != MCODE)//if user input wrong code -> return to main page
			{
				mvaddstr(index + 3, 0, "      !!       YOUR MCODE IS WRONG. RETURN TO MAIN PAGE . . .");
				refresh();
				sleep(5);

				return 0;
			}

			newUser.isMaster = 1;
			break;
		}
		else if (c == 'n')
		{
			newUser.isMaster = 0;
			break;
		}

		mvprintw(9, 0, "   !!    YOUR ANSWER SHOULD BE 'y' OR 'n' !!  TRY AGAIN  ");

	}

	userData[++userData_Size]=newUser;
	sprintf(fullStatus,"%s %s %s %d", userData[userData_Size].ID, userData[userData_Size].PW, userData[userData_Size].Name, userData[userData_Size].isMaster);
	fprintf(userData_file, "%s\n",fullStatus);

	mvprintw(22, 0, "                 YOUR SIGN IN IS DONE. TRY LOGIN ");
	mvprintw(23, 0, "                             -> LETS GO TO LOGIN PAGE. . . . .");
	refresh();

	sleep(5);

	fclose(userData_file);

	return log_In();	//now return login return value

}

/*	< check user's input in signin function >
if quit : exit
if length or duplication error : try again until it is correct answrt
*/
int signIn_Check(int minLen, int maxLen, int index, char* buffer, char* label)
{
	while (1)
	{
		refresh();

		mvscanw(index, 15, "%s", buffer);

		if (strcmp(QUIT_SIGN, buffer) == 0)//1. quit check  
			return 0;//->quit
		else if (strlen(buffer) < minLen || strlen(buffer) > maxLen)//2. length check
		{
			mvprintw(9, 0, "   !!   YOUR %s SHOULD BE IN %d~%d LENGTH !!  TRY AGAIN  ", label, minLen, maxLen);
			//remove input
			mvprintw(index, 15, "                                                      ");
		}
		else if (dup_Check(label,buffer))//3. duplication check
		{

			mvprintw(9, 0, "   !!  THIS %s IS ALREADY EXIST !!  TRY AGAIN  ", label, minLen, maxLen);
			//remove input
			mvprintw(index, 15, "                                                      ");
		}
		else
			return 1;
	}
}

/*	< check input's duplication >
return 1 : its already exist 
return 0 : its new input
*/
int dup_Check(char* label, char* buffer)	
{
	if (strcmp(label, "ID") == 0)
	{
		for (int i=0; i < userData_Size; i++)
			if (strcmp(userData[i].ID, buffer) == 0)
				return 1;
	}
	else if (strcmp(label, "PW") == 0)
	{
		for (int i=0; i < userData_Size; i++)
			if (strcmp(userData[i].PW, buffer) == 0)
				return 1;
	}
	else if (strcmp(label, "Name") == 0)
	{
		for (int i=0; i < userData_Size; i++)
			if (strcmp(userData[i].Name, buffer) == 0)
				return 1;
	}

	return 0;

}
