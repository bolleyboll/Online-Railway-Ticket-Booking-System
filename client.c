#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "./def/client.h"
#include "./def/const.h"

int main(void)
{
	int sd;
	struct sockaddr_in server;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("Unable to create socket");
	}

	server.sin_addr.s_addr = inet_addr(SERVER_IP);
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);

	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
		perror("connect() failed.");

	while (client(sd) != 3)
		;
	close(sd);

	return 0;
}

int client(int sd)
{
	int ch, valid;
	
	sleep(2);
	system("clear");
	printf("\n\n\t\t\tIRCTC - Train Booking System\n\n");
	printf("1. Sign-in to an existing account\n");
	printf("2. Create a new account\n");
	printf("3. Exit\n\n");
	
	printf("Please enter your choice: ");
	scanf("%d", &ch);
	
	write(sd, &ch, sizeof(ch));
	
	if (ch == 1)	// Sign-in to an existing account
	{
		int id, type;
		char password[50];
		
		printf("Please enter your Login ID: ");
		scanf("%d", &id);
		strcpy(password, getpass("Password: "));
		
		write(sd, &id, sizeof(id));
		write(sd, &password, sizeof(password));
		read(sd, &valid, sizeof(valid));
		
		if (valid)
		{
			printf("Login Successful\n");
			read(sd, &type, sizeof(type));
			
			while (menu(sd, type) != -1)
				;
			
			return 1;
		}
		else
		{
			printf("Login Failed: Incorrect ID or password.\n");
			return 1;
		}
		sleep(5);
	}

	else if (ch == 2)	// Create a new account
	{
		int type, id;
		char name[50], password[50], secret_pin[6];
				
		printf("Enter the type of your account: \n");
		printf("\t0. Admin Account\n\t1. Agent Account\n\t2. Customer Account\n\n");
		
		printf("Please enter your choice: ");
		scanf("%d", &type);
		
		printf("\nEnter your Name: ");
		scanf("%s", name);
		
		strcpy(password, getpass("\nEnter your password: "));

		if (!type)
		{
			int ctr = 0;
			while (ctr < 3)
			{
				printf("You will need to enter the secret Key in order to create an ADMIN account!\n");
				strcpy(secret_pin, getpass("Enter secret key: "));
				if (strcmp(secret_pin, ADMIN_KEY) != 0)
					printf("Invalid PIN.");
				else
					break;
				ctr++;
			}
		}

		write(sd, &type, sizeof(type));
		write(sd, &name, sizeof(name));
		write(sd, &password, strlen(password));

		read(sd, &id, sizeof(id));
		printf("Your Login ID: %d\n", id);
		printf("Please remember your Login ID!\n");
		return 2;
	}
	else // Exit
		return 3;
}

int menu(int sd, int type)
{
	int choice;
	if (type == 2 || type == 1)
	{
		printf("1. Book a ticket\n");
		printf("2. View your existing bookings\n");
		printf("3. Update an existing booking\n");
		printf("4. Cancel your booking\n");
		printf("5. Logout from your account\n\n");

		printf("Please enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));
		return user_function(sd, choice);
	}
	else if (type == 0)
	{
		printf("1. Train Operations\n");
		printf("2. User Operations\n");
		printf("3. Logout\n\n");

		printf("Please enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));
		
		if (choice == 1)
		{
			printf("1. Add train\n");
			printf("2. View train\n");
			printf("3. Modify train\n");
			printf("4. Delete train\n\n");

			printf("Please enter your choice: ");
			scanf("%d", &choice);
			
			write(sd, &choice, sizeof(choice));
			
			return crud_train(sd, choice);
		}
		else if (choice == 2)
		{
			printf("1. Add User\n");
			printf("2. View all users\n");
			printf("3. Modify user\n");
			printf("4. Delete user\n\n");

			printf("Please enter your choice: ");
			scanf("%d", &choice);
			
			write(sd, &choice, sizeof(choice));
			
			return crud_user(sd, choice);
		}
		else if (choice == 3)
			return -1;
	}
}

int crud_train(int sd, int ch)
{
	int valid = 0;

	if (ch == 1)	// New Train
	{
		char tname[50];

		printf("Enter the name of the new train: ");
		scanf("%s", tname);
		
		write(sd, &tname, sizeof(tname));
		
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Train has been added successfully!\n");

		return valid;
	}

	else if (ch == 2)	// Show Trains
	{
		int no_of_trains;
		int tno;
		char tname[50];
		int tseats;
		int aseats;
		
		read(sd, &no_of_trains, sizeof(no_of_trains));

		printf("Train No.\tTrain Name\tTotal Seats\tAvailable Seats\n");
		
		while (no_of_trains--)
		{
			read(sd, &tno, sizeof(tno));
			read(sd, &tname, sizeof(tname));
			read(sd, &tseats, sizeof(tseats));
			read(sd, &aseats, sizeof(aseats));

			if (strcmp(tname, "deleted") != 0)
				printf("\t%d\t%s  \t%d\t\t  %d\n", tno, tname, tseats, aseats);
		}

		return valid;
	}

	else if (ch == 3)	// Update Train
	{
		int tseats, choice = 2, valid = 0, tid;
		char tname[50];
		
		write(sd, &choice, sizeof(int));
		crud_train(sd, choice);
		
		printf("Enter the train number you want to modify: ");
		scanf("%d", &tid);
		write(sd, &tid, sizeof(tid));

		printf("\n\t1. Train Name\n\t2. Total Seats\n");
		printf("\tPlease enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));

		if (choice == 1)
		{
			read(sd, &tname, sizeof(tname));
			
			printf("Current name: %s", tname);
			printf("New name: ");
			scanf("%s", tname);
			
			write(sd, &tname, sizeof(tname));
		}
		else if (choice == 2)
		{
			read(sd, &tseats, sizeof(tseats));
			
			printf("Current value: %d", tseats);
			printf("\nNew value: ");
			scanf("%d", &tseats);
			
			write(sd, &tseats, sizeof(tseats));
			write(sd, &tseats, sizeof(tseats));
		}
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Train data has been updated successfully!\n");
		return valid;
	}

	else if (ch == 4)	// Delete Train
	{
		int choice = 2, tid, valid = 0;
		write(sd, &choice, sizeof(int));
		crud_train(sd, choice);

		printf("Enter the train number you want to delete: ");
		scanf("%d", &tid);
		
		write(sd, &tid, sizeof(tid));
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Train deleted successfully\n");
		
		return valid;
	}
}

int crud_user(int sd, int ch)
{
	int valid = 0;
	
	if (ch == 1)	// Add User
	{
		int type, id;
		char name[50], password[50];
		
		printf("Enter The Type Of Account: \n");
		
		printf("\t1. Agent\n\t2. Customer\n\n");
		
		printf("\tPlease enter your choice: ");
		scanf("%d", &type);
		
		printf("Please enter the Name: ");
		scanf("%s", name);
		
		strcpy(password, getpass("Please enter the Password: "));
		
		write(sd, &type, sizeof(type));
		write(sd, &name, sizeof(name));
		write(sd, &password, strlen(password));
		
		read(sd, &valid, sizeof(valid));
		
		if (valid)
		{
			read(sd, &id, sizeof(id));
			printf("Your login ID: %d\n", id);
			printf("Please remember your Login ID!\n");

		}
		
		return valid;
	}

	else if (ch == 2)	// View Users
	{
		int no_of_users;
		int id, type;
		char uname[50];
		
		read(sd, &no_of_users, sizeof(no_of_users));

		printf("User ID\tUser's Name\tUser's Type\n");
		while (no_of_users--)
		{
			read(sd, &id, sizeof(id));
			read(sd, &uname, sizeof(uname));
			read(sd, &type, sizeof(type));

			if (strcmp(uname, "deleted") != 0)
				printf("%d\t%s\t\t\t%d\n", id, uname, type);
		}

		return valid;
	}

	else if (ch == 3)	// Update User
	{
		int choice = 2, valid = 0, uid;
		char name[50], pass[50];
		
		write(sd, &choice, sizeof(int));
		
		crud_user(sd, choice);
		
		printf("Enter the User ID you want to modify: ");
		scanf("%d", &uid);
		
		write(sd, &uid, sizeof(uid));

		printf("\n\t1. User's Name\n\t2. Password\n\n");
		printf("\tPlease enter your choice: ");
		
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));

		if (choice == 1)
		{
			read(sd, &name, sizeof(name));
			
			printf("\nCurrent name: %s\n", name);
			printf("Enter new name:");
			scanf("%s", name);
			
			write(sd, &name, sizeof(name));
			
			read(sd, &valid, sizeof(valid));
		}
		else if (choice == 2)
		{
			printf("\nEnter Current password: ");
			scanf("%s", pass);
			
			write(sd, &pass, sizeof(pass));
			read(sd, &valid, sizeof(valid));
			
			if (valid)
			{
				printf("\nEnter new password:");
				scanf("%s", pass);
			}
			else
				printf("\nIncorrect password\n");

			write(sd, &pass, sizeof(pass));
		}
		
		if (valid)
		{
			read(sd, &valid, sizeof(valid));
			if (valid)
				printf("\nUser data updated successfully\n");
		}
		return valid;
	}

	else if (ch == 4)	// Delete User
	{
		int choice = 2, uid, valid = 0;
		
		write(sd, &choice, sizeof(int));
		crud_user(sd, choice);

		printf("Enter the User ID you want to delete: ");
		scanf("%d", &uid);
		
		write(sd, &uid, sizeof(uid));
		
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("\nUser deleted successfully\n");
		
		return valid;
	}
}

int user_function(int sd, int ch)
{
	int valid = 0;
	
	if (ch == 1)	// Book Tickets
	{
		int view = 2, tid, seats;
		
		write(sd, &view, sizeof(int));
		
		crud_train(sd, view);
		
		printf("\nEnter the Train No. you want to book: ");
		scanf("%d", &tid);
		
		write(sd, &tid, sizeof(tid));

		printf("\nEnter the no. of seats you want to book: ");
		scanf("%d", &seats);
		
		write(sd, &seats, sizeof(seats));

		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Ticket booked successfully.\n");
		else
			printf("Seats were not available.\n");

		return valid;
	}

	else if (ch == 2)	// View Bookings
	{
		int no_of_bookings;
		int id, tid, seats;
		
		read(sd, &no_of_bookings, sizeof(no_of_bookings));

		printf("Booking ID\tTrain No.\tSeats\n");
		while (no_of_bookings--)
		{
			read(sd, &id, sizeof(id));
			read(sd, &tid, sizeof(tid));
			read(sd, &seats, sizeof(seats));

			if (seats != 0)
				printf("\t%d\t\t%d\t\t%d\n", id, tid, seats);
		}

		return valid;
	}

	else if (ch == 3)	// Update Booking
	{
		int choice = 2, bid, val, valid;
		
		user_function(sd, choice);
		
		printf("\nEnter the Booking ID you want to modify: ");
		scanf("%d", &bid);
		
		write(sd, &bid, sizeof(bid));

		printf("\n\t1. Increase number of seats\n\t2. Decrease number of seats\n");
		
		printf("\tPlease enter your choice: ");
		scanf("%d", &choice);
		
		write(sd, &choice, sizeof(choice));

		if (choice == 1)
		{
			printf("\n\tNew No. of tickets: ");
			scanf("%d", &val);
			
			write(sd, &val, sizeof(val));
		}
		else if (choice == 2)
		{
			printf("\n\tNew No. of tickets: ");
			scanf("%d", &val);
			
			write(sd, &val, sizeof(val));
		}
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Booking updated successfully.\n");
		else
			printf("Updation failed. No more seats available.\n");
		
		return valid;
	}

	else if (ch == 4)	// Cancel Booking
	{
		int choice = 2, bid, valid;
		
		user_function(sd, choice);
		
		printf("\n\t Enter the Booking ID you want to cancel: ");
		scanf("%d", &bid);
		
		write(sd, &bid, sizeof(bid));
		
		read(sd, &valid, sizeof(valid));
		
		if (valid)
			printf("Booking cancelled successfully.\n");
		else
			printf("Cancellation failed.\n");
		
		return valid;
	}
	else if (ch == 5)	// Logout
		return -1;
}
