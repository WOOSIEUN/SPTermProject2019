// 'Call out other user' part added
// .Bomi
// there's ** mark to UPDATED PART
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>//**
#include <sys/types.h>//**
#include <sys/wait.h>//**
#include <sys/socket.h>//**
#include <sys/time.h>//**
#include <netdb.h>//**

#define USER_MAX 100
#define MCODE 486
#define QUIT_SIGN "quit"
#define UP 119	
#define DOWN 115	
#define ENTER 13	

#define HOSTNAME "ip-172-31-47-228"	//**
#define PENDING_NUM 5			//**

typedef struct
{
	char ID[20];
	char PW[20];
	char Name[20];
	int isMaster;
	int portNum;
}user;

int login_return = 0;
int signin_return = 0;
int choice = 0;
user userData[USER_MAX]; 
int userData_Size;	
int thisUser_Index;

void set_SIGIO();
void start_Screen();
void start_Handler(int sn);
void get_userData();
int log_In();
int find_LoginUser(char*);
int sign_In();
int signIn_Check(int, int, int, char*, char*);
int dup_Check(char*, char* );

//***---------socket plus part
void server(void);    	//**server fuction
void callOut(void);		//**send callout
int checkValidName(char* name);	//**check input name in callOut function
void callout_handler(int signum);
void calloutPrint_handler(int signum);
int set_ticker(int);

int portNumIndex;	//used in sign in to assign portnum to newUser

#define READ_END 0
#define WRITE_END 1

int thepipe[2];
pid_t parent_pid;
char callout[20];
int printcounter= 0;

int main()
{
	int forkreturn;

	set_SIGIO();	
	get_userData();	
	start_Screen();
	
	//********FORK PART SHOULD BE CALLED AFTER LOGIN
	/*
	** make 2 process for socket
		-parent : main process
		-child  : server process

		IF SERVER GET CALL FROM OTHER P, 
		CHILD SEND SIGNAL TO PARENT 
		THEN PARENT GET DATA FROM PIPE AND SAVE 
	 */

	parent_pid = getpid();    //save parent process's pid
	pipe(thepipe);
	
	if(forkreturn=fork())//if parent process
	{
		//set handlers
		signal(SIGUSR1,callout_handler); 
		signal(SIGALRM,calloutPrint_handler);

		//	->READ
		close(thepipe[WRITE_END]);
		
		callOut();
	}
	else//child p
	{
		//      ->WRITE
		close(thepipe[READ_END]);

		server();
	}
	endwin();
}

/*
	< handler for signal SIGUSR1>
if server get callout from other p, 
server send sigusr1
	-get callout from pipe
	-set alarm for print callout
*/
void callout_handler(int signum)
{
	//read callout from pipe
	read(thepipe[READ_END],callout,sizeof(callout));
	
	set_ticker(2000);
}

//	< set timer>
int set_ticker(int n_msecs)
{
	struct itimerval new_timeset;
	long n_sec,n_usecs;

	n_sec = n_msecs /1000;
	n_usecs = (n_msecs % 1000) * 1000L;

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_usecs;
	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_usecs;

	return setitimer(ITIMER_REAL,&new_timeset,NULL);
}


/*
   	< handler for signal SIGALRM >
 */
void calloutPrint_handler(int signum)
{
	if(printcounter%2 == 0)
		standout();
	
	mvprintw(LINES-3,5,"^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^");
	mvprintw(LINES-2,5,"!!!     [%5s] IS CALLING YOU NOW      !!!",callout);
	mvprintw(LINES-1,5,"v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v");


	if(printcounter%2 == 0)
		standend();
	refresh();

	if(printcounter++ == 10)
	{
		//remove callout
		mvprintw(LINES-3,0,"                                                              ");
	        mvprintw(LINES-2,0,"                                                              ");
		mvprintw(LINES-1,0,"                                                              ");
		refresh();


		alarm(0);	//turn off alarm
		printcounter = 1;
	}
}


/*	< server socket >
 recieve message from other client 
 and save message data in file
*/
void server(void)//********************************************************************
{
	int serversock_id,sock_fd;	//server socket's fd , active sicket's fd
	struct sockaddr_in server_add;	
	struct hostent* hp;
	char call_from[20];	//buffer to receive message
	int myport = userData[thisUser_Index].portNum;

	//1. get a socket
	serversock_id = socket(AF_INET, SOCK_STREAM, 0);

	//2. build address ( host, port )
	bzero(&server_add, sizeof(server_add));	//init server address structure
	hp = gethostbyname(HOSTNAME);			//get host ip

	//3. fill in socket address structure & bind
	bcopy((void*)hp->h_addr, (void *)&server_add.sin_addr, hp->h_length);
	server_add.sin_port = htons(myport);
	server_add.sin_family = AF_INET;
	
	bind(serversock_id, (struct sockaddr*) & server_add, sizeof(server_add));

	//4. allow incoming message
	listen(serversock_id,PENDING_NUM);
	
	//5. main loop
	while (1)
	{
		//!connect
		sock_fd = accept(serversock_id, NULL, NULL);
		
		//we should recieve message(read)
		read(sock_fd , call_from, sizeof(call_from));	//get message from client
		
		//signal to parent process to send message
		kill(parent_pid,SIGUSR1);
		write(thepipe[WRITE_END],call_from,sizeof(call_from));

	}
	
	
}

//*********************************************************************************************
/*
	<CALLOUT other user>
  make active socket 
  send callout message 
*/
void callOut(void)
{	
	int pnum;		//portnum of receiver
	char to[20];		//Name of message receiver
	char m_context[100];	//context of message
	time_t msendTime;	//sending time of message
	char fullmessage[150];	
	char answer;		//answer of (y/n) question
	
	signal(SIGIO,SIG_IGN);	//ignore sigio

	clear();
	mvaddstr(0,0,"-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
	mvaddstr(1,0,"      LETS CALL OUT FELLOW  |  input 'quit' if you want to quit         ");
	mvaddstr(2,0,"-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
	mvaddstr(3,0,"              please  input name of fellow to call out                  ");

	//1. get receiver's Name
	while(1)
	{
		mvaddstr(8,0,"      To : ");	refresh();	scanw("%s",to);
		
		//check quit sign
		if(strcmp("quit",to)==0)
			return;

		//check valid of User Name
		if(pnum = checkValidName(to))
		{	
			mvaddstr(6,0,"                                              ");//remove warning
			break;
		}
	
		mvaddstr(6,0," !! INPUT NAME IS NOT VALID, TRY AGAIN !!   ");
		mvaddstr(8,0,"                                            ");

	}

	//2. get final answer 
	while(1)
	{
		mvprintw(11,0,"         WILL YOU SEND MESSAGE TO ' %s ' ? (y/n)     ",to);
	        mvaddstr(12,0,"                                 : "); refresh();
        	scanw("%c",&answer);

		if(answer == 'n')
			break;
		else if( answer == 'y')	//-----!!LETS SEND MESSAGE
		{	
			struct sockaddr_in servadd;
			struct hostent *hp;
			int sock_id;
			
			//1. get a socket
			sock_id = socket(AF_INET,SOCK_STREAM,0);
			//2. set server data
			bzero(&servadd,sizeof(servadd));
			hp = gethostbyname(HOSTNAME);
			bcopy(hp->h_addr,(struct sockaddr*)&servadd.sin_addr,hp->h_length);
			servadd.sin_port = htons(pnum);
			servadd.sin_family = AF_INET;
			//3. connect
			if(connect(sock_id,(struct sockaddr*)&servadd,sizeof(servadd)) != 0)
			{
				clear();
				mvprintw(15,0,"%s is OFFLINE NOW. TRY NEXT TIME",to);
				refresh();
				sleep(3);
				return;
			}

			//send message 
			//sprintf(

			write(sock_id,userData[thisUser_Index].Name,sizeof(userData[thisUser_Index].Name));
			close( sock_id );

			clear();
			mvaddstr(14,0,"+-+-+-+-+-+-+                                                                 ");
			mvprintw(15,0,"    YOUR CALLOUT TO %s IS SUCCESSFULLY SENDED. RETURN TO MAIN PAGE....      ",to);
			mvaddstr(16,0,"+                                                                +-+-+-+-+-+-+");
			refresh();
                        sleep(3);
			break;
		}
		else
		{
			mvaddstr(9,0,"    !!   YOU SHOULD ANSWER  y or n. TRY AGAIN   !!");
			mvaddstr(11,0,"                                                            ");//remove answer
		}
	}

}

/*	< check valid name and return portNum >
if name is not exist in User, return -1
 */
int checkValidName(char* name)
{
	for(int i ;i<userData_Size;i++)
	{
		if(strcmp(userData[i].Name, name) == 0)
			return userData[i].portNum;
	}
	return -1;
}

//*******************************************************************************************************************************

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
	int fork_return;
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

	fscanf(uData, "%d ",&portNumIndex);//*****************
	while (feof(uData) == 0)
	{
		fscanf(uData, "%s %s %s %d %d", userData[i].ID, userData[i].PW, userData[i].Name, &(userData[i].isMaster),&(userData[i].portNum));//***
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
	mvprintw(13, 0, "             LOG IN SUCCESSED");
	mvprintw(14, 0, "           WELCOME %s  . . .", userData[thisUser_Index].Name);
	refresh();
	
	sleep(3);

//	sendMessage();
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
	//5. port number*************************
	newUser.portNum = portNumIndex++;

	//append new user to userData.txt file 
	userData[++userData_Size]=newUser;
	sprintf(fullStatus,"%s %s %s %d %d", userData[userData_Size].ID, userData[userData_Size].PW, userData[userData_Size].Name, userData[userData_Size].isMaster,userData[userData_Size].portNum);
	fprintf(userData_file, "%s\n",fullStatus);
	fclose(userData_file);

	//open userData.txt again "r+" to renew port number index
	userData_file = fopen("../Data/SystemData/userData.txt", "r+");
	rewind(userData_file);
	fprintf(userData_file, "%d",portNumIndex);
	fclose(userData_file);

	mvprintw(22, 0, "                 YOUR SIGN IN IS DONE. TRY LOGIN ");
	mvprintw(23, 0, "                             -> LETS GO TO LOGIN PAGE. . . . .");
	refresh();

	sleep(5);

	

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
