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

//void save_file(Schedule,char *);
void get_userID();
void input_file(Schedule);
void write_content(Schedule);
void write_title(Schedule);
void write_admit(Schedule);
void write_end_time(Schedule);
void write_start_time(Schedule);
void write_date(Schedule);
void screen_fix_info();

typedef struct Schedule{
	char date[10];
	char start_time[5];
	char end_time[5];
	char admit[3];
	char title[40];
	char content[2000];
}Schedule;
Schedule s;

/* If there is an empty box, print out a message to write it again. */
#define ISEMPTY 0
#define ISFULL 1
typedef struct is_empty_box{
	int date;
	int start_time;
	int end_time;
	int admit;
	int title;
	int content;
}BOX;
BOX scedule_box_ox;

int main(){
	initscr();
	clear();
	screen_fix_info();
	
	write_date(s);
	input_file(&s);
//	getch();
	while(getch()!=ESC);

	endwin();
	return 0;
}

void save_file(Schedule *s, char output_file_name[50])
{
	FILE* file;
	file = fopen(output_file_name,"w");
	if(!file){
		perror("file error\n");
		exit(1);
	}

	fprintf(file,":%s",(*s).date);
	fprintf(file,":%s",(*s).start_time);
	fprintf(file,":%s",(*s).end_time);
	fprintf(file,":%s",(*s).admit);
	fprintf(file,":%s",(*s).title);
	fprintf(file,":%s",(*s).content);

	fclose(file);
}

void input_file(Schedule *s)
{
	char id[40];
	//id = get_userID();
	strcpy(id,"currenTuserid");

	struct tm *t;
	time_t timer;
	int year, mon,day;

	timer = time(NULL);
	t = localtime(&timer);
	year=t->tm_year+1900;
	mon=t->tm_mon+1;
	day =t->tm_mday;



	char input_file_name[100];
	sprintf(input_file_name,"%s_%d%d%d.txt",id,year,mon,day);
	

char name[100];
	printf(" : %s\n",getcwd(name,100));

		// Move Directory Location
	//chdir("SPTermProject2019/Data/ScheduleData/Data/ScheduleData");
	chdir("Data/ScheduleData");
		
	printf(" : %s\n",getcwd(name,100));

	FILE* f;
	f = fopen("hello.txt","w");
	if(!f){
		perror("f error\n");
		exit(1);
	}

	fprintf(f,":  hello bro");
	fclose(f);
	printf("FILE\n");
	
//	save_file(&s,input_file_name);

	FILE* file;
	file = fopen(input_file_name,"w");
	if(!file){
		perror("file error\n");
		exit(1);
	}

	fprintf(file,":%s",s->date);
	fprintf(file,":%s",s->start_time);
	fprintf(file,":%s",s->end_time);
	fprintf(file,":%s",s->admit);
	fprintf(file,":%s",s->title);
	fprintf(file,":%s",s->content);

	fclose(file);
	printf("FILE2\n");

}

void write_content(Schedule s)
{
	move(15,25);
	char content[2000];
	scanw("%s",content);
	scedule_box_ox.content=ISFULL;
	strcpy(s.content, content);
	//getch();
	refresh();
	printf("DATA1\n");
}

void write_title(Schedule s)
{
	move(13,25);
	char title[40];
	scanw("%s",title);
	refresh();
	scedule_box_ox.title=ISFULL;
	strcpy(s.title, title);
	write_content(s);
	printf("DATA2\n");

}

void write_admit(Schedule s)
{
	move(11,25);
	char admit[5];
	scanw("%s",admit);
	refresh();
	scedule_box_ox.admit=ISFULL;
	strcpy(s.admit, admit);
	write_title(s);
	printf("DATA3\n");
}

void write_end_time(Schedule s)
{
	move(9,25);
	char end_time[5];
	scanw("%s",end_time);
	refresh();
	scedule_box_ox.end_time=ISFULL;
	strcpy(s.end_time,end_time);
	write_admit(s);
	printf("DATA4\n");
}

void write_start_time(Schedule s)
{
	move(7,25);
	char start_time[5];
	scanw("%s",start_time);
	refresh();
	scedule_box_ox.start_time=ISFULL;
	strcpy(s.start_time,start_time);
	write_end_time(s);
	printf("DATA5\n");
}

void write_date(Schedule s)
{
	move(5,25);
	char date[10];
	scanw("%s",date);
	refresh();
	scedule_box_ox.date=ISFULL;
	strcpy(s.date,date);
printw("%s",s.date);
	write_start_time(s);
	printf("DATA6\n");
	
}

/*
void write_date()
{
	move(5,25);
	char data[10];
	scanw("%s",data);
	refresh();
	
	while(1){
		if(getch()=='\0'){
			write_start_time();
		}
		else if(getch()==KEY_F(1)){	 Press F1 and write again. 
			data[0]='\0';
			move(5,25);
			addstr("                ");	 Clear Content 
			refresh();
			scanw("%s",data);
			refresh();
		}
		else if(getch()==KEY_F(2)){
			
		}
	}
}
*/

void screen_fix_info()
{
	move(5,10);
	addstr("Date");
	move(5,20);
	addstr(":");

	move(7,10);
	addstr("Start time");
	move(7,20);
	addstr(":");

	move(9,10);
	addstr("End time");
	move(9,20);
	addstr(":");

	move(11,10);
	addstr("Admit");
	move(11,20);
	addstr(":");

	move(13,10);
	addstr("Title");
	move(13,20);
	addstr(":");

	move(15,10);
	addstr("write");
	move(15,20);
	addstr(":");

	refresh();

}
