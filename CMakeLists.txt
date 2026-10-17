cmake_minimum_required(VERSION 3.25)

project(moca
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

add_executable(moca
    src/main.cpp
)

target_include_directories(moca
    PRIVATE include
)

target_link_libraries(moca
    PRIVATE
        nlohmann_json::nlohmann_json
        cpr::cpr
)

target_compile_options(moca
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic>
)