#include "USER.h"

USER::USER() {

}
USER::USER(SOCKET socket, SOCKADDR_IN addr) 
{
	this->socket = socket;
	this->state = State::auth;
	this->addr = addr;
	name = "";
	myRoom = nullptr;
}
void USER::SetName(string s) 
{
	name = s;
}
string USER::GetName() 
{
	return name;
}
void USER::SetState(State st) 
{
	this->state = st;
}
char* USER::GetIP() 
{
	return inet_ntoa(addr.sin_addr);
}
short USER::GetPort() 
{
	return ntohs(addr.sin_port);
}
State USER::GetState() 
{
	return this->state;
}
void USER::SetmyRoom(ROOM* room,string jointime) 
{
	this->joinTime = jointime;
	myRoom = room;
}
ROOM* USER::GetmyRoom() 
{
	return myRoom;
}
string USER::GetJoinTime() 
{
	return joinTime;
}
void USER::SendMsg(const char c[]) 
{
	send(this->socket, c, strlen(c), 0);
}
void USER::SendMsg(const string s) 
{
	send(this->socket, s.c_str(), s.length(), 0);
}
bool USER::operator < (const USER& ref) const 
{
	return name < ref.name;
}
bool USER::CatchOrder(char *c) 
{
	return (buffer.size() > 0 && *(buffer.end() - 1) == '\r' && *c == '\n');
}
char* USER::AssembleBuffer() 
{
	char* out;
	char data[PACKET_SIZE];
	int i;
	for (i = 0; i < buffer.size() - 1; i++) {
		data[i] = *(buffer.begin() + i);
	}
	//명령어 확인
	out = new char[i + 1];
	out[i] = '\0';
	memcpy(out, data, i);
	return out;
}