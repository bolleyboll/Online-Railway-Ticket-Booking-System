
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

void service_cli(int sock);
void login(int client_sock);
void signup(int client_sock);
int menu(int client_sock, int type, int id);
void crud_train(int client_sock);
void crud_user(int client_sock);
int user_function(int client_sock, int choice, int type, int id);
