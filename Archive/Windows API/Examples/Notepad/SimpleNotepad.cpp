// SimpleNotepad (Win32 예제 3)
// SimpleNotepad.cpp
// Windows API를 사용하여 간단한 메모장 기능을 구현하는 예제 코드입니다.

#include <windows.h>
#include <windowsx.h>
#include "resource.h" // 리소스 헤더 파일 포함
#include <algorithm>
#include <limits>
#include <stdio.h>
#include <vector>

// 전역 변수 선언
HINSTANCE hInst;
WCHAR szTitle[] = L"Simple Notepad";
WCHAR szWindowClass[] = L"MyNotepadWindowClass";
HWND hEdit; // 에디트 컨트롤의 핸들을 저장할 전역 변수

// 함수 선언
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                DoFileOpen(HWND hWnd);
void                DoFileSave(HWND hWnd);
void                ShowCommonDialogError(HWND hWnd);
void                ShowFileError(HWND hWnd, LPCWSTR action);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDR_MENU); // 리소스에서 메뉴를 로드
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

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
    switch (message)
    {
    case WM_CREATE: // 윈도우가 처음 생성될 때
        {
            // 에디트 컨트롤 생성
            hEdit = CreateWindowEx(
                0, L"EDIT",   // 클래스 이름
                L"",          // 초기 텍스트
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0, 0, 0, 0,   // 크기와 위치는 WM_SIZE에서 조정
                hWnd,         // 부모 윈도우
                (HMENU)100,   // 컨트롤 ID
                hInst,
                NULL);
            if (hEdit)
            {
                SendMessage(hEdit, EM_SETLIMITTEXT, 0x7FFFFFFE, 0);
            }
            else
            {
                return -1;
            }
        }
        break;

    case WM_SIZE: // 윈도우 크기가 변경될 때
        {
            // 에디트 컨트롤의 크기를 클라이언트 영역에 꽉 차게 조절
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            MoveWindow(hEdit, 0, 0, rcClient.right, rcClient.bottom, TRUE);
        }
        break;

    case WM_COMMAND: // 메뉴나 컨트롤로부터 메시지가 올 때
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_FILE_OPEN:
                DoFileOpen(hWnd);
                break;
            case IDM_FILE_SAVE:
                DoFileSave(hWnd);
                break;
            case IDM_FILE_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_HELP_ABOUT:
                MessageBoxW(hWnd, L"Simple Notepad\nWin32 controls, resources, and UTF-8 file I/O example.", szTitle, MB_OK | MB_ICONINFORMATION);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
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

void DoFileOpen(HWND hWnd)
{
    OPENFILENAMEW ofn = {};
    WCHAR szFile[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            ShowFileError(hWnd, L"open");
            return;
        }

        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(hFile, &fileSize))
        {
            DWORD error = GetLastError();
            CloseHandle(hFile);
            SetLastError(error);
            ShowFileError(hWnd, L"read");
            return;
        }
        if (fileSize.QuadPart < 0 ||
            static_cast<unsigned long long>(fileSize.QuadPart) > (std::numeric_limits<size_t>::max)() ||
            fileSize.QuadPart > (std::numeric_limits<int>::max)())
        {
            CloseHandle(hFile);
            MessageBoxW(hWnd, L"The selected file is too large to display.", L"Simple Notepad", MB_OK | MB_ICONWARNING);
            return;
        }

        std::vector<char> bytes(static_cast<size_t>(fileSize.QuadPart));
        size_t totalRead = 0;
        while (totalRead < bytes.size())
        {
            DWORD bytesRead = 0;
            DWORD remaining = static_cast<DWORD>((std::min)(bytes.size() - totalRead, static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
            if (!ReadFile(hFile, bytes.data() + totalRead, remaining, &bytesRead, NULL))
            {
                DWORD error = GetLastError();
                CloseHandle(hFile);
                SetLastError(error);
                ShowFileError(hWnd, L"read");
                return;
            }
            if (bytesRead == 0)
            {
                CloseHandle(hFile);
                SetLastError(ERROR_HANDLE_EOF);
                ShowFileError(hWnd, L"read");
                return;
            }
            totalRead += bytesRead;
        }
        CloseHandle(hFile);
        bytes.resize(totalRead);

        size_t offset = bytes.size() >= 3 &&
            static_cast<unsigned char>(bytes[0]) == 0xEF &&
            static_cast<unsigned char>(bytes[1]) == 0xBB &&
            static_cast<unsigned char>(bytes[2]) == 0xBF ? 3 : 0;
        int byteCount = static_cast<int>(bytes.size() - offset);
        if (std::find(bytes.begin() + offset, bytes.end(), '\0') != bytes.end())
        {
            MessageBoxW(hWnd, L"The selected file contains binary data and cannot be displayed as text.", L"Simple Notepad", MB_OK | MB_ICONWARNING);
            return;
        }

        int wideCharCount = byteCount == 0 ? 0 : MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data() + offset, byteCount, NULL, 0);
        if (byteCount != 0 && wideCharCount == 0)
        {
            MessageBoxW(hWnd, L"The selected file is not valid UTF-8 text.", L"Simple Notepad", MB_OK | MB_ICONWARNING);
            return;
        }

        std::vector<WCHAR> text(static_cast<size_t>(wideCharCount) + 1, L'\0');
        if (wideCharCount != 0 &&
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data() + offset, byteCount, text.data(), wideCharCount) == 0)
        {
            ShowFileError(hWnd, L"convert");
            return;
        }
        if (!SetWindowTextW(hEdit, text.data()))
            ShowFileError(hWnd, L"display");
    }
    else
    {
        ShowCommonDialogError(hWnd);
    }
}

void DoFileSave(HWND hWnd)
{
    OPENFILENAMEW ofn = {};
    WCHAR szFile[MAX_PATH] = {};

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = ARRAYSIZE(szFile);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn) == TRUE)
    {
        int textLength = GetWindowTextLengthW(hEdit);
        std::vector<WCHAR> text(static_cast<size_t>(textLength) + 1, L'\0');
        int copied = GetWindowTextW(hEdit, text.data(), textLength + 1);
        if (copied != textLength)
        {
            ShowFileError(hWnd, L"read editor text");
            return;
        }

        int byteCount = copied == 0 ? 0 : WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), copied, NULL, 0, NULL, NULL);
        if (copied != 0 && byteCount == 0)
        {
            ShowFileError(hWnd, L"convert");
            return;
        }

        std::vector<char> bytes(static_cast<size_t>(byteCount));
        if (byteCount != 0 &&
            WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), copied, bytes.data(), byteCount, NULL, NULL) == 0)
        {
            ShowFileError(hWnd, L"convert");
            return;
        }

        HANDLE hFile = CreateFileW(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            ShowFileError(hWnd, L"create");
            return;
        }

        const BYTE utf8Bom[] = { 0xEF, 0xBB, 0xBF };
        const char* data = bytes.data();
        size_t dataSize = bytes.size();
        bool writeSucceeded = true;
        DWORD writeError = ERROR_SUCCESS;
        for (int part = 0; part < 2 && writeSucceeded; ++part)
        {
            if (part == 0)
            {
                data = reinterpret_cast<const char*>(utf8Bom);
                dataSize = sizeof(utf8Bom);
            }
            else
            {
                data = bytes.data();
                dataSize = bytes.size();
            }

            size_t writtenTotal = 0;
            while (writtenTotal < dataSize)
            {
                DWORD bytesWritten = 0;
                DWORD remaining = static_cast<DWORD>((std::min)(dataSize - writtenTotal, static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
                if (!WriteFile(hFile, data + writtenTotal, remaining, &bytesWritten, NULL))
                {
                    writeError = GetLastError();
                    writeSucceeded = false;
                    break;
                }
                if (bytesWritten == 0)
                {
                    writeError = ERROR_WRITE_FAULT;
                    writeSucceeded = false;
                    break;
                }
                writtenTotal += bytesWritten;
            }
        }
        CloseHandle(hFile);
        if (!writeSucceeded)
        {
            SetLastError(writeError);
            ShowFileError(hWnd, L"write");
        }
    }
    else
    {
        ShowCommonDialogError(hWnd);
    }
}

void ShowCommonDialogError(HWND hWnd)
{
    DWORD error = CommDlgExtendedError();
    if (error != 0)
    {
        WCHAR message[128] = {};
        swprintf_s(message, L"The file dialog failed (error 0x%08lX).", error);
        MessageBoxW(hWnd, message, L"Simple Notepad", MB_OK | MB_ICONERROR);
    }
}

void ShowFileError(HWND hWnd, LPCWSTR action)
{
    DWORD error = GetLastError();
    WCHAR message[512] = {};
    WCHAR details[384] = {};
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, details, ARRAYSIZE(details), NULL);
    swprintf_s(message, L"Could not %s the file.\n%s", action, details);
    MessageBoxW(hWnd, message, L"Simple Notepad", MB_OK | MB_ICONERROR);
}
