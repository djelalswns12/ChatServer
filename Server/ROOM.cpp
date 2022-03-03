#include "ROOM.h"

ROOM::ROOM()
{
	number = -1;
	SetOpen(false);
}

void ROOM::SetROOM(string name, string owner, string openTime, int maxCnt, int idx)
{
	this->name = name;
	this->owner = owner;
	this->openTime = openTime;
	this->maxCnt = maxCnt;
	SetNumber(idx);
	SetOpen(true);
}
void ROOM::SetNumber(int n)
{
	number = n;
}
int ROOM::GetNumber() {
	return number;
}
void ROOM::SetOpen(bool state)
{
	isOpen = state;
}
bool ROOM::GetOpen()
{
	return isOpen;
}
bool ROOM::isFull() {
	return users.size() >= maxCnt;
}
int ROOM::GetUsersSize()
{
	return users.size();
}
int ROOM::GetMaxCnt() {
	return maxCnt;
}
void ROOM::SetUser(USER* user)
{
	users.insert(user);
	user->SetmyRoom(this, m->GetNowTime());
	user->SetState(State::room);
}
set<USER*> ROOM::GetUsers()
{
	return users;
}
string ROOM::GetName() {
	return name;
}
string ROOM::GetOpenTime() {
	return openTime;
}
void ROOM::DisConnectUser(USER* u)
{
	u->SetState(State::lobby);
	users.erase(u);
	if (users.size() <= 0) 
	{
		SetOpen(false);
	}
}
