#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iterator>

#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

void process_packet(int c_id, char* packet);

mutex m;
array<int, 3> Room{ -1,-1,-1 };
int Rank = 3;

array<XMFLOAT3, 8> Poses{};
array<int, 8> Pos_List{-1,-1,-1,-1,-1,-1,-1};
int RandomNumber;

XMFLOAT3 Default_Pos{0,0,0};

enum STAGE { ST_OFFLINE, ST_LOGIN, ST_READY_ROOM, ST_INGAME };

array<bool, MAX_BULLETS> default_bullets;
array<bool, MAX_MINES> default_mines;

class Status {
private:
	int hp;
	int speed;
	XMFLOAT3 pos;
	XMFLOAT3 topDir;
	XMFLOAT3 bottomDir;
	
public:
	array<XMFLOAT3, MAX_BULLETS> bullets_pos;
	array<XMFLOAT3, MAX_BULLETS> bullets_dir;
	array<bool, MAX_BULLETS> in_use_bullets;
	array<XMFLOAT3, MAX_MINES> mines_pos;
	array<bool, MAX_MINES> in_use_mines;
	// setter
	void change_hp(int Hp) {
		hp = Hp;
	}
	void change_speed(int Speed) {
		speed = Speed;
	}
	void change_pos(XMFLOAT3 Pos) {
		pos = Pos;
	}
	void change_top_dir(XMFLOAT3 TopDir) {
		topDir = TopDir;
	}
	void change_bottom_dir(XMFLOAT3 BottomDir) {
		bottomDir = BottomDir;
	}

	// getter
	int get_hp() {
		return hp;
	}
	int get_speed() {
		return speed;
	}
	XMFLOAT3 get_pos() {
		return pos;
	}
	XMFLOAT3 get_top_dir() {
		return topDir;
	}
	XMFLOAT3 get_bottom_dir() {
		return bottomDir;
	}
};

class SESSION {
public:
	int				id;
	SOCKET			socket;
	Status			status;
	int				color;
	std::string		userName;
	bool			ready;
	char			recvBuffer[BUF_SIZE];
	int				recvLen;
	char			sendBuffer[BUF_SIZE];
	int				sendLen;
	bool			error;
	int				pos_num;
	char			remainBuffer[BUF_SIZE*2];
	int				remainLen;
	int				nowPacketSize;
	STAGE			stage;
public:
	SESSION() : socket(0)
	{
		id = -1;
		status.change_pos({ 0.f, 0.f, 0.f });
		status.change_top_dir({ 0.f, 1.f, 0.f });
		status.change_bottom_dir({ 0.f, 1.f, 0.f });
		status.change_hp(100);
		remainLen = 0;
		nowPacketSize = 0;
		ready = false;
		memset(remainBuffer, 0, sizeof(remainBuffer));
		for (int i = 0; i < 30; ++i) {
			status.in_use_bullets[i] = false;
		}
		stage = ST_OFFLINE;
	}

	~SESSION() {}

	void do_recv()																																			// 데이터 수신
	{
		recvLen = ::recv(socket, recvBuffer, BUF_SIZE, 0);

		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			std::cout << "Recv ErrorCode : " << errCode << endl;
			error = true;
			m.lock();
			Room[pos_num] = -1;

			m.unlock();

			status.change_hp(0);
			return;
		}

		char* ptr = recvBuffer;
		if (remainLen == 1) {
			memcpy(remainBuffer + 1, ptr, 1);
			ptr += 1;
			remainLen = 2;
			recvLen -= 1;
			nowPacketSize = MAKEWORD(remainBuffer[0], remainBuffer[1]);
		}
		while (recvLen > 0) {
			if (nowPacketSize == 0)
				if (recvLen >= 2)
					nowPacketSize = MAKEWORD(ptr[0], ptr[1]);
			if (nowPacketSize > 0) {
				if (nowPacketSize <= recvLen + remainLen) {
					memcpy(remainBuffer + remainLen, ptr, nowPacketSize - remainLen);
					process_packet(id,remainBuffer);
					ptr += (nowPacketSize - remainLen);
					recvLen -= (nowPacketSize - remainLen);
					nowPacketSize = 0;
					remainLen = 0;
				}
				else {
					memcpy(remainBuffer+remainLen, ptr, recvLen);
					remainLen += recvLen;
					recvLen = 0;
				}
			}
			else {
				memcpy(remainBuffer+remainLen, ptr, recvLen);
				remainLen += recvLen;
				recvLen = 0;
			}
		}
	}

	void do_send(void* packet)																																// 데이터 송신
	{
		memcpy(sendBuffer, packet,sendLen);
		sendLen = send(socket, sendBuffer, sendLen, 0);
		if (sendLen == SOCKET_ERROR) {
			int errCode = ::WSAGetLastError();
			std::cout << "Send ErrorCode : " << errCode << endl;
			error = true;
		}
	}
	
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.id = id;
		strcpy(p.userName, userName.c_str());
		p.pos = status.get_pos();
		p.top_dir = status.get_top_dir();
		p.bottom_dir = status.get_bottom_dir();
		do_send(&p);
	}
	void send_ready_packet(int c_id);
	void send_enter_room_packet(int c_id);
	void send_exit_room_packet(int c_id);
	void send_add_player_packet(int c_id);
	void send_move_packet(int c_id);
	void send_hitted_packet(int c_id);
	void send_game_start_packet();
	void send_remove_player_packet(int c_id);
	void send_bullet_packet(int c_id);
	void send_result_packet(int c_id, int rank);
};

array<SESSION, MAX_USER> clients;		// 클라이언트 배열 생성

void SESSION::send_ready_packet(int c_id)
{
	SC_READY_PACKET p;
	p.size = sizeof(SC_READY_PACKET);
	p.type = SC_READY;
	p.id = c_id;
	p.ready = clients[c_id].ready;
	sendLen = p.size;
	do_send(&p);
}
void SESSION::send_enter_room_packet(int c_id)
{
	cout << id << " 에게 " << c_id << "의 입장정보 전송" << endl;
	SC_ENTER_ROOM_PACKET p;
	p.size = sizeof(SC_ENTER_ROOM_PACKET);
	p.type = SC_ENTER_ROOM;
	p.id = c_id;
	strcpy(p.name, clients[c_id].userName.c_str());
	p.color = clients[c_id].color;
	p.pos_num = clients[c_id].pos_num;
	p.ready = clients[c_id].ready;
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_exit_room_packet(int c_id)
{
	SC_EXIT_ROOM_PACKET p;
	p.size = sizeof(SC_EXIT_ROOM_PACKET);
	p.type = SC_EXIT_ROOM;
	p.id = c_id;
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_add_player_packet(int c_id)
{
	SC_ADD_PLAYER_PACKET p;
	p.size = sizeof(SC_ADD_PLAYER_PACKET);
	p.type = SC_ADD_PLAYER;
	p.id = c_id;
	p.hp = clients[c_id].status.get_hp();
	strcpy(p.name, clients[c_id].userName.c_str());
	p.pos = Poses[c_id];
	p.top_dir = Default_Pos;
	p.bottom_dir = Default_Pos;
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.id = c_id;
	p.pos = clients[c_id].status.get_pos();
	p.top_dir = clients[c_id].status.get_top_dir();
	p.bottom_dir = clients[c_id].status.get_bottom_dir();
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_game_start_packet()
{
	SC_GAME_START_PACKET p;
	p.size = sizeof(SC_GAME_START_PACKET);
	p.type = SC_GAME_START;
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_remove_player_packet(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.size = sizeof(SC_REMOVE_PLAYER_PACKET);
	p.type = SC_REMOVE_PLAYER;
	sendLen = p.size;
	p.id = c_id;
	do_send(&p);
}

void SESSION::send_bullet_packet(int c_id)
{
	SC_BULLET_PACKET p;
	p.size = sizeof(SC_BULLET_PACKET);
	p.type = SC_BULLET;
	p.id = c_id;
	memcpy(&p.bullets_pos,&clients[c_id].status.bullets_pos, sizeof(clients[c_id].status.bullets_pos));

	memcpy(&p.bullets_dir,&clients[c_id].status.bullets_dir, sizeof(clients[c_id].status.bullets_dir));

	if(clients[c_id].status.get_hp() > 0)
		memcpy(&p.in_use_bullets,&clients[c_id].status.in_use_bullets, sizeof(clients[c_id].status.in_use_bullets));
	else
		memcpy(&p.in_use_bullets, &default_bullets, sizeof(default_bullets));

	memcpy(&p.mines_pos, &clients[c_id].status.mines_pos, sizeof(clients[c_id].status.mines_pos));

	if (clients[c_id].status.get_hp() > 0)
		memcpy(&p.in_use_mines, &clients[c_id].status.in_use_mines, sizeof(clients[c_id].status.in_use_mines));
	else
		memcpy(&p.in_use_mines, &default_mines, sizeof(default_mines));

	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_result_packet(int c_id, int rank)
{
	SC_RESULT_PACKET p;
	p.size = sizeof(SC_RESULT_PACKET);
	p.type = SC_RESULT;
	p.id = c_id;
	p.rank = rank;
	sendLen = p.size;
	do_send(&p);
}

void SESSION::send_hitted_packet(int c_id)
{
	SC_HITTED_PACKET p;
	p.size = sizeof(SC_HITTED_PACKET);
	p.type = SC_HITTED;
	p.id = c_id;
	p.hp = clients[c_id].status.get_hp();
	sendLen = p.size;
	do_send(&p);
}

void set_clientId(int c_id)
{
	clients[c_id].id = c_id;
}

void process_packet(int c_id, char* packet)
{
	//std::cout << "받은 아이디" << c_id << std::endl;

	switch (packet[2]) {
	case CS_LOGIN: {
		std::cout << "Recv Login Packet From Client Num : " << c_id << endl;
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		set_clientId(c_id);
		clients[c_id].userName = p->name;
		clients[c_id].send_login_info_packet();
		std::cout << "User Name - " << clients[c_id].userName << endl;
		std::cout << "Send Login Info Packet To Client Num : " << c_id << endl;
		break;
	}
	case CS_ENTER_ROOM: {
		std::cout << "Recv Enter Room Packet From Client Num : " << c_id << endl;
		CS_ENTER_ROOM_PACKET* p = reinterpret_cast<CS_ENTER_ROOM_PACKET*>(packet);

		m.lock();
		for (int i = 0; i < Room.size(); ++i) {
			if (Room[i] == -1) {
				Room[i] = c_id;
				clients[c_id].pos_num = i;
				break;
			}
		}
		m.unlock();

		clients[c_id].color = p->color;
		clients[c_id].stage = ST_READY_ROOM;
		/*clients[c_id].status.change_pos({ 0.f,0.f,0.f });
		clients[c_id].status.change_top_dir({ 0.f,0.f,0.f });
		clients[c_id].status.change_bottom_dir({ 0.f,0.f,0.f });*/

		for (auto& pl : clients) {
			if (pl.stage != ST_READY_ROOM)
				continue;
			pl.send_enter_room_packet(c_id);
			if (c_id == pl.id)
				continue;
			clients[c_id].send_enter_room_packet(pl.id);
		}
		break;
	}
	case CS_READY: {
		std::cout << "Recv ready Packet From Client Num : " << c_id << endl;

		if (clients[c_id].ready)
			clients[c_id].ready = false;
		else
			clients[c_id].ready = true;

		if (clients[c_id].stage != ST_READY_ROOM)
			break;
		//std::cout << "레디 패킷 전송" << std::endl;

		for (auto& pl : clients) {
			if (pl.stage != ST_READY_ROOM)
				continue;
			pl.send_ready_packet(c_id);
		}

		m.lock();
		for (int i = 0; i < Room.size(); ++i) {
			if (Room[i] == -1) {
				m.unlock();
				return;
			}
		}

		if (clients[Room[0]].ready && clients[Room[1]].ready && clients[Room[2]].ready) {
			for (int i = 0; i < Room.size(); ++i) {
				Room[i] = -1;
			}
			for (auto& pl : clients) {
				if (pl.stage != ST_READY_ROOM)
					continue;
				pl.stage = ST_INGAME;
			}
			for (auto& pl : clients) {
				if(pl.stage == ST_INGAME)
					pl.send_game_start_packet();
			}
			m.unlock();

			for (auto& pl : clients) {										// 각 클라이언트 생성 위치 지정
				for (;;) {
					RandomNumber = std::rand() % 8;
					if (Pos_List[RandomNumber] == -1) {
						Pos_List[RandomNumber] = pl.id;
						break;
					}
				}
				pl.status.change_pos(Poses[RandomNumber]);
			}

			for (auto& pl : clients) {
				if (pl.stage != ST_INGAME)
					continue;
				pl.send_add_player_packet(0);
				pl.send_add_player_packet(1);
				pl.send_add_player_packet(2);
			}
		}
		else
			m.unlock();
		break;
	}

	case CS_EXIT_ROOM: {
		std::cout << "exit Room Packet From Client Num : " << c_id << endl;

		if (clients[c_id].stage != ST_READY_ROOM)
			break;
		CS_EXIT_ROOM_PACKET* p = reinterpret_cast<CS_EXIT_ROOM_PACKET*>(packet);
		clients[c_id].ready = false;
		m.lock();
		Room[clients[c_id].pos_num] = -1;
		clients[c_id].stage = ST_LOGIN;
		m.unlock();
		for (auto& pl : clients) {
			if (pl.stage != ST_READY_ROOM)
				continue;
			pl.send_exit_room_packet(c_id);
		}
		break;
	}
	case CS_MOVE: {
		if (clients[c_id].stage != ST_INGAME)
			break;
		if (clients[c_id].status.get_hp() <= 0)
			break;
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);

		clients[c_id].status.change_pos(p->pos);
		clients[c_id].status.change_top_dir(p->top_dir);
		clients[c_id].status.change_bottom_dir(p->bottom_dir);

		for (auto& pl : clients) {
			if (pl.stage != ST_INGAME)
				continue;
			if (pl.id == c_id)
				continue;
			pl.send_move_packet(c_id);
		}

		if (Rank == 1) {
			clients[c_id].stage = ST_LOGIN;
			clients[c_id].send_result_packet(c_id, 1);
			for (auto& pl : clients) {
				pl.status.change_hp(100);
				pl.ready = false;
				Rank = 3;
			}
			for (int i = 0; i < Pos_List.size(); ++i) {
				Pos_List[i] = -1;
			}
		}
		break;
	}
	case CS_BULLET: {
		if (clients[c_id].stage != ST_INGAME)
			break;
		if (clients[c_id].status.get_hp() <= 0)
			break;
		CS_BULLET_PACKET* p = reinterpret_cast<CS_BULLET_PACKET*>(packet);
		memcpy(&clients[c_id].status.bullets_pos, &p->bullets_pos, sizeof(p->bullets_pos));
		memcpy(&clients[c_id].status.bullets_dir, &p->bullets_dir, sizeof(p->bullets_dir));
		memcpy(&clients[c_id].status.in_use_bullets, &p->in_use_bullets, sizeof(p->in_use_bullets));
		memcpy(&clients[c_id].status.mines_pos, &p->mines_pos, sizeof(p->mines_pos));
		memcpy(&clients[c_id].status.in_use_mines, &p->in_use_mines, sizeof(p->in_use_mines));

		for (auto& pl : clients) {
			if (pl.stage != ST_INGAME)
				continue;
			if (pl.id == c_id)
				continue;
			pl.send_bullet_packet(c_id);
		}
		break;
	}

	case CS_MINE_ATTACK: {
		if (clients[c_id].stage != ST_INGAME)
			break;
		if (clients[c_id].status.get_hp() <= 0)
			break;

		CS_MINE_ATTACK_PACKET* p = reinterpret_cast<CS_MINE_ATTACK_PACKET*>(packet);


		m.lock();
		clients[p->id].status.change_hp(clients[p->id].status.get_hp() - 50);

		for (auto& pl : clients) {
			if (pl.stage != ST_INGAME)
				continue;
			if (pl.id == c_id)
				continue;
			if (clients[p->id].status.get_hp() <= 0){
				pl.send_remove_player_packet(p->id);
				pl.send_bullet_packet(p->id);
			}
			else
				pl.send_hitted_packet(p->id);

		}

		if (clients[p->id].status.get_hp() <= 0) {
			if (clients[p->id].stage == ST_INGAME)
			{
				clients[p->id].stage = ST_LOGIN;
				clients[p->id].send_result_packet(p->id, Rank);
				Rank--;
			}
		}
		m.unlock();

		if (Rank == 1) {
			clients[c_id].stage = ST_LOGIN;
			clients[c_id].send_result_packet(c_id, 1);
			for (auto& pl : clients) {
				pl.status.change_hp(100);
				pl.ready = false;
			}
			Rank = 3;
			for (int i = 0; i < Pos_List.size(); ++i) {
				Pos_List[i] = -1;
			}
		}

		break;
	}
	case CS_ATTACK: {
		if (clients[c_id].stage != ST_INGAME)
			break;
		if (clients[c_id].status.get_hp() <= 0)
			break;
		CS_ATTACK_PACKET* p = reinterpret_cast<CS_ATTACK_PACKET*>(packet);


		m.lock();
		clients[p->id].status.change_hp(clients[p->id].status.get_hp() - 10);

		for (auto& pl : clients) {
			if (pl.stage != ST_INGAME)
				continue;
			if (clients[p->id].status.get_hp() <= 0){
				pl.send_remove_player_packet(p->id);
				if(pl.id != p->id)
					pl.send_bullet_packet(p->id);
			}
			else
				pl.send_hitted_packet(p->id);
		}

		if (clients[p->id].status.get_hp() <= 0) {
			if(clients[p->id].stage == ST_INGAME)
			{
				clients[p->id].stage = ST_LOGIN;
				clients[p->id].send_result_packet(p->id, Rank);
				Rank--;
			}
		}
		m.unlock();

		if (Rank == 1) {
			clients[c_id].stage = ST_LOGIN;
			clients[c_id].send_result_packet(c_id, 1);
			for (auto& pl : clients) {
				pl.status.change_hp(100);
				pl.ready = false;
			}
			Rank = 3;
			for (int i = 0; i < Pos_List.size(); ++i) {
				Pos_List[i] = -1;
			}
		}

		break;
	}
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
		if (clients[i].stage == ST_OFFLINE) {
			return i;
		}
	return -1;
}



																																				// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	m.lock();
	int client_id = get_new_client_id();
	if (client_id != -1) {
		clients[client_id].stage = ST_LOGIN;
		m.unlock();
		clients[client_id].status.change_pos({ 0.f,0.f,0.f });
		clients[client_id].status.change_top_dir({ 0.f,0.f,0.f });
		clients[client_id].status.change_bottom_dir({ 0.f,0.f,0.f });
		clients[client_id].status.change_hp(100);

		clients[client_id].id = client_id;
		clients[client_id].ready = false;
		clients[client_id].error = false;
		clients[client_id].socket = (SOCKET)arg;
	
		cout << "Client [" << client_id << "] Login" << endl;
	}
	else {
		m.unlock();
		cout << "Max user exceeded.\n";
	}

	while (1) {
		clients[client_id].do_recv();
		if (clients[client_id].error) {
			clients[client_id].stage = ST_OFFLINE;
			for (auto& pl : clients)
				if (pl.stage == ST_READY_ROOM)
					pl.send_exit_room_packet(client_id);
			break;
		}
	}
																												// 소켓 닫기
	closesocket(clients[client_id].socket);
	clients[client_id].stage = ST_OFFLINE;
	cout << clients[client_id].id << " 번 클라이언트 종료"<<endl;
	return 0;
}

void InitPos()
{
	Poses[0].x = 485.1;
	Poses[0].y = 2;
	Poses[0].z = 279.6;

	Poses[1].x = -163.12;
	Poses[1].y = 2;
	Poses[1].z = 509.44;

	Poses[2].x = -44;
	Poses[2].y = 2;
	Poses[2].z = -12.53;

	Poses[3].x = 418.77;
	Poses[3].y = 2;
	Poses[3].z = 12.48;

	Poses[4].x = 181.94;
	Poses[4].y = 2;
	Poses[4].z = 670.8;

	Poses[5].x = 225.98;
	Poses[5].y = 2;
	Poses[5].z = 175.19;

	Poses[6].x = 195;
	Poses[6].y = 2;
	Poses[6].z = 665;

	Poses[7].x = 110.5;
	Poses[7].y = 2;
	Poses[7].z = 282.4;

	for (int i = 0; i < MAX_BULLETS; ++i) {
		default_bullets[i] = false;
	}

	for (int i = 0; i < MAX_MINES; ++i) {
		default_mines[i] = false;
	}
}

int main(int argc, char* argv[])
{
	InitPos();
	std::srand(static_cast<unsigned int>(std::time(nullptr)));
	WSAData wsaData;																																	// 윈속 초기화
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

																																						// 소켓 생성
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		int errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

																																						// bind()

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(PORT_NUM);

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

																																						// listen()
	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		int errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

																																		// 데이터 통신에 사용할 변수
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrLen;

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	while (1) {
																																						// accept()
		addrLen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			return 0;
		}

																																						// 손님 입장!
		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected! IP = " << ipAddress << endl;

																																						// 스레드 생성
		worker_threads.emplace_back(ProcessClient, (LPVOID)clientSocket);
	}
	for (auto& th : worker_threads)
		th.join();
																																						// 소켓 닫기
	closesocket(listenSocket);

																																						// 윈속 종료
	WSACleanup();
	return 0;
}
