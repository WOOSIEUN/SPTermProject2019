#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <time.h>
#include <curses.h>
#include <string.h>

typedef struct node *nodeptr;
typedef struct node{
    nodeptr next;
    int date;
    int start_time;
    int end_time;
    char userID[20];
    int permissionBit;
    char scheduleName[100];
    char filepath[100];
} node;


void print_today();
void print_menu();
void print_today_schedule();
void search_schedule(int target);



void main(){
    initscr();
    clear();
    print_today();
    print_menu();
    getchar();   
    endwin();
}

void print_today(){ //Print Today's Date. Use header file time.h.
    struct tm *t;
    time_t timer;

    timer = time(NULL);
    t = localtime(&timer);
    addstr("*************************************************\n");
    addstr("                      Today                      \n");
    printw("                    %d.%d.%d                     \n", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
    addstr("*************************************************\n"); 
    //This parted should be modified. Mismatch between AWS's server time and Korea's local time.
    //Add some function to calculate correct time.
    refresh();
}

void print_menu(){//print menu. Details of this function need to be modified.
    addstr("*************************************************\n");
    addstr("plz add menu here!!\n");
    addstr("*************************************************\n");
    refresh();
}

void print_today_schedule(){
    int today;
    //need to add a function to modify the algorithm to make today's date.
    search_schedule(today)
    return;
}

void search_schedule(int target){
    int year, year_and_month, date;
    FILE *f=NULL;
    char filename[100];
    nodeptr head = NULL;
    
    if(target>10000000){
        return; // If the target is larger than 8 digits number, it will not print.
    }
    if (target>1000000){ //if target is date.
        year_and_month = target/100;
        date = target-(year_and_month*100);
    }else{ //if target is month
        year_and_month = target;
        date = 0;
    }
    year = year_and_month/100; //get year 

    sprintf(filename, "./Data/Schedule/%d/%d_Schedule.txt",year,year_and_month); //make file path
    
    if(fopen_s(&f, "filename", "r") != NULL){
        addstr("\n       NO SCHEDULE        \n"); //If Schedule file doesn't exist, print NO SCHEDULE
        return;
    }
    
    //make Linked List
    head = make_schedulelist(0, filename);

    //if date is 0, our program determines that the user wants to find a month schedule.
    //if date is not 0, our program determines that the user wants to find a specific date schedule.
    
    if (head == NULL) {// This case means file is empty. Print NO SCHEDULE
        addstr("\n        NO SCHEDULE        \n");
        return;
    }
}

struct node * make_schedulelist(int mode, char *filename){
    //mode 0 is search mode, mode 1 is edit and make file or dir mode.
    
    FILE *f=NULL;
    nodeptr first = NULL; //head of linked list. return this value.
    nodeptr newnode = NULL;
    nodeptr current = NULL;
    char str[256], t_ID[20], t_sname[100], t_filepath[100];
    char ct_date[2], ct_start[4], ct_end[4], ct_permission[2];
    int t_date, t_start, t_end, t_permission;
    int i = 0;

    if (mode == 0){ //search mode. If Schedule file doesn't exist, print NO SCHEDULE.
        if(fopen_s(&f, "filename", "r") != NULL){
            addstr("\n        NO SCHEDULE        \n");
            return;
        }
    } else if (mode == 1){ //add mode
        if(fopen_s(&f, "filename", "r") != NULL){
            if(fopen_s(&f, "filename", "w" != NULL)){
                //If the target directory doesn't exists, an algorithm to create dir is required.
                //make file and write input. return here.
            }
        }
    } else{ //incorrect mode input error.
        return;
    }

    while(fgets(str,sizeof(str),f)!=NULL){ //read data file one by one.
        str[strlen(str)]=0;
        sscanf(str, "%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:", ct_date, ct_start, ct_end, t_ID, ct_permission, t_sname, t_filepath);
        t_date = atoi(ct_date);
        t_start = atoi(ct_start);
        t_end = atoi(ct_end);
        t_permission = atoi(ct_permission);
        //make linked list
        if(first == NULL){
            first = allocate_node();
            initialize_node(first, t_date, );

        }
         
    }

    
}

struct node * allocate_node(){
    nodeptr ptr = NULL;
    ptr = (struct node *)malloc(sizeof(struct node));
    return ptr;
}

void initialize_node(nodeptr current, int date, int start, int end, char *ID, int permission, char *sname, char *filepath){
    current-> date = date;
    current->start_time = start;
    current->end_time = end;
    strcpy(current->userID, ID);
    current->permissionBit = permission;
    strcpy(current->scheduleName, sname);
    strcpy(current->filepath, filepath);
    current->next = NULL;
    return;
}
