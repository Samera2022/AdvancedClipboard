cmake_minimum_required(VERSION 3.10)

project(AdvancedClipboard)

set(CMAKE_CXX_STANDARD 20)

# Add definitions for Unicode support in Win32, which is the modern standard
add_compile_definitions(UNICODE _UNICODE)

# Define the executable target with all its source files, including the resource script.
# The WIN32 keyword is crucial for creating a GUI application without a console window.
add_executable(AdvancedClipboard WIN32
        src/main.cpp
        src/update_info.cpp
        src/resources.rc
        )

# Add the 'include' directory to the include path for all source files.
target_include_directories(AdvancedClipboard PUBLIC include)

# Link necessary system libraries
target_link_libraries(AdvancedClipboard PRIVATE user32 gdi32 shell32 ole32)

# Force a fully static link. This tells the linker to include all possible
# libraries directly into the final .exe file.
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Add the post-build command to run the PowerShell script.
add_custom_command(TARGET AdvancedClipboard POST_BUILD
        COMMAND powershell.exe
        ARGS -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/build_settings/post_build.ps1" -SourceExe "$<TARGET_FILE:AdvancedClipboard>"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Running post-build script to package application..."
        VERBATIM
)
