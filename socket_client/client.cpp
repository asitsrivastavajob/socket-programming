#include<iostream>
#include<winsock.h>
#include<vector>
using namespace std;
#define PORT 9090

fd_set fd_read, fd_write, fd_exception;
sockaddr_in socAddr;
int nMaxFd = 0;
int nSoc = 0;
vector<int> client_fd_array;
void ProcessTheNewRequest()
{
	if (FD_ISSET(nSoc, &fd_read))
	{
		int nLen = sizeof(struct sockaddr);
		int clientSocket = accept(nSoc, NULL, &nLen);
		if (clientSocket > 0)
		{
			client_fd_array.push_back(clientSocket);
			send(clientSocket, "received connection request", sizeof("received connection request"), 0);
		}
	}
}

int main()
{
	//initialize
	int ret = 0;
	WSADATA ws;
	ret = WSAStartup(MAKEWORD(2,2), &ws);
	if(ret < 0)
		cout << "WSAStartup api failed" << endl;
	else
		cout << "WSAStartup api success" << endl;

	//create socket
	nSoc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nSoc < 0)
	{
		cout << "socket api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "socket api success" << endl;

	
	//bind socket to a port
	socAddr.sin_port = htons(PORT);;
	socAddr.sin_family = AF_INET;
	socAddr.sin_addr.s_addr = inet_addr("192.168.1.10");
	memset(&socAddr.sin_zero, 0, 8);
	ret = connect(nSoc, (sockaddr*)&socAddr, sizeof(socAddr));
	if (ret < 0)
	{
		cout << "connect api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
	{
		cout << "connect api success, connected to the server" << endl;
		char buff[255] = { 0, };
		recv(nSoc, buff, 255, 0);
		cout << buff << endl;

		cout << "Please send your message : " << endl;
		while (1)
		{
			fgets(buff, 256, stdin);
			//sends message to the server
			send(nSoc, buff, 256, 0);

			//receives message send by server
			recv(nSoc, buff, 256, 0);
			cout << "Now send next message" << endl;
		}
	}
	
	//while (true) // its a pooling loop
	//{
	//	
	//}
	return ret;
}