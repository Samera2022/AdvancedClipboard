#ifndef ADVANCEDCLIPBOARD_UPDATE_INFO_H
#define ADVANCEDCLIPBOARD_UPDATE_INFO_H

#include <string>
#include <vector>

// A struct to hold the data for a single update log entry.
// This mimics a Java enum entry with associated data.
struct UpdateInfo {
    std::wstring version;
    std::wstring releaseDate;
    std::wstring description;

    // A static function to get all log entries, similar to Java's `values()`.
    static const std::vector<UpdateInfo>& getAllLogs();

    // A helper function to get all logs formatted into a single string for display.
    static std::wstring getAllLogsAsString();
};

#endif //ADVANCEDCLIPBOARD_UPDATE_INFO_H
