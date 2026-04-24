cmake_minimum_required(VERSION 3.10)
project(i7ed)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

set(SUBMOD "${CMAKE_SOURCE_DIR}/submodules")
set(IMGUI_DIR "${SUBMOD}/imgui")
set(IMGUI_FILE_DIALOG_DIR "${SUBMOD}/ImGuiFileDialog")
set(SOL_DIR "${SUBMOD}/sol2")
set(LUA_DIR "${SUBMOD}/lua")
set(RTMIDI_DIR "${SUBMOD}/rtmidi")
set(RTMIDI_BUILD_STATIC_LIBS ON CACHE BOOL "Build RtMidi as static library")

set(LUA_SRC "${CMAKE_SOURCE_DIR}/lua")
set(LUA_SRC_DST "${CMAKE_BINARY_DIR}/lua")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    # third_party
    ${SUBMOD}/imgui-knobs/imgui-knobs.cpp
    ${SUBMOD}/imgui-command-palette/imcmd_command_palette.cpp #no submodule for purpose because of build errors in the outdated origin
    ${SUBMOD}/imgui-command-palette/imcmd_fuzzy_search.cpp
    ${SUBMOD}/imsearch/imsearch.cpp
    ${SUBMOD}/imgui_toggle/imgui_toggle.cpp
    ${SUBMOD}/imgui_toggle/imgui_toggle_palette.cpp
    ${SUBMOD}/imgui_toggle/imgui_toggle_presets.cpp
    ${SUBMOD}/imgui_toggle/imgui_toggle_renderer.cpp
    ${IMGUI_FILE_DIALOG_DIR}/ImGuiFileDialog.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)

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
)
add_library(lua STATIC ${LUA_SOURCES})

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/widgets
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SOL_DIR}/include
    ${LUA_DIR}
    ${SUBMOD}/imgui-knobs
    ${SUBMOD}/imgui-command-palette
    ${SUBMOD}/imsearch
    ${SUBMOD}/imgui_toggle
    ${IMGUI_FILE_DIALOG_DIR}
    ${SUBMOD}
)

add_subdirectory(${RTMIDI_DIR})

add_executable(i7ed
    src/main.cpp
    src/Midi.cpp
    src/LuaBridge.cpp
    src/SysexIO.cpp
    src/Rendering.cpp
    src/Sidebar.cpp
    src/Settings.cpp
    src/CommandPalette.cpp
    src/SysexFileDialog.cpp
    src/widgets/imgui_knob_image.cpp
)
target_link_libraries(i7ed PRIVATE glfw OpenGL::GL imgui lua rtmidi)

option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(i7ed PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(i7ed PRIVATE -fsanitize=address)
endif()


add_custom_command(
    TARGET i7ed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${LUA_SRC}" "${LUA_SRC_DST}"
)

add_custom_command(
    TARGET i7ed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/images" "${CMAKE_BINARY_DIR}/assets/images"
)

# ── Install ──────────────────────────────────────────────────────────────────
install(TARGETS i7ed RUNTIME DESTINATION bin)
install(DIRECTORY "${LUA_SRC}/" DESTINATION share/i7ed/lua)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets/images/" DESTINATION share/i7ed/assets/images)

# ── CPack ────────────────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME               "i7ed")
set(CPACK_PACKAGE_VENDOR             "integra7-editor")
set(CPACK_PACKAGE_VERSION            "0.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Roland Integra-7 patch editor")
set(CPACK_RESOURCE_FILE_LICENSE      "${CMAKE_SOURCE_DIR}/LICENSE")

# Linux DEB
set(CPACK_DEBIAN_PACKAGE_MAINTAINER  "samba.godschynski@gmail.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS     "libglfw3, libasound2")
set(CPACK_DEBIAN_PACKAGE_SECTION     "sound")

# Linux RPM
set(CPACK_RPM_PACKAGE_REQUIRES       "glfw, alsa-lib")
set(CPACK_RPM_PACKAGE_LICENSE        "MIT")

# Windows WiX
set(CPACK_WIX_UPGRADE_GUID           "F3A2B1C0-D4E5-4F6A-8B9C-0D1E2F3A4B5C")

include(CPack)