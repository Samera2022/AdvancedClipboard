#include <windows.h>
#include <vector>
#include <string>
#include <variant>
#include <algorithm>
#include <shellapi.h> // For tray icon
#include "update_info.h"
#include "resource.h" // Include resource definitions

// --- Global Variables & Constants ---
#define MAX_CLIPBOARD_HISTORY 200
#define WM_TRAYICON (WM_USER + 1)
#define HOTKEY_ID_UP 1
#define HOTKEY_ID_DOWN 2
#define ID_MENU_EXIT 2
#define ID_MENU_LOG 3

HINSTANCE g_hInst;
HWND g_hWnd;
HWND g_hListBox;

std::vector<std::variant<std::wstring, HBITMAP>> g_clipboardHistory;
bool g_isRestoringClipboard = false; // Flag to ignore self-triggered clipboard updates

// --- Function Prototypes ---
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
void CreateTrayIcon(HWND hWnd);
void RegisterHotKeys(HWND hWnd);
void AddToClipboardHistory();
void UpdateHistoryListbox();
void LoadItemToClipboard(int selectedIndex);
void ShowPopupWindow();

// --- WinMain: Application Entry Point ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance;
    const wchar_t CLASS_NAME[] = L"AdvancedClipboardClass";

    WNDCLASSW wc = {}; // Use WNDCLASSW for Unicode
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON)); // Set the class icon

    RegisterClassW(&wc); // Use RegisterClassW for Unicode

    // Create the main window (initially hidden)
    g_hWnd = CreateWindowExW( // Use CreateWindowExW for Unicode
            0, CLASS_NAME, L"Advanced Clipboard",
            WS_POPUP | WS_BORDER,
            0, 0, 300, 400,
            NULL, NULL, hInstance, NULL
    );

    if (g_hWnd == NULL) {
        return 0;
    }

    // Start listening to clipboard changes
    AddClipboardFormatListener(g_hWnd);

    CreateTrayIcon(g_hWnd);
    RegisterHotKeys(g_hWnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            if (GetFocus() == g_hListBox) {
                int selectedIndex = SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR) {
                    LoadItemToClipboard(selectedIndex);
                    ShowWindow(g_hWnd, SW_HIDE);
                }
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    RemoveClipboardFormatListener(g_hWnd);
    return 0;
}

// --- WndProc: Window Procedure ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            // Create the list box to show history
            g_hListBox = CreateWindowW(L"LISTBOX", NULL,
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                                       0, 0, 284, 400, // Adjust size for border
                                       hWnd, (HMENU)1, g_hInst, NULL);
            break;

        case WM_CLIPBOARDUPDATE:
            if (g_isRestoringClipboard) {
                g_isRestoringClipboard = false; // Reset the flag and ignore this update
                break;
            }
            AddToClipboardHistory();
            UpdateHistoryListbox();
            break;

        case WM_HOTKEY:
            if (wParam == HOTKEY_ID_UP || wParam == HOTKEY_ID_DOWN) {
                ShowPopupWindow();
                int current_sel = SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
                if (current_sel == LB_ERR) current_sel = 0;

                if (wParam == HOTKEY_ID_UP) {
                    current_sel = (current_sel > 0) ? current_sel - 1 : SendMessage(g_hListBox, LB_GETCOUNT, 0, 0) - 1;
                } else { // HOTKEY_ID_DOWN
                    current_sel = (current_sel < SendMessage(g_hListBox, LB_GETCOUNT, 0, 0) - 1) ? current_sel + 1 : 0;
                }
                SendMessage(g_hListBox, LB_SETCURSEL, current_sel, 0);
            }
            break;

        case WM_KEYDOWN:
            if (wParam == VK_RETURN) {
                int selectedIndex = SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
                if (selectedIndex != LB_ERR) {
                    LoadItemToClipboard(selectedIndex);
                    ShowWindow(hWnd, SW_HIDE);
                }
            } else if (wParam == VK_ESCAPE) {
                ShowWindow(hWnd, SW_HIDE);
            }
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case 1: // Listbox command
                    if (HIWORD(wParam) == LBN_DBLCLK) { // Double click on listbox item
                         int selectedIndex = SendMessage(g_hListBox, LB_GETCURSEL, 0, 0);
                         if (selectedIndex != LB_ERR) {
                            LoadItemToClipboard(selectedIndex);
                            ShowWindow(hWnd, SW_HIDE);
                         }
                    }
                    break;
                case ID_MENU_EXIT: // Exit command
                    DestroyWindow(hWnd);
                    break;
                case ID_MENU_LOG: // Update Log command
                    {
                        std::wstring log_content = UpdateInfo::getAllLogsAsString();
                        MessageBoxW(hWnd, log_content.c_str(), L"更新日志", MB_OK | MB_ICONINFORMATION);
                    }
                    break;
                case ID_MENU_ABOUT:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlgProc);
                    break;
            }
            break;

        case WM_TRAYICON:
            if (lParam == WM_LBUTTONUP) {
                ShowPopupWindow();
            } else if (lParam == WM_RBUTTONUP) {
                POINT curPoint;
                GetCursorPos(&curPoint);
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, ID_MENU_ABOUT, L"关于作者");
                AppendMenuW(hMenu, MF_STRING, ID_MENU_LOG, L"更新日志");
                AppendMenuW(hMenu, MF_STRING, ID_MENU_EXIT, L"Exit");
                SetForegroundWindow(hWnd); // Necessary to make the menu disappear correctly
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, curPoint.x, curPoint.y, 0, hWnd, NULL);
                DestroyMenu(hMenu);
            }
            break;

        case WM_ACTIVATE:
             // If window loses focus, hide it
            if (wParam == WA_INACTIVE) {
                ShowWindow(hWnd, SW_HIDE);
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, (NOTIFYICONDATAW*)GetWindowLongPtrW(hWnd, GWLP_USERDATA));
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hWnd, message, wParam, lParam); // Use DefWindowProcW for Unicode
    }
    return 0;
}

// --- About Dialog Procedure ---
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_INITDIALOG:
        {
            SetWindowTextW(hDlg, L"关于作者");
            const wchar_t* aboutText = L"你好，我是AdvancedClipboard的作者Samera2022。首先感谢您的使用，谨在此致以最为诚挚的欢迎。\n"
                                     L"你可以通过B站UID: 583460263 / QQ: 3517085924来找到我来反馈各类使用问题，当然闲聊之类的也是肯定可以的！不过反馈使用问题还是建议在Github提交Issues，这样我能更及时看得到。\n"
                                     L"本项目的地址为https://github.com/Samera2022/AdvancedClipboard，如果你觉得本项目比较有趣的话还请不要吝啬你的star啦！";
            SetDlgItemTextW(hDlg, IDC_ABOUT_TEXT, aboutText);
            return (INT_PTR)TRUE;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDC_GITHUB_BUTTON) {
                ShellExecuteW(NULL, L"open", L"https://github.com/Samera2022/AdvancedClipboard", NULL, NULL, SW_SHOWNORMAL);
            }
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}


// --- Feature Implementations ---

void CreateTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW nid = {}; // Use NOTIFYICONDATAW for Unicode
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    // Load the icon from the application's resources
    nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_APPICON));
    wcscpy_s(nid.szTip, L"Advanced Clipboard");
    Shell_NotifyIconW(NIM_ADD, &nid); // Use Shell_NotifyIconW for Unicode
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)&nid);
}

void RegisterHotKeys(HWND hWnd) {
    // Register Ctrl + Up Arrow
    RegisterHotKey(hWnd, HOTKEY_ID_UP, MOD_CONTROL, VK_UP);
    // Register Ctrl + Down Arrow
    RegisterHotKey(hWnd, HOTKEY_ID_DOWN, MOD_CONTROL, VK_DOWN);
}

void AddToClipboardHistory() {
    if (!OpenClipboard(g_hWnd)) return;

    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* text = static_cast<wchar_t*>(GlobalLock(hData));
            if (text) {
                if (g_clipboardHistory.empty() || !std::holds_alternative<std::wstring>(g_clipboardHistory[0]) || std::get<std::wstring>(g_clipboardHistory[0]) != text) {
                    g_clipboardHistory.insert(g_clipboardHistory.begin(), std::wstring(text));
                }
                GlobalUnlock(hData);
            }
        }
    } else if (IsClipboardFormatAvailable(CF_BITMAP)) {
        HBITMAP hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);
        if (hBitmap) {
             HBITMAP copiedBitmap = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
             if (copiedBitmap) {
                g_clipboardHistory.insert(g_clipboardHistory.begin(), copiedBitmap);
             }
        }
    }
    // TODO: Add support for files (CF_HDROP)

    CloseClipboard();

    // Trim history if it exceeds the max size
    if (g_clipboardHistory.size() > MAX_CLIPBOARD_HISTORY) {
        // If the last item is a bitmap, it needs to be deleted to avoid memory leaks
        if(std::holds_alternative<HBITMAP>(g_clipboardHistory.back())) {
            DeleteObject(std::get<HBITMAP>(g_clipboardHistory.back()));
        }
        g_clipboardHistory.pop_back();
    }
}

void UpdateHistoryListbox() {
    SendMessage(g_hListBox, LB_RESETCONTENT, 0, 0); // Clear the listbox
    for (const auto& item : g_clipboardHistory) {
        if (std::holds_alternative<std::wstring>(item)) {
            SendMessageW(g_hListBox, LB_ADDSTRING, 0, (LPARAM)std::get<std::wstring>(item).c_str());
        } else if (std::holds_alternative<HBITMAP>(item)) {
            SendMessageW(g_hListBox, LB_ADDSTRING, 0, (LPARAM)L"[Image]");
        }
    }
}

void LoadItemToClipboard(int selectedIndex) {
    if (selectedIndex < 0 || selectedIndex >= g_clipboardHistory.size()) return;

    // 1. Get the item and remove it from its current position
    auto item = g_clipboardHistory[selectedIndex];
    g_clipboardHistory.erase(g_clipboardHistory.begin() + selectedIndex);

    // 2. Insert the item at the beginning of the history (making it the most recent)
    g_clipboardHistory.insert(g_clipboardHistory.begin(), item);

    // 3. Set the flag to ignore the upcoming clipboard update that we are about to cause
    g_isRestoringClipboard = true;

    // 4. Open clipboard and set the data
    if (!OpenClipboard(g_hWnd)) {
        g_isRestoringClipboard = false; // Reset flag on failure
        return;
    }
    EmptyClipboard();

    if (std::holds_alternative<std::wstring>(item)) {
        const std::wstring& text = std::get<std::wstring>(item);
        HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
        if (!hg) {
            CloseClipboard();
            g_isRestoringClipboard = false; // Reset flag on failure
            return;
        }
        memcpy(GlobalLock(hg), text.c_str(), (text.length() + 1) * sizeof(wchar_t));
        GlobalUnlock(hg);
        SetClipboardData(CF_UNICODETEXT, hg);
    } else if (std::holds_alternative<HBITMAP>(item)) {
        HBITMAP hBitmap = std::get<HBITMAP>(item);
        HBITMAP hCopiedBitmap = (HBITMAP)CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
        if(hCopiedBitmap) {
            SetClipboardData(CF_BITMAP, hCopiedBitmap);
        }
    }

    CloseClipboard();

    // 5. Update the listbox UI to reflect the new order
    UpdateHistoryListbox();
    // The WM_CLIPBOARDUPDATE message will be processed after this, but our flag will make it do nothing.
}

void ShowPopupWindow() {
    // Position the window at the top center of the screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int windowWidth = 300;
    int x = (screenWidth - windowWidth) / 2;
    SetWindowPos(g_hWnd, HWND_TOPMOST, x, 0, windowWidth, 400, SWP_SHOWWINDOW);
    SetForegroundWindow(g_hWnd);
    SetActiveWindow(g_hWnd);
    SetFocus(g_hListBox); // Set focus to the list box for keyboard navigation
}
