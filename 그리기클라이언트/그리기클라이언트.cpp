// 그리기클라이언트.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "그리기클라이언트.h"
#include "CRingBuffer.h"

#define WM_SOCKET (WM_USER+1)
#define SERVERPORT 25000
#define SERVERIP L"127.0.0.1"
#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
HWND hWnd;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

struct stHEADER
{
    unsigned short len;
};

struct stDRAWPACKET
{
    int iStartX;
    int iStartY;
    int iEndX;
    int iEndY;
};

SOCKET g_Socket; //전역소켓
CRingBuffer RecvRingBuffer; //수신링버퍼
CRingBuffer SendRingBuffer; //송신링버퍼
bool g_bConnect; //현재 connect상태인지 판단
bool g_SendFlag; //현재 보낼수있는 상태인지 판단

void ReadEvent();
void WriteEvent();
void Send(stHEADER*, char*, int);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY));

    MSG msg;

    int retVal;

    WSADATA wsa;

    WSAStartup(MAKEWORD(2, 2), &wsa);

    g_Socket = socket(AF_INET, SOCK_STREAM, NULL);

    WSAAsyncSelect(g_Socket, hWnd, WM_SOCKET, FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);//여기서 논블락소켓으로 자동전환

    SOCKADDR_IN clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(SERVERPORT);
    InetPton(AF_INET, SERVERIP, &clientAddr.sin_addr);

    retVal = connect(g_Socket, (SOCKADDR*)&clientAddr, sizeof(clientAddr));//논블락소켓이므로 우드블락이 무조건 뜰것임
    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg); //WndProc을 호출하는 주체 만약 네트워크 신호가 왔을 시 WM_SOCKET 메시지가 왔을것
        }
    }

    return (int)msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int X;
    static int Y;
    static bool isClick = false;
    switch (message)
    {
    case WM_LBUTTONUP:
        isClick = false;
        break;
    case WM_LBUTTONDOWN:
        isClick = true;
        X = LOWORD(lParam);
        Y = HIWORD(lParam);
        break;
    case WM_MOUSEMOVE:
    {
        if (isClick == false)
            break;
        stHEADER header;
        stDRAWPACKET drawPacket;
        drawPacket.iStartX = X;
        drawPacket.iStartY = Y;
        X = LOWORD(lParam);
        Y = HIWORD(lParam);
        drawPacket.iEndX = X;
        drawPacket.iEndY = Y;
        header.len = 16;

        Send(&header, (char*)&drawPacket, 16);
    }
        
        break;
    case WM_SOCKET:
        if (WSAGETSELECTERROR(lParam) == SOCKET_ERROR)
        {
            printf("SOCKET_ERROR\n");
        }
        else
        {
            switch (WSAGETSELECTEVENT(lParam))
            {
            case FD_CONNECT://서버와 연결됬을 시 이 메시지가 호출됨
                printf("connect 됬따\n");
                break;
            case FD_READ:
                ReadEvent();
                break;
            case FD_WRITE://다시 송신 가능입니다.
                g_SendFlag = true;
                WriteEvent();
                break;
            }
        }
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
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

void ReadEvent()
{
    int recvBytes;
    
    int enqueueBytes;

    char Buffer[1000];

    while (1)
    {
        recvBytes = recv(g_Socket, Buffer, sizeof(Buffer), NULL);

        //예외처리가 들어가야합니다.
        if (recvBytes == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) //이게 떳다는 말은 수신버퍼가 비워져서 가져올 데이터가 없다는 뜻
            {
                return;
            }
            return;
        }

        enqueueBytes = RecvRingBuffer.Enqueue(Buffer, recvBytes);

        if (enqueueBytes != recvBytes)
        {
            printf("이상한데이터들어옴\n");
            return;
        }

        stHEADER header;

        stDRAWPACKET drawPacket;

        RecvRingBuffer.MoveFront(2); //헤더는 굳이 필요없으니까 밀어버림

        RecvRingBuffer.Dequeue((char*)&drawPacket, sizeof(drawPacket));

        HDC hdc = GetDC(hWnd);
        MoveToEx(hdc, drawPacket.iStartX, drawPacket.iStartY, NULL);
        LineTo(hdc, drawPacket.iEndX, drawPacket.iEndY);
        ReleaseDC(hWnd, hdc);
    }
    
}

void WriteEvent() //실질적으로 보내는 행위
{
    int size1;
    int size2;

    while (1)
    {
        char Buffer[1000];

        if (SendRingBuffer.GetUseSize() == 0)
        {
            return;
        }

        size1 = SendRingBuffer.Peek(Buffer, sizeof(Buffer));

        if (size1 == 0)
        {
            printf("Peek error\n");
            return;
        }

        if (g_SendFlag != true)
        {
            return;
        }

        size2 = send(g_Socket, Buffer, size1, NULL);

        if (size2 == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {
                g_SendFlag = false;
                return;
            }
            return;
        }

        SendRingBuffer.MoveFront(size2);
    }
}

void Send(stHEADER* pHeader, char* packet, int size) //송신링버퍼에 밀어넣는 행위
{
    int localSize;
    
    localSize = SendRingBuffer.Enqueue((char*)pHeader, 2);
    if (localSize != 2)
    {
        printf("Enqueue error\n");
        return;
    }
    localSize = SendRingBuffer.Enqueue(packet, size);
    if (localSize != size)
    {
        printf("Enqueue error\n");
        return;
    }

    WriteEvent();
}