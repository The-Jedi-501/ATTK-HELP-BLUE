cmake_minimum_required(VERSION 3.16)
project(blueROV)

set(CMAKE_CXX_STANDARD 17)

# find SFML and GDAL
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
find_package(GDAL REQUIRED)

# all your source files
add_executable(blueROV
    blueROV.cpp
    src/mapCreation.cpp
    src/mapVisualizer.cpp
    src/spawnConfig.cpp
)

target_include_directories(blueROV PRIVATE src)

# link the libraries
target_link_libraries(blueROV
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
    GDAL::GDAL
)