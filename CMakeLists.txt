cmake_minimum_required(VERSION 3.16)
project(MyProject)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

find_package(OpenCV REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
add_executable(MainExec
    main.cpp
    common/Shader.cpp
    common/Camera.cpp
    common/Scene.cpp
    common/Object.cpp
    common/Quad.cpp
    common/Texture.cpp
    common/TextureShader.cpp
)
target_include_directories(MainExec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(MainExec PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(MainExec PRIVATE
    glfw
    glad::glad
    glm::glm
    ${OpenCV_LIBS}
)

# ---------- simpleWindow executable ----------

add_executable(simpleWindow "OpenGLIntro/OpenGLIntro/Part01/simpleWindow.cpp")
target_link_libraries(simpleWindow PRIVATE 
    glfw
    glad::glad
    glm::glm
)

# ---------- redTriangle executable ----------
add_executable(redTriangle
    OpenGLIntro/OpenGLIntro/Part02/redTriangle.cpp
    common/Shader.cpp
    common/Camera.cpp
    common/Scene.cpp
    common/Object.cpp
    common/Triangle.cpp
    common/Quad.cpp
    common/ColorShader.cpp
)

target_include_directories(redTriangle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# ---- fix the GLM experimental error ----
target_compile_definitions(redTriangle PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(redTriangle PRIVATE glfw glad::glad glm::glm)


# ---------- list of shader data files ----------
set(SHADERS
    OpenGLIntro/OpenGLIntro/Part02/simpleColor.vert
    OpenGLIntro/OpenGLIntro/Part02/simpleColor.frag
    OpenGLIntro/OpenGLIntro/Part02/basicShader.vert
    OpenGLIntro/OpenGLIntro/Part02/basicShader.frag
)

# forward slashes are safe on every platform
target_compile_definitions(redTriangle PRIVATE
    SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/OpenGLIntro/OpenGLIntro/Part02/")






# ---------- webCamQuad executable ----------
find_package(OpenCV REQUIRED)       

add_executable(webcamQuad
    OpenGLIntro/OpenGLIntro/Part03/webcamQuad.cpp
    common/Shader.cpp
    common/Camera.cpp
    common/Scene.cpp
    common/Object.cpp
    common/Quad.cpp
    common/Texture.cpp
    common/TextureShader.cpp
)

target_include_directories(webcamQuad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(webcamQuad PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(webcamQuad PRIVATE
    glfw
    glad::glad
    glm::glm
    ${OpenCV_LIBS}
)

set(P3_SHADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLIntro/OpenGLIntro/Part03/videoTextureShader.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLIntro/OpenGLIntro/Part03/videoTextureShader.frag
)
add_custom_command(TARGET webcamQuad POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${P3_SHADERS} $<TARGET_FILE_DIR:webcamQuad>)