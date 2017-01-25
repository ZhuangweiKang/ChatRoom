#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT	15656
#define	MAX_PENDING	5
#define	MAX_LINE	256

void main(){
	//Initialize Winsock
	WSADATA	wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != NO_ERROR){
		printf("Error at WSAStartup()\n");
		return;
	}

	//Create a Socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listenSocket == INVALID_SOCKET){
		printf("Error at socket()\n");
		WSACleanup();
		return;
	}

	//Bind the Socket
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;//use local address
	addr.sin_port = htons(SERVER_PORT);
	if(bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR){
		printf("bind() failed.\n");
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	//Listen on the Socket
	if(listen(listenSocket, MAX_PENDING) == SOCKET_ERROR){
		printf("Error listening on socket.\n");
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	//Accept connections.
	SOCKET s;
	printf("My chat room server. Version one.\n");
	int logined = 0;//check whether the client has logined, 0 represents hasn't logined
	bool wait = false;
		s = accept(listenSocket, NULL, NULL);
		if(s == SOCKET_ERROR){
			printf("accept() error.\n");
			closesocket(listenSocket);
			WSACleanup();
			return;
		}

		//send and recieve data
		char buf[MAX_LINE];//buffer to save data
		FILE *user;
		char userID[10];
		while(1){
				char *arg[MAX_LINE];//store the arguments 
				memset(arg,'\0',sizeof(arg));
				char sendContent[MAX_LINE] = "";
				int len = recv(s, buf, MAX_LINE, 0);
				buf[len] = 0;
				bool username = false;
				if(len < 0 || len >= MAX_LINE){
					printf("Invalid comand.\n");
					break;
				}
				const char *d = " ";
				char *p;
				int i = 0;
				p = strtok(buf,d);
				while(p){
					arg[i] = p;
					p = strtok(NULL,d);
					i++;
				}
				if(strcmp(arg[0],"send") == 0){
					int i = 1;
					while(arg[i]!= '\0'){
						if(i == 1){
							strcat(sendContent,arg[i]);
						}
						else{
							strcat(sendContent," ");
							strcat(sendContent,arg[i]);
						}
						i++;
					}
				}
				if(strcmp(arg[0],"login") != 0 && strcmp(arg[0],"newuser") != 0 && strcmp(arg[0],"logout") != 0 && strcmp(arg[0],"send") != 0 ){
					send(s,"Invalid command!",16,0);
				}
				else{
					if(logined == 0) {//the user has not logined
					if(strcmp(arg[0],"login") != 0 && strcmp(arg[0],"newuser") !=0){//if the client has not logined and the operation isn't login
						send(s, "Denied, Please login first.", 28, 0);
					
					}
					else if(strcmp(arg[0],"newuser") ==0){
						if((user = fopen("user.txt","r")) == NULL){//open user info file
								printf("File open failed.\n");
							}
						else{
							if(arg[1] != '\0' && arg[2] != '\0'){
							char info[32];
							char *temp[3];
							fseek(user,0,SEEK_SET);
							while(fgets(info,sizeof(info),user)){
								const char *a = "(, )";
								char *q;
								int i = 0;
								q = strtok(info,a);
								while(q){
									temp[i] = q;
									q = strtok(NULL,a);
									i++;
								}
								if(strcmp(temp[0], arg[1]) == 0){
									send(s, "This userID has existed.", strlen("This userID has existed."),0);
									fclose(user);
									username = true;
									break;
								}
							}
							fclose(user);
							}
							else{
								send(s,"Invalid userID or password.",strlen("Invalid userID or password."),0);
								continue;
							}
						}
						if(!username){
							char temp[32];
							strcpy(temp,arg[1]);
							if(strlen(arg[1])>32){//userID is too long
								sprintf(buf,"Invalid userID.");
								send(s, buf, strlen(buf), 0);
							}
							else if(strlen(arg[2]) < 4 || strlen(arg[2]) > 8){//password is invalid
								sprintf(buf,"Invalid password.");
								send(s, buf, strlen(buf), 0);
								}
							else{
								sprintf(buf, "(%s, %s)\n",arg[1],arg[2]);
								if((user = fopen("user.txt","a+")) == NULL){//open user info file
									printf("File open failed.\n");
								}
								else{
									fputs(buf, user);
									fseek(user,0, SEEK_SET);
									send(s, strcat(temp, " registered."),strlen(strcat(temp, " registered.")), 0);
									fclose(user);
								}
							}
						}
					}
					else{//the operation is login
						user = fopen("user.txt","r");//open user info file
						if(user == NULL){
							printf("File open error.\n");
						}
						else{
							if(arg[1] != '\0' && arg[2] != '\0'){
							char info[32];
							char *temp[3];
							fseek(user,0,SEEK_SET);
							while(fgets(info,sizeof(info),user)){
								const char *a = "(, )";
								char *q;
								int i = 0;
								q = strtok(info,a);
								while(q){
									temp[i] = q;
									q = strtok(NULL,a);
									i++;
								}
								if(strcmp(temp[0], arg[1]) == 0){//get a right userID
									strcpy(userID,temp[0]);
									if(strcmp(temp[1],arg[2]) == 0){//password is correct
										logined = 1;
										printf("%s login.\n",arg[1]);
										send(s, strcat(arg[1], " joins."),strlen(strcat(arg[1], " joins.")),0);
										fclose(user);
									}
									else{//password is incorrect
										send(s, "Password is incorrect!", 23, 0);
										fclose(user);
									}
									break;
								}
							}
							//can not find the username
							if(feof(user)){
								send(s,"Can not find the user.", 24, 0);
								fclose(user);
							}
							}
							else{
								send(s, "Invalid userID or password.", 30,0);
							}
						}
					}
				}
				else{//the user has logined
					if(strcmp(arg[0],"logout") == 0){//user wants to logout
						printf("%s logout.\n",userID);
						send(s, strcat(userID," left."), strlen(strcat(userID," left.")),0);
						logined = 0;
						closesocket(listenSocket);
					}
					else if(strcmp(arg[0], "send") == 0){//user send message to server
						printf("%s: %s.\n",userID, sendContent);
						sprintf(buf,"%s: %s.",userID, sendContent);
						send(s, buf, strlen(buf), 0);
					}
					else if(strcmp(arg[0], "newuser") == 0){//newuser when the current user has logined
						send(s,"To newuser, please logout your current account first.",strlen("To newuser, please logout your current account first."),0);
					}
					else if(strcmp(arg[0], "login") == 0){//user want to login again
						send(s, "Denied, you have logined.",strlen("Denied, you have logined."),0);
					}
				}
				}
		}
		closesocket(listenSocket);
}
