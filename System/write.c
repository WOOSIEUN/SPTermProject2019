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
#define KEY_F0 0410	/* Function keys. Space for 64 */
//#define KEY_F(n) (KEY_F0+n)	/* Value of function key n */

typedef struct Schedule {
	char date[10];
	char start_time[5];
	char end_time[5];
	char admit[3];
	char title[40];
	char content[2000];
}Schedule;

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

typedef struct detail_node *detail_nodeptr;
typedef struct detail_node {
	int date;
	int start_time;
	int end_time;
	char userID[20];
	char permissionBit[3];
	char scheduleName[100];
	char filepath[100];
	char content[2000];
	nodeptr pre;
	nodeptr next;
} detail_node;

//void save_file(Schedule,char *);

void smaller_than_ten(int target, char *targetstr);
//void print_detail_file(detail_nodeptr);
void read_file(char *filename);


void get_userID();
void input_file(Schedule);

void save_brief_file(Schedule, char[], int, int);
void save_detailed_file(Schedule, char[]);
void insert_content_into_file(Schedule);

Schedule write_content(Schedule);
Schedule write_title(Schedule);
Schedule write_admit(Schedule);
Schedule write_end_time(Schedule);
Schedule write_start_time(Schedule);
Schedule write_date(Schedule);

void screen_fix_info();


Schedule s;

/* If there is an empty box, print out a message to write it again. */
#define ISEMPTY 0
#define ISFULL 1
typedef struct is_empty_box {
	int date;
	int start_time;
	int end_time;
	int admit;
	int title;
	int content;
}BOX;
BOX scedule_box_ox;

int main() {
	initscr();
	clear();
	screen_fix_info();
	char write_or_correction;
	move(1, 1);
	char c;
	c = getchar();
	if (c == 'w') {
		s = write_date(s);
		insert_content_into_file(s);
	}
	else if (c == 'r') {
		char filename[100] = "../Data/ScheduleData/2019/currentuserID_20191212.txt";
		read_file(filename);
	}
	while (getch() != ESC);
	endwin();
	return 0;
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
//=============================================================================================================
void read_file(char *filepath) {

	FILE *file = fopen(filepath, "r");
	if (!file) {
		printf("read file error\n");
		getch();
		exit(1);
	}
	detail_nodeptr detail_file = NULL;
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
	while(1){
		if(feof(file)) break;
		fscanf(file, "%s", word);
		sprintf(content,"%s %s",content,word);
		
	}

	fclose(file);

	clear();
	move(1, 0);
	addstr(" *************************************************\n");
	addstr("   date       : "); addstr(date_c); addstr("\n");
	addstr("   start time : "); addstr(start_time_c); addstr("\n");
	addstr("   end time   : "); addstr(end_time_c); addstr("\n");
	addstr("   permission : "); addstr(permissionBit); addstr("\n");
	addstr("   title      : "); addstr(scheduleName); addstr("\n");
	addstr("   detail     : "); addstr(content); addstr("\n");
	addstr(" *************************************************\n");
	refresh();
}
//=============================================================================================================


void save_file(Schedule s, char output_file_name[50])
{
	FILE* file;
	file = fopen(output_file_name, "w");
	if (!file) {
		perror("file error\n");
		exit(1);
	}

	fprintf(file, ":%s", s.date);
	fprintf(file, ":%s", s.start_time);
	fprintf(file, ":%s", s.end_time);
	fprintf(file, ":%s", s.admit);
	fprintf(file, ":%s", s.title);
	fprintf(file, ":%s", s.content);

	fclose(file);
}

void save_brief_file(Schedule s, char userid[], int year, int mon) {
	char brief_file_name[100];
	sprintf(brief_file_name, "%d%d_Schedule.txt", year, mon);

}

void save_detailed_file(Schedule s, char file_name[]) {
	FILE* file;
	file = fopen(file_name, "w");
	if (!file) {
		perror("file error\n");
		exit(1);
	}

	fprintf(file, "%s/", s.date);
	fprintf(file, "%s/", s.start_time);
	fprintf(file, "%s/", s.end_time);
	fprintf(file, "%s/", s.admit);
	fprintf(file, "%s/", s.title);
	fprintf(file, "%s", s.content);

	fclose(file);
	printf("FILE2\n");
}

void insert_content_into_file(Schedule s) {

	/* Get user ID */
	char id[40];
	//id = get_userID();
	strcpy(id, "currentuserID");

	/* Get current time */
	struct tm *t;
	time_t timer;
	int year, mon, day;
	timer = time(NULL);
	t = localtime(&timer);
	year = t->tm_year + 1900;
	mon = t->tm_mon + 1;
	day = t->tm_mday;

	/* Move Directory Location*/
	char path_name[100];
	if (mon < 10) {
		sprintf(path_name, "../Data/ScheduleData/%d/0%d/", year, mon);
	}
	else {
		sprintf(path_name, "../Data/ScheduleData/%d/%d/", year, mon);
	}
	chdir(path_name);	/*The location to store the detailed file.*/


	char detailed_file_name[100];
	sprintf(detailed_file_name, "%s_%d%d%d.txt", id, year, mon, day);
	save_detailed_file(s, detailed_file_name);


	save_brief_file(s, id, year, mon);
	//	save_file(&s,input_file_name);



}

Schedule write_content(Schedule s)
{
	move(15, 25);
	refresh();

	char content[2000];
	int content_i = 0;

	//fgets(content,sizeof(content),stdin);
	//scanw("%s",content);

	int line = 15;
	char c;
	int i = 0;

	while ((c = getchar()) != ESC) {
		if (c == '\r' || i >= 50) {
			line += 1;
			move(line, 25);
			refresh();
			i = 0;
		}
		else {
			putchar(c);
			content[content_i] = c;
			content_i++;
			i++;
		}
	}

	scedule_box_ox.content = ISFULL;
	strcpy(s.content, content);
	//getch();
	refresh();
	printf("DATA1\n");
	return s;

}

Schedule write_title(Schedule s)
{
	move(13, 25);
	char title[40];
	scanw("%s", title);
	refresh();
	scedule_box_ox.title = ISFULL;
	strcpy(s.title, title);
	s = write_content(s);
	printf("DATA2\n");
	return s;


}

Schedule write_admit(Schedule s)
{
	move(11, 25);
	char admit[5];
	scanw("%s", admit);
	refresh();
	scedule_box_ox.admit = ISFULL;
	strcpy(s.admit, admit);
	s = write_title(s);
	printf("DATA3\n");
	return s;

}

Schedule write_end_time(Schedule s)
{
	move(9, 25);
	char end_time[5];
	scanw("%s", end_time);
	refresh();
	scedule_box_ox.end_time = ISFULL;
	strcpy(s.end_time, end_time);
	s = write_admit(s);
	printf("DATA4\n");
	return s;

}

Schedule write_start_time(Schedule s)
{
	move(7, 25);
	char start_time[5];
	scanw("%s", start_time);
	refresh();
	scedule_box_ox.start_time = ISFULL;
	strcpy(s.start_time, start_time);
	s = write_end_time(s);
	printf("DATA5\n");
	return s;

}

Schedule write_date(Schedule s)
{
	move(5, 25);
	char date[10];
	scanw("%s", date);
	refresh();
	scedule_box_ox.date = ISFULL;
	strcpy(s.date, date);
	s = write_start_time(s);
	printf("DATA6\n");
	return s;

}

void screen_fix_info()
{
	move(5, 10);
	addstr("Date");
	move(5, 20);
	addstr(":");

	move(7, 10);
	addstr("Start time");
	move(7, 20);
	addstr(":");

	move(9, 10);
	addstr("End time");
	move(9, 20);
	addstr(":");

	move(11, 10);
	addstr("Admit");
	move(11, 20);
	addstr(":");

	move(13, 10);
	addstr("Title");
	move(13, 20);
	addstr(":");

	move(15, 10);
	addstr("write");
	move(15, 20);
	addstr(":");

	refresh();

}
