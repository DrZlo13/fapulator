cmake_minimum_required(VERSION 3.1.0)

project("fapulator" LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/qt@5/5.15.6/")

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/app")
include_directories("${CMAKE_SOURCE_DIR}/fapulator")
include_directories("${CMAKE_SOURCE_DIR}/fapulator/flipper")
include_directories("${CMAKE_SOURCE_DIR}/fapulator/flipper/furi")
include_directories("${CMAKE_SOURCE_DIR}/fapulator/flipper/lib")
include_directories("${CMAKE_SOURCE_DIR}/fapulator/flipper/applications")
include_directories("${CMAKE_SOURCE_DIR}/fapulator/theseus")

file(GLOB_RECURSE CORE_SOURCES
    "app/*.c"
    "app/*.cpp"
    "fapulator/*.c"
    "fapulator/*.cpp"
)

add_executable("${PROJECT_NAME}" ${CORE_SOURCES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)