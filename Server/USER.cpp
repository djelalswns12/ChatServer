#include "USER.h"

USER::USER() 
{

}
USER::USER(SOCKET socket, SOCKADDR_IN addr)
{
	this->Socket = socket;
	this->state = EState::Auth;
	this->Addr = addr;
	SetFin(false);
	MyRoom = nullptr;
}
void USER::SetFin(bool f)
{
	IsFin = f;
}
bool USER::GetIsUE()
{
	return IsUE;
}
void USER::SetIsUE(bool isUe)
{
	IsUE = isUe;
}
bool USER::GetFin()
{
	return IsFin;
}
void USER::SetName(string s)
{
	Name = s;
}
string USER::GetName()
{
	return Name;
}
void USER::SetState(EState st)
{
	this->state = st;
}
char* USER::GetIP()
{
	return inet_ntoa(Addr.sin_addr);
}
short USER::GetPort()
{
	return ntohs(Addr.sin_port);
}
EState USER::GetState()
{
	return this->state;
}
void USER::SetmyRoom(ROOM* room, string jointime)
{
	this->JoinTime = jointime;
	MyRoom = room;
}
ROOM* USER::GetmyRoom()
{
	return MyRoom;
}
string USER::GetJoinTime()
{
	return JoinTime;
}
void USER::SendMsg(const char c[])
{
	
	send(this->Socket, c, strlen(c), 0);
}
void USER::SendMsg(const string& s)
{
	
	send(this->Socket, s.c_str(), s.length(), 0);
}
bool USER::operator < (const USER& ref) const
{
	return Name < ref.Name;
}
bool USER::CatchOrder(char* c)
{
	return (Buffer.size() > 0 && *(Buffer.end() - 1) == '\r' && *c == '\n');
}
char* USER::AssembleBuffer()
{
	char* out = new char[Buffer.size()];
	string s = "";
	char data[PACKET_SIZE + 1];
	int i;
	int p = 0;
	for (int cnt = 0; cnt < ((Buffer.size() - 1) / PACKET_SIZE) + 1; cnt++)
	{
		//PACKET_SIZE를 넘는 버퍼에 대한 처리
		int size = Buffer.size() - 1;
		if ((Buffer.size() - 1) < (cnt + 1) * PACKET_SIZE)
		{
			size = (Buffer.size()-1) % PACKET_SIZE;
		}
		else
		{
			size = PACKET_SIZE;
		}
		for (i = 0; i < size; i++)
		{
			data[i] = *(Buffer.begin() + i);
		}
		data[size] = '\0';
		memcpy(out + p, data, strlen(data) + 1);
		p += strlen(data);
	}
	return out;
}
char* USER::FixData()
{
	char* out = new char[Buffer.size()];
	string s = "";
	char data[PACKET_SIZE + 1];
	int i;
	int p = 0;
	for (int cnt = 0; cnt < ((Buffer.size() - 1) / PACKET_SIZE) + 1; cnt++)
	{
		//PACKET_SIZE를 넘는 버퍼에 대한 처리
		int size = Buffer.size() - 1;
		if ((Buffer.size() - 1) < (cnt + 1) * PACKET_SIZE)
		{
			size = (Buffer.size() - 1) % PACKET_SIZE;
		}
		else
		{
			size = PACKET_SIZE;
		}
		for (i = 0; i < size; i++)
		{
			data[i] = *(Buffer.begin() + i);
		}
		data[size] = '\0';
		memcpy(out + p, data, strlen(data) + 1);
		p += strlen(data);
	}
	return out;
}