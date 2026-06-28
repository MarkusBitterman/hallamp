cmake_minimum_required(VERSION 3.25)
project(hallamp VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Wasabi platform selector — bfc/platform/platform.h checks for LINUX, not __linux__
if(UNIX AND NOT APPLE)
  add_compile_definitions(LINUX)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

enable_testing()

add_subdirectory(Src/nu)
add_subdirectory(Src/pfc)
add_subdirectory(Src/Components/wac_network)
