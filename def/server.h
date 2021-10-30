
struct train
{
	int train_number;
	char train_name[50];
	int total_seats;
	int available_seats;
};
struct user
{
	int login_id;
	char password[50];
	char name[50];
	int type;
};

struct booking
{
	int booking_id;
	int type;
	int uid;
	int tid;
	int seats;
};

void service_cli(int);
void login(int);
void signup(int);
int menu(int, int, int);
void crud_train(int);
void crud_user(int);
int user_function(int, int, int, int);
