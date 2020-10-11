#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

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

BOOL GetIPAddr(const char* name, IN_ADDR* addr)
{
	HOSTENT* ptr = gethostbyname(name);
	if (ptr == NULL) {
		err_display("gethostbyname()");
		return FALSE;
	}

	memcpy(addr, ptr->h_addr, ptr->h_length);
	return TRUE;
}


int GetIPAddrAll(const char* name, char**& addrs) {
	HOSTENT* ptr = gethostbyname(name);
	if (ptr == NULL) {
		err_display("gethostbyname()");
		return 0;
	}

	addrs = ptr->h_addr_list;
	return ptr->h_length;
}

BOOL GetDomainName(IN_ADDR addr, char* name) {
	HOSTENT* ptr = gethostbyaddr((char*)&addr,	sizeof(addr), AF_INET);
	
	if (ptr == NULL) {
		err_display("gethostbyaddr()");
		return FALSE;
	}

	strcpy(name, ptr->h_name);
	return TRUE;
}

int GetDomainAliases(char* name, char**& aliases, int* len) {
	HOSTENT* ptr = gethostbyname(name);
	len = 0;
	if (ptr == NULL) {
		err_display("gethostbyaddr()");
		return FALSE;
	}
	if (ptr->h_aliases == NULL) {
		return FALSE;
	}
	
	aliases = ptr->h_aliases;
	while (*aliases != NULL) {
		len++;
		aliases++;
	}

	return TRUE;
}

void PrintDomainNameAndAliases(const char* name) {
	HOSTENT* ptr = gethostbyname(name);
	if (ptr == NULL) {
		err_display("gethostbyname()");
		return;
	}
	
	std::cout << ptr->h_name << std::endl;

	while (*ptr->h_addr_list != NULL)
	{
		long int* add = (long int*)*ptr->h_addr_list;
		IN_ADDR addr;
		addr.s_addr = *add;
		printf("%s\n", inet_ntoa(addr));
		ptr->h_addr_list++;
	}
}

//https://dnslytics.com/ip/216.58.197.238 확인
int main(int argc, char* argv[]) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}
	IN_ADDR addr;
	std::string str;
	std::cin >> str;
	if (GetIPAddr(str.c_str(), &addr)) {
		const char* in = str.c_str();
		char** addrs = NULL;
		int length = GetIPAddrAll(in, addrs); 
		//std::cout << "IP 주소 개수 = " << addr_len << std::endl;
		
		while (*addrs != NULL) {
			memcpy(&addr, addrs, length);
			std::cout << "IP주소 - " << inet_ntoa(addr) << std::endl;
			addrs++;
		}
		char name[256];
		if (GetDomainName(addr, name)) {
			std::cout << "도메인 이름 - " << name << std::endl;
		}

		char** aliases = NULL;
		int len = 0;
		if (GetDomainAliases((char*)in, aliases, &len)) {
			std::cout << "도메인 별명 개수 = " << len << std::endl;
			for (int i = 0; i < len; ++i)
				std::cout << "별명 이름 - " << aliases[i] << std::endl;
		}
	}
	
	system("pause");

	WSACleanup();
	return 0;
}