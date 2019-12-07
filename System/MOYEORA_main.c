#include <curses.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include "MOYEORA_header.h"

/*==================================================** MAIN **==============================================================
> MULTI PROCESS
PARENT : MAIN PROCESS FOR SCHEDULE PROGRAM
CHILD  : SERVER PROCESS FOR RECEIVE CALLOUT FROM OTHER USER
==========================================================================================================================*/
void main()
{
	int forkreturn;

	set_SIGIO();   		//1. set sigio signal
	get_userData();  	//2. make user structure array from userData.txt

	startScreen_MAIN(); 	//3. start screen 

	parent_pid = getpid();    //save parent process's pid
	pipe(thepipe);

	if (forkreturn = fork())//if parent process
	{
		//set handlers
		signal(SIGUSR1, callout_handler);
		signal(SIGALRM, calloutPrint_handler);

		//	->READ
		close(thepipe[WRITE_END]);
		mainScreen_MAIN();		//4. print main screen
	}
	else//child p
	{
		//      ->WRITE
		close(thepipe[READ_END]);

		server();
	}
	endwin();


}

/*================================================== INITIALIZE ===========================================================
> SET SIGIO & SET USERDATA
==========================================================================================================================*/
//   < set sigio signal >
void set_SIGIO()
{
	int fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags | O_ASYNC));
}

/*   < make userStucture >
read userData.txt and make userData array
*/
void get_userData()
{
	char dd[10];
	int i = 0;
	FILE* uData = fopen("../Data/SystemData/userData.txt", "r");


	fscanf(uData, "%d ", &portNumIndex);

	while (feof(uData) == 0)
	{
		fscanf(uData, "%s %s %s %d %d", userData[i].ID, userData[i].PW, userData[i].Name, &(userData[i].isMaster), &(userData[i].portNum));
		i++;
	}

	//save number of all users
	userData_Size = i;

	//########## need 
	strcpy(user_ID, userData[thisUser_Index].ID);
	strcpy(user_Name, userData[thisUser_Index].Name);
	user_isMaster = userData[thisUser_Index].isMaster;


	fclose(uData);
}

/*================================================== START SCREEN =========================================================
1. START SCREEN -> LOGIN -> MAIN SCREEN
OR
2. START SCREEN -> SIGN IN -> MAIN SCREEN
==========================================================================================================================*/
//   < print start menu >
void startScreen_MAIN()
{
	noecho();   //set noecho mode

				//print main screen
	initscr();

	while (login_return == 0 && signin_return == 0)
	{
		signal(SIGIO, start_Handler);//set sigio handler
		move(LINES - 1, COLS - 1);

		clear();

		mvaddstr(2, 18, "+-+-+-+-+-+-+-+-+-+-+");
		mvaddstr(3, 18, "  M o  y e o  R a    ");
		mvaddstr(4, 18, "   S C H E D U L E R ");
		mvaddstr(5, 18, " +-+-+-+-+-+-+-+-+-+-+");

		mvaddstr(7, 23, "   ^    :   w   ");
		mvaddstr(8, 23, " < v >  : a s d ");
		mvaddstr(9, 23, " select : enter ");

		if (start_choice)
		{
			mvaddstr(15, 23, "     LOG IN     ");   // choice (0)  default -> login 
			standout();
			mvaddstr(16, 23, "     SIGN IN    ");   // choice (1)
			standend();
		}
		else
		{
			standout();
			mvaddstr(15, 23, "     LOG IN     ");   // choice (0)  
			standend();
			mvaddstr(16, 23, "     SIGN IN    ");   // choice (1)
		}

		refresh();
		pause();
	}
}

/*  - < Handler for startMenu input >
if UP ,DOWN : change state
if ENTER : execute function
*/
void start_Handler(int signum)
{
	int  input = getchar();   //get input 

	switch (input)
	{
	case UP:case DOWN:// [w] [s]
		start_choice = !start_choice;
		break;
	case ENTER:// [enter]
		if (start_choice == 0) //---->login
			login_return = log_In();
		else if (start_choice == 1)
			signin_return = sign_In();
		break;
	}
}

/*  - < Login >
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

/*   -- < find index of inputID >
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

/*   - < Sign in >
if return 1 : back to start menu
if return 0 : go next step
*/
int sign_In(void)
{
	FILE* userData_file = fopen("../Data/SystemData/userData.txt", "a");
	char fullStatus[70];   //ID PW Name isMaster
	char buffer[100];   //buffer to get user input

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

	userData[++userData_Size] = newUser;
	sprintf(fullStatus, "%s %s %s %d", userData[userData_Size].ID, userData[userData_Size].PW, userData[userData_Size].Name, userData[userData_Size].isMaster);
	fprintf(userData_file, "%s\n", fullStatus);

	mvprintw(22, 0, "                 YOUR SIGN IN IS DONE. TRY LOGIN ");
	mvprintw(23, 0, "                             -> LETS GO TO LOGIN PAGE. . . . .");
	refresh();

	sleep(5);

	fclose(userData_file);

	return log_In();   //now return login return value

}

/*   -- < check user's input in signin function >
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
		else if (dup_Check(label, buffer))//3. duplication check
		{

			mvprintw(9, 0, "   !!  THIS %s IS ALREADY EXIST !!  TRY AGAIN  ", label, minLen, maxLen);
			//remove input
			mvprintw(index, 15, "                                                      ");
		}
		else
			return 1;
	}
}

/*  --- < check input's duplication >
return 1 : its already exist
return 0 : its new input
*/
int dup_Check(char* label, char* buffer)
{
	if (strcmp(label, "ID") == 0)
	{
		for (int i = 0; i < userData_Size; i++)
			if (strcmp(userData[i].ID, buffer) == 0)
				return 1;
	}
	else if (strcmp(label, "PW") == 0)
	{
		for (int i = 0; i < userData_Size; i++)
			if (strcmp(userData[i].PW, buffer) == 0)
				return 1;
	}
	else if (strcmp(label, "Name") == 0)
	{
		for (int i = 0; i < userData_Size; i++)
			if (strcmp(userData[i].Name, buffer) == 0)
				return 1;
	}

	return 0;

}


/*=================================================== MAIN SCREEN ==========================================================
MENU	 :		1. add	 / 2. view detail & delete	 / 3. seach		/ 4. callOut	/ 5. exit
-----------
SCHEDULE 1
prev	<<	SCHEDULE 2	>>	next
SCHEDULE 3
============================================================================================================================*/
//	<	main function for MAIN SCREEN	> 
void mainScreen_MAIN()
{
	int year, year_and_month, date, today, index;
	char input;
	nodeptr isHead = NULL;

	//0. init screen	
	today = return_today();
	init_mainScreen(&year, &year_and_month, &date, &today);

	while (1) {
		//1. print linked list
		index = print_Brief_list(year, year_and_month);

		//2. get input of user
		while (1)
		{
			input = getchar();
			if (input == 'a') { //<---move to prev page of list
				if (head != NULL) {//if its not null list
					isHead = move_Brief_list(0, index);
					if (isHead == head) { // if ishead is head, program should not print brief list again.
						continue;
					}
					else { //print the list.
						break;
					}
				}
				else { //head is null.
					continue;
				}
			}
			else if (input == 'd') { //--->move to next page of list
				if (head != NULL && current->next != NULL) {//if list is not end
					move_Brief_list(1, index);
					break;
				}
				else if (current->next == NULL) // if current->next is null, program should not print brief list again.
					continue;
			}
			else if (input == '1') { //---	Menu [1] : move to add schedule screen 
				addSchedule_MAIN();
				init_mainScreen(&year, &year_and_month, &date, &today);
				break;
			}
			else if (input == '2') { //---	Menu [2] : view detail of 1 schedule
				viewDetail_MAIN(index, year, year_and_month);
				init_mainScreen(&year, &year_and_month, &date, &today);
				break;
			}
			else if (input == '3') { //---	Menu [3] : search schedule
				move(11, 53);
				echo();
				scanw("%d", &today);
				noecho();
				move(11, 53);
				addstr("         ");

				init_mainScreen(&year, &year_and_month, &date, &today);
				break;
			}
			else if (input == '4') { //---	Menu [4] : send callout
				callOut_MAIN();
				init_mainScreen(&year, &year_and_month, &date, &today);
				break;
			}
			else if (input == '5') { //---	Menu [5] : quit program
				  //NEED TO END SOCKET!! (CHILD P)
				endwin();
				exit(1);
				break;
			}
		}
	}

}

//	- < init mainScreen upward part >
void init_mainScreen(int* year, int* year_and_month, int* date, int* today)
{
	clear();

	return_today();
	print_menu();

	search_schedule(year, year_and_month, date, *today);

	current = head;
}

void search_schedule(int* year, int* year_and_month, int* date, int target) {
	char filename[100];

	if (target > 100000000) {
		return; // If the target is larger than 8 digits number, it will not print.
	}
	if (target > 1000000) { //if target is date.
		*year_and_month = target / 100;
		*date = target - (*year_and_month * 100);
	}
	else { //if target is month
		*year_and_month = target;
		*date = 0;
	}
	*year = *year_and_month / 100; //get year 

	sprintf(filename, "../Data/ScheduleData/%d/%d_Schedule.txt", *year, *year_and_month); //make file path

																						  //make Linked List
	head = make_schedulelist(0, filename, date);

	return;
}

//	- < print brief list to main page>
int print_Brief_list(int year, int year_and_month)
{
	int i = 0;

	if (head == NULL) {// if linked list is empty -> Print NO SCHEDULE
		mvaddstr(List_Yp_2, List_Xp, "        NO SCHEDULE        \n");
	}
	else
		for (i = 0; i < 3; i++)
		{
			if (current == head && i == 0) { //special case. we want to print head.
				printBrief3(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
			}
			else {
				current = current->next;
				printBrief3(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
			}
			if (current->next == NULL)
				break;
		}
	refresh();

	return i;
}

//	* < move list <<- or ->> >
nodeptr move_Brief_list(int how, int index)
{
	nodeptr isHead = NULL;

	if (how)// case 1. next page ->>>
	{
		clear_list_detail();
	}
	else//case 2. prev page <<<-
	{
		isHead = current;

		if (index == 3)
			index--;

		for (int j = 0; j < index; j++)
			isHead = isHead->pre;

		if (isHead == head)
			return isHead;
		else
			for (int j = 0; j < index + 3; j++)
				current = current->pre;

		clear_list_detail();
	}
	return isHead;
}

//	< remove breif list >
void clear_list_detail() {
	int xp, yp;

	xp = List_Xp;
	yp = List_Yp_1;
	mvaddstr(yp, xp, "                                                                                    ");
	mvaddstr(yp + 1, xp, "                                                                                    ");
	mvaddstr(yp + 2, xp, "                                                                                    ");
	yp = List_Yp_2;
	mvaddstr(yp, xp, "                                                                                    ");
	mvaddstr(yp + 1, xp, "                                                                                    ");
	mvaddstr(yp + 2, xp, "                                                                                    ");
	yp = List_Yp_3;
	mvaddstr(yp, xp, "                                                                                    ");
	mvaddstr(yp + 1, xp, "                                                                                    ");
	mvaddstr(yp + 2, xp, "                                                                                    ");
}

//	< print 3 brief list >
void printBrief3(int order, int group, int year, int year_and_month) {
	//This program can print 3 list. order is used to determine a y-position.
	char sort[6];
	int xp, yp, i;
	int nstart_time, nstart_min, nend_time, nend_min;
	char start_time[3], start_min[3], end_time[3], end_min[3];

	if (group == 0) {
		strcpy(sort, "Group");
	}
	else {
		strcpy(sort, "User");
	}

	if (order == 1) {
		xp = List_Xp;
		yp = List_Yp_1;
	}
	else if (order == 2) {
		xp = List_Xp;
		yp = List_Yp_2;
	}
	else if (order == 3) {
		xp = List_Xp;
		yp = List_Yp_3;
	}
	else if (order == 4) {
		return;
	}

	nstart_time = (current->start_time / 100);
	nstart_min = (current->start_time - (nstart_time * 100));
	nend_time = (current->end_time / 100);
	nend_min = (current->end_time - (nend_time * 100));

	smaller_than_ten(nstart_time, start_time);
	smaller_than_ten(nstart_min, start_min);
	smaller_than_ten(nend_time, end_time);
	smaller_than_ten(nend_min, end_min);

	if (strcmp(userData[thisUser_Index].ID, current->userID) != 0) { //search name
		for (i = 0; i < userData_Size; i++) {
			if (strcmp(userData[i].ID, current->userID) == 0) {
				move(yp, xp);
				printw("[%s]  written by %s\n", sort, userData[i].Name);
			}
		}
	}
	else { //login user == write user
		move(yp, xp);
		printw("[%s]  written by %s\n", sort, userData[thisUser_Index].Name);
	}
	move(yp + 1, xp);
	printw("%d.%d.%d  %s:%s ~ %s:%s\n", year, year_and_month - (year * 100), current->date, start_time, start_min, end_time, end_min);
	move(yp + 2, xp);
	printw("%s\n", current->scheduleName);
	return;
}

//	< Print Today's Date >
//	return today's date 
//	Use header file time.h
int return_today()
{
	struct tm* t = return_Time();
	int today;

	today = (t->tm_year + 1900) * 100;
	today = (today + (t->tm_mon + 1)) * 100;
	today = (today + t->tm_mday);

	mvaddstr(1, 10, "*********************************************************\n");
	mvaddstr(2, 10, "                          Today                          \n");
	mvprintw(3, 10, "                         %d.%d.%d                        \n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	mvaddstr(4, 10, "*********************************************************\n");

	return today;
}

struct tm* return_Time(void)
{
	struct tm* t;
	time_t timer;

	//change time to Korea's time
	timer = time(NULL);
	timer += TIMEOFFSET * 3600;
	t = localtime(&timer);

	return t;
}

void smaller_than_ten(int target, char* targetstr)
{
	if (target < 10)
		sprintf(targetstr, "0%d", target);
	else
		sprintf(targetstr, "%d", target);

}

void print_menu() {//print menu. Details of this function need to be modified.
	mvaddstr(6, 10, "*********************************************************\n");
	mvaddstr(7, 12, "1. Add Schedule.\n");
	mvaddstr(8, 12, "2. View Schedule Detail. \n");
	mvaddstr(9, 12, "  (Enter Schedule Number. Press Enter Key.)  :\n");
	mvaddstr(10, 12, "3. Search Schedule.\n");
	mvaddstr(11, 12, "  (Enter YYYYMMDD. Press Enter Key.)  :\n");
	mvaddstr(12, 12, "4. Call out\n");
	mvaddstr(13, 12, "5. Quit.\n");
	mvaddstr(14, 10, "*********************************************************\n");
	refresh();
}

struct node* make_newNode(int date, int start, int end, char* ID, char* permission, char* sname, char* filepath)
{
	nodeptr newNode = (struct node*)malloc(sizeof(struct node));

	newNode->date = date;
	newNode->start_time = start;
	newNode->end_time = end;
	strcpy(newNode->userID, ID);
	strcpy(newNode->permissionBit, permission);
	strcpy(newNode->scheduleName, sname);
	strcpy(newNode->filepath, filepath);
	newNode->next = NULL;
	newNode->pre = NULL;

	return newNode;
}

struct node* make_schedulelist(int mode, char* filename, int* date) {
	//mode 0 is search mode, mode 1 is edit and make file or dir mode.

	FILE* f = NULL;
	nodeptr first = NULL; //head of linked list. return this value.
	nodeptr newnode = NULL;
	nodeptr current = NULL;
	nodeptr prev = NULL;
	char str[256], t_ID[20], t_sname[100], t_filepath[100], t_permission[3];
	char ct_date[3], ct_start[5], ct_end[5];
	int t_date, t_start, t_end;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror("open error");
		return first;
	}

	current = first;
	while (fgets(str, sizeof(str), f) != NULL) { //read data file one by one.
		str[strlen(str)] = 0;
		sscanf(str, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:", ct_date, ct_start, ct_end, t_ID, t_permission, t_sname, t_filepath);
		t_date = atoi(ct_date);
		t_start = atoi(ct_start);
		t_end = atoi(ct_end);

		if (mode == 0) { //make linked list for print purpose.
						 //if date is 0, our program determines that the user wants to find a month schedule.
						 //if date is not 0, our program determines that the user wants to find a specific date schedule.
			if (*date != 0 && t_date != *date) {
				continue;
			}
			else if (strcmp(t_permission, "01") == 0) { //private file.
														//Check permission that allows the program print private file or not.
				if (strcmp(t_ID, user_ID) != 0 && user_isMaster != 1) {
					continue;
				}
			}
		}
		// If mode is 1, program make total linked list for add or delete schedule.

		//make linked list
		newnode = make_newNode(t_date, t_start, t_end, t_ID, t_permission, t_sname, t_filepath);

		if (first == NULL) {
			first = newnode;
			current = first;
		}
		else {
			prev = current;
			current->next = newnode;
			current = current->next;
			current->pre = prev;
		}
	}
	fclose(f);
	return first;
}

/*================================================= M [1] ADD NEW SCHEDULE ===================================================
GET USER'S SCHEDULE INPUT ->	SAVE DATA TO	1. DETAIL FILE	2. BRIEF FILE
============================================================================================================================*/
/*	* < print add schedule page >
process user's data and call filesave function
*/
void addSchedule_MAIN()
{
	Schedule schedule_info;

	clear();

	//print upward screen.
	clear();
	mvaddstr(1, 10, "*********************************************************\n");
	mvaddstr(2, 10, "                       ADD SCHEDULE                      \n");
	mvaddstr(3, 10, "*********************************************************\n");
	mvaddstr(4, 10, "                  press ENTER : next step                \n");
	mvaddstr(5, 10, "                  press ESC   : previous step            \n");
	mvaddstr(6, 10, "*********************************************************\n");

	mvaddstr(7, 10, "           Date        :  ");
	mvaddstr(8, 10, "          (YYYYMMDD) ");

	mvaddstr(10, 10, "           Start time  :  ");
	mvaddstr(11, 10, "          (HHMM) ");

	mvaddstr(13, 10, "           End time    :  ");
	mvaddstr(14, 10, "          (HHMM) ");
	mvaddstr(15, 10, "*********************************************************\n");
	mvaddstr(16, 10, "   Title      :");

	mvaddstr(18, 10, "   Write      :");

	move(7, 37);	//To move to where the date is written:
	refresh();

	// get user's input & save data to file

	save_newSfile(get_newSinput());

}

//	< get user's input of new schedule >
void clear_current_line(int x, int y) {
	mvaddstr(x, y, "                                                                ");
	refresh();
}
char* write_start(int x,int y, int size, int write_size) {
	move(x, y);
	addstr("                                                               ");
	refresh();
	move(x, y);
	refresh();

	char c;
	char *wt;
	wt = (char *)malloc(size * sizeof(char));
	while (1) {
		c = getchar();

		if (c == ENTER) {
			write_location++;
			break;
		}
		else if (c == ESC) {

			mvaddstr(x, y, "                                      ");
			refresh();
			write_location--;
			break;
		}
		else if (c == DEL) {
			/* delete char */
			putchar(c);
			write_size--;
			wt[write_size] = '\0';
		}
		else {
			putchar(c);
			wt[write_size] = c;
			wt[write_size + 1] = '\0';
			write_size++;
		}
		refresh();
	}
	refresh();

	//schedule_list_check_o_or_x.content = ISFULL;

	//strcpy(schedule_info.content, content);
	return wt;
}

Schedule get_newSinput()
{
	Schedule schedule_info;
	write_location = 1;
	echo();
	//get user's input
	while (1) {
		if (write_location == 0) {
			// Ask	: Are you sure you want to exit the write screen?
			//	->yes	: go back main screen
			//	->no	: go to the date bar
			mvaddstr(LINES - 1, 10, "        >> do you want to exit write screen (y/n) ");
			refresh();
			char yes_or_no;
			yes_or_no = getchar();
			if (yes_or_no == 'y') {
				// go back main screen
				schedule_info.is_it_full = 0;	// if is 0 than do not save
				break;
			}
			else if (yes_or_no == 'n') {
				// cleared quistion
				mvaddstr(LINES - 1, 10, "                                                      ");
				refresh();
				write_location = 1;
			}

		}
		else if (write_location == 1) {
			//1. date
			//move(7, 0);
			//clear_current_line(7, 35);
			mvaddstr(7, 10, "           Date        : ");

			move(7, 35);
			refresh();
			char *date;
			int date_i = 0;

			//date = write_start(date, 10, date_i);
			date = write_start(7,35, 10, date_i);

			schedule_info.date = atoi(date);
			schedule_list_check_o_or_x.date = ISFULL;

			move(10, 0); refresh();
		}
		else if (write_location == 2) {
			//2. start time
			move(10, 0);			refresh();
			clear_current_line(10, 35);
			//mvaddstr(10, 10, "           Start time  :  ");	
			move(10, 35);
			refresh();
			char *start_time;
			int start_time_i = 0;

			start_time = write_start(10,35, 5, start_time_i);
			//start_time = write_start(start_time, 5, start_time_i);
			schedule_info.start_time = atoi(start_time);
			schedule_list_check_o_or_x.start_time = ISFULL;

			move(13, 0); refresh();
		}
		else if (write_location == 3) {
			//3. end time
			clear_current_line(13, 35);
			//mvaddstr(13, 10, "           End time    :  ");	
			move(13, 35);
			refresh();
			char *end_time;
			int end_time_i = 0;

			end_time = write_start(13,35, 5, end_time_i);
			//end_time = write_start(end_time, 5, end_time_i);

			schedule_info.end_time = atoi(end_time);
			schedule_list_check_o_or_x.end_time = ISFULL;

			move(16, 0); refresh();
		}
		else if (write_location == 4) {
			//5. schedule name
			clear_current_line(16, 26);
			//mvaddstr(16, 10, "   Title      :");
			move(16, 26);
			refresh();
			char *scheduleName;
			int scheduleName_i = 0;

			scheduleName = write_start(16,26, 100, scheduleName_i);
			//scheduleName = write_start(scheduleName, 100, scheduleName_i);
			strcpy(schedule_info.scheduleName, scheduleName);

			move(18, 0); refresh();

		}
		else if (write_location == 5) {
			//6. content
			clear_current_line(18, 26);
			//mvaddstr(18, 10, "   Write      :");	
			move(18, 26);
			refresh();
			char *content;
			int content_i = 0;

			content = write_start(18,26, 2000, content_i);
			//content = write_start(content, 2000, content_i);
			strcpy(schedule_info.content, content);
		}
		else {
			// finished writing schedule and going to save
			schedule_info.is_it_full = 1;	// if is 1 than save schedule
			if (userData[thisUser_Index].isMaster) {
				// if master -> the schedule is public
				// go to save
				strcpy(schedule_info.permissionBit, "00");
			}
			else {
				// if not master -> ask if it's a public or private schedule
				mvaddstr(LINES - 1, 10, "        >> Is this schedule public? (y/n)");
				refresh();
				char yes_or_no;
				yes_or_no = getchar();
				if (yes_or_no == 'y') {
					// this schedule is public
					strcpy(schedule_info.permissionBit, "10");
				}
				else if (yes_or_no == 'n') {
					// this schedule is private.
					strcpy(schedule_info.permissionBit, "11");
				}

			}
			break;
		}
	}

	refresh();
	noecho();

	return schedule_info;
}

/*	< save new schedule to file >
detail & brief file
*/
void save_newSfile(Schedule schedule_info)
{/* Get current time */
 //FILE* brief_file;
 //nodeptr newnode;

	if (schedule_info.is_it_full == 0) {
		// there is no schedule to add
		// go to the main screen
		return;
	}
	move(LINES - 1, 0);
	addstr("                      =>>save schedule...\n");
	refresh();
	sleep(1);

	struct tm* t = return_Time();
	int year, mon, day, now;
	int schedule_year, schedule_year_and_month, date;
	char now_c[10];	char day_c[3];	char mon_c[3];
	char detail_file_path[100];
	char brief_file_path[100];

	char userID[20];
	strcpy(userID, userData[thisUser_Index].ID);

	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	day = t->tm_mday;

	// hhmmss variable to make brief file path.
	now = (t->tm_hour) * 100;
	now = (now + t->tm_min) * 100;
	now = now + t->tm_sec;

	smaller_than_ten(mon, mon_c);
	smaller_than_ten(day, day_c);

	if (now < 100000)
		sprintf(now_c, "0%d", now);
	else
		sprintf(now_c, "%d", now);

	sprintf(detail_file_path, "../Data/ScheduleData/%d/%s/%s_%d%s%s%s.txt", year, mon_c, userID, year, mon_c, day_c, now_c);

	FILE* detail_file = fopen(detail_file_path, "w");
	if (!detail_file) {
		printf("detail file error\n");
		exit(1);
	}

	fprintf(detail_file, "%d\n", schedule_info.date);
	fprintf(detail_file, "%d\n", schedule_info.start_time);
	fprintf(detail_file, "%d\n", schedule_info.end_time);
	fprintf(detail_file, "%s\n", schedule_info.permissionBit);
	fprintf(detail_file, "%s\n", schedule_info.scheduleName);
	fprintf(detail_file, "%s\n", schedule_info.content);

	fclose(detail_file);

	//save brief list file
	date = schedule_info.date % 100;
	nodeptr newnode = make_newNode(date, schedule_info.start_time, schedule_info.end_time, userID, schedule_info.permissionBit, schedule_info.scheduleName, detail_file_path);

	schedule_year_and_month = schedule_info.date / 100;
	schedule_year = schedule_year_and_month / 100;

	sprintf(brief_file_path, "../Data/ScheduleData/%d/%d_Schedule.txt", schedule_year, schedule_year_and_month);

	totalhead = make_schedulelist(1, brief_file_path, 0);
	add_file(newnode);	//add node to linked list
	save_brief_file(brief_file_path);
	return;
}

//add file node to brief list
void add_file(node* insert)
{
	nodeptr cur = NULL, prev = NULL;

	//doubly linked list insertion
	cur = totalhead;
	while (cur != NULL) { //file is not empty.
		if (cur->date >= insert->date) { //find position.
			if (cur->date == insert->date) { //if date is same, program needs to compare time to put new node.
				while (cur->start_time < insert->start_time) { // find correct position
					prev = cur;
					cur = cur->next;
				}
			}
			// insert node
			if (cur == totalhead) { //if new node should insert first.
				insert->next = cur;
				cur->pre = insert;
				totalhead = insert;
			}
			else { //normal case
				insert->next = cur;
				cur->pre = insert;
				prev->next = insert;
				insert->pre = prev;
			}
			return;
		}
		// go to next node.
		prev = cur;
		cur = cur->next;
	}
	if (cur == NULL) { //if file is empty.
		totalhead = insert;
	}
	return;
}

//	< save brief file at correct path >
void save_brief_file(char* tempfilename)
{
	FILE* f = NULL;
	nodeptr cur = NULL;

	f = fopen(tempfilename, "w");
	if (f == NULL) {
		perror("open error");
	}

	cur = totalhead;
	while (cur != NULL) {
		fprintf(f, "%d:%d:%d:%s:%s:%s:%s:\n", cur->date, cur->start_time, cur->end_time, cur->userID, cur->permissionBit, cur->scheduleName, cur->filepath);
		cur = cur->next;
	}
	fclose(f);
	return;
}


/*============================================== M [2] PRINT DETAIL OF SCHEDULE ==============================================
1. SEE DATAIL OF FILE
2. DELETE SCHEDULE FILE
============================================================================================================================*/

//	* < print detail data of schedule >
void viewDetail_MAIN(int index, int year, int year_and_month)
{
	char tempfilename[100];
	nodeptr choiceptr = NULL;
	int choice;

	move(9, 60);
	echo();
	scanw("%d", &choice);
	noecho();
	move(9, 60);
	addstr("    ");

	if (index != 3) {
		index++;
	}

	choiceptr = current;
	for (int j = index; j > choice; j--) //calculate file path
		choiceptr = choiceptr->pre;

	//make total linked list to delete files.
	sprintf(tempfilename, "../Data/ScheduleData/%d/%d_Schedule.txt", year, year_and_month); //make file path
	totalhead = make_schedulelist(1, tempfilename, 0);
	//view detail schedule. open file.
	read_file(choiceptr->filepath);
	//save total linked list
	save_brief_file(tempfilename);
}

//	< print Detail file's context >
void read_file(char* filepath) {

	FILE* file = fopen(filepath, "r");
	if (!file) {
		printf("read file error\n");
		exit(1);
	}

	char ch;
	char date_c[10];
	char start_time_c[5];
	char end_time_c[5];
	char permissionBit[5];
	char scheduleName[40];
	char content[2000];
	char word[30];

	fscanf(file, "%s", date_c);
	fscanf(file, "%s", start_time_c);
	fscanf(file, "%s", end_time_c);
	fscanf(file, "%s", permissionBit);
	fscanf(file, "%s", scheduleName);

	fscanf(file, "%s", content);
	while (1) {
		if (feof(file)) break;
		fscanf(file, "%s", word);
		sprintf(content, "%s %s", content, word);

	}

	fclose(file);

	clear();
	move(1, 0);
	addstr(" *************************************************\n");
	addstr("   Date       :"); addstr(date_c); addstr("\n");
	addstr("   Start time :"); addstr(start_time_c); addstr("\n");
	addstr("   End time   :"); addstr(end_time_c); addstr("\n");
	addstr("   Permission :"); addstr(permissionBit); addstr("\n");
	addstr("   Title      :"); addstr(scheduleName); addstr("\n");
	addstr("   Write      :"); addstr(content); addstr("\n");
	addstr(" *************************************************\n");
	refresh();


	//if you want to delete file, delete node and  orginal schedule file.
	//This code will be modified by WOOSIEUN.
	while (1) {
		ch = getchar();
		if (ch == 'q') { //exit
			return;
		}
		else if (ch == 'd') { //delete schedule
			del_file(filepath);
			return;
		}
	}
}

//delete file from brief list and data system.
void del_file(char* filepath)
{
	nodeptr cur = NULL, prev = NULL;

	//doubly linked list deletion
	cur = totalhead;
	while (cur != NULL) {
		if (strcmp(cur->filepath, filepath) == 0) {
			//if filepath is same, we determine current node is the node we need to delete.
			if (cur == totalhead) { //if current is head.
				totalhead = cur->next;
				totalhead->pre = NULL;
			}
			else { //normal case.
				prev->next = cur->next;
				if (cur->next != NULL) { //current is not end node.
					cur->next->pre = prev;
				}
			}
			free(cur);
			break;
		}
		//go to next node
		prev = cur;
		cur = cur->next;
	}
	//delete real file.
	remove(filepath);
	return;
}


/*============================================== M [4] CALL OUT OTHER USER ==================================================
1. SEE DATAIL OF FILE
2. DELETE SCHEDULE FILE
============================================================================================================================*/
/*	<CALLOUT other user>
make active socket
send callout message
*/
void callOut_MAIN(void)
{
	int pnum;		//portnum of receiver
	char to[20];		//Name of message receiver
	char m_context[100];	//context of message
	time_t msendTime;	//sending time of message
	char fullmessage[150];
	char answer;		//answer of (y/n) question

	signal(SIGIO, SIG_IGN);	//ignore sigio

	clear();
	mvaddstr(0, 0, "-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
	mvaddstr(1, 0, "      LETS CALL OUT FELLOW  |  input 'quit' if you want to quit         ");
	mvaddstr(2, 0, "-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
	mvaddstr(3, 0, "              please  input name of fellow to call out                  ");

	//1. get receiver's Name
	while (1)
	{
		mvaddstr(8, 0, "      To : ");	refresh();	scanw("%s", to);

		//check quit sign
		if (strcmp("quit", to) == 0)
			return;

		//check valid of User Name
		if (pnum = checkValidName(to))
		{
			mvaddstr(6, 0, "                                              ");//remove warning
			break;
		}

		mvaddstr(6, 0, " !! INPUT NAME IS NOT VALID, TRY AGAIN !!   ");
		mvaddstr(8, 0, "                                            ");

	}

	//2. get final answer 
	while (1)
	{
		mvprintw(11, 0, "         WILL YOU SEND MESSAGE TO ' %s ' ? (y/n)     ", to);
		mvaddstr(12, 0, "                                 : "); refresh();
		scanw("%c", &answer);

		if (answer == 'n')
			break;
		else if (answer == 'y')	//-----!!LETS SEND MESSAGE
		{
			struct sockaddr_in servadd;
			struct hostent* hp;
			int sock_id;

			//1. get a socket
			sock_id = socket(AF_INET, SOCK_STREAM, 0);
			//2. set server data
			bzero(&servadd, sizeof(servadd));
			hp = gethostbyname(HOSTNAME);
			bcopy(hp->h_addr, (struct sockaddr*) & servadd.sin_addr, hp->h_length);
			servadd.sin_port = htons(pnum);
			servadd.sin_family = AF_INET;
			//3. connect
			if (connect(sock_id, (struct sockaddr*) & servadd, sizeof(servadd)) != 0)
			{
				clear();
				mvprintw(15, 0, "%s is OFFLINE NOW. TRY NEXT TIME", to);
				refresh();
				sleep(3);
				return;
			}

			//send message 
			//sprintf(

			write(sock_id, userData[thisUser_Index].Name, sizeof(userData[thisUser_Index].Name));
			close(sock_id);

			clear();
			mvaddstr(14, 0, "+-+-+-+-+-+-+                                                                 ");
			mvprintw(15, 0, "    YOUR CALLOUT TO %s IS SUCCESSFULLY SENDED. RETURN TO MAIN PAGE....      ", to);
			mvaddstr(16, 0, "+                                                                +-+-+-+-+-+-+");
			refresh();
			sleep(3);
			break;
		}
		else
		{
			mvaddstr(9, 0, "    !!   YOU SHOULD ANSWER  y or n. TRY AGAIN   !!");
			mvaddstr(11, 0, "                                                            ");//remove answer
		}
	}

}

/*	< check valid name and return portNum >
if name is not exist in User, return -1
*/
int checkValidName(char* name)
{
	for (int i; i < userData_Size; i++)
	{
		if (strcmp(userData[i].Name, name) == 0)
			return userData[i].portNum;
	}
	return -1;
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
	read(thepipe[READ_END], callout, sizeof(callout));

	set_ticker(2000);
}

//	< set timer>
int set_ticker(int n_msecs)
{
	struct itimerval new_timeset;
	long n_sec, n_usecs;

	n_sec = n_msecs / 1000;
	n_usecs = (n_msecs % 1000) * 1000L;

	new_timeset.it_interval.tv_sec = n_sec;
	new_timeset.it_interval.tv_usec = n_usecs;
	new_timeset.it_value.tv_sec = n_sec;
	new_timeset.it_value.tv_usec = n_usecs;

	return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

/*	< handler for signal SIGALRM >
-print callout from other user
-recur standout/standend *10
*/
void calloutPrint_handler(int signum)
{
	if (printcounter % 2 == 0)
		standout();

	mvprintw(LINES - 3, 5, "^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^.^");
	mvprintw(LINES - 2, 5, "!!!     [%5s] IS CALLING YOU NOW      !!!", callout);
	mvprintw(LINES - 1, 5, "v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v.v");


	if (printcounter % 2 == 0)
		standend();
	refresh();

	if (printcounter++ == 10)
	{
		//remove callout
		mvprintw(LINES - 3, 0, "                                                              ");
		mvprintw(LINES - 2, 0, "                                                              ");
		mvprintw(LINES - 1, 0, "                                                              ");
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
	int serversock_id, sock_fd;	//server socket's fd , active sicket's fd
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
	bcopy((void*)hp->h_addr, (void*)&server_add.sin_addr, hp->h_length);
	server_add.sin_port = htons(myport);
	server_add.sin_family = AF_INET;

	bind(serversock_id, (struct sockaddr*) & server_add, sizeof(server_add));

	//4. allow incoming message
	listen(serversock_id, PENDING_NUM);

	//5. main loop
	while (1)
	{
		//!connect
		sock_fd = accept(serversock_id, NULL, NULL);

		//we should recieve message(read)
		read(sock_fd, call_from, sizeof(call_from));	//get message from client

														//signal to parent process to send message
		kill(parent_pid, SIGUSR1);
		write(thepipe[WRITE_END], call_from, sizeof(call_from));

	}


}