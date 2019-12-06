#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <curses.h>
#include <sys/time.h>

#define TIMEOFFSET 9

int total_words;
void clear_time();
void *print_time(void *a);

pthread_mutex_t cursor_lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
	pthread_t t1;
    initscr();
    clear();

	pthread_create(&t1, NULL, print_time, NULL);
	pthread_join(t1, NULL);
    
    endwin();
	return 0;
}

void *print_time(void *a) {
	struct tm *t;
	time_t timer;
    
    while(1){
	    //change time to Korea's time
	    timer = time(NULL);
	    timer += TIMEOFFSET * 3600;
	    t = localtime(&timer);

	    //clear screen and print Today's date.
	    pthread_mutex_lock(&cursor_lock);
	    clear_time();
	    move(1, 1);
	    addstr("*********************************************************\n");
	    addstr("                          Today                          \n");
	    printw("                    %d.%d.%d %d:%d:%d                    \n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	    addstr("*********************************************************\n");
	    pthread_mutex_unlock(&cursor_lock);
	    refresh();
        sleep(1);
    }
	return NULL;
}

void clear_time() {
	move(1, 1);
	addstr("                                                         \n");
	addstr("                                                         \n");
	printw("                                                         \n");
	addstr("                                                         \n");

}
