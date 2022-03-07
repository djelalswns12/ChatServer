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
enum class EState
{
	Auth,
	Lobby,
	Room,
};
class Manager;
class ROOM;
class USER 
{
private:
	bool IsFin;
	string Name;
	string JoinTime;
	ROOM* MyRoom;
	EState state;

public:
	SOCKET Socket;
	vector<char> Buffer;
	string DataBuffer;
	SOCKADDR_IN Addr;

	USER();
	USER(SOCKET, SOCKADDR_IN);

	void SetName(string);
	string GetName();

	char* GetIP();
	short GetPort();

	EState GetState();
	void SetState(EState);

	bool GetFin();
	void SetFin(bool);

	string GetJoinTime();

	ROOM* GetmyRoom();
	void SetmyRoom(ROOM*,string);

	bool operator < (const USER&) const;
	void SendMsg(const char c[]);
	void SendMsg(const string s);
	bool CatchOrder(char*);
	char* AssembleBuffer();
	char* FixData();

};
