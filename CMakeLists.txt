cmake_minimum_required(VERSION 3.10)
project(HelloImGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0072 NEW)

# GLFW und OpenGL
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# ImGui Pfad
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/submodules/imgui")

# ImGui Quellen
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImGui Header
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Dein Source-Code
add_executable(HelloImGui main.cpp ${IMGUI_SOURCES})

# Link mit GLFW und OpenGL
target_link_libraries(HelloImGui PRIVATE glfw OpenGL::GL)