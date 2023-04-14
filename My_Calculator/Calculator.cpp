// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c


#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <tchar.h>
#include <strsafe.h>
#include <iostream>

// link to get some modern Win styles for buttons design
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Global variables

static TCHAR szWindowClass[] = _T("Calculator"); // The main window class name.
static TCHAR czChildClassName[] = _T("ChildButtonClass"); // The child windows class name (not in use yet)

// signs on the calculator's basic buttons
LPCWSTR button_name[20] = { _T("%"), _T("CE"),_T("C"),_T("BackSpace"), _T("1"),_T("2"), _T("3"), _T("+"),
                            _T("4"), _T("5"), _T("6"),_T("-"), _T("7"), _T("8"), _T("9"), _T("x"), _T(","),
                            _T("0"), _T("/"), _T("=") };

// signs on the calculator's memory buttons
LPCWSTR mem_button_name[5]{ _T("MC"), _T("MR"), _T("M+"), _T("M-"), _T("MS") };

static TCHAR szTitle[] = _T("Calculator");  // The string that appears in the application's title bar
static HWND buttons[20];                    // Handles of calc basic buttons
static HWND mem_buttons[5];                 // Handles of calc memory buttons
static HFONT newFont;                       // Font for main calc input field
int firstline;  // Variable is used just to set up the upper line of the button's area (to format calc UI)
static int arithmetic_operation;            // It stores current value of pressed arithmetic button
float mem_cell = 0.0;                       // Memory cell to process transactions with five memory buttons
TCHAR buffer_for_edit[15]{ };   // Buffer of main Edit field is used to store the values of the current input string
TCHAR cast_buffer[15]{ };          //Biffer for cast operation from int to char
size_t cast_buffer_size = sizeof(cast_buffer) / sizeof(TCHAR); //amount of chars in the cast buffer
int flag = 0;       //flag to process multiple button "=" press
double temp = 0.0;  // var to save second_input in case of multiple button "=" press

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void fill_buffer_edit(char);// fill the main buffer for user input in te Edit field
void del_zero();            // removes extra zeros after the floating point  - 5.4080000000


int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
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
    PAINTSTRUCT ps; // The address of a structure variable of type PAINTSTRUCT
    HDC hdc;        // The handle of the device context of the main window
    RECT Rect;      // The address of a structure variable of type RECT
    static int cxClient, cyClient; // width and height of the client area
    
    static HWND hEdit, hStatic; 

    static double first_input = 0;  // First user input string of numbers
    static double second_input = 0; // User input string of numbers after pressing one of the arithmetic operation button
    double equals = 0;              // Total result.  Button: =
    

    switch (message)
    {
    case WM_CREATE:
        GetClientRect(hWnd, &Rect); // Get client area

        // Create memory calc buttons
        for (int i = 0; i < 5; i++) 
        {
           // _itot_s(i, cast_buffer, _countof(cast_buffer), 10); // Cast int i to the char 'i' for using in the (HMENU)i
            mem_buttons[i] = CreateWindowEx(NULL, _T("button"), mem_button_name[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                Rect.left + 5, 0, 0, 0,
                hWnd, (HMENU)(i+50), hInst, NULL);
        }
        
        // Create basic calc buttons
        for (int i = 0; i < 20; i++)
        {
            //_itot_s(i, cast_buffer, _countof(cast_buffer), 10); // Cast int i to the char 'i' for using in the (HMENU)i
            buttons[i] = CreateWindowEx(NULL, _T("button"), button_name[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0,
                hWnd, (HMENU)i,
                hInst, NULL);
        }

        // Create main input field for user input
        hEdit = CreateWindowEx(NULL, _T("edit"), _T("0"), WS_CHILD | WS_VISIBLE | ES_RIGHT,
            (Rect.right - 380) / 2, 20, 380, 70,
            hWnd, (HMENU)101, hInst, NULL);  

        break;

    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED) break; // button - minimize the window 
        cxClient = LOWORD(lParam) - 20; // Get the width of the client area and reduce it just to get suitable position for buttons
        cyClient = HIWORD(lParam);      // Get the height of the client area
        firstline = cyClient - 65 * 5;  // Set the upper line for the button's area (just to format the UI)

        // Create new font for the main input area
        newFont = CreateFont(50, 0, 0, 0, FW_NORMAL, 0, 0, 0,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

        SendMessage(hEdit, WM_SETFONT, WPARAM(newFont), TRUE); // Set new font

        // Show and place memory buttons on the main window
        for (int i = 0; i < 5; i++)
        {
            MoveWindow(mem_buttons[i], i % 5 * 80, 90, cxClient / 5, 40, TRUE);
            UpdateWindow(mem_buttons[i]);
        }

        // Show and place basic buttons on the main window
        for (int i = 0; i < 20; i++)
        {
            MoveWindow(buttons[i], (i % 4 * 100), firstline + (i / 4 * 65), cxClient / 4, 60, TRUE);
            UpdateWindow(buttons[i]);     }

        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    /************************ processing the buttons pressed *************************************************************/
    case WM_COMMAND:
        switch (LOWORD(wParam))
           
        {
        case(50): // MC memory clear
            mem_cell = 0.0;
            break;
        case(51): // MR memory recall
            StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), mem_cell); // Cast the mem_cell from int to string
            del_zero();                         // Delete unnecessary zero chars at the end of the string of numbers
            SetWindowText(hEdit, cast_buffer);  // output var mem_cell into the window
            break;
        case(52): // M+ memoryh add
            mem_cell = mem_cell + (_tstof(buffer_for_edit)); //add new input to the stored value
            StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), mem_cell);  // Cast the mem_cell from int to string
            del_zero();                         // Delete unnecessary zero chars at the end of the string of numbers
            SetWindowText(hEdit, cast_buffer);  // output var mem_cell into the window
            memset(cast_buffer, 0, sizeof(cast_buffer)); //clear temp cast_buffer which we used before
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            break;
        case(53): // M- memory subtract
            mem_cell = mem_cell - (_tstof(buffer_for_edit)); //subtract new input from the stored value
            StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), mem_cell); // Cast the mem_cell from int to string
            del_zero();                         // Delete unnecessary zero chars at the end of the string of numbers
            SetWindowText(hEdit, cast_buffer);  // output var mem_cell into the window
            memset(cast_buffer, 0, sizeof(cast_buffer)); //clear temp cast_buffer which we used before
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            break;
        case(54): // MS memory store            
            mem_cell = _tstof(buffer_for_edit); //put user input into the var mem_sel
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            fill_buffer_edit('0');                // Put one char '0' in the buffer to display it for the user
            SetWindowText(hEdit, buffer_for_edit); // output '0' into the window
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            break; 
        case(0): //button %              
            second_input = _tstof(buffer_for_edit);
            memset(cast_buffer, 0, sizeof(cast_buffer));
            // Logic for % calculation:
            // case(43) when we want to add some percents to the first input 50+25%
            // case(45) when we want to subtract some percents from the first input 50-25%
            // case default - when we want to calculate a value where first input is any number and second input is
            // percent from the first input: 100 and 25 (25% from the 100)
            switch (arithmetic_operation)
            {
            case(43): //button +
                equals = first_input + first_input * (second_input / 100);
                break;
            case(45): //button -
                equals = first_input - first_input * (second_input / 100);
                break;
            default:
                equals = first_input * (second_input / 100);
                break;
            }            
            StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), equals);
            del_zero();
            SetWindowText(hEdit, cast_buffer);
            first_input = 0;
            second_input = 0;
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));            
            break;
        case(1): //button CE (clear the last input)
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            fill_buffer_edit('0');                                  // Put one char '0' in the buffer to display it for the user
            SetWindowText(hEdit, buffer_for_edit);                  // Display '0' in the main calc field
            break;
        case(2): //button C (clear all)
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit)); // Clear "buffer for edit" and fill it with NULLs
            first_input = 0;                                        // Clear even the first user input
            fill_buffer_edit('0');                                  // Put one char '0' in the buffer to display it for the user
            SetWindowText(hEdit, buffer_for_edit);                  // Display '0' in the main calc field
            break;
        case(3): //button BackSpace
            // Here we sequentially delete numbers from the user input "Edit" field from the right to the left
            // When we reach the very first number in the line we change it to the '0' at once operation
            for (int i = sizeof(buffer_for_edit) / sizeof(TCHAR) - 1; i >= 0; i--)
            {
                if (i == 0) // check if we reach the very first number in the line
                {
                    buffer_for_edit[i] = '0';
                    SetWindowText(hEdit, buffer_for_edit);
                    break;
                }
                if (buffer_for_edit[i] != NULL) // delete the current last number in the line
                {
                    buffer_for_edit[i] = NULL;
                    SetWindowText(hEdit, buffer_for_edit);
                    break;
                }
                else
                {
                    continue;
                }                 
            }            
            break;
        case(4): //button number 1
            fill_buffer_edit('1');
            SetWindowText(hEdit, buffer_for_edit); 
            break;
        case(5): //button number 2
            fill_buffer_edit('2');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(6): //button number 3
            fill_buffer_edit('3');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(7): //button + plus
            first_input = _tstof(buffer_for_edit); //Cast user input from text string to the float
            arithmetic_operation = 43;//43 is the code number of arithmetic operation button "plus" in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            flag = 1;
            break;
        case(8): //button number 4
            fill_buffer_edit('4');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(9): //button number 5
            fill_buffer_edit('5');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(10): //button number 6
            fill_buffer_edit('6');
            SetWindowText(hEdit, buffer_for_edit);            
            break;
        case(11): //button -  minus
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 45;//45 is the code number of arithmetic operation button "minus" in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            flag = 1;
            break;
        case(12): //button number 7
            fill_buffer_edit('7');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(13): //button number 8
            fill_buffer_edit('8');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(14): //button number 9
            fill_buffer_edit('9');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(15): //button x multiply
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 42;//42 is the code number of arithmetic operation button "multiply" in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            flag = 1;
            break;
        case(16): //button , (floating point)            
            fill_buffer_edit('.');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(17): //button number 0
            if (buffer_for_edit[0] == '0') break;
            fill_buffer_edit('0');
            SetWindowText(hEdit, buffer_for_edit);
            break;
        case(18): //button / divide           
            first_input = _tstof(buffer_for_edit);
            arithmetic_operation = 47; //47 is the code number of arithmetic operation button "divide" in UTF8
            memset(buffer_for_edit, NULL, sizeof(buffer_for_edit));
            flag = 1;
            break;
        case(19): // button = (calculate the total)
            /* 
                next odd "if else" operation is made just to process unusual user behavior when user, insted of pressing
                arithmetic buttons, starts to press buttons "=" many times in a row.  In sach a case calculator starts to
                execute last arithmetic operation with last second_input (2+3=5+3=8+3=11+3=14+3=17 and so on)
             */
            if (flag == 0)  
            {               
                second_input = temp;                   
                first_input = _tstof(buffer_for_edit); //this time the buffer_for_edit contain last result of the var "equals"
            }
            else
            {
                second_input = _tstof(buffer_for_edit); // get second user input and cast it to float
                temp = second_input; // save last second_input for using the previous "if(flag == 0)" 
            }
            memset(cast_buffer, 0, sizeof(cast_buffer)); // Clear buffer
            switch (arithmetic_operation) // all calculations for button = 
            {
            case(43)://calculation for plus               
                equals = first_input + second_input;
                StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), equals); // Cast equals from int to string                
                del_zero(); // Delete unnecessary zero chars at the end of the string of numbers
                SetWindowText(hEdit, cast_buffer); // output string of numbers to the window
                StringCbPrintf(buffer_for_edit, sizeof(buffer_for_edit), _T("%.15lf"), equals);             
                flag = 0;
                break;
            case(45): //calculation for minus
                equals = first_input - second_input;
                StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, cast_buffer);
                StringCbPrintf(buffer_for_edit, sizeof(buffer_for_edit), _T("%.15lf"), equals);
                flag = 0;
                break;
            case(42): //calculations for multiply
                equals = first_input * second_input;
                StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, cast_buffer);
                StringCbPrintf(buffer_for_edit, sizeof(buffer_for_edit), _T("%.15lf"), equals);
                flag = 0;
                break;
            case(47): //calculations for divide
                if (second_input == 0)
                {
                    MessageBox(NULL, _T("You can't divide by zero"), NULL, MB_ICONWARNING | MB_OK);
                    break;
                }                
                equals = first_input / second_input;
                StringCbPrintf(cast_buffer, sizeof(cast_buffer), _T("%.15lf"), equals);
                del_zero();
                SetWindowText(hEdit, cast_buffer);
                StringCbPrintf(buffer_for_edit, sizeof(buffer_for_edit), _T("%.15lf"), equals);
                flag = 0;                
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

    return 0;}



//The  fill_buffer_edit() function fill the buffer with the string of the user input numbers
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

//removes extra zeros after the floating point  - 5.4080000000
void del_zero(){
   

    for (int i = cast_buffer_size - 1; i > 0; i--)

    {                
        if (cast_buffer[i] == NULL) continue;

        if (cast_buffer[i] == '0') // We try to catch zero and change it to NULL
        {
            cast_buffer[i] = NULL;
            continue;
        }

        if (cast_buffer[i] == '.') // If we reach a floating point, we break the loop
        {
            cast_buffer[i] = NULL;
            break;
        }
        else               
            break;                 //If we reach any number (except of a zero), we break the loop
    }
    return;
}