﻿#pragma comment(lib, "ws2_32")
#define WS202
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#define BUFSIZE 512

void err_quit(char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCTSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

void err_display(char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

BOOL IsLittleEndian() {
	int i = 1;
	char c = *(char*)&i;
	return c == 1;
}

BOOL IsBigEndian() {
	int i = 1;
	char c = *(char*)&i;
	return c == 0;
}

int main(int argc, char* argv[]) {
	std::string s = IsLittleEndian() ? "True" : "False";
	std::cout << s << std::endl;
	s = IsBigEndian() ? "True" : "False";
	std::cout << s << std::endl;
//	int retval;
//	WSADATA wsa;
//
//#ifdef WS101
//	if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
//		return -1;
//	}
//#else
//	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
//		return -1;
//	}
//#endif
//	
//	MessageBox(NULL, (LPWSTR)L"윈속 초기화 성공", (LPWSTR)L"성공", MB_OK);
//
//	SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
//	if (tcp_sock == INVALID_SOCKET)
//		err_quit((char*)"socket()");
//	MessageBox(NULL, (LPWSTR)L"TCP소켓 생성 성공", (LPWSTR)L"성공", MB_OK);
//
//	
//	std::cout << "Window Socket API 필드" << std::endl;
//	std::hex(std::cout);
//	std::cout << "ㄴwVersion " << wsa.wVersion << std::endl //윈도우 소켓 버전
//		<< "ㄴwHighVersion " << wsa.wHighVersion << std::endl; //사용 가능한 윈도우 소켓 최상위 버전 
//	std::dec(std::cout);
//	std::cout << "ㄴszDescription " << wsa.szDescription << std::endl //윈도우 소켓 설명
//		<< "ㄴszSystemStatus " << wsa.szSystemStatus << std::endl; //윈도우 시스템 상태
	

//	closesocket(tcp_sock);

	//SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	//if (listen_sock == INVALID_SOCKET)
	//	err_quit((char*)"socket()");

	//SOCKADDR_IN serveraddr;
	//ZeroMemory(&serveraddr, sizeof(serveraddr));
	//serveraddr.sin_family = AF_INET;
	//serveraddr.sin_port = htons(9000);
	//serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
	//	sizeof(serveraddr));
	//if (retval == SOCKET_ERROR) {
	//	err_quit((char*)"bind()");
	//}

	//retval = listen(listen_sock, SOMAXCONN);
	//if (retval == SOCKET_ERROR) {
	//	err_quit((char*)"listen()");
	//}

	//SOCKET client_sock;
	//SOCKADDR_IN clientaddr;
	//int addrlen;
	//char buf[BUFSIZE + 1];
	//while (1) {
	//	addrlen = sizeof(clientaddr);
	//	client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
	//	if (client_sock == INVALID_SOCKET) {
	//		err_display((char*)"accept()");
	//		continue;
	//	}
	//	printf("\n[TCP 서버] 클라이언트 접속: IP주소 = &s, 포트 번호 = %d\n",
	//		inet_ntoa(clientaddr.sin_addr),
	//		ntohs(clientaddr.sin_port));
	//	while (1) {
	//		retval = recv(client_sock, buf, BUFSIZE, 0);
	//		if (retval == SOCKET_ERROR) {
	//			err_display((char*)"recv()");
	//			break;
	//		}
	//		else if (retval == 0) {
	//			break;
	//		}
	//		else {
	//			buf[retval] = '\0';
	//			printf("%s", buf);
	//		}
	//	}

	//}

	//closesocket(client_sock);
	//printf("\n[TCP 서버] 클라이언트 종료: IP주소 = &s, 포트 번호 = %d\n",
	//	inet_ntoa(clientaddr.sin_addr),
	//	ntohs(clientaddr.sin_port));
	//closesocket(listen_sock);

	WSACleanup();
	return 0;
}