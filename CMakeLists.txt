cmake_minimum_required(VERSION 3.10)

project(AdvancedClipboard)

set(CMAKE_CXX_STANDARD 20)

# Add definitions for Unicode support in Win32, which is the modern standard
add_compile_definitions(UNICODE _UNICODE)

# --- Main GUI Application Target ---
add_executable(AdvancedClipboard WIN32
        src/main.cpp
        src/update_info.cpp
        src/resources.rc
        )
target_include_directories(AdvancedClipboard PUBLIC include)
target_link_libraries(AdvancedClipboard PRIVATE user32 gdi32 shell32 ole32)

if(CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(AdvancedClipboard PROPERTIES LINK_FLAGS "-static")
endif()

add_custom_command(TARGET AdvancedClipboard POST_BUILD
        COMMAND powershell.exe
        ARGS -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/build_settings/post_build.ps1" -SourceExe "$<TARGET_FILE:AdvancedClipboard>"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running post-build script to package application..."
        VERBATIM
)

# --- Separate Test Executable for update_info.cpp ---
# This creates a small CONSOLE application just for testing purposes.
add_executable(TestUpdateInfo
        src/update_info.cpp
        )
# It also needs to find the header files.
target_include_directories(TestUpdateInfo PUBLIC include)
# We define the UPDATE_INFO_MAIN macro to activate the main() function inside update_info.cpp
target_compile_definitions(TestUpdateInfo PRIVATE UPDATE_INFO_MAIN)
