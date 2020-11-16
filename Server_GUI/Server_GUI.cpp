// Server_GUI.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "framework.h"
#include "Server_GUI.h"

#define MAX_LOADSTRING 100

#define MAXBUFSIZE 1024

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI recvFile(LPVOID arg);



//에디트 컨트롤 
HWND hEdit;
void DisplayText(const char* fmt, ...);

CRITICAL_SECTION cs;
bool end;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.
    InitializeCriticalSection(&cs);
	end = false;

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SERVERGUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SERVERGUI));

    MSG msg;

    //소켓 통신 담당 스레드 생성
    CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	end = true;
    DeleteCriticalSection(&cs);
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVERGUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SERVERGUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL |
            WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_READONLY,
            0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    case WM_SETFOCUS:
        SetFocus(hEdit);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void DisplayText(const char* fmt, ...) {
    va_list arg;
    va_start(arg, fmt);
    char cbuf[256];

    wvsprintf(cbuf, fmt, arg);

	EnterCriticalSection(&cs);
    int nLength = GetWindowTextLength(hEdit);
    SendMessage(hEdit, EM_SETSEL, nLength, nLength);
    SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	LeaveCriticalSection(&cs);

    va_end(arg);

}

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
	DisplayText("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
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

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void printConnectClient(const SOCKADDR_IN& clientaddr) {
	static WCHAR cbuf[256];
	DisplayText("[TCP 서버] 클라이언트 접속: IP주소 = %s, 포트 번호 = %d\r\n",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
}

void printInfoClient(const SOCKADDR_IN& clientaddr) {
	DisplayText("[TCP 서버] 클라이언트(IP주소 = %s, 포트 번호 = %d): ",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));
	DisplayText("\r\n");
}

struct Client {
	SOCKADDR_IN clientaddr;
	char* filename = nullptr;
	WCHAR* w_filename = nullptr;
	int totalbytes = 0;
	float sendedpersent = 0.f;
	int numtotal = 0;
	int bufsize = 0;
	ULONGLONG ulong;
};

CRITICAL_SECTION ccs;
class ClientManager {
private:
	Client** m_client_list;
	int m_iNowClientNum;
	int m_iMaxClinetNum;

	ClientManager() {
		InitializeCriticalSection(&ccs);
		m_iNowClientNum = 0;
		m_iMaxClinetNum = 10;
		m_client_list = new Client * [m_iMaxClinetNum];
	}
	~ClientManager() {
		DeleteCriticalSection(&ccs);
		delete[] m_client_list;
	}
public:
	static ClientManager& getInstance() {
		static ClientManager* __instance = new ClientManager();
		return *__instance;
	}
	static void freeInstance() {
	}

	int registerClient(Client* client) {
		EnterCriticalSection(&ccs);
		if (m_iMaxClinetNum == m_iNowClientNum) {
			m_iMaxClinetNum *= 2;
			Client** new_client_list = new Client * [m_iMaxClinetNum];
			for (int i = 0; i < m_iNowClientNum; ++i) {
				new_client_list[i] = m_client_list[i];
			}
			delete[] m_client_list;
		}
		m_client_list[m_iNowClientNum] = client;
		m_iNowClientNum++;

		LeaveCriticalSection(&ccs);
		return m_iNowClientNum - 1;
	}
	bool deleteClient(Client* client) {
		EnterCriticalSection(&ccs);
		for (int i = 0; i < m_iNowClientNum; i++) {
			if (m_client_list[i] == client) {
				m_client_list[i] = NULL;
				m_iNowClientNum--;
				m_client_list[i] = m_client_list[m_iNowClientNum];

				LeaveCriticalSection(&ccs);
				return true;
			}
		}
		LeaveCriticalSection(&ccs);
		return false;
	}
	Client* findClient(int index) {
		EnterCriticalSection(&ccs);
		if (index >= m_iNowClientNum || index < 0) {
			LeaveCriticalSection(&ccs);
			return NULL;
		}
		else {
			LeaveCriticalSection(&ccs);
			return m_client_list[index];
		}
	}
	int findClient(Client* client) {
		EnterCriticalSection(&ccs);
		for (int i = 0; i < m_iNowClientNum; i++) {
			if (m_client_list[i] == client) {
				LeaveCriticalSection(&ccs);
				return i;
			}
		}

		LeaveCriticalSection(&ccs);
		return -1;
	}
	void printClinet(int index) {
		EnterCriticalSection(&ccs);
		DisplayText("%d번 클라이언트 - ", index);
		printInfoClient(m_client_list[index]->clientaddr);
		DisplayText("ㄴ받을 파일 이름 : %s\r\n", 	m_client_list[index]->filename);
		int num = m_client_list[index]->sendedpersent;
		DisplayText("ㄴ파일 수신 : %d 진행\r\n", num);
		LeaveCriticalSection(&ccs);
	}
	void printClinetsAll() {
		for (int i = 0; i < m_iNowClientNum; ++i) {
			printClinet(i);
		}
	}
};

DWORD WINAPI recvFile(LPVOID arg) {
	SOCKET client_sock = (SOCKET)arg;
	Client* client = new Client;
	SOCKADDR_IN& clientaddr = client->clientaddr;
	int addrlen = sizeof(client);
	int retval;

	getpeername(client_sock, (SOCKADDR*)&clientaddr, &addrlen);

	const int filenameSize = 256;
	client->filename = new char[filenameSize];
	ZeroMemory(client->filename, filenameSize);
	retval = recvn(client_sock, client->filename, filenameSize, 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn name");
		closesocket(client_sock);
		return 0;
	}
	DisplayText("ㄴ받을 파일 이름 : %s\r\n", client->filename);
	//clinet->w_filename = char2WCHAR(clinet->filename);

	int& totalbytes = client->totalbytes;
	retval = recvn(client_sock, (char*)&totalbytes, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn total size");
		closesocket(client_sock);
		return 0;
	}
	DisplayText("ㄴ받을 파일 용량 : %d 바이트\r\n", totalbytes );

	float& sendedpersent = client->sendedpersent;
	int& numtotal = client->numtotal;
	int& bufsize = client->bufsize;
	retval = recvn(client_sock, (char*)&bufsize, sizeof(int), 0);
	if (retval == SOCKET_ERROR) {
		err_display("error - recvn buffer size");
		closesocket(client_sock);
		return 0;
	}
	DisplayText("ㄴ전송 단위 : %d 바이트\r\n", bufsize);

	char* buf = NULL;
	if (bufsize != 0)
		buf = new char[bufsize];

	FILE* fp = fopen(client->filename, "wb");
	if (fp == NULL) {
		perror("파일 입출력 오류");
		closesocket(client_sock);
		return 0;
	}

	ClientManager::getInstance().registerClient(client);
	ClientManager::freeInstance();

	while (1) {
		retval = recvn(client_sock, buf, bufsize, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv data");
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
				sendedpersent = persent;
				ClientManager& inst = ClientManager::getInstance();
				inst.printClinet(inst.findClient(client));
				ClientManager::freeInstance();
				Sleep(1);
			}
			else if (numtotal == totalbytes) {
				break;
			}
		}
	}
	fclose(fp);
	if (buf != NULL)
		delete[] buf;

	if (numtotal == totalbytes) {
		printInfoClient(clientaddr);
		DisplayText("파일 수신 완료! : 총 %d 전송 바이트\r\n", numtotal);
		sendedpersent = 100.f;
	}
	else {
		printInfoClient(clientaddr);
		DisplayText("파일 전송 중 오류 발생\r\n");
	}

	ClientManager& inst = ClientManager::getInstance();
	inst.printClinet(inst.findClient(client));
	inst.deleteClient(client);
	ClientManager::freeInstance();

	delete client;
	closesocket(client_sock);
	DisplayText("\n[TCP 서버] 클라이언트 종료: IP주소 = %s, 포트 번호 = %d\n",
		inet_ntoa(clientaddr.sin_addr),
		ntohs(clientaddr.sin_port));

	return 0;
}

DWORD WINAPI printClient(LPVOID arg) {
	while (!end) {
		//DisplayText(L"\r\n--------printClient Thread--------\r\n");
		ClientManager& instance = ClientManager::getInstance();
		instance.printClinetsAll();
		ClientManager::freeInstance();
		//DisplayText(L"\r\n--------printClient Thread--------\r\n");
	}
	return 0;
}


// MainServer Thread
DWORD WINAPI ServerMain(LPVOID arg) {
	int retval;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return -1;
	}

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) {
		err_quit((char*)"socket()");
	}

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr,
		sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("bind()");
	}

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		err_quit("listen()");
	}

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	HANDLE hThread;
	DWORD ThreadId;

//	HANDLE hThreadPrinter = CreateThread(NULL, 0, printClient, NULL, 0, &ThreadId);

	while (!end) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, 
			(SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			continue;
		}
		DisplayText("\n[TCP 서버] 클라이언트 접속: IP주소 = %s, 포트 번호 = %d\n",
			inet_ntoa(clientaddr.sin_addr),
			ntohs(clientaddr.sin_port));
		hThread = CreateThread(NULL, 0, recvFile, (LPVOID)client_sock,
			0, &ThreadId);
		if (hThread == NULL)
			DisplayText("[ERROR] Failed Create Thread! \n");
		else
			CloseHandle(hThread);
	}
	closesocket(listen_sock);
//	CloseHandle(hThreadPrinter);

	WSACleanup();
	return 0;
}
