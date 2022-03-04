#include "Manager.h"
#include "USER.h"
#include "ROOM.h"
#include "DataParse.h"


using namespace std;
	
int main()
{
	SOCKET* targetSocket;
	Manager& m = Manager::getIncetance();
	m.ServerON("Data.json");

	fd_set read, tmp;
	TIMEVAL time;
	FD_ZERO(&read);
	FD_SET(m.ServerSocket, &read);
	int req;
	while (true)
	{
		tmp = read;
		time.tv_sec = 1;  
		time.tv_usec = 0;
		req = select(NULL, &tmp, NULL, NULL, &time);
		if (SOCKET_ERROR == req)
		{
			cout << "req Error";
			break;
		}
		if (0 == req)
		{
			continue;
		}
		for (int i = 0; i < read.fd_count; i++)
		{
			targetSocket = &read.fd_array[i];
			if (!FD_ISSET(*targetSocket, &tmp))
			{
				continue;
			}
			if (m.ServerSocket == *targetSocket)
			{
				SOCKADDR_IN clientAddr;
				int size = sizeof(clientAddr);

				SOCKET cid = accept(m.ServerSocket, (SOCKADDR*)&clientAddr, &size);
				USER* user = new USER(cid, clientAddr);

				m.UserList.insert(make_pair(cid, user));
				FD_SET(cid, &read);
				user->SendMsg("안녕하세요. 텍스트 채팅 서버 입니다. 환영합니다.\r\n**로그인 명령어(LOGIN)를 사용해주세요.\r\n");
				printf("%s:%d connected\n", m.UserList[cid]->GetIP(), m.UserList[cid]->GetPort());
			}
			else
			{
				char c;
				int len = recv(*targetSocket, &c, sizeof(char), 0);
				if (0 == len)
				{
					//연결 종료
					printf("[%d] is Disconneted\n", *targetSocket);
					FD_CLR(*targetSocket, &read);
					m.DisConnect(&tmp.fd_array[i]);
				}
				else
				{
					if (m.UserList.at(*targetSocket)->CatchOrder(&c))
					{
						//엔터 입력시
						//버퍼 결합
						char* msg = m.UserList[*targetSocket]->AssembleBuffer();
						m.Print(string(m.UserList[*targetSocket]->GetIP()) + ":" + to_string(m.UserList[*targetSocket]->GetPort()) + " [" + m.UserList[*targetSocket]->GetName() + "]" + ":" + msg + "\r\n");
						
						string msgString(msg);
						vector<string> orderList = m.Split(msgString, " ", 2);

						bool order = m.ExcuteOrder(m.UserList[*targetSocket], orderList);
						switch (m.UserList[*targetSocket]->GetState())
						{
						case EState::Auth:
							if (!order) 
							{
								if (m.UserList[*targetSocket]->GetState() == EState::Auth)
								{
									m.UserList[*targetSocket]->SendMsg("**로그인 명령어(LOGIN)를 사용해주세요.\r\n");
								}
							}
							break;

						case EState::Lobby:
							if (m.UserList[*targetSocket]->GetState() == EState::Lobby)
							{
								//입력창 출력
								m.SendPrompt(m.UserList[*targetSocket]);
							}
							break;

						case EState::Room:
							//룸 전체에게 채팅 보내기
							if (!order) {
								for (USER* u : m.UserList[*targetSocket]->GetmyRoom()->GetUsers())
								{
									u->SendMsg(m.UserList[*targetSocket]->GetName() + ">" + msgString + "\r\n");
								}
							}
							break;
						default:
							break;
						}
						//버퍼 비우기
						delete msg;
						m.UserList[*targetSocket]->Buffer.clear();

						//소켓 종료 요청 확인
						if (m.UserList[*targetSocket]->GetFin()) 
						{
							FD_CLR(*targetSocket, &read);
							m.DisConnect(&tmp.fd_array[i]);
							continue;
						}
					}
					else
					{
						//명령어 수집
						m.UserList[*targetSocket]->Buffer.push_back(c);
					}
				}
			}
		}
	}
}