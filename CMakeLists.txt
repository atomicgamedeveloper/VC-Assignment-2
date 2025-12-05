cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

set(Stb_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/stb")

add_executable(Calibration
    src/Calibration.cpp
    ${COMMON_SOURCES} )

add_executable(makeCharucoBoard
    src/makeCharucoBoard.cpp
 )

add_executable(App
    src/App.cpp
 )

target_include_directories(makeCharucoBoard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(makeCharucoBoard PRIVATE
    ${OpenCV_LIBS}
)

foreach(target Calibration App)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Stb_INCLUDE_DIR}
    )

    target_link_libraries(${target} PRIVATE
        glfw
        glad::glad
        glm::glm
        ${OpenCV_LIBS}
    )
endforeach()