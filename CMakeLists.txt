cmake_minimum_required(VERSION 3.10)
project(HelloImGui)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/submodules/imgui")
set(SOL_DIR "${CMAKE_SOURCE_DIR}/submodules/sol2")
set(LUA_DIR "${CMAKE_SOURCE_DIR}/submodules/lua")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})

set(LUA_SOURCES
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/linit.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/ltests.c
    ${LUA_DIR}/lzio.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/ltm.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/lua.c
)
add_library(lua STATIC ${LUA_SOURCES})

include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SOL_DIR}/include
    ${LUA_DIR}
)

add_executable(HelloImGui main.cpp)
target_link_libraries(HelloImGui PRIVATE glfw OpenGL::GL imgui lua)