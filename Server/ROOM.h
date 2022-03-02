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
class ROOM {
private:
	Manager* m;
	bool isOpen;
	string name;
	string owner;
	string openTime;
	int maxCnt;
	int number;
	set<USER*> users;
public:
	ROOM();

	void SetROOM(string name, string owner, string openTime, int maxCnt, int idx);
	void SetNumber(int n);
	int GetNumber();
	void SetOpen(bool state);
	bool GetOpen();
	bool isFull();
	int GetUsersSize();
	int GetMaxCnt();
	void SetUser(USER* user);
	set<USER*> GetUsers();
	string GetName();
	string GetOpenTime();
	void DisConnectUser(USER* u);
};
