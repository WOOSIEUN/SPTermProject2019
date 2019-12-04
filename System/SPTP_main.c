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



//----------DEFINITIONS
#define USER_MAX 100
#define MCODE 486   //master code 
#define QUIT_SIGN "quit"
#define UP 119      //[w] = up
#define DOWN 115   //[s] = down
#define ENTER 13   //[enter] = enter

#define TIMEOFFSET 9
#define List_Xp 10
#define List_Yp_1 15
#define List_Yp_2 21
#define List_Yp_3 27

#define ESC 0x1b
#define DEL 0x7f
#define ISEMPTY 0
#define ISFULL 1



//-------------------------STRUCTURES and GLOBAL VARIABLES
//user data structure
typedef struct
{
	char ID[20];
	char PW[20];
	char Name[20];
	int isMaster;
}user;
//1. startmenu variables
int login_return = 0;
int signin_return = 0;
int choice = 0;
//2. Login
user userData[USER_MAX]; //structure array for userData
int userData_Size;   //size of userData array
int thisUser_Index;   //index of this user in structure array


/* Brief File Structure */
typedef struct node *nodeptr;
typedef struct node {
	int date;
	int start_time;
	int end_time;
	char userID[20];
	char permissionBit[3];
	char scheduleName[100];
	char filepath[100];
	nodeptr pre;
	nodeptr next;
} node;
nodeptr totalhead;
nodeptr head;
nodeptr current;
char user_ID[20];
char user_Name[20];
int user_isMaster;


/* Detailed File Structure */
typedef struct Schedule {
	int date;
	int start_time;
	int end_time;
	char permissionBit[3];
	char scheduleName[100];
	char content[2000];
}Schedule;

/* If there is an empty box, print out a message to write it again. */
typedef struct is_empty_box {
	int date;
	int start_time;
	int end_time;
	int permissionBit;
	int scheduleName;
	int content;
}BOX;
BOX scedule_list_check_o_or_x;



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
int dup_Check(char*, char*);

//main list
int return_today();
void print_menu();
void search_schedule(int *year, int *year_and_month, int *date, int target);
struct node * make_schedulelist(int mode, char *filename, int *date);
struct node *allocate_node();
void initialize_node(nodeptr current, int date, int start, int end, char *ID, char *permission, char *sname, char *filepath);
void print_list_detail(int order, int group, int year, int year_and_month);
void smaller_than_ten(int target, char *targetstr);
void clear_list_detail();
void del_file(char *filepath);
void add_file(node *insert);
void save_brief_file(char *tempfilename);

//add file or print schedule details.
void insert_schedule_file(Schedule schedule_info);
void save_detail_file(Schedule schedule_info, char *userID, int year, int mon, int day);
Schedule write_content(Schedule schedule_info);
Schedule write_scheduleName(Schedule schedule_info);
Schedule write_permissionBit(Schedule schedule_info);
Schedule write_end_time(Schedule schedule_info);
Schedule write_start_time(Schedule schedule_info);
Schedule write_date(Schedule schedule_info);
void read_file(char *filepath);
void screen_fix_info();

void main() {
	int i, j, year, year_and_month, date, today, choice;
	char ch;
	nodeptr isHead = NULL, choiceptr = NULL;
	char tempfilename[100];

	set_SIGIO();   //1. set sigio signal
	get_userData();   //2. make user structure array from userData.txt

	start_Screen(); //3. start screen 
					//-> in this function, login & signin are excecuted.
					// now you can start main page ( schedule )
					//but should do clear();


					//start main screen
	strcpy(user_ID, userData[thisUser_Index].ID);
	strcpy(user_Name, userData[thisUser_Index].Name);
	user_isMaster = userData[thisUser_Index].isMaster;

	initscr();
	clear();

	today = return_today();
	print_menu();

	search_schedule(&year, &year_and_month, &date, today);

	current = head;
	while (1) {
		if (head == NULL) {// This case means linked list is empty. Print NO SCHEDULE
			move(List_Yp_1, List_Xp);
			addstr("\n        NO SCHEDULE        \n");
			return;
		}
		else {
			for (i = 0; i < 3; i++) {
				if (current != head && current->next == NULL) {
					break;
				}
				else if (current == head && i == 0) {
					print_list_detail(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
					if (current->next == NULL) {
						break;
					}
					continue;
				}
				current = current->next;
				print_list_detail(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
			}
			if (i != 1) {
				i--;
			}
		}
		refresh();

		while (1) {
			ch = getchar();
			if (ch == 'a') { //prev page
				isHead = current;
				if (head == NULL) { //if linked list is empty, ignore next page call
					continue;
				}
				for (j = 0; j < i;j++) {
					isHead = isHead->pre;
				}
				if (isHead == head) { //ignore prev page call
					continue;
				}
				for (j = 0; j < 3;j++) {
					isHead = isHead->pre;
				}
				if (isHead == head) {
					current = head;
				}
				else {
					for (j = 0; j < i + 3; j++) {
						current = current->pre;
					}
				}
				
				clear_list_detail();
				break;
			}
			else if (ch == 'd') { //next page
				if (head == NULL || current->next == NULL) { //list is end. ignore next page call
					continue;
				}
				clear_list_detail();
				break;
			}
			else if (ch == '1') { //if user wants to add schedule.
				clear();
				echo();
				screen_fix_info();
				Schedule schedule_information;
				schedule_information = write_date(schedule_information);
				insert_schedule_file(schedule_information);
				noecho();
				//reset the settings of main screen.
				clear();
				return_today();
				print_menu();
				search_schedule(&year, &year_and_month, &date, today);
				current = head;
				break;
			}
			else if (ch == '2') { //if user wants to view schedule detail.
				move(8, 50);
				echo();
				scanw("%d", &choice);
				noecho();
				move(8, 50);
				addstr("    ");
				choiceptr = current;
				for (j = i; j > choice; j--) {//calculate file path
					choiceptr = choiceptr->pre;
				}
				//make total linked list to delete files.
				sprintf(tempfilename, "../Data/ScheduleData/%d/%d_Schedule.txt", year, year_and_month); //make file path
				totalhead = make_schedulelist(1, tempfilename, 0);
				//view detail schedule. open file.
				read_file(choiceptr->filepath);
				//save total linked list
				save_brief_file(tempfilename);
				//reset the settings of main screen.
				clear();
				return_today();
				print_menu();
				search_schedule(&year, &year_and_month, &date, today);
				current = head;
				break;
			}
			else if (ch == '3') { //if user wants to search schedule
				move(10, 42);
				echo();
				scanw("%d", &today);
				noecho();
				move(10, 42);
				addstr("         ");
				//reset the settings of main screen.
				clear();
				return_today();
				print_menu();
				search_schedule(&year, &year_and_month, &date, today);
				current = head;
				break;
			}
			else if (ch == '4') { //if user wants to quit this program, return here.
				endwin();
				return;
			}
		}
	}
}

//   < set sigio signal >
void set_SIGIO()
{
	int fd_flags;

	fcntl(0, F_SETOWN, getpid());
	fd_flags = fcntl(0, F_GETFL);
	fcntl(0, F_SETFL, (fd_flags | O_ASYNC));
}

//   < print start menu >
void start_Screen()
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

		if (choice)
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

/*   < Handler for startMenu input >
if UP ,DOWN : change state
if ENTER : execute function
*/
void start_Handler(int signum)
{
	int  input;

	input = getchar();   //get input 

	switch (input)
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


/*   < make userStucture >
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

/*   < Login >
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

/*   < find index of inputID >
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

/*   < Sign in >
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

/*   < check user's input in signin function >
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

/*   < check input's duplication >
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



// main screen
void clear_list_detail() {
	int xp, yp;

	xp = List_Xp;
	yp = List_Yp_1;
	move(yp, xp);
	addstr("                                                                                    ");
	move(yp + 1, xp);
	addstr("                                                                                    ");
	move(yp + 2, xp);
	addstr("                                                                                    ");
	xp = List_Xp;
	yp = List_Yp_2;
	move(yp, xp);
	addstr("                                                                                    ");
	move(yp + 1, xp);
	addstr("                                                                                    ");
	move(yp + 2, xp);
	addstr("                                                                                    ");
	xp = List_Xp;
	yp = List_Yp_3;
	move(yp, xp);
	addstr("                                                                                    ");
	move(yp + 1, xp);
	addstr("                                                                                    ");
	move(yp + 2, xp);
	addstr("                                                                                    ");

}

void print_list_detail(int order, int group, int year, int year_and_month) {
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

void smaller_than_ten(int target, char *targetstr) {
	if (target < 10) {
		sprintf(targetstr, "0%d", target);
	}
	else {
		sprintf(targetstr, "%d", target);
	}
	return;
}

int return_today() { //Print Today's Date and return today's date. Use header file time.h.
	struct tm *t;
	time_t timer;
	int today;

	//change time to Korea's time
	timer = time(NULL);
	timer += TIMEOFFSET * 3600;
	t = localtime(&timer);
	today = (t->tm_year + 1900) * 100;
	today = (today + (t->tm_mon + 1)) * 100;
	today = (today + t->tm_mday);
	move(1, 1);
	addstr("*********************************************************\n");
	addstr("                          Today                          \n");
	printw("                         %d.%d.%d                        \n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	addstr("*********************************************************\n");

	return today;
}

void print_menu() {//print menu. Details of this function need to be modified.
	move(5, 1);
	addstr("*********************************************************\n");
	addstr("1. Add Schedule.\n");
	addstr("2. View Schedule Detail. \n   (Enter Schedule Number. Press Enter Key.)  :\n");
	addstr("3. Search Schedule.\n   (Enter YYYYMMDD. Press Enter Key.)  :\n");
	addstr("4. Quit.\n");
	addstr("*********************************************************\n");
	refresh();
}

void search_schedule(int *year, int *year_and_month, int *date, int target) {
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

struct node * make_schedulelist(int mode, char *filename, int *date) {
	//mode 0 is search mode, mode 1 is edit and make file or dir mode.

	FILE *f = NULL;
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
		newnode = allocate_node();
		initialize_node(newnode, t_date, t_start, t_end, t_ID, t_permission, t_sname, t_filepath);
		newnode->next = NULL;
		newnode->pre = NULL;

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
	return first;
}

struct node *allocate_node() {
	nodeptr ptr = NULL;
	ptr = (struct node *)malloc(sizeof(struct node));
	return ptr;
}

void initialize_node(nodeptr current, int date, int start, int end, char *ID, char * permission, char *sname, char *filepath) {
	current->date = date;
	current->start_time = start;
	current->end_time = end;
	strcpy(current->userID, ID);
	strcpy(current->permissionBit, permission);
	strcpy(current->scheduleName, sname);
	strcpy(current->filepath, filepath);
	current->next = NULL;
	current->pre = NULL;
	return;
}



//=============================================SCREEN================================================================
void screen_fix_info()
{
	//It is on the writing screen.
	clear();
	move(0, 0);
	addstr("w: write    r: read\n");

	move(1, 0);
	addstr("*************************************************\n");
	addstr("   Date       :\n"); //2,0
	addstr("   Start time :\n"); //3,0
	addstr("   End time   :\n"); //4,0
	addstr("   Permission :\n"); //5,0
	addstr("   Title      :\n"); //6,0
	addstr("   Write      :\n"); //7,0
	move(2, 15);
	refresh();

}

//=============================================WRITE_DETAIL_FILE================================================================

Schedule write_date(Schedule schedule_info)
{
	move(2, 15);
	char date[10];
	scanw("%s", date);
	refresh();
	scedule_list_check_o_or_x.date = ISFULL;
	schedule_info.date = atoi(date);
	schedule_info = write_start_time(schedule_info);
	return schedule_info;
}

Schedule write_start_time(Schedule schedule_info)
{
	move(3, 15);
	char start_time[5];
	scanw("%s", start_time);
	refresh();
	scedule_list_check_o_or_x.start_time = ISFULL;
	schedule_info.start_time = atoi(start_time);
	schedule_info = write_end_time(schedule_info);
	return schedule_info;
}

Schedule write_end_time(Schedule schedule_info)
{
	move(4, 15);
	char end_time[5];
	scanw("%s", end_time);
	refresh();
	scedule_list_check_o_or_x.end_time = ISFULL;
	schedule_info.end_time = atoi(end_time);
	schedule_info = write_permissionBit(schedule_info);
	return schedule_info;
}

Schedule write_permissionBit(Schedule schedule_info)
{
	move(5, 15);
	char permissionBit[3];
	scanw("%s", permissionBit);
	refresh();
	scedule_list_check_o_or_x.permissionBit = ISFULL;
	strcpy(schedule_info.permissionBit, permissionBit);
	schedule_info = write_scheduleName(schedule_info);
	return schedule_info;
}

Schedule write_scheduleName(Schedule schedule_info)
{
	move(6, 15);
	char scheduleName[100];
	scanw("%s", scheduleName);
	refresh();
	scedule_list_check_o_or_x.scheduleName = ISFULL;
	strcpy(schedule_info.scheduleName, scheduleName);
	schedule_info = write_content(schedule_info);
	return schedule_info;
}

Schedule write_content(Schedule schedule_info)
{
	move(7, 15);
	refresh();
	char content[2000];
	int content_i = 0;

	char c;
	int i = 0;
	int line = 8;

	move(line, 0);
	addstr("=>");
	refresh();

	while ((c = getchar()) != ESC) {
		if (c == ENTER) {
			/* new line */
			move(0, 0);
			line = line + 1;
			move(line, 2);
			refresh();
			i = 0;
		}
		else if (c == DEL) {
			/* delete char */
			putchar(c);
			content_i--;
			content[content_i] = ' ';
			i--;
		}
		else {
			putchar(c);
			content[content_i] = c;
			content_i++;
			i++;
		}
	}
	strcpy(schedule_info.content, content);
	refresh();
	scedule_list_check_o_or_x.content = ISFULL;
	return schedule_info;
}

//=============================================SAVE_DETAIL_FILE================================================================

void save_detail_file(Schedule schedule_info, char *userID, int year, int mon, int day) {
	FILE *brief_file;
	int schedule_year, schedule_year_and_month, date;
	nodeptr newnode;

	char detail_file_path[100];
	char brief_file_path[100];
	char mon_c[3];
	smaller_than_ten(mon, mon_c);
	char day_c[3];
	smaller_than_ten(day, day_c);
	sprintf(detail_file_path, "../Data/ScheduleData/%d/%s/%s_%d%s%s.txt", year, mon_c, userID, year, mon_c, day_c);

	FILE *detail_file;
	detail_file = fopen(detail_file_path, "w");
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
	newnode = allocate_node();
	initialize_node(newnode, date, schedule_info.start_time, schedule_info.end_time, userID, schedule_info.permissionBit, schedule_info.scheduleName, detail_file_path);
	
	schedule_year_and_month = schedule_info.date / 100;
	schedule_year = schedule_year_and_month / 100;
	
	sprintf(brief_file_path, "../Data/ScheduleData/%d/%d_Schedule.txt", schedule_year, schedule_year_and_month);
	
	totalhead = make_schedulelist(1, brief_file_path, 0);
	add_file(newnode);
	save_brief_file(brief_file_path);
	return;
}

//=============================================READ_DETAIL_FILE================================================================
void read_file(char *filepath) {

	FILE *file = fopen(filepath, "r");
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
		else if (ch == 'd') {
			del_file(filepath);
			return;
		}
	}
}

//=============================================SAVE_FILE================================================================
void insert_schedule_file(Schedule schedule_info) {
	/* Get user ID */
	char userID[20];
	strcpy(userID, userData[thisUser_Index].ID);

	/* Get current time */
	struct tm *t;
	time_t timer;
	int year, mon, day;
	timer = time(NULL);
	t = localtime(&timer);
	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	day = t->tm_mday;


	/* Date written by the user */
	int s_date;
	int s_year, s_mon, s_day;

	save_detail_file(schedule_info, userID, year, mon, day);

}





//delete file from brief list and data system.
void del_file(char *filepath) {
	nodeptr cur = NULL, prev = NULL;

	//doubly linked list deletion
	cur = totalhead;
	while (cur != NULL) {
		if (strcmp(cur->filepath, filepath) == 0) {
			if (cur == totalhead) {
				totalhead = cur->next;
				totalhead->pre = NULL;
			}
			else {
				prev->next = cur->next;
				cur->next->pre = prev;
			}
			free(cur);
			break;
		}
		prev = cur;
		cur = cur->next;
	}
	remove(filepath);
	return;
}

//add file to brief list
void add_file(node *insert) {
	nodeptr cur = NULL, prev = NULL;

	//doubly linked list insertion
	cur = totalhead;
	while (cur != NULL) {
		if (cur->date >= insert->date) {
			if (cur->date == insert->date) {
				while (cur->start_time < insert->start_time) {
					prev = cur;
					cur = cur->next;
				}
			}
			if (cur == totalhead) {
				insert->next = cur;
				cur->pre = insert;
				totalhead = insert;
			}
			else {
				insert->next = cur;
				cur->pre = insert;
				prev->next = insert;
				insert->pre = prev;
			}
			return;
		}
		prev = cur;
		cur = cur->next;
	}
	prev->next = insert;
	insert->pre = prev;
	return;
}

void save_brief_file(char *tempfilename) {
	FILE * f = NULL;
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