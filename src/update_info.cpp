#include "../include/update_info.h"
#include <sstream> // For wstringstream
#include <vector>

const std::vector<UpdateInfo>& UpdateInfo::getAllLogs() {
    // This static vector will be initialized only once, the first time this function is called.
    // This is a thread-safe way to handle static initialization in C++11 and later.
    static const std::vector<UpdateInfo> all_logs = {
        {L"0.0.1", L"2026-01-15 14:47",
         L"## [Added]\n"
         L" - 实现剪贴板历史记录功能 (文本和图片)。\n"
         L" - 实现系统托盘图标及退出菜单。\n"
         L" - 实现全局热键 (Ctrl+Up/Down) 弹出历史窗口。\n"
         L" - 实现从历史窗口选择并恢复到剪贴板的功能。"
        }
        // You can add more entries here, following the same format.
    };
    return all_logs;
}

std::wstring UpdateInfo::getAllLogsAsString() {
    std::wstringstream wss;
    const auto& all_logs = getAllLogs();

    // Iterate through the logs in reverse order to show the newest first.
    for (auto it = all_logs.rbegin(); it != all_logs.rend(); ++it) {
        wss << L"版本 " << it->version << L" [" << it->releaseDate << L"]\n";
        wss << it->description << L"\n\n";
    }

    return wss.str();
}
