#include "RandW.h"

int handle_file(LPCWSTR file_name, INT8 mode);
int file_compress(HANDLE hfile_read, HANDLE hfile_write, huffman_code huffman_table[256]);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            // Check which control sent the message
            if (LOWORD(wParam) == 2 && HIWORD(wParam) == BN_CLICKED) {
                OPENFILENAME ofn;
                wchar_t szFile[260] = {0};

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
                ofn.lpstrFilter = L"All Files\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn) == TRUE)
                {
                    HWND hEdit = GetDlgItem(hwnd, 3);
                    SetWindowText(hEdit, ofn.lpstrFile);
                }
            }
            else if (LOWORD(wParam) == 4 && HIWORD(wParam) == BN_CLICKED) {
                HWND hEdit = GetDlgItem(hwnd, 3);
                wchar_t filePath[260];
                GetWindowText(hEdit, filePath, 260);

                if (wcslen(filePath) > 0) {
                    wchar_t outFilePath[300];
                    wcscpy(outFilePath, filePath);
                    wcscat(outFilePath, L".huff");

                    int result = handle_file(filePath, COMPRESS);
                    if (result == 0) {
                        MessageBox(hwnd, L"文件压缩成功!", L"成功", MB_OK);
                    } else {
                        MessageBox(hwnd, L"文件压缩失败!", L"错误", MB_ICONERROR | MB_OK);
                    }
                } else {
                    MessageBox(hwnd, L"请先选择一个文件。", L"提示", MB_OK);
                }
            }
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
            EndPaint(hwnd, &ps);
        }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default: return DefWindowProc(hwnd, uMsg, wParam, lParam);     
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR    lpCmdLine,
                   int       nCmdShow)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // 添加这一行
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);  // 可选，设置背景颜色

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        L"MyWindowClass",
        L"Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HWND selectControl = CreateWindowEx(
        0,
        L"BUTTON",
        L"选择文件",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 50, 90, 30,
        hwnd,
        (HMENU)2,
        hInstance,
        NULL
    );

    HWND compressButton = CreateWindowEx(
        0,
        L"BUTTON",
        L"压缩",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        10, 130, 90, 30,
        hwnd,
        (HMENU)4,
        hInstance,
        NULL
    );

    HWND hEdit = CreateWindowEx(
        0, L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        10, 90, 200, 30,
        hwnd,
        (HMENU)3, // Control ID
        hInstance,
        NULL
    );

    if (hEdit == NULL) {
        MessageBox(NULL, L"Edit Control Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (selectControl == NULL) {
        MessageBox(NULL, L"ComboBox Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (compressButton == NULL) {
        MessageBox(NULL, L"Compress Button Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);


    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}