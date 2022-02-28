#pragma comment( lib, "ws2_32.lib")

#define PACKET_SIZE 1024
#include <iostream>
#include <WS2tcpip.h>
using namespace std;
SOCKET serverSock;
int main()
{
	WSADATA _wsaData;//현재 이 소켓프로그래밍에서 사용할 소켓정보
	SOCKADDR_IN _serverAddr; // 서버 IP , PORT 구조체

	short _port = 4444;

	// MAKEWORD = 사용할 소켓버전 선택 , socekt함수에서 사용될 소켓 버전설정
	if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
		cout << "socekt Init Error";
	}

	//PF_INET = IPV4
	//SOCK_STREAM = TCP / IP
	serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (serverSock == INVALID_SOCKET) {
		cout << "소켓 생성 실패\n";
	}

	//_serverAddr세팅
	memset(&_serverAddr, 0, sizeof(_serverAddr));
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //IP설정
	_serverAddr.sin_port = htons(_port);

	if (bind(serverSock, (SOCKADDR*)&_serverAddr, sizeof(_serverAddr)) == SOCKET_ERROR) {
		cout << "소켓 바인드 실패\n";
	}

	if (listen(serverSock, 5) == SOCKET_ERROR) {
		cout << "소켓 리슨 실패\n";
	}
	cout << "Server ON\n\n";


	fd_set read, tmp;
	TIMEVAL time;

	FD_ZERO(&read);
	FD_SET(serverSock, &read);
	int re;
	while (true)
	{
		tmp = read;
		time.tv_sec = 1;  // 초기화가 매번 이루어져야 한다.
		time.tv_usec = 0;
		re = select(NULL, &tmp, NULL, NULL, &time);
		if (SOCKET_ERROR == re)break;
		if (0 == re)continue;

		for (int i = 0; i < read.fd_count; i++)
		{
			if (FD_ISSET(read.fd_array[i], &tmp))
			{
				if (serverSock == read.fd_array[i])
				{
					SOCKADDR_IN caddr;
					int size = sizeof(caddr);

					SOCKET cid = accept(serverSock, (SOCKADDR*)&caddr, &size);

					FD_SET(cid, &read);
					printf("[%d] Login\n", cid);
				}
				else
				{
					char buf[1024] = { 0 };
					int len = recv(read.fd_array[i], buf, 1024, 0);
					if (0 == len)
					{
						FD_CLR(read.fd_array[i], &read);
						printf("[%d] LogOut\n", read.fd_array[i]);
						closesocket(tmp.fd_array[i]);
					}
					else
					{
						printf("%s", buf);
						send(read.fd_array[i], buf, 1024, 0);
					}
				}
			}
		}
	}



}

