#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <signal.h>

#define ESC 0x1b
#define DEL 0x7f
#define ENTER 0x0d

typedef struct {
	int date;
	int year;
	int mon;
	int day;
}DATE;
/* Detailed File Structure */
typedef struct Schedule {
	DATE d;
	int start_time;
	int end_time;
	char permissionBit[3];
	char scheduleName[100];
	char content[2000];
}Schedule;
Schedule schedule_information;

/* Brief File Structure */
typedef struct node *nodeptr;
typedef struct node {
	int date;
	int start_time;
	int end_time;
	char userID[20];
	char permissionBit[3];
	char scheduleName[100];
	char filepath[200];
	nodeptr pre;
	nodeptr next;
} node;

#define ISEMPTY 0
#define ISFULL 1
/* If there is an empty box, print out a message to write it again. */
typedef struct is_empty_box {
	int date;
	int start_time;
	int end_time;
	int permissionBit;
	int scheduleName;
	int content;
}BOX;
BOX scedule_list_check_o_or_x = { 0,0,0,0,0,0 };


int check_box();
void sig_handler(int signo);


void smaller_than_ten(int target, char *targetstr);
void save_brief_file(Schedule schedule_info, nodeptr brief_schedule);
nodeptr create_brief_file(Schedule schedule_info, char *userID, char *detail_file_path);
void save_detail_file(Schedule schedule_info, char *detail_file_path);
void insert_schedule_file(Schedule schedule_info);

Schedule write_content(Schedule schedule_info);
Schedule write_scheduleName(Schedule schedule_info);
Schedule write_permissionBit(Schedule schedule_info);
Schedule write_end_time(Schedule schedule_info);
Schedule write_start_time(Schedule schedule_info);
void year_mon_day(int date, int *year, int *mon, int *day);
Schedule write_date(Schedule schedule_info);

void read_file(char *filepath);
void screen_fix_info();


int main() {
	initscr();
	clear();
	screen_fix_info();
	signal(SIGINT, (void *)sig_handler); // Press cntl+c to go to save function
	char c;
	c = getchar();

	if (c == 'w') {
		schedule_information = write_date(schedule_information);
		//insert_schedule_file(schedule_information);
		sig_handler(2);
	}
	else if (c == 'r') {
		char filepath[100] = "../Data/ScheduleData/2019/12/currentuserID_20191212.txt";
		read_file(filepath);
	}

	while (getch() != ESC);
	endwin();
	return 0;
}
//=============================================SIGNAL================================================================
int check_box() {
	//scedule_list_check_o_or_x
	int not_empty = 1;
	if (scedule_list_check_o_or_x.date == 0) not_empty = ISEMPTY;
	if (scedule_list_check_o_or_x.start_time == 0) not_empty = ISEMPTY;
	if (scedule_list_check_o_or_x.end_time == 0) not_empty = ISEMPTY;
	if (scedule_list_check_o_or_x.permissionBit == 0) not_empty = ISEMPTY;
	if (scedule_list_check_o_or_x.scheduleName == 0) not_empty = ISEMPTY;
	if (scedule_list_check_o_or_x.content == 0) not_empty = ISEMPTY;

	return not_empty;
}
void sig_handler(int signo)
{
	move(LINES-6, 0);
	addstr("Please select a number : 1. Save\n");
	addstr("                         2. Rewrite\n");
	addstr("                         3. Exit\n");
	addstr("                      =>>");
	refresh();
	char choose_number;
	choose_number = getchar();
	switch (choose_number)
	{
	case '1':
		/* check write box*/
		if (check_box()) {
			// save
			addstr("                      =>>save schedule\n");
			refresh();
			insert_schedule_file(schedule_information);
			while (getch() != ESC);
			endwin();

		}
		else {
			// write again
			addstr("                      =>>have an empty block\n");
			refresh();
			while (getch() != ESC);
			endwin();

		}
		break;
	case '2':
		break;
	case '3':
		break;
	default:
		printw("wrong\n");
		break;
	}
	//printw("I Received SIGINT(%d)\n", SIGINT);
	refresh();
}
//=============================================SAVE_FILE================================================================
void insert_schedule_file(Schedule schedule_info) {

	/* Get user ID */
	char userID[20];
	//userID = get_userID();
	strcpy(userID, "twkp6");

	/* Get current time */
	struct tm *t;
	time_t timer;
	int year, mon, day;
	timer = time(NULL);
	t = localtime(&timer);
	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	day = t->tm_mday;

	/*  */
	char detail_file_path[100];
	char mon_c[3];
	smaller_than_ten(mon, mon_c);
	char day_c[3];
	smaller_than_ten(day, day_c);
	sprintf(detail_file_path, "../Data/ScheduleData/%d/%s/%s_%d%s%s.txt", year, mon_c, userID, year, mon_c, day_c);

//	save_detail_file(schedule_info, userID, year, mon, day);
	save_detail_file(schedule_info, detail_file_path);

//	create_brief_file(schedule_info, userID, year, mon, day, detail_file_path);
	nodeptr brief_schedule;
	brief_schedule = create_brief_file(schedule_info, userID, detail_file_path);

	save_brief_file(schedule_info, brief_schedule);

}
//=============================================SAVE_DETAIL_FILE================================================================
void smaller_than_ten(int target, char *targetstr) {
	if (target < 10) {
		sprintf(targetstr, "0%d", target);
	}
	else {
		sprintf(targetstr, "%d", target);
	}
	return;
}

//void save_detail_file(Schedule schedule_info, char *userID, int year, int mon, int day) {
void save_detail_file(Schedule schedule_info, char *detail_file_path) {

	FILE *detail_file;
	detail_file = fopen(detail_file_path, "w");
	if (!detail_file) {
		printf("detail file error\n");
		endwin();
		exit(1);
	}

	fprintf(detail_file, "%d\n", schedule_info.d.date);
	fprintf(detail_file, "%d\n", schedule_info.start_time);
	fprintf(detail_file, "%d\n", schedule_info.end_time);
	fprintf(detail_file, "%s\n", schedule_info.permissionBit);
	fprintf(detail_file, "%s\n", schedule_info.scheduleName);
	fprintf(detail_file, "%s\n", schedule_info.content);

	fprintf(detail_file, "%d\n", schedule_info.d.year);
	fprintf(detail_file, "%d\n", schedule_info.d.mon);
	fprintf(detail_file, "%d\n", schedule_info.d.day);
	fprintf(detail_file, "%s\n", detail_file_path);

	fclose(detail_file);

}

//=============================================STRUCTER_BRIEF_FILE================================================================
nodeptr create_brief_file(Schedule schedule_info, char *userID, char *detail_file_path) {

	nodeptr brief_schedule;
	brief_schedule = (nodeptr)malloc(sizeof(node));
	brief_schedule->date = schedule_info.d.date;
	brief_schedule->start_time = schedule_info.start_time;
	brief_schedule->end_time = schedule_info.end_time;
	strcpy(brief_schedule->userID, userID);
	strcpy(brief_schedule->permissionBit, schedule_info.permissionBit);
	strcpy(brief_schedule->scheduleName, schedule_info.scheduleName);
	strcpy(brief_schedule->filepath, detail_file_path);
	brief_schedule->next = NULL;
	brief_schedule->pre = NULL;

	return brief_schedule;
}

void save_brief_file(Schedule schedule_info, nodeptr brief_schedule) {
	char brief_file_path[100];
	int year = schedule_info.d.year;
	char mon_c[3];
	smaller_than_ten(schedule_info.d.mon, mon_c);
	sprintf(brief_file_path, "../Data/ScheduleData/%d/%d%s_Schedule.txt", year, year, mon_c);
	
	FILE *brief_file;
	brief_file = fopen(brief_file_path, "a");
	if (!brief_file) {
		printf("brief file error\n");
		endwin();
		exit(1);
	}

	fprintf(brief_file, "%d:", schedule_info.d.day);

	fprintf(brief_file, "%d:", brief_schedule->start_time);
	fprintf(brief_file, "%d:", brief_schedule->end_time);
	
	fprintf(brief_file, "%s:", brief_schedule->userID);
	fprintf(brief_file, "%s:", brief_schedule->permissionBit);
	fprintf(brief_file, "%s:", brief_schedule->scheduleName);
	fprintf(brief_file, "%s:\n", brief_schedule->filepath);

	fclose(brief_file);
}
//=============================================WRITE_DETAIL_FILE================================================================

void year_mon_day(int date, int *year, int *mon, int *day) {
	// Divide into years, months, and days
	// 20191204
	int temp = date;
	*year = temp / 10000;
	*mon = (temp % 10000) / 100;
	*day = temp % 100;
}

Schedule write_date(Schedule schedule_info)
{
	move(2, 15);
	char date[10];
	int year, mon, day;
	scanw("%s", date);
	refresh();
	scedule_list_check_o_or_x.date = ISFULL;
	schedule_info.d.date = atoi(date);
	year_mon_day(schedule_info.d.date, &year, &mon, &day);
	schedule_info.d.year = year;
	schedule_info.d.mon = mon;
	schedule_info.d.day = day;
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
	int scheduleName_i = 0;
	addstr("");
	refresh();
	char c;
	


	while ((c = getchar()) != ENTER) {
		if (c == DEL) {
			/* delete char */
			putchar(c);
			scheduleName_i--;
			scheduleName[scheduleName_i] = ' ';
		}
		else {
			putchar(c);
			scheduleName[scheduleName_i] = c;
			scheduleName_i++;
		}
	}
	//scanw("%s", scheduleName);
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
	//int x = 0, y = 0;
	//int i = 0;
	//int line = 8;

	move(8, 0);
	addstr("=>");
	refresh();

	while ((c = getchar()) != ENTER) {
		/*if (i >= 50) {
			line = line + 1;
			move(line, 0);
			printw(" ");
			refresh();
			i = 0;
		}
		else */
		//if (c == ENTER) {

		//}else

		if (c == DEL) {
			/* delete char */
			putchar(c);
			content_i--;
			content[content_i] = ' ';
			//i--;
		}
		else {
			putchar(c);
			content[content_i] = c;
			content_i++;
			//i++;
		}
	}
	strcpy(schedule_info.content, content);
	refresh();
	scedule_list_check_o_or_x.content = ISFULL;
	return schedule_info;
}
//=============================================READ_DETAIL_FILE================================================================
void read_file(char *filepath) {

	FILE *file = fopen(filepath, "r");
	if (!file) {
		printf("read file error\n");
		endwin();
		exit(1);
	}

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
}
//=============================================SCREEN================================================================

void screen_fix_info()
{
	//It is on the writing screen.
	clear();
	move(0, 0);
	addstr("w: write 	r: read		<ctrl+c> : save		<Enter> : next step");

	move(1, 0);
	addstr("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	//addstr("*************************************************\n");
	addstr("   Date       :\n"); //2,0
	addstr("   Start time :\n"); //3,0
	addstr("   End time   :\n"); //4,0
	addstr("   Permission :\n"); //5,0
	addstr("   Title      :\n"); //6,0
	addstr("   Write      :\n"); //7,0
	move(2, 15);	//move back of date
	refresh();

}