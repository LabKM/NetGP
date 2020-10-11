#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#define MAXBUFSIZE 1024

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

void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char* buf, int len, int flags) {
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}

DWORD WINAPI recvFile(LPVOID arg){
	SOCKET client_sock = (SOCKET)arg;
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	int retval;

	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	const int filenameSize = 256;
	char filename[filenameSize];
	ZeroMemory(filename, filenameSize);
	retval = recvn(client_sock, filename, filenameSize, 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn name");
		closesocket(client_sock);
		return 0;
	}
	std::cout << "ㄴ받을 파일 이름 : " << filename << std::endl;

	int totalbytes = 0;
	retval = recvn(client_sock, (char*)&totalbytes, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn total size");
		closesocket(client_sock);
		return 0;
	}
	std::cout << "ㄴ받을 파일 용량 : " << totalbytes << "바이트" << std::endl;

	float sendedpersent = 0;
	int numtotal = 0;
	int bufsize = 0;
	retval = recvn(client_sock, (char*)&bufsize, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn buffer size");
		closesocket(client_sock);
		return 0;
	}
	std::cout << "ㄴ전송 단위 : " << bufsize << "바이트" << std::endl;

	char* buf = NULL;
	if (bufsize != 0)
		buf = new char[bufsize];

	FILE* fp = fopen(filename, "wb");
	if (fp == NULL) {
		perror("파일 입출력 오류");
		closesocket(client_sock);
		return 0;
	}

	while (1) {
		retval = recvn(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display((char*)"recv data");
			break;
		}
		else if (retval == 0) {
			break;
		}
		else {
			fwrite(buf, 1, retval, fp);
			if (ferror(fp)) {
				perror("파일 입출력 오류");
				break;
			}
			numtotal += retval;
			float persent = (float)numtotal / (float)totalbytes * 100;
			if (persent - sendedpersent > 10) {
				std::cout << "파일 수신 " << persent << "% 진행" << std::endl;
				sendedpersent = persent;
			}
			else if(numtotal == totalbytes) {
				std::cout << "파일 수신 " << persent << "% 진행" << std::endl;
				break;
			}
		}
	}
	fclose(fp);
	if (buf != NULL)
		delete[] buf;

	if (numtotal == totalbytes)
		std::cout << "파일 수신 완료! : 총 전송" << numtotal << "바이트" << std::endl;
	else
		std::cout << "파일 전송 중 오류 발생" << std::endl;

	closesocket(client_sock);
	printf("\n[TCP 서버] 클라이언트 종료: IP주소 = &s, 포트 번호 = %d\n",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	return 0;
}

int main(int argc, char* argv[]) {
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit((char*)"socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit((char*)"bind()");
	}

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit((char*)"listen()");
	}

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;
	DWORD ThreadId;

	while (1) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display((char*)"accept()");
			continue;
		}
		printf("\n[TCP 서버] 클라이언트 접속: IP주소 = &s, 포트 번호 = %d\n",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port));
		hThread = CreateThread(NULL, 0, recvFile, (LPVOID)client_sock,
			0, &ThreadId);
		if (hThread == NULL)
			printf("[ERROR] Failed Create Thread! \n");
		else
			CloseHandle(hThread);
	}
	closesocket(listen_sock);

	WSACleanup();
	return 0;
}