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
#include "Manager.h"
using namespace std;
enum class State
{
	auth,
	lobby,
	room
};
class Manager;
class ROOM;
class USER {
private:
	bool isFin;
	string name;
	string joinTime;
	State state;
	ROOM* myRoom;

public:
	SOCKET socket;
	vector<char> buffer;
	SOCKADDR_IN addr;

	USER();
	USER(SOCKET, SOCKADDR_IN);
	void SetName(string);
	string GetName();
	void SetState(State);
	char* GetIP();
	short GetPort();
	State GetState();
	void SetmyRoom(ROOM*,string);
	ROOM* GetmyRoom();
	string GetJoinTime();
	bool operator < (const USER&) const;
	void SendMsg(const char c[]);
	void SendMsg(const string s);
	bool CatchOrder(char*);
	char* AssembleBuffer();
	bool GetFin();
	void SetFin(bool);
};
