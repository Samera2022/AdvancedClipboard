#include "../include/update_info.h"
#include <sstream> // For wstringstream
#include <vector>

// Implementation of the new toString() member function
std::wstring UpdateInfo::toString() const {
    std::wstringstream wss;
    wss << L"[" << this->releaseDate << L"] " << this->version << L"\n\n";
    wss << this->description;
    return wss.str();
}

const std::vector<UpdateInfo>& UpdateInfo::getAllLogs() {
    // This static vector will be initialized only once, the first time this function is called.
    // The newest log should be placed at the top of this list.
    static const std::vector<UpdateInfo> all_logs = {
        {L"0.1.0", L"2026-01-15 15:30",
         L"## [Added]\n"
         L" - 实现剪贴板历史记录功能 (文本和图片)。\n"
         L" - 实现系统托盘图标及退出菜单。\n"
         L" - 实现全局热键 (Ctrl+Up/Down) 弹出历史窗口。\n"
         L" - 实现从历史窗口选择并恢复到剪贴板的功能。"
        },
    {L"0.2.0", L"2026-01-16 23:42",
        L"## [Added]\n"
        L" - 添加鼠标左键托盘图标呼出UI界面的功能。\n"
        L" - 实现回车确认UI选中项的功能。\n"
        L" - 托盘右键菜单添加更新日志条目。\n"
        L" - 实现从历史窗口选择并恢复到剪贴板的功能。"
        }
    };
    return all_logs;
}

std::wstring UpdateInfo::getAllLogsAsString() {
    std::wstringstream wss;
    const auto& all_logs = getAllLogs();
    for (const auto& log : all_logs) {
        wss << log.toString() << L"\n\n"; // Now uses the toString() method
    }
    return wss.str();
}

std::wstring UpdateInfo::getLatestLogAsString() {
    const auto& all_logs = getAllLogs();
    if (all_logs.empty()) {
        return L"没有可用的更新日志。";
    }
    return all_logs.front().toString(); // Now uses the toString() method
}

std::optional<UpdateInfo> UpdateInfo::getLogByIndex(size_t index) {
    const auto& all_logs = getAllLogs();
    if (index < all_logs.size()) {
        return all_logs[index];
    }
    return std::nullopt;
}

// --- Main entry point for testing this class ---
#ifdef UPDATE_INFO_MAIN
#include <iostream>
#include <io.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    // Set console to output Unicode (UTF-16) to correctly display Chinese characters.
    _setmode(_fileno(stdout), _O_U16TEXT);
    // Here we call the new toString() method on the returned object.
    std::wcout << UpdateInfo::getLogByIndex(0)->toString() << L"\n\n";
    return 0;
}
#endif // UPDATE_INFO_MAIN
