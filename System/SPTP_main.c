#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <curses.h>
#include <string.h>

#define TIMEOFFSET 9
#define List_Xp 10
#define List_Yp_1 12
#define List_Yp_2 18
#define List_Yp_3 24

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
nodeptr head;
nodeptr current;
nodeptr choice;

int print_today();
void print_menu();
void search_schedule(int *year, int *year_and_month, int *date, int target);
struct node * make_schedulelist(int mode, char *filename, int *date);
struct node *allocate_node();
void initialize_node(nodeptr current, int date, int start, int end, char *ID, char *permission, char *sname, char *filepath);
void print_list_detail(int order, int group, int year, int year_and_month);
char user_ID[20];
char user_Name[20];
int user_isMaster;
void smaller_than_ten(int target, char *targetstr);
void clear_list_detail();

void main() {
	int i,j, year, year_and_month, date, today;
    int listend = 0;
    char ch;
    nodeptr isHead = NULL;
    

	strcpy(user_ID, "ramtk6726");
	strcpy(user_Name, "EUN");
	user_isMaster = 0;

	initscr();
	clear();
    //keypad(stdscr, TRUE);

	today = print_today();
	print_menu();

	search_schedule(&year, &year_and_month, &date, today);

	//if date is 0, our program determines that the user wants to find a month schedule.
	//if date is not 0, our program determines that the user wants to find a specific date schedule.
    current = head;
	while (1) {
		if (head == NULL) {// This case means file is empty. Print NO SCHEDULE
			move(List_Yp_1, List_Xp);
			addstr("\n        NO SCHEDULE        \n");
			return;
		}
		else {
			for (i = 0; i < 3; i++) {
				if (current->next == NULL) {
                    break;
				} else if(current == head && i == 0){
                    print_list_detail(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
                    continue;
                }
                current = current->next;
                print_list_detail(i + 1, strcmp(current->permissionBit, "10"), year, year_and_month);
            }	
		}
		refresh();
        
        while(1){
            ch = getchar();
            if(ch == 'a'){
                isHead = current;
                for(j=0;j<i-1;j++){
                    isHead = isHead->pre;
                }
                if(isHead==head){ //ignore prev page call
                    continue;
                }
                for(j=0;j<i+2;j++){
                    current = current->pre;
                }
                clear_list_detail();
                break;
            } else if(ch == 'd'){
                if(current->next==NULL){ //list is end. ignore next page call
                    continue;
                }
                clear_list_detail();
                break;
            }else if (ch == '4'){ //if user want to quit this program, return here.
                endwin();
                return;
            }
        }

		//new while loop is needed here and write code that under this line in while loop.
		//
		//search : Receive new search date -> break new while loop -> search use value today yyyymmdd -> search_schedule
		//add schedule : add -> break new while loop -> Give same value (today) to search_schedule
		//change choice pointer : Receive key -> If the page has to be changed, break.
		//up and down : move choice pointer, and mark where the choice pointer is. choice cnt ==1 or 3, limit up or down.
		//search_schedule maybe needed here.
	}
}

void clear_list_detail(){
    int xp, yp;
    
    xp = List_Xp;
    yp = List_Yp_1;
    move(yp,xp);
    addstr("                                                                                    ");
    move(yp+1,xp);
    addstr("                                                                                    ");
    move(yp+2,xp);
    addstr("                                                                                    ");
    xp = List_Xp;
    yp = List_Yp_2;
    move(yp,xp);
    addstr("                                                                                    ");
    move(yp+1,xp);
    addstr("                                                                                    ");
    move(yp+2,xp);
    addstr("                                                                                    ");
    xp = List_Xp;
    yp = List_Yp_3;
    move(yp,xp);
    addstr("                                                                                    ");
    move(yp+1,xp);
    addstr("                                                                                    ");
    move(yp+2,xp);
    addstr("                                                                                    ");
    
}
    
void print_list_detail(int order, int group, int year, int year_and_month) {
	//This program can print 3 list. order is used to determine a y-position.
	char sort[6];
	int xp, yp;
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

	move(yp, xp);
	printw("[%s]  written by %s\n", sort, current->userID);
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

int print_today() { //Print Today's Date and return today's date. Use header file time.h.
	struct tm *t;
	time_t timer;
	int today;

	//change time to Korea's time
	timer = time(NULL);
	timer += TIMEOFFSET * 3600;

	t = localtime(&timer);
	addstr("*************************************************\n");
	addstr("                      Today                      \n");
	printw("                    %d.%d.%d                     \n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	addstr("*************************************************\n");

	today = (t->tm_year + 1900) * 100;
	today = (today + (t->tm_mon + 1)) * 100;
	today = (today + t->tm_mday);

	return today;
}

void print_menu() {//print menu. Details of this function need to be modified.
	addstr("*************************************************\n");
	addstr("1. Add Schedule.\n");
    addstr("2. View Schedule Detail.\n");
    addstr("3. Search Schedule.\n   (Enter YYYYMMDD. Press Enter Key.)  :\n");
	addstr("4. Quit.\n");
	addstr("*************************************************\n");
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

	if (mode == 0) { //search mode. If Schedule file doesn't exist, print NO SCHEDULE.
		f = fopen(filename, "r");
		if (f == NULL) {
			move(List_Yp_1, List_Xp);
			addstr("\n        NO SCHEDULE        \n");
			return first;
		}
	}
	else if (mode == 1) { //add mode
						  //code that check and mkdir should be added here.
		f = fopen(filename, "r");
		if (f == NULL) {
			f = fopen(filename, "w");
			if (f == NULL) {
                perror("open error");
                return first;
			}
			//If the target directory doesn't exists, an algorithm to create dir is required.
			//make file and write input. return here.
		}
	}
	else { //incorrect mode input error.
		return first;
	}

	current = first;
	while (fgets(str, sizeof(str), f) != NULL) { //read data file one by one.
		str[strlen(str)] = 0;
		sscanf(str, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:", ct_date, ct_start, ct_end, t_ID, t_permission, t_sname, t_filepath);
		t_date = atoi(ct_date);
		t_start = atoi(ct_start);
		t_end = atoi(ct_end);

		if (*date != 0 && t_date != *date) {
			continue;
		} else if (strcmp(t_permission, "01") == 0) { //private file.
            //Check permission that allows the program print private file or not.
            if (strcmp(t_ID, user_ID) != 0 ||  user_isMaster != 0) { 
                continue;
            }
        }

		//make linked list
		newnode = allocate_node();
		initialize_node(newnode, t_date, t_start, t_end, t_ID, t_permission, t_sname, t_filepath);
		newnode->next = NULL;
        
        if (first == NULL) {
			first = newnode;
			current = first;
		}
		else {
			if (prev == NULL) {
				prev = first;
			}
			else {
				prev = current;
			}
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
