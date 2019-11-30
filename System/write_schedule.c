#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>

#define ESC 0x1b
#define DEL 0x7f

/* Detailed File Structure */
typedef struct Schedule {
	int date;
	int start_time;
	int end_time;
	char permissionBit[3];
	char scheduleName[100];
	char content[2000];
}Schedule;

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
BOX scedule_list_check_o_or_x;

void smaller_than_ten(int target, char *targetstr);
void insert_schedule_file(Schedule schedule_info);
void save_detail_file(Schedule schedule_info, char *userID, int year, int mon, int day);
nodeptr save_brief_file(Schedule schedule_info, char *userID, int year, int mon, int day, char *detail_file_path);

Schedule write_content(Schedule schedule_info);
Schedule write_scheduleName(Schedule schedule_info);
Schedule write_permissionBit(Schedule schedule_info);
Schedule write_end_time(Schedule schedule_info);
Schedule write_start_time(Schedule schedule_info);
Schedule write_date(Schedule schedule_info);

void read_file(char *filepath);
void screen_fix_info();

int main() {
	initscr();
	clear();
	screen_fix_info();

	char c;
	c = getchar();

	if (c == 'w') {
		Schedule schedule_information;
		schedule_information = write_date(schedule_information);
		insert_schedule_file(schedule_information);
	}
	else if (c == 'r') {
		char filepath[100] = "../Data/ScheduleData/2019/12/currentuserID_20191212.txt";
		read_file(filepath);
	}

	while (getch() != ESC);
	endwin();
	return 0;
}
//=============================================SAVE_FILE================================================================
void insert_schedule_file(Schedule schedule_info) {

	/* Get user ID */
	char userID[20];
	//id = get_userID();
	strcpy(userID, "userID");

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

void save_detail_file(Schedule schedule_info, char *userID, int year, int mon, int day) {
	char detail_file_path[100];
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

	//save_brief_file(schedule_info, userID, year, mon, day, detail_file_path);


}

//=============================================STRUCTER_BRIEF_FILE================================================================
nodeptr save_brief_file(Schedule schedule_info, char *userID, int year, int mon, int day, char *detail_file_path) {

	nodeptr brief_schedule;
	brief_schedule = (nodeptr)malloc(sizeof(node));
	brief_schedule->date = schedule_info.date;
	brief_schedule->start_time = schedule_info.start_time;
	brief_schedule->end_time = schedule_info.end_time;
	strcpy(brief_schedule->userID, userID);
	strcpy(brief_schedule->permissionBit,schedule_info.permissionBit);
	strcpy(brief_schedule->scheduleName, schedule_info.scheduleName);
	strcpy(brief_schedule->filepath, detail_file_path);
	brief_schedule->next = NULL;
	brief_schedule->pre = NULL;
	return brief_schedule;
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
		if (c == '\r' || i >= 50) {
			line += 1;
			move(line, 0);
			refresh();
			i = 0;
		}
		else if (c == DEL) {
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
//=============================================READ_DETAIL_FILE================================================================
void read_file(char *filepath) {

	FILE *file = fopen(filepath, "r");
	if (!file) {
		printf("read file error\n");
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
