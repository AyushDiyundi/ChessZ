# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.28)

# Project name and version
project(ChessZ VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the SFML directory path to the correct location
set(SFML_DIR "D:/ChessZ/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1")

# Set CMAKE_PREFIX_PATH to help CMake find SFML
set(CMAKE_PREFIX_PATH "${SFML_DIR}/lib/cmake/SFML")

# Find the SFML package and link the necessary components
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add the executable target
add_executable(ChessZ main.cpp)

# Link SFML libraries using the same signature
target_link_libraries(ChessZ PRIVATE sfml-graphics sfml-window sfml-system)

# Link against the Windows system libraries (optional but recommended for complete Windows integration)
if(WIN32)
    target_link_libraries(ChessZ PRIVATE
            user32
            gdi32
            winspool
            shell32
            ole32
            oleaut32
            uuid
            comdlg32
            advapi32
    )
endif()