// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c


#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <tchar.h>
#include <strsafe.h>
#include <iostream>

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("Calculator");
static TCHAR czChildClassName[] = _T("ChildButtonClass");
LPCWSTR button_name[20] = { _T("%"), _T("CE"),_T("C"),_T("BackSpace"), _T("1"),_T("2"), _T("3"), _T("+"),
                            _T("4"), _T("5"), _T("6"),_T("-"), _T("7"), _T("8"), _T("9"), _T("x"), _T(","),
                            _T("0"), _T("/"), _T("=") };

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Calculator");
static HWND buttons[20];
static int arithmetic_operation;
static HFONT newFont;

TCHAR buffer_for_edit[64]{ };

TCHAR buffer[64]; //biffer for int to char cast
size_t buffer_size = sizeof(buffer) / sizeof(TCHAR);

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonsProc(HWND, UINT, WPARAM, LPARAM);
void fill_buffer_edit(char);
void del_zero();

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    //size_t gg = sizeof(float);

    WNDCLASSEX wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        NULL,
        szWindowClass,
        szTitle,
        WS_OVERLAPPED | WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT,
        420, 500,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    RECT Rect;
    static int cxClient, cyClient;
    int firstline;
    static HWND hEdit, hStatic;

    static double first_input = 0;
    static double second_input = 0;
    double equals = 0;




    switch (message)
    {
    case WM_CREATE:
        GetClientRect(hWnd, &Rect);

        for (int i = 0; i < 20; i++)
        {
            _itot_s(i, buffer, _countof(buffer), 10);
            buttons[i] = CreateWindowEx(NULL, _T("button"), button_name[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0,
                hWnd, (HMENU)i,
                hInst, NULL);
        }

        hEdit = CreateWindowEx(NULL, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_RIGHT,
            (Rect.right - 380) / 2, 10, 380, 80,
            hWnd, (HMENU)101, hInst, NULL);
        hStatic = CreateWindowEx(NULL, _T("static"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
            (Rect.right - 380) / 2, 100, 380, 30,
            hWnd, (HMENU)101, hInst, NULL);

        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) break; // нопка свертывани¤ окна
        cxClient = LOWORD(lParam) - 20;
        cyClient = HIWORD(lParam);
        firstline = cyClient - 65 * 5;

        newFont = CreateFont(50, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

        SendMessage(hEdit, WM_SETFONT, WPARAM(newFont), TRUE);

        for (int i = 0; i < 20; i++)
        {
            MoveWindow(buttons[i], (i % 4 * 100), firstline + (i / 4 * 65), cxClient / 4, 60, TRUE);
            UpdateWindow(buttons[i]);
        }


        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);


        EndPaint(hWnd, &ps);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case(0): //button %
            fill_buffer_edit('2');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(1): //button CE
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            fill_buffer_edit('0');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(2): //button C
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            first_input = 0;
            fill_buffer_edit('0');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(3): //button BackSpace
            fill_buffer_edit('2');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(4): //button 1
            fill_buffer_edit('1');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(5): //button 2
            fill_buffer_edit('2');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(6): //button 3

            fill_buffer_edit('3');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(7): //button +
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 43;//code number of arithmetic operation plus in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            break;
        case(8): //button 4
            fill_buffer_edit('4');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(9): //button 5
            fill_buffer_edit('5');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(10): //button 6
            fill_buffer_edit('6');
            SetWindowText(hEdit, buffer_for_edit);
            //MessageBox(NULL, _T("stop2"), NULL, NULL);
            break;
        case(11): //button -
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 45;//code number of arithmetic operation minus in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            break;
        case(12): //button 7
            fill_buffer_edit('7');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(13): //button 8
            fill_buffer_edit('8');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(14): //button 9
            fill_buffer_edit('9');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(15): //button x
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 42;//code number of arithmetic operation multiply in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            break;
        case(16): //button ,            
            fill_buffer_edit('.');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(17): //button 0
            if (buffer_for_edit[0] == '0') break;
            fill_buffer_edit('0');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(18): //button /            
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 47; //code number of arithmetic operation divide in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            break;
        case(19):
            second_input = _tstof(buffer_for_edit);
            memset(buffer, 0, sizeof(buffer));
            switch (arithmetic_operation)
            {
            case(43):
                equals = first_input + second_input;
                StringCbPrintf(buffer, sizeof(buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, buffer);
                memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
                break;
            case(45):
                equals = first_input - second_input;
                StringCbPrintf(buffer, sizeof(buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, buffer);
                memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
                break;
            case(42):
                equals = first_input * second_input;
                StringCbPrintf(buffer, sizeof(buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, buffer);
                memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
                break;
            case(47):
                equals = first_input / second_input;
                StringCbPrintf(buffer, sizeof(buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, buffer);
                memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
                break;
            }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

LRESULT CALLBACK ButtonsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
    return 0;
}

void fill_buffer_edit(char number)
{

    for (int i = 0; i < 64; i++)
    {
        if (buffer_for_edit[0] == '0')
        {
            buffer_for_edit[0] = number;
            return;
        }

        if (buffer_for_edit[i] == NULL)
        {
            buffer_for_edit[i] = number;
            break;
        }

    }
}

void del_zero()
{
    for (int i = buffer_size - 1; i > 0; i--)
    {
        if (buffer[i] == 48 || buffer[i] == '.' || buffer[i] == NULL)
            buffer[i] = NULL;
        else
            break;
    }
}