#include "Manager.h"
#include "USER.h"
#include "ROOM.h"
#include "DataParse.h"


using namespace std;

int main()
{
	Manager m;

	m.ServerON("Data.json");

	fd_set read, tmp;
	TIMEVAL time;
	FD_ZERO(&read);
	FD_SET(m.ServerSocket, &read);
	int req;
	while (true)
	{
		tmp = read;
		time.tv_sec = 1;  // 초기화가 매번 이루어져야 한다.
		time.tv_usec = 0;
		req = select(NULL, &tmp, NULL, NULL, &time);
		if (SOCKET_ERROR == req)
		{
			cout << "에러발생";
			break;
		}
		if (0 == req)
		{
			continue;
		}
		for (int i = 0; i < read.fd_count; i++)
		{
			if (FD_ISSET(read.fd_array[i], &tmp))
			{
				if (m.ServerSocket == read.fd_array[i])
				{
					SOCKADDR_IN clientAddr;
					int size = sizeof(clientAddr);

					SOCKET cid = accept(m.ServerSocket, (SOCKADDR*)&clientAddr, &size);
					USER* user = new USER(cid, clientAddr);

					m.UserList.insert(make_pair(cid, user));
					FD_SET(cid, &read);
					user->SendMsg("채팅에 온걸 환영합니다.\r\n\0");
					printf("%s:%d connected\n", m.UserList[cid]->GetIP(), m.UserList[cid]->GetPort());
				}
				else
				{
					char c;
					int len = recv(read.fd_array[i], &c, sizeof(char), 0);
					if (0 == len)
					{
						//연결 종료
						printf("[%d] LogOut\n", read.fd_array[i]);
						FD_CLR(read.fd_array[i], &read);
						m.DisConnect(&tmp.fd_array[i]);
					}
					else
					{
						if (m.UserList.at(read.fd_array[i])->CatchOrder(&c))
						{
							//엔터 입력시
							//버퍼 결합
							char* msg = m.UserList[read.fd_array[i]]->AssembleBuffer();
							m.Print(string(m.UserList[read.fd_array[i]]->GetIP()) + ":" + to_string(m.UserList[read.fd_array[i]]->GetPort()) + " [" + m.UserList[read.fd_array[i]]->GetName() + "]" + ":" + msg + "\r\n");

							string msgString(msg);
							vector<string> orderList = m.split(msgString, " ", 2);
							bool order=m.ExcuteOrder(m.UserList[read.fd_array[i]], orderList);
							switch (m.UserList[read.fd_array[i]]->GetState())
							{
							case State::auth:
								if (!order) {
									if (m.UserList[read.fd_array[i]]->GetState() == State::auth)
									{
										m.UserList[read.fd_array[i]]->SendMsg("**로그인 명령어(LOGIN)를 사용해주세요.\r\n");
									}
								}
								break;

							case State::lobby:
								if (m.UserList[read.fd_array[i]]->GetState() == State::lobby)
								{
									//입력창 출력
									m.SendPrompt(m.UserList[read.fd_array[i]]);
								}
								break;

							case State::room:
								//룸 전체에게 채팅 보내기
								if (!order) {
									for (USER* u : m.UserList[read.fd_array[i]]->GetmyRoom()->GetUsers())
									{
										u->SendMsg(m.UserList[read.fd_array[i]]->GetName() + ">" + msgString + "\r\n");
									}
								}
								break;
							default:
								break;
							}
							//버퍼 비우기
							delete msg;
							m.UserList[read.fd_array[i]]->buffer.clear();

							//소켓 종료 요청 확인
							if (m.UserList[read.fd_array[i]]->GetFin()) {
								FD_CLR(read.fd_array[i], &read);
								m.DisConnect(&tmp.fd_array[i]);
								continue;
							}
						}
						else
						{
							//명령어 수집
							m.UserList[read.fd_array[i]]->buffer.push_back(c);
						}
					}
				}
			}
		}

	}
}