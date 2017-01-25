#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

#define SERVER_PORT 15656
#define MAX_LINE	256	

void main(int argc, char **argv){
	if(argc < 2){
		printf("\nUseage: client serverName\n");
		return;
	}

	//Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(iResult != NO_ERROR){
		printf("Error at WSAStartup()\n");
		getchar();
		return;
	}

	//translate the server name or IP address to resolved IP address
	unsigned int ipaddr;
	//if the user input is an alpha name for the host, use gethostbyname()
	//if not, get host by addr(assume IPV4)
	if(isalpha(argv[1][0])){//host address is a name
		hostent* remoteHost = gethostbyname(argv[1]);
		if(remoteHost == NULL){
			printf("Host not found.\n");
			WSACleanup();
			return;
		}
		ipaddr = *((unsigned long *)remoteHost -> h_addr);
	}
	else 
		ipaddr = inet_addr(argv[1]);

	//Create a socket
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s == INVALID_SOCKET){
		printf("Error at socket().\n");
		WSACleanup();
		return;
	}

	//Connect to a server.
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(SERVER_PORT);
	if(connect(s,(SOCKADDR*)&addr,sizeof(addr)) == SOCKET_ERROR){
		printf("Failed to connect.\n");
		WSACleanup();
		return;
	}

	//begine send and recieved
	char buf[MAX_LINE];
	printf("My chat room client. Version one.\n");
	while(1){
		gets(buf);
		send(s, buf, strlen(buf), 0);
		int len = recv(s, buf, MAX_LINE, 0);
		buf[len] = 0;
		printf("Server: %s\n", buf);
	}
	closesocket(s);
}
