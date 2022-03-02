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
#include "DataParse.h"

using namespace std;

const int PACKET_SIZE{ 1024 };
enum class State;
class USER;
class ROOM;
typedef void (Manager::* OrderFunc)(USER*, vector<string>&);
class Manager
{
private:

public:

	map<string, OrderFunc> OrderFuncs;
	vector<OrderFunc> OrderFuncsList{ &Manager::H,&Manager::H_,&Manager::US,&Manager::LT,&Manager::J,&Manager::O,&Manager::TO,&Manager::ST,&Manager::PF,&Manager::Login,&Manager::Q,&Manager::IN_,&Manager::X};
	DataParse DB;
	SOCKET ServerSocket;
	WSADATA soData;//���� �� �������α׷��ֿ��� ����� ��������
	SOCKADDR_IN soAddr; // ���� IP , PORT ����ü
	short _port = 4444;
	map<SOCKET, USER*> UserList;
	map<string, USER*> NameList;
	vector<ROOM> RoomList;

	string GetNowTime();
	vector<string> split(string, string);
	vector<string> split(string, string, int);
	bool isNumber(string);
	void ServerON(string);
	void InitSocket();
	void CreateSocket();
	void SetSocketInfo();
	void Bind();
	void Listen();

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
	void X(USER*, vector<string>&);
	void IN_(USER*, vector<string>&);
	void Chat(USER*, vector<string>&);
	void SetOrder(vector<pair<string, string>> data)
	{
		for (auto v : data) {
			OrderFuncs.insert(make_pair(v.first, OrderFuncsList[stoi(v.second)]));
		}
	}
	bool ExcuteOrder(USER*, vector<string>&);
	//void X(USER*, vector<string>&);
};

