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

/* Detailed File Structure */
typedef struct {
	int date;
	int year;
	int mon;
	int day;
}DATE;
typedef struct Schedule {
	DATE d;
	int start_time;
	int end_time;
	char permissionBit[3];
	char scheduleName[100];
	char content[2000];
}Schedule;
Schedule schedule_info;

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

struct node *allocate_node();
void smaller_than_ten(int target, char *targetstr);
void save_brief_file(Schedule schedule_info, nodeptr brief_schedule);
nodeptr create_brief_file(Schedule schedule_info, char *userID, char *detail_file_path);
void save_detail_file(Schedule schedule_info, char *detail_file_path);
void create_save_schedule_file(Schedule schedule_info);

void write_content();
void write_scheduleName();
void write_permissionBit();
void write_end_time();
void write_start_time();
void year_mon_day(int date, int *year, int *mon, int *day);
void write_date();

int get_move_x();
int get_move_y();
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
		write_date();
		create_save_schedule_file(schedule_info);
		move(LINES - 1, 0);
		addstr("                      =>>save schedule...\n");
		refresh();
		sleep(1);
		//sig_handler(2);
	}
	else if (c == 'r') {
		char filepath[100] = "../Data/ScheduleData/2019/12/currentuserID_20191212.txt";
		read_file(filepath);
	}

	move(LINES - 1, 0);
	addstr("                                             ");
	move(LINES - 1, 0);
	addstr("press ESC : exit\n");
	refresh();

	while (getch() != ESC);
	endwin();
	return 0;
}
//=============================================SIGNAL================================================================
int get_move_x() {
	if (scedule_list_check_o_or_x.date == 0) return 3;
	if (scedule_list_check_o_or_x.start_time == 0) return 4;
	if (scedule_list_check_o_or_x.end_time == 0) return 5;
	if (scedule_list_check_o_or_x.permissionBit == 0) return 6;
	if (scedule_list_check_o_or_x.scheduleName == 0) return 7;
	if (scedule_list_check_o_or_x.content == 0)return 9;

}
int get_move_y() {
	if (scedule_list_check_o_or_x.content == 0) return 3;
	else return 15;
}

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
	move(LINES - 6, 0);
	addstr("Please select a number : 1. Save\n");
	addstr("                         2. Rewrite\n");
	addstr("                         3. Exit\n");
	refresh();

	char choose_number;
	choose_number = getchar();

	switch (choose_number)
	{
	case '1':
		/* check write box*/
		if (check_box()) {
			// save
			sleep(2);
			move(LINES - 6, 0);
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");

			
			move(LINES - 1, 0);
			addstr("                      =>>save schedule...\n");
			refresh();
			create_save_schedule_file(schedule_info);
			sleep(1);
			move(LINES - 1, 0);
			addstr("                                             ");
			move(LINES - 1, 0);
			addstr("press ESC : exit\n");
			refresh();
			while (getch() != ESC);
			endwin();
			exit(1);
		}
		else {
			// write continuou
			move(LINES - 6, 0);
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");
			addstr("                                          ");


			move(LINES - 1, 0);
			addstr("\n                      =>>have an empty block\n");
			refresh();

			sleep(1);
			move(LINES - 1, 0);
			addstr("                                                  ");
			refresh();
			int x_y[2];
			x_y[0] = get_move_x();
			x_y[1] = get_move_y();
			move(x_y[1], x_y[0]);
			refresh();

			while (getch() != ESC);
			endwin();

		}
		break;
	case '2':
		/* rewrite */
		break;
	case '3':
		/* exit */
		break;
	default:
		printw("wrong\n");
		break;
	}
	//printw("I Received SIGINT(%d)\n", SIGINT);
	refresh();
	move(LINES - 6, 0);
	addstr("                                          ");
	addstr("                                          ");
	addstr("                                          ");
	addstr("                                          ");
	addstr("                                          ");
	addstr("                                          ");

	refresh();

}
//===========================================CREATE_SAVE_FILE================================================================
void create_save_schedule_file(Schedule schedule_info) {

	/* Get user ID */
	char userID[20];
	//userID = get_userID();
	strcpy(userID, "twkp6");

	/* Get current time */
	struct tm *t;
	time_t timer;
	int year, mon, day;
	int hour, min, sec;
	timer = time(NULL);
	t = localtime(&timer);
	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	day = t->tm_mday;
	hour = t->tm_hour;
	min = t->tm_min;
	sec = t->tm_sec;

	/* create detail file path */
	char detail_file_path[100];
	char mon_c[3];
	smaller_than_ten(mon, mon_c);
	char day_c[3];
	smaller_than_ten(day, day_c);
	sprintf(detail_file_path, "../Data/ScheduleData/%d/%s/%s_%d%s%s%d%d%d.txt", year, mon_c, userID, year, mon_c, day_c, hour, min, sec);

	save_detail_file(schedule_info, detail_file_path);

	nodeptr brief_schedule = NULL;
	//brief_schedule = allocate_node();
	/* put the details of the file into a brief file. */
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

	fclose(detail_file);

}

//=============================================STRUCTER_BRIEF_FILE================================================================
struct node *allocate_node() {
	nodeptr ptr = NULL;
	ptr = (struct node *)malloc(sizeof(struct node));
	return ptr;
}

nodeptr create_brief_file(Schedule schedule_info, char *userID, char *detail_file_path) {

	nodeptr brief_schedule;
	brief_schedule = allocate_node();

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
	// 2019 12 04
	int temp = date;
	*year = temp / 10000;
	*mon = (temp % 10000) / 100;
	*day = temp % 100;
}

void write_date()
{
	move(3, 15);
	char date[10];
	int year, mon, day;
	scanw("%s", date);
	refresh();
	schedule_info.d.date = atoi(date);
	year_mon_day(schedule_info.d.date, &year, &mon, &day);
	schedule_info.d.year = year;
	schedule_info.d.mon = mon;
	schedule_info.d.day = day;
	scedule_list_check_o_or_x.date = ISFULL;
	write_start_time();
	//schedule_info = write_start_time(schedule_info);
	//return schedule_info;
}

void write_start_time()
{
	move(4, 15);
	char start_time[5];
	scanw("%s", start_time);
	refresh();
	schedule_info.start_time = atoi(start_time);
	scedule_list_check_o_or_x.start_time = ISFULL;
	write_end_time();
	//schedule_info = write_end_time(schedule_info);
	//return schedule_info;
}

void write_end_time()
{
	move(5, 15);
	char end_time[5];
	scanw("%s", end_time);
	refresh();
	schedule_info.end_time = atoi(end_time);
	scedule_list_check_o_or_x.end_time = ISFULL;
	write_permissionBit();
	//schedule_info = write_permissionBit(schedule_info);
	//return schedule_info;
}

void write_permissionBit()
{
	move(6, 15);
	char permissionBit[3];
	scanw("%s", permissionBit);
	refresh();
	strcpy(schedule_info.permissionBit, permissionBit);
	scedule_list_check_o_or_x.permissionBit = ISFULL;
	write_scheduleName();
//	schedule_info = write_scheduleName(schedule_info);
//	return schedule_info;
}

void write_scheduleName()
{
	move(7, 15);
	addstr("");
	refresh();
	char scheduleName[100];
	int scheduleName_i = 0;

	char c;

	while ((c = getchar()) != ENTER) {
		scedule_list_check_o_or_x.scheduleName = ISFULL;
		if (c == DEL) {
			/* delete char */
			putchar(c);
			scheduleName_i--;
			scheduleName[scheduleName_i] = '\0';
		}
		else {
			putchar(c);
			scheduleName[scheduleName_i] = c;
			scheduleName[scheduleName_i+1] = '\0';

			scheduleName_i++;
		}
		refresh();
	}
	strcpy(schedule_info.scheduleName, scheduleName);
	refresh();
	//scedule_list_check_o_or_x.scheduleName = ISFULL;
	//schedule_info = write_content(schedule_info);
	write_content();
	//return schedule_info;
}

void write_content()
{
	move(8, 15);
	addstr("");
	refresh();
	char content[2000];
	int content_i = 0;

	char c;
	move(9, 0);
	addstr("=>");
	refresh();

	while ((c = getchar()) != ENTER) {
		scedule_list_check_o_or_x.content = ISFULL;

		if (c == DEL) {
			/* delete char */
			putchar(c);
			content_i--;
			content[content_i] = '\0';
		}
		else {
			putchar(c);
			content[content_i] = c;
			content[content_i + 1] = '\0';
			content_i++;
		}
		refresh();
		strcpy(schedule_info.content, content);
	}
	//strcpy(schedule_info.content, content);
	refresh();
//	scedule_list_check_o_or_x.content = ISFULL;
	//return schedule_info;
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

	int start_point = 2;

	move(start_point, 0);
	addstr("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	//addstr("*************************************************\n");
	addstr("   Date       :\n"); //2,0
	addstr("   Start time :\n"); //3,0
	addstr("   End time   :\n"); //4,0
	addstr("   Permission :\n"); //5,0
	addstr("   Title      :\n"); //6,0
	addstr("   Write      :\n"); //7,0
	move(start_point + 1, 15);	//move back of date
	refresh();

}