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


void ProcessNewMessage(int _clientSocket)
{
	cout << "Process the new message from the client" << endl;
	char buff[255] = { 0, };
	int ret = recv(_clientSocket, buff, 255, 0);
	if (ret < 0)
	{
		cout << "Something goes wrong, closing socket" << endl;
		closesocket(_clientSocket);
	}
	else
	{
		cout << "The message received from the client is : "<<buff<<endl;
		send(_clientSocket, "Processed your message", sizeof("Processed your message"), 0);
		cout << "**************************************************************" << endl; 
	}
	
}

void ProcessTheNewRequest()
{
	if (FD_ISSET(nSoc, &fd_read))
	{
		int nLen = sizeof(struct sockaddr);
		int clientSocket = accept(nSoc, NULL, &nLen);
		if (clientSocket > 0)
		{
			client_fd_array.push_back(clientSocket);
			send(clientSocket, "Hi , its server this end , i received your connection request", sizeof("Hi , its server this end , i received your connection request"), 0);
		}
	}
	else
	{
		for (auto client : client_fd_array)
		{
			if (FD_ISSET(client, &fd_read))
			{
				//Got the new message from client
				//Just recv new message
				//Just queue that for the new worker 
				ProcessNewMessage(client);
			}
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

	//set blocking or non blocking socket here
	//NOTE:- not required in case of blocking socket as by default all sockets are blocking
	u_long optval = 0;  //set the value as 0 for blocking socket , as 1 for non blocking socket
	ret = ioctlsocket(nSoc, FIONBIO, & optval);
	if (ret != 0)
	{
		cout << "ioctlsocket api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "ioctlsocket api success" << endl;

	//Set socket options
	int optvall = 0;
	int optlen = sizeof(optvall);
	ret = setsockopt(nSoc, SOL_SOCKET, SO_REUSEADDR, (const char * )&optvall, optlen);
	if (ret != 0)
	{
		cout << "setsockopt api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "setsockopt api success" << endl;


	//bind socket to a port
	socAddr.sin_port = htons(PORT);;
	socAddr.sin_family = AF_INET;
	//socAddr.sin_addr.s_addr = inet_addr("127.6.2.12");
	socAddr.sin_addr.s_addr = INADDR_ANY;
	memset(&socAddr.sin_zero, 0, 8);
	ret = bind(nSoc, (sockaddr*)&socAddr, sizeof(socAddr));
	if (ret < 0)
	{
		cout << "bind api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "bind api success" << endl;

	//listen
	ret = listen(nSoc, 5);
	if (ret < 0)
	{
		cout << "listen api failed" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "listen api success" << endl;

	//wait for connection
	nMaxFd = nSoc;

	while (true) // its a pooling loop
	{
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_exception);

		FD_SET(nSoc, &fd_read);
		FD_SET(nSoc, &fd_exception);

		for (auto client : client_fd_array)
		{
			FD_SET(client, &fd_read);
			FD_SET(client, &fd_exception);
		}

		timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		cout << endl;
		cout << "fd_read count before select call " << fd_read.fd_count << endl;
		//keep waiting for the new request and proceed as per the requset
		ret = select(nMaxFd + 1, &fd_read, &fd_write, &fd_exception, &tv);

		cout << "fd_read count after select call " << fd_read.fd_count << endl;
		if (ret > 0)
		{
			cout << "someone is connected" << endl;
			ProcessTheNewRequest();
			//WSACleanup();
			//break;
		}
		else if (ret == 0)
		{
			cout << "no one is connected" << endl;
		}
		else
		{
			cout << "nfailed to connected" << endl;
			WSACleanup();
			return -1;
		}
		cout << endl;
	}
	return ret;
}