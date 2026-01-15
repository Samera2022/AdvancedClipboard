#ifndef ADVANCEDCLIPBOARD_UPDATE_INFO_H
#define ADVANCEDCLIPBOARD_UPDATE_INFO_H

#include <string>
#include <vector>
#include <optional> // Include for std::optional

// A struct to hold the data for a single update log entry.
struct UpdateInfo {
    std::wstring version;
    std::wstring releaseDate;
    std::wstring description;

    // A new member function to format this specific log entry as a string.
    std::wstring toString() const;

    // A static function to get all log entries.
    static const std::vector<UpdateInfo>& getAllLogs();

    // A helper function to get all logs formatted into a single string for display.
    static std::wstring getAllLogsAsString();

    // A helper function to get only the latest log entry as a formatted string.
    static std::wstring getLatestLogAsString();

    // A function to get a specific log entry by its index.
    // Returns an empty optional if the index is out of bounds.
    static std::optional<UpdateInfo> getLogByIndex(size_t index);
};

#endif //ADVANCEDCLIPBOARD_UPDATE_INFO_H
