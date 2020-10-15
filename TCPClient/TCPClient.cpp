#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define MAXBUFSIZE 1024

void err_quit(const char* msg) {
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
	printf("[%s] %s\n", msg, (LPCTSTR)lpMsgBuf);
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

int main(int argc, char* argv[]) {
	int retval;
	const int filenameSize = 256;
	char filename[filenameSize];
	ZeroMemory(filename, filenameSize);

	
	if (argc < 2) {
		//fprintf(stderr, "Usage: %s [FileName]\n", argv[0]);
		std::cout << "Usage: [FileName]\n" << std::endl;
		std::cin >> filename;
		std::cout << filename << std::endl;
	}
	else {
		sprintf(filename, argv[1]);
	}


	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit((char*)"socket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	retval = connect(sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("connnect()");
	}
	//이름 전송
	retval = send(sock, filename, filenameSize, 0);

	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		perror("파일 입출력 오류");
		system("pause");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	int totalbytes = ftell(fp);

	//파일 크기 전송
	retval = send(sock, (char*)&totalbytes, sizeof(totalbytes), 0);
	if (retval == SOCKET_ERROR)
		err_quit("send()");

	int bufsize = totalbytes > MAXBUFSIZE ? MAXBUFSIZE : totalbytes;
	std::cout << "전송 파일 용량 : " << totalbytes << std::endl;
	std::cout << "전송 단위 용량 : " << bufsize << std::endl;
	//파일 버퍼 크기 전송
	retval = send(sock, (char*)&bufsize, sizeof(bufsize), 0);

	char *buf = new char[bufsize];
	int numread;
	int numtotal = 0;

	int sendedpersent = 0;

	rewind(fp);
	while (1) {
		numread = fread(buf, 1, bufsize, fp);
		if (numread > 0) { // 파일 전송
			retval = send(sock, buf, numread, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			} 
			numtotal += numread;
			float persent = (float)numtotal / (float)totalbytes * 100;
			if (persent - sendedpersent*10 >= 0) {
				std::cout << "파일 전송 " << persent << "% 진행" << std::endl;
				sendedpersent++;
			}
		}
		else if (numread == 0 && numtotal == totalbytes) {
			std::cout << "파일 전송 완료! : 총 전송" << numtotal << "바이트" << std::endl;
			break;
		}
		else {
			std::cout << "파일 전송 중 오류 발생" << std::endl;
		}
	}

	fclose(fp);
	closesocket(sock);

	WSACleanup();
	system("pause");
	return 0;
}