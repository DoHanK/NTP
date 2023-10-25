#include <iostream>
#include <thread>
#include <vector>

#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int recvLen;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char recvBuffer[BUF_SIZE + 1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// 데이터 받기
		recvLen = ::recv(client_sock, recvBuffer, BUF_SIZE, 0);
		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			return 0;
		}

		// 받은 데이터 출력
		recvBuffer[recvLen] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), recvBuffer);

		//// 데이터 보내기
		//recvLen = send(client_sock, recvBuffer, recvLen, 0);
		//if (recvLen == SOCKET_ERROR) {
		//	break;
		//}
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSAData wsaData;
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

	SOCKADDR_IN serverAddr; // Ipv4;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // < 니가 알아서 해줘
	serverAddr.sin_port = ::htons(PORT_NUM);    // 80 : HTTP

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
