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

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int recvLen;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char recvBuffer[BUF_SIZE + 1];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		// ������ �ޱ�
		recvLen = ::recv(client_sock, recvBuffer, BUF_SIZE, 0);
		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			return 0;
		}

		// ���� ������ ���
		recvBuffer[recvLen] = '\0';
		printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), recvBuffer);

		//// ������ ������
		//recvLen = send(client_sock, recvBuffer, recvLen, 0);
		//if (recvLen == SOCKET_ERROR) {
		//	break;
		//}
	}

	// ���� �ݱ�
	closesocket(client_sock);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}

int main(int argc, char* argv[])
{
	// ���� �ʱ�ȭ
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// ���� ����
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
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // < �ϰ� �˾Ƽ� ����
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

	// ������ ��ſ� ����� ����
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

		// �մ� ����!
		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected! IP = " << ipAddress << endl;

		// ������ ����
		worker_threads.emplace_back(ProcessClient, (LPVOID)clientSocket);
	}
	for (auto& th : worker_threads)
		th.join();
	// ���� �ݱ�
	closesocket(listenSocket);

	// ���� ����
	WSACleanup();
	return 0;
}
