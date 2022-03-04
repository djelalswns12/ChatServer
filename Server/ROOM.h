#pragma once
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
#include "USER.h"
using namespace std;
class USER;
class Manager;
class ROOM 
{
private:
	set<USER*> Users;
	int RoomIdx;
	string Name;
	bool IsOpen;
	string Owner;
	string OpenTime;
	int MaxCnt;


public:
	ROOM();
	void SetRoom(string name, string owner, string openTime, int maxCnt, int idx);
	void SetUser(USER* user);
	set<USER*> GetUsers();
	void DisConnectUser(USER* u);

	void SetNumber(int n);
	int GetNumber();
	void SetOpen(bool state);
	bool GetOpen();
	bool IsFull();
	int GetUsersSize();
	int GetMaxCnt();

	string GetName();
	string GetOpenTime();

};
