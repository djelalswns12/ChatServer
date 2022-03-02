#pragma once
#pragma comment( lib, "ws2_32.lib")
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <tchar.h>
#include <locale.h>
#include <WS2tcpip.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <ctime>
#include "USER.h"
#include "ROOM.h"
using namespace std;
const int PACKET_SIZE{ 1024 };
class USER;
class ROOM;
class Manager
{
private:

public:
	SOCKET ServerSocket;
	WSADATA soData;//현재 이 소켓프로그래밍에서 사용할 소켓정보
	SOCKADDR_IN soAddr; // 서버 IP , PORT 구조체
	short _port = 4444;
	map<SOCKET, USER*> UserList;
	map<string, USER*> NameList;
	vector<ROOM> RoomList;

	string GetNowTime();
	vector<string> split(string, string);
	vector<string> split(string, string,int);
	bool isNumber(string);
	void ServerON();
	void InitSocket();
	void CreateSocket();
	void SetSocketInfo();
	void Bind();
	void Listen();

	void SendMsg(SOCKET&, const char);
	void DisConnectRoom(SOCKET*);
	void DisConnect(SOCKET*);

	void SendPrompt(USER*);
	int FindEmptyRoomIdx();
	void Print(string);

	void Login(USER*, vector<string>&);
	void H(USER*, vector<string>&);
	void H_(USER*, vector<string>&);
	void US(USER*, vector<string>&);
	void LT(USER*, vector<string>&);
	void J(USER*, vector<string>&);
	void O(USER*, vector<string>&);
	void TO(USER*, vector<string>&);
	void ST(USER*, vector<string>&);
	void PF(USER*, vector<string>&);
	void Q(USER*, vector<string>&);
	void IN_(USER*, vector<string>&);

	void Chat(USER*, vector<string>&);
	//void X(USER*, vector<string>&);
};

