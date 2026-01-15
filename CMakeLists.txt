cmake_minimum_required(VERSION 3.10)

project(AdvancedClipboard)

set(CMAKE_CXX_STANDARD 20)

# Add definitions for Unicode support in Win32, which is the modern standard
add_compile_definitions(UNICODE _UNICODE)

# Ensure only the correct main.cpp is being built
add_executable(AdvancedClipboard main.cpp)

# Link the necessary Windows libraries
target_link_libraries(AdvancedClipboard PRIVATE user32 gdi32 shell32 ole32)
