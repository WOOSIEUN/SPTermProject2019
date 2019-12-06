
//----------DEFINITIONS
#define USER_MAX 100
#define MCODE 486   //master code 
#define QUIT_SIGN "quit"
#define UP 119      //[w] = up
#define DOWN 115   //[s] = down
#define ENTER 13   //[enter] = enter

#define TIMEOFFSET 9
#define List_Xp 10
#define List_Yp_1 16
#define List_Yp_2 22
#define List_Yp_3 28

#define ESC 0x1b
#define DEL 0x7f
#define ISEMPTY 0
#define ISFULL 1

#define HOSTNAME "ip-172-31-47-228"	
#define PENDING_NUM 5			
#define READ_END 0
#define WRITE_END 1

//----------STRUCTURES & GLOBAL VARIABLES
//user data structure
typedef struct
{
	char ID[20];
	char PW[20];
	char Name[20];
	int isMaster;
	int portNum;
}user;

//1. startmenu variables
int login_return = 0;
int signin_return = 0;
int start_choice = 0;
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

BOX schedule_list_check_o_or_x;

/*	socket variables	*/
int thepipe[2];
int  parent_pid;
char callout[20];
int printcounter= 0;
int portNumIndex;	//used in sign in to assign portnum to newUser

//-----------FUNCTIONS
// INITIALIZE
void set_SIGIO();
void get_userData();

// START SCREEN
void startScreen_MAIN();

void start_Handler(int sn);
int log_In();
int find_LoginUser(char*);
int sign_In();
int signIn_Check(int, int, int, char*, char*);
int dup_Check(char*, char*);

//	MAIN SCREEN
void mainScreen_MAIN();

void init_mainScreen(int* year, int* year_and_month, int* date, int* today);
void search_schedule(int* year, int* year_and_month, int* date, int target);
int print_Brief_list(int year, int year_and_month);
nodeptr move_Brief_list(int how, int index);
void clear_list_detail();
void printBrief3(int order, int group, int year, int year_and_month);
int return_today();
struct tm* return_Time(void);
void smaller_than_ten(int target, char* targetstr);
void print_menu();
struct node* make_newNode(int date, int start, int end, char* ID, char* permission, char* sname, char* filepath);
struct node* make_schedulelist(int mode, char* filename, int* date);

//	ADD NEW SCHEDULE
void addSchedule_MAIN();

Schedule get_newSinput();
void save_newSfile(Schedule schedule_info);
void add_file(node* insert);
void save_brief_file(char* tempfilename);

//	VIEW DETAIL
void viewDetail_MAIN(int index, int year, int year_and_month);

void read_file(char* filepath);
void del_file(char* filepath);

//	SOCKET
void callOut_MAIN(void);
int checkValidName(char* name);
void callout_handler(int signum);
int set_ticker(int n_msecs);
void calloutPrint_handler(int signum);
void server(void);
