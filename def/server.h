
struct train
{
	int trainNumber;
	char trainName[50];
	int seats;
	int seatsAvailable;
};
struct user
{
	int loginId;
	char password[50];
	char name[50];
	int type;
};

struct booking
{
	int bookingId;
	int type;
	int uid;
	int tid;
	int seats;
};

void handler(int);
void login(int);
void signup(int);
int menu(int, int, int);
void trainCRUD(int);
void userCRUD(int);
int user(int, int, int, int);
