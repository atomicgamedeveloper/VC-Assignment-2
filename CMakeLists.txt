cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Vcpkg toolchain
# -----------------------------
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# -----------------------------
# Dependencies
# -----------------------------
find_package(OpenCV REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# -----------------------------
# Directories and paths
# -----------------------------
set(CUSTOM_SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/customShaders")

# -----------------------------
# Source files
# -----------------------------
add_executable(MainExec
    src/main.cpp
    src/Filters.cpp
    src/InputHandler.cpp
    src/common/Camera.cpp
    src/common/Scene.cpp
    src/common/Object.cpp
    src/common/Quad.cpp
    src/common/Shader.cpp
    src/common/Texture.cpp
    src/common/TextureShader.cpp
)

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(MainExec PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -----------------------------
# Compile definitions
# -----------------------------
target_compile_definitions(MainExec PRIVATE
    GLM_ENABLE_EXPERIMENTAL
    SHADERS_DIR="${CUSTOM_SHADERS_DIR}/"
)

# -----------------------------
# Linking
# -----------------------------
target_link_libraries(MainExec PRIVATE
    glfw
    glad::glad
    glm::glm
    ${OpenCV_LIBS}
)

# -----------------------------
# Shader copy step
# -----------------------------
file(GLOB SHADER_FILES
    "${CUSTOM_SHADERS_DIR}/*.frag"
    "${CUSTOM_SHADERS_DIR}/*.vert"
)

add_custom_command(
    TARGET MainExec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER_FILES} $<TARGET_FILE_DIR:MainExec>
)
