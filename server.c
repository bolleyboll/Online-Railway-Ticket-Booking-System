#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "./def/server.h"
#include "./def/const.h"

int main(void)
{

	int sd, nsd, c;
	struct sockaddr_in server, client;
	char buf[100];

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("Socket couldn't be created.");
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
		perror("bind() failed.");

	listen(sd, 3);
	c = sizeof(struct sockaddr_in);

	while (1)
	{

		nsd = accept(sd, (struct sockaddr *)&client, (socklen_t *)&c);

		if (!fork())
		{
			close(sd);
			handler(nsd);
			exit(0);
		}
		else
			close(nsd);
	}
	return 0;
}

void handler(int sd)
{
	int choice;
	printf("Client connected on socket: %d\n", sd);
	do
	{
		read(sd, &choice, sizeof(int));
		
		if (choice == 1)
			login(sd);
		
		if (choice == 2)
			signup(sd);
		
		if (choice == 3)
			break;
	} while (1);

	close(sd);
	
	printf("Client disconnected on socket: %d\n", sd);
}

void login(int sd)
{
	int fdUsr = open("data/userDB.dat", O_RDWR);
	int id, type, valid = 0, user_valid = 0;
	char password[50];
	
	struct user u;
	
	read(sd, &id, sizeof(id));
	read(sd, &password, sizeof(password));

	struct flock lock;

	lock.l_start = (id - 1) * sizeof(struct user);
	lock.l_len = sizeof(struct user);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	lock.l_type = F_WRLCK;
	fcntl(fdUsr, F_SETLKW, &lock);

	while (read(fdUsr, &u, sizeof(u)))
	{
		if (u.loginId == id)
		{
			user_valid = 1;
			if (!strcmp(u.password, password))
			{
				valid = 1;
				type = u.type;
				break;
			}
			else
			{
				valid = 0;
				break;
			}
		}
		else
		{
			user_valid = 0;
			valid = 0;
		}
	}

	if (type != 2)	// Unlocking Agent Record for multiple logins

	{
		lock.l_type = F_UNLCK;
		fcntl(fdUsr, F_SETLK, &lock);
		close(fdUsr);
	}

	if (user_valid)
	{
		write(sd, &valid, sizeof(valid));
		if (valid)
		{
			write(sd, &type, sizeof(type));
			while (menu(sd, type, id) != -1)
				;
		}
	}
	else
		write(sd, &valid, sizeof(valid));

	if (type == 2)
	{
		lock.l_type = F_UNLCK;
		fcntl(fdUsr, F_SETLK, &lock);
		close(fdUsr);
	}
}

void signup(int sd)
{
	int fd_user = open("data/userDB.dat", O_RDWR);
	int type, id = 0;
	char name[50], password[50];
	
	struct user u, temp;

	read(sd, &type, sizeof(type));
	read(sd, &name, sizeof(name));
	read(sd, &password, sizeof(password));

	int fp = lseek(fd_user, 0, SEEK_END);

	struct flock lock;
	
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	fcntl(fd_user, F_SETLKW, &lock);

	if (fp == 0)
	{
		u.loginId = 1;
		strcpy(u.name, name);
		strcpy(u.password, password);
		
		u.type = type;
		write(fd_user, &u, sizeof(u));
		write(sd, &u.loginId, sizeof(u.loginId));
	}
	else
	{
		fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
		read(fd_user, &u, sizeof(u));
		
		u.loginId++;
		strcpy(u.name, name);
		strcpy(u.password, password);
		
		u.type = type;
		write(fd_user, &u, sizeof(u));
		write(sd, &u.loginId, sizeof(u.loginId));
	}
	lock.l_type = F_UNLCK;
	fcntl(fd_user, F_SETLK, &lock);

	close(fd_user);
}

int menu(int sd, int type, int id)
{
	int choice, ret;

	if (type == 0)	// Admin
	{
		read(sd, &choice, sizeof(choice));

		if (choice == 1)	// Train CRUD
		{
			trainCRUD(sd);
			
			return menu(sd, type, id);
		}
		else if (choice == 2)	// User CRUD
		{
			userCRUD(sd);
			
			return menu(sd, type, id);
		}
		else if (choice == 3)	// Logout
			return -1;
	}
	else if (type == 2 || type == 1)	// Agent & Customer
	{
		read(sd, &choice, sizeof(choice));
		
		ret = user(sd, choice, type, id);
		if (ret != 5)
			return menu(sd, type, id);
		else if (ret == 5)
			return -1;
	}
}

void trainCRUD(int sd)
{
	int valid = 0;
	int choice;
	
	read(sd, &choice, sizeof(choice));
	
	if (choice == 1)	// Add Train
	{
		char tname[50];
		int tid = 0;
		
		read(sd, &tname, sizeof(tname));
		
		struct train tdb, temp;
		struct flock lock;
		
		int fd_train = open("data/trainDB.dat", O_RDWR);

		tdb.trainNumber = tid;
		strcpy(tdb.trainName, tname);
		tdb.seats = 100;
		tdb.seatsAvailable = 100;

		int fp = lseek(fd_train, 0, SEEK_END);

		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);

		if (fp == 0)
		{
			valid = 1;
			
			write(fd_train, &tdb, sizeof(tdb));
			lock.l_type = F_UNLCK;
			fcntl(fd_train, F_SETLK, &lock);
			
			close(fd_train);
			
			write(sd, &valid, sizeof(valid));
		}
		else
		{
			valid = 1;
			
			lseek(fd_train, -1 * sizeof(struct train), SEEK_END);
			read(fd_train, &temp, sizeof(temp));
			tdb.trainNumber = temp.trainNumber + 1;
			write(fd_train, &tdb, sizeof(tdb));
			write(sd, &valid, sizeof(valid));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);
	}

	else if (choice == 2)	// View Trains
	{
		struct flock lock;
		struct train tdb;
		
		int fd_train = open("data/trainDB.dat", O_RDONLY);

		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);
		
		int fp = lseek(fd_train, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		
		write(sd, &no_of_trains, sizeof(int));

		lseek(fd_train, 0, SEEK_SET);
		while (fp != lseek(fd_train, 0, SEEK_CUR))
		{
			read(fd_train, &tdb, sizeof(tdb));
			write(sd, &tdb.trainNumber, sizeof(int));
			write(sd, &tdb.trainName, sizeof(tdb.trainName));
			write(sd, &tdb.seats, sizeof(int));
			write(sd, &tdb.seatsAvailable, sizeof(int));
		}
		valid = 1;
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		close(fd_train);
	}

	else if (choice == 3)	 // Update Train
	{
		trainCRUD(sd);
		
		int choice, valid = 0, tid;
		struct flock lock;
		struct train tdb;
		
		int fd_train = open("data/trainDB.dat", O_RDWR);

		read(sd, &tid, sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid) * sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);

		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid) * sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));

		read(sd, &choice, sizeof(int));
		if (choice == 1)	// Update Train Name
		{
			write(sd, &tdb.trainName, sizeof(tdb.trainName));
			read(sd, &tdb.trainName, sizeof(tdb.trainName));
		}
		else if (choice == 2)	// Update Seats
		{
			write(sd, &tdb.seats, sizeof(tdb.seats));
			read(sd, &tdb.seats, sizeof(tdb.seats));
			read(sd, &tdb.seatsAvailable, sizeof(tdb.seatsAvailable));
		}

		lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		
		valid = 1;
		
		write(sd, &valid, sizeof(valid));
		
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		
		close(fd_train);
	}

	else if (choice == 4)	// Delete Train
	{
		trainCRUD(sd);
		
		struct flock lock;
		struct train tdb;
		
		int fd_train = open("data/trainDB.dat", O_RDWR);
		
		int tid, valid = 0;

		read(sd, &tid, sizeof(tid));

		lock.l_type = F_WRLCK;
		lock.l_start = (tid) * sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lock);

		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (tid) * sizeof(struct train), SEEK_CUR);
		read(fd_train, &tdb, sizeof(struct train));
		strcpy(tdb.trainName, "deleted");
		lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
		write(fd_train, &tdb, sizeof(struct train));
		
		valid = 1;
		
		write(sd, &valid, sizeof(valid));
		lock.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lock);
		
		close(fd_train);
	}
}

void userCRUD(int sd)
{
	int valid = 0;
	int choice;
	
	read(sd, &choice, sizeof(choice));
	
	if (choice == 1)	// Add User
	{
		char name[50], password[50];
		int type;
		
		read(sd, &type, sizeof(type));
		read(sd, &name, sizeof(name));
		read(sd, &password, sizeof(password));

		struct user udb;
		struct flock lock;
		
		int fd_user = open("data/userDB.dat", O_RDWR);
		
		int fp = lseek(fd_user, 0, SEEK_END);

		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);

		if (fp == 0)
		{
			udb.loginId = 1;
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			
			udb.type = type;
			write(fd_user, &udb, sizeof(udb));
			
			valid = 1;
			
			write(sd, &valid, sizeof(int));
			write(sd, &udb.loginId, sizeof(udb.loginId));
		}
		else
		{
			fp = lseek(fd_user, -1 * sizeof(struct user), SEEK_END);
			
			read(fd_user, &udb, sizeof(udb));
			
			udb.loginId++;
			strcpy(udb.name, name);
			strcpy(udb.password, password);
			
			udb.type = type;
			
			write(fd_user, &udb, sizeof(udb));
			
			valid = 1;
			
			write(sd, &valid, sizeof(int));
			write(sd, &udb.loginId, sizeof(udb.loginId));
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}

	else if (choice == 2)	// View Users
	{
		struct flock lock;
		struct user udb;
		
		int fd_user = open("data/userDB.dat", O_RDONLY);

		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);
		
		int fp = lseek(fd_user, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);
		
		no_of_users--;
		
		write(sd, &no_of_users, sizeof(int));

		lseek(fd_user, 0, SEEK_SET);
		while (fp != lseek(fd_user, 0, SEEK_CUR))
		{
			read(fd_user, &udb, sizeof(udb));
			if (udb.type != 0)
			{
				write(sd, &udb.loginId, sizeof(int));
				write(sd, &udb.name, sizeof(udb.name));
				write(sd, &udb.type, sizeof(int));
			}
		}
		valid = 1;
		
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		close(fd_user);
	}

	else if (choice == 3)	// Update User
	{
		userCRUD(sd);
		
		int choice, valid = 0, uid;
		char pass[50];
		
		struct flock lock;
		struct user udb;
		
		int fd_user = open("data/userDB.dat", O_RDWR);

		read(sd, &uid, sizeof(uid));

		lock.l_type = F_WRLCK;
		lock.l_start = (uid - 1) * sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);

		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid - 1) * sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));

		read(sd, &choice, sizeof(int));

		if (choice == 1)	// Update Name
		{
			write(sd, &udb.name, sizeof(udb.name));
			read(sd, &udb.name, sizeof(udb.name));
			
			valid = 1;
			
			write(sd, &valid, sizeof(valid));
		}
		else if (choice == 2)	// Update Password
		{
			read(sd, &pass, sizeof(pass));
			
			if (!strcmp(udb.password, pass))
				valid = 1;
			
			write(sd, &valid, sizeof(valid));
			read(sd, &udb.password, sizeof(udb.password));
		}

		lseek(fd_user, -1 * sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		
		if (valid)
			write(sd, &valid, sizeof(valid));
		
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		
		close(fd_user);
	}

	else if (choice == 4)	// Delete User
	{
		userCRUD(sd);
		
		struct flock lock;
		struct user udb;
		
		int fd_user = open("data/userDB.dat", O_RDWR);
		
		int uid, valid = 0;

		read(sd, &uid, sizeof(uid));

		lock.l_type = F_WRLCK;
		lock.l_start = (uid - 1) * sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_user, F_SETLKW, &lock);

		lseek(fd_user, 0, SEEK_SET);
		lseek(fd_user, (uid - 1) * sizeof(struct user), SEEK_CUR);
		read(fd_user, &udb, sizeof(struct user));
		
		strcpy(udb.name, "deleted");
		strcpy(udb.password, "");
		
		lseek(fd_user, -1 * sizeof(struct user), SEEK_CUR);
		write(fd_user, &udb, sizeof(struct user));
		
		valid = 1;
		
		write(sd, &valid, sizeof(valid));
		
		lock.l_type = F_UNLCK;
		fcntl(fd_user, F_SETLK, &lock);
		
		close(fd_user);
	}
}

int user(int sd, int ch, int type, int id)
{
	int valid = 0;
	
	if (ch == 1)	// Book Ticket
	{
		trainCRUD(sd);
		
		struct flock lockt;
		struct flock lockb;
		struct train tdb;
		struct booking bdb;
		
		int fd_train = open("data/trainDB.dat", O_RDWR);
		int fd_book = open("data/bookingDB.dat", O_RDWR);
		
		int tid, seats;
		
		read(sd, &tid, sizeof(tid));

		lockt.l_type = F_WRLCK;
		lockt.l_start = tid * sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		lockb.l_type = F_WRLCK;
		lockb.l_start = 0;
		lockb.l_len = 0;
		lockb.l_whence = SEEK_END;
		lockb.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train, tid * sizeof(struct train), SEEK_SET);

		read(fd_train, &tdb, sizeof(tdb));
		read(sd, &seats, sizeof(seats));

		if (tdb.trainNumber == tid)
		{
			if (tdb.seatsAvailable >= seats)
			{
				valid = 1;
				
				tdb.seatsAvailable -= seats;
				fcntl(fd_book, F_SETLKW, &lockb);
				
				int fp = lseek(fd_book, 0, SEEK_END);

				if (fp > 0)
				{
					lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);
					read(fd_book, &bdb, sizeof(struct booking));
					
					bdb.bookingId++;
				}
				else
					bdb.bookingId = 0;

				bdb.type = type;
				bdb.uid = id;
				bdb.tid = tid;
				bdb.seats = seats;
				
				write(fd_book, &bdb, sizeof(struct booking));
				
				lockb.l_type = F_UNLCK;
				fcntl(fd_book, F_SETLK, &lockb);
				
				close(fd_book);
			}

			lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);
			write(fd_train, &tdb, sizeof(tdb));
		}

		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		
		close(fd_train);
		write(sd, &valid, sizeof(valid));
		
		return valid;
	}

	else if (ch == 2)	// View Bookings
	{
		struct flock lock;
		struct booking bdb;
		
		int fd_book = open("data/bookingDB.dat", O_RDONLY);
		
		int no_of_bookings = 0;

		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(fd_book, F_SETLKW, &lock);

		while (read(fd_book, &bdb, sizeof(bdb)))
		{
			if (bdb.uid == id)
				no_of_bookings++;
		}

		write(sd, &no_of_bookings, sizeof(int));
		lseek(fd_book, 0, SEEK_SET);

		while (read(fd_book, &bdb, sizeof(bdb)))
		{
			if (bdb.uid == id)
			{
				write(sd, &bdb.bookingId, sizeof(int));
				write(sd, &bdb.tid, sizeof(int));
				write(sd, &bdb.seats, sizeof(int));
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lock);
		
		close(fd_book);
		
		return valid;
	}

	else if (ch == 3)	// Update Booking
	{
		int choice = 2, bid, val;
		
		user(sd, choice, type, id);
		
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		
		int fd_book = open("data/bookingDB.dat", O_RDWR);
		int fd_train = open("data/trainDB.dat", O_RDWR);
		
		read(sd, &bid, sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid * sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();

		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book, bid * sizeof(struct booking), SEEK_SET);
		
		read(fd_book, &bdb, sizeof(bdb));
		lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);

		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid) * sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train, (bdb.tid) * sizeof(struct train), SEEK_SET);
		read(fd_train, &tdb, sizeof(tdb));
		lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);

		read(sd, &choice, sizeof(choice));

		if (choice == 1)	// Increase Seats
		{
			read(sd, &val, sizeof(val));
			
			if (tdb.seatsAvailable >= val)
			{
				valid = 1;
				tdb.seatsAvailable -= val;
				bdb.seats += val;
			}
		}
		else if (choice == 2)	// Decrease Seats
		{
			valid = 1;
			read(sd, &val, sizeof(val));
			tdb.seatsAvailable += val;
			bdb.seats -= val;
		}

		write(fd_train, &tdb, sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);

		write(fd_book, &bdb, sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);

		write(sd, &valid, sizeof(valid));
		return valid;
	}
	else if (ch == 4)	// Cancel Booking
	{
		int choice = 2, bid;
		
		user(sd, choice, type, id);
		
		struct booking bdb;
		struct train tdb;
		struct flock lockb;
		struct flock lockt;
		
		int fd_book = open("data/bookingDB.dat", O_RDWR);
		int fd_train = open("data/trainDB.dat", O_RDWR);
		
		read(sd, &bid, sizeof(bid));

		lockb.l_type = F_WRLCK;
		lockb.l_start = bid * sizeof(struct booking);
		lockb.l_len = sizeof(struct booking);
		lockb.l_whence = SEEK_SET;
		lockb.l_pid = getpid();

		fcntl(fd_book, F_SETLKW, &lockb);
		lseek(fd_book, bid * sizeof(struct booking), SEEK_SET);
		read(fd_book, &bdb, sizeof(bdb));
		lseek(fd_book, -1 * sizeof(struct booking), SEEK_CUR);

		lockt.l_type = F_WRLCK;
		lockt.l_start = (bdb.tid) * sizeof(struct train);
		lockt.l_len = sizeof(struct train);
		lockt.l_whence = SEEK_SET;
		lockt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &lockt);
		lseek(fd_train, (bdb.tid) * sizeof(struct train), SEEK_SET);
		read(fd_train, &tdb, sizeof(tdb));
		lseek(fd_train, -1 * sizeof(struct train), SEEK_CUR);

		tdb.seatsAvailable += bdb.seats;
		bdb.seats = 0;
		valid = 1;

		write(fd_train, &tdb, sizeof(tdb));
		lockt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &lockt);
		close(fd_train);

		write(fd_book, &bdb, sizeof(bdb));
		lockb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &lockb);
		close(fd_book);

		write(sd, &valid, sizeof(valid));
		return valid;
	}
	else if (ch == 5)	// Logout
		return 5;
}
