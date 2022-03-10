#include "ROOM.h"

ROOM::ROOM()
{
	RoomIdx = -1;
	SetOpen(false);
}

void ROOM::SetRoom(string name, string owner, string openTime, int maxCnt, int idx)
{
	this->Name = name;
	this->Owner = owner;
	this->OpenTime = openTime;
	this->MaxCnt = maxCnt;
	SetNumber(idx);
	SetOpen(true);
	Manager::getIncetance().RoomChangeEvent();
}
void ROOM::SetNumber(int n)
{
	RoomIdx = n;
}
int ROOM::GetNumber() 
{
	return RoomIdx;
}
void ROOM::SetOpen(bool state)
{
	IsOpen = state;
}
bool ROOM::GetOpen()
{
	return IsOpen;
}
bool ROOM::IsFull() 
{
	return Users.size() >= MaxCnt;
}
int ROOM::GetUsersSize()
{
	return Users.size();
}
int ROOM::GetMaxCnt() 
{
	return MaxCnt;
}
void ROOM::SetUser(USER* user)
{
	Users.insert(user);
	user->SetmyRoom(this, Manager::getIncetance().GetNowTime());
	user->SetState(EState::Room);
	Manager::getIncetance().RoomChangeEvent();
}
set<USER*> ROOM::GetUsers()
{
	return Users;
}
string ROOM::GetName() 
{
	return Name;
}
string ROOM::GetOpenTime() 
{
	return OpenTime;
}
void ROOM::DisConnectUser(USER* u)
{
	u->SetState(EState::Lobby);
	Users.erase(u);

	if (Users.size() <= 0) 
	{
		SetOpen(false);
	}
	Manager::getIncetance().RoomChangeEvent();
}
