// FileSearcher.cpp
// 멀티스레딩을 이용한 파일 검색기 예제 코드입니다.

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include "resource.h"
#include <new>
#include <stdio.h>

#pragma comment(lib, "shlwapi.lib")

constexpr UINT WM_APP_SEARCH_RESULT = WM_APP + 1;
constexpr UINT WM_APP_SEARCH_FINISHED = WM_APP + 2;

struct DialogState
{
    HANDLE searchThread;
    HANDLE cancelEvent;
    DWORD displayedResultCount;
    bool resultListError;
};

struct ThreadArgs
{
    HWND hDlg;
    HANDLE cancelEvent;
    WCHAR szPath[MAX_PATH];
    WCHAR szTerm[MAX_PATH];
};

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI SearchThread(LPVOID lpParam);
void SearchInDirectory(LPCWSTR szPath, LPCWSTR szTerm, HWND hDlg, HANDLE cancelEvent);
void EndSearchDialog(HWND hDlg, INT_PTR result);
void SaveLastPath(LPCWSTR szPath);
void LoadLastPath(HWND hEdit);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    INT_PTR result = DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DlgProc);
    return result == -1 ? 1 : 0;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        DialogState* state = new (std::nothrow) DialogState{ NULL, NULL, 0, false };
        if (state)
            state->cancelEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        if (!state || !state->cancelEvent)
        {
            if (state)
                delete state;
            MessageBoxW(hDlg, L"Could not initialize search state.", L"File Searcher", MB_OK | MB_ICONERROR);
            EndDialog(hDlg, 1);
            return TRUE;
        }

        SetWindowLongPtrW(hDlg, DWLP_USER, reinterpret_cast<LONG_PTR>(state));
        LoadLastPath(GetDlgItem(hDlg, IDC_EDIT_PATH));
        SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Enter a folder and search text.");
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_SEARCH)
        {
            DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
            if (!state || state->searchThread)
                return TRUE;

            ThreadArgs* args = new (std::nothrow) ThreadArgs{};
            if (!args)
            {
                SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Could not allocate search state.");
                return TRUE;
            }

            args->hDlg = hDlg;
            args->cancelEvent = state->cancelEvent;
            GetDlgItemTextW(hDlg, IDC_EDIT_PATH, args->szPath, ARRAYSIZE(args->szPath));
            GetDlgItemTextW(hDlg, IDC_EDIT_TERM, args->szTerm, ARRAYSIZE(args->szTerm));
            if (args->szPath[0] == L'\0' || args->szTerm[0] == L'\0')
            {
                delete args;
                SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Enter both a folder and search text.");
                return TRUE;
            }

            DWORD attributes = GetFileAttributesW(args->szPath);
            if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                delete args;
                SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"The search path is not an accessible folder.");
                return TRUE;
            }

            SendDlgItemMessageW(hDlg, IDC_LIST_RESULTS, LB_RESETCONTENT, 0, 0);
            state->displayedResultCount = 0;
            state->resultListError = false;
            ResetEvent(state->cancelEvent);
            SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Searching...");
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SEARCH), FALSE);

            state->searchThread = CreateThread(NULL, 0, SearchThread, args, 0, NULL);
            if (!state->searchThread)
            {
                DWORD error = GetLastError();
                delete args;
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SEARCH), TRUE);
                SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Could not start the search thread.");
                SetLastError(error);
                WCHAR messageText[256] = {};
                FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, messageText, ARRAYSIZE(messageText), NULL);
                MessageBoxW(hDlg, messageText, L"File Searcher", MB_OK | MB_ICONERROR);
            }
            return TRUE;
        }

        if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK)
        {
            EndSearchDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;

    case WM_APP_SEARCH_RESULT:
    {
        WCHAR* filePath = reinterpret_cast<WCHAR*>(lParam);
        if (filePath)
        {
            LRESULT added = SendDlgItemMessageW(hDlg, IDC_LIST_RESULTS, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(filePath));
            delete[] filePath;
            if (added == LB_ERR || added == LB_ERRSPACE)
            {
                DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
                if (state)
                    state->resultListError = true;
                SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, L"Could not add a result to the list.");
            }
            else
            {
                DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
                if (state)
                    ++state->displayedResultCount;
            }
        }
        return TRUE;
    }

    case WM_APP_SEARCH_FINISHED:
    {
        DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
        if (state && state->searchThread)
        {
            WaitForSingleObject(state->searchThread, INFINITE);
            CloseHandle(state->searchThread);
            state->searchThread = NULL;
            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SEARCH), TRUE);

            WCHAR status[128] = {};
            if (state->resultListError)
                swprintf_s(status, ARRAYSIZE(status), L"List error. %lu result(s) shown.", state->displayedResultCount);
            else if (wParam)
                swprintf_s(status, ARRAYSIZE(status), L"Search canceled. %lu result(s) shown.", state->displayedResultCount);
            else
                swprintf_s(status, ARRAYSIZE(status), L"Search complete. %lu result(s) shown.", state->displayedResultCount);
            SetDlgItemTextW(hDlg, IDC_STATIC_STATUS, status);
        }
        return TRUE;
    }

    case WM_CLOSE:
        EndSearchDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}

void EndSearchDialog(HWND hDlg, INT_PTR result)
{
    DialogState* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hDlg, DWLP_USER));
    if (state)
    {
        if (state->searchThread)
        {
            SetEvent(state->cancelEvent);
            WaitForSingleObject(state->searchThread, INFINITE);
            CloseHandle(state->searchThread);
        }

        MSG message;
        while (PeekMessageW(&message, hDlg, WM_APP_SEARCH_RESULT, WM_APP_SEARCH_RESULT, PM_REMOVE))
            delete[] reinterpret_cast<WCHAR*>(message.lParam);

        WCHAR path[MAX_PATH] = {};
        GetDlgItemTextW(hDlg, IDC_EDIT_PATH, path, ARRAYSIZE(path));
        SaveLastPath(path);
        CloseHandle(state->cancelEvent);
        delete state;
        SetWindowLongPtrW(hDlg, DWLP_USER, 0);
    }
    EndDialog(hDlg, result);
}

DWORD WINAPI SearchThread(LPVOID lpParam)
{
    ThreadArgs* args = static_cast<ThreadArgs*>(lpParam);
    SearchInDirectory(args->szPath, args->szTerm, args->hDlg, args->cancelEvent);
    WPARAM canceled = WaitForSingleObject(args->cancelEvent, 0) == WAIT_OBJECT_0;
    PostMessageW(args->hDlg, WM_APP_SEARCH_FINISHED, canceled, 0);
    delete args;
    return 0;
}

void SearchInDirectory(LPCWSTR szPath, LPCWSTR szTerm, HWND hDlg, HANDLE cancelEvent)
{
    if (WaitForSingleObject(cancelEvent, 0) == WAIT_OBJECT_0)
        return;

    WCHAR searchPath[MAX_PATH];
    if (FAILED(StringCchPrintfW(searchPath, ARRAYSIZE(searchPath), L"%s\\*", szPath)))
        return;

    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (WaitForSingleObject(cancelEvent, 0) == WAIT_OBJECT_0)
            break;
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
            continue;

        WCHAR fullPath[MAX_PATH];
        if (FAILED(StringCchPrintfW(fullPath, ARRAYSIZE(fullPath), L"%s\\%s", szPath, findData.cFileName)))
            continue;

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                SearchInDirectory(fullPath, szTerm, hDlg, cancelEvent);
            continue;
        }

        HANDLE hFile = CreateFileW(fullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
            continue;

        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize) && fileSize.QuadPart > 0 && fileSize.QuadPart < 10 * 1024 * 1024)
        {
            DWORD byteCount = static_cast<DWORD>(fileSize.QuadPart);
            char* buffer = new (std::nothrow) char[static_cast<size_t>(byteCount) + 1];
            if (buffer)
            {
                DWORD totalRead = 0;
                BOOL readSucceeded = TRUE;
                while (totalRead < byteCount && WaitForSingleObject(cancelEvent, 0) != WAIT_OBJECT_0)
                {
                    DWORD bytesRead = 0;
                    if (!ReadFile(hFile, buffer + totalRead, byteCount - totalRead, &bytesRead, NULL))
                    {
                        readSucceeded = FALSE;
                        break;
                    }
                    if (bytesRead == 0)
                        break;
                    totalRead += bytesRead;
                }

                if (readSucceeded && WaitForSingleObject(cancelEvent, 0) != WAIT_OBJECT_0)
                {
                    buffer[totalRead] = '\0';
                    char term[MAX_PATH] = {};
                    int termLength = WideCharToMultiByte(CP_ACP, 0, szTerm, -1, term, ARRAYSIZE(term), NULL, NULL);
                    if (termLength > 1 && StrStrIA(buffer, term))
                    {
                        size_t length = wcslen(fullPath) + 1;
                        WCHAR* resultPath = new (std::nothrow) WCHAR[length];
                        if (resultPath)
                        {
                            wcscpy_s(resultPath, length, fullPath);
                            if (!PostMessageW(hDlg, WM_APP_SEARCH_RESULT, 0, reinterpret_cast<LPARAM>(resultPath)))
                                delete[] resultPath;
                        }
                    }
                }
                delete[] buffer;
            }
        }
        CloseHandle(hFile);
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
}

void SaveLastPath(LPCWSTR szPath)
{
    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\MyFileSearcher", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, L"LastPath", 0, REG_SZ, reinterpret_cast<const BYTE*>(szPath), static_cast<DWORD>((wcslen(szPath) + 1) * sizeof(WCHAR)));
        RegCloseKey(hKey);
    }
}

void LoadLastPath(HWND hEdit)
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\MyFileSearcher", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        WCHAR path[MAX_PATH] = {};
        DWORD size = sizeof(path);
        if (RegQueryValueExW(hKey, L"LastPath", NULL, NULL, reinterpret_cast<LPBYTE>(path), &size) == ERROR_SUCCESS)
            SetWindowTextW(hEdit, path);
        RegCloseKey(hKey);
    }
}
