cmake_minimum_required(VERSION 3.24.0)
project(BlueBall VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 QUIET CONFIG)
if(NOT SDL3_FOUND)
    message(WARNING "SDL3 package not found. Attempting to fetch it from the repository...")
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.12
    )
    FetchContent_MakeAvailable(SDL3)
endif()

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    "src/HelloGraphicsApplication.cpp"
)

target_include_directories(${PROJECT_NAME}
    PUBLIC "include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
)

if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy
        $<TARGET_FILE:SDL3::SDL3-shared>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        VERBATIM
    )
endif()
