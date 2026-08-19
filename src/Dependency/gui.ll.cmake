# gui.ll.cmake
#
# Copy this file anywhere into your project and include() it from your CMakeLists.txt,
# after project(). It locates gui.ll through GUI_LL_PATH, downloading the project when
# that directory is not populated yet.
#
# GUI_LL_PATH is cached, so several submodules of the same project (each one carrying its
# own copy of this file) share a single gui.ll checkout: the first one to resolve it wins.
#
# gui.ll builds on top of fs.ll and includes gui.ll's own copy of fs.ll.cmake, so a consumer that
# only draws does not have to include fs.ll.cmake as well. A consumer that uses fs.ll directly
# (its own file access) carries that file too, which is what keeps the two from being downloaded
# twice: whoever resolves FS_LL_PATH first wins and the other include reuses the cached checkout.
# In that case the fs.ll.cmake include has to come FIRST: gui.ll replaces fs.ll's HAL.c with its
# own, and a later include would put it back and break the link with duplicate symbols.
#
# Inputs:
#   GUI_LL_PATH    - path to the gui.ll root directory (variable or environment).
#                    Relative paths are resolved against CMAKE_SOURCE_DIR.
#                    Defaults to a 'gui.ll' folder next to this file.
#   PLATFORM_NAME  - Simulator (default), RP2040 or ESP32.
#
# Outputs:
#   GUI_LL_PATH    - cached, absolute path to the gui.ll root directory.
#   SOURCES        - appended with the gui.ll, libpng, zlib and fs.ll sources.
#   INCLUDE_DIRS   - appended with the matching include directories.
#
# On the Simulator platform SDL2 is located with find_package, and the consumer target is the
# one that has to link ${SDL2_LIBRARIES} and m.

if(DEFINED ENV{GUI_LL_PATH} AND (NOT GUI_LL_PATH))
    set(GUI_LL_PATH $ENV{GUI_LL_PATH})
    message("Using GUI_LL_PATH from environment ('${GUI_LL_PATH}')")
endif()

if(NOT GUI_LL_PATH)
    set(GUI_LL_PATH "${CMAKE_CURRENT_LIST_DIR}/gui.ll")
endif()

get_filename_component(GUI_LL_PATH "${GUI_LL_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")

# Sentinel file used to tell a populated checkout from an empty/missing directory.
set(GUI_LL_SENTINEL_FILE "${GUI_LL_PATH}/src/lib/GUI/Canvas.c")

if(NOT EXISTS "${GUI_LL_SENTINEL_FILE}")
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        message(FATAL_ERROR
            "gui.ll was not found at '${GUI_LL_PATH}' and git is not available to download it. "
            "Please install git or set GUI_LL_PATH to an existing gui.ll checkout.")
    endif()

    message("Downloading gui.ll into '${GUI_LL_PATH}'")
    # libpng and zlib are submodules of gui.ll, hence --recurse-submodules.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --branch main --depth 1
                --recurse-submodules --shallow-submodules
                https://github.com/juliannojungle/gui.ll.git "${GUI_LL_PATH}"
        RESULT_VARIABLE GUI_LL_CLONE_RESULT
        ERROR_VARIABLE GUI_LL_CLONE_ERROR)

    if(NOT GUI_LL_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to download gui.ll into '${GUI_LL_PATH}': ${GUI_LL_CLONE_ERROR}")
    endif()

    if(NOT EXISTS "${GUI_LL_SENTINEL_FILE}")
        message(FATAL_ERROR "Directory '${GUI_LL_PATH}' does not appear to contain gui.ll")
    endif()
endif()

set(GUI_LL_PATH "${GUI_LL_PATH}" CACHE PATH "Path to the gui.ll root directory" FORCE)

if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator")
endif()

set(GUI_LL_LIB_DIR "${GUI_LL_PATH}/src/lib")
set(GUI_LL_DEPENDENCY_DIR "${GUI_LL_PATH}/src/Dependency")
set(GUI_LL_PLATFORM_DIR "${GUI_LL_LIB_DIR}/Platform/${PLATFORM_NAME}")

if(NOT EXISTS "${GUI_LL_PLATFORM_DIR}")
    message(FATAL_ERROR "gui.ll has no support for platform '${PLATFORM_NAME}' ('${GUI_LL_PLATFORM_DIR}' not found)")
endif()

if(PLATFORM_NAME STREQUAL "Simulator")
    set(GUI_LL_LCD_TYPE "Simulator")
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
else()
    set(GUI_LL_LCD_TYPE "1in28")
    set(SOURCES ${SOURCES} "${GUI_LL_LIB_DIR}/Driver/GC9A01/Driver.c")
    set(INCLUDE_DIRS ${INCLUDE_DIRS} "${GUI_LL_LIB_DIR}/Driver/GC9A01")
endif()

# gui.ll
set(SOURCES
    ${SOURCES}
    "${GUI_LL_LIB_DIR}/LCD/${GUI_LL_LCD_TYPE}/LCDSetup.c"
    "${GUI_LL_LIB_DIR}/LCD/${GUI_LL_LCD_TYPE}/LCDRenderer.c"
    "${GUI_LL_LIB_DIR}/GUI/Canvas.c"
    "${GUI_LL_LIB_DIR}/GUI/Fonts/font8.c"
    "${GUI_LL_LIB_DIR}/GUI/Fonts/font12.c"
    "${GUI_LL_LIB_DIR}/GUI/Fonts/font16.c"
    "${GUI_LL_LIB_DIR}/GUI/Fonts/font20.c"
    "${GUI_LL_LIB_DIR}/GUI/Fonts/font24.c"
    "${GUI_LL_LIB_DIR}/Helper/Trigonometry.c"
    "${GUI_LL_PLATFORM_DIR}/HAL.c")

# zlib
set(SOURCES
    ${SOURCES}
    "${GUI_LL_DEPENDENCY_DIR}/zlib/adler32.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/compress.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/crc32.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/deflate.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/inflate.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/inftrees.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/inffast.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/trees.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/uncompr.c"
    "${GUI_LL_DEPENDENCY_DIR}/zlib/zutil.c")

# libpng
set(SOURCES
    ${SOURCES}
    "${GUI_LL_DEPENDENCY_DIR}/libpng/png.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngerror.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngget.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngmem.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngpread.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngread.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngrio.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngrtran.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngrutil.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngset.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngtrans.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngwio.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngwrite.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngwtran.c"
    "${GUI_LL_DEPENDENCY_DIR}/libpng/pngwutil.c")

if(NOT CMAKE_SCRIPT_MODE_FILE)
    configure_file(
        "${GUI_LL_DEPENDENCY_DIR}/libpng/scripts/pnglibconf.h.prebuilt" # libpng default configs
        "${GUI_LL_DEPENDENCY_DIR}/libpng/pnglibconf.h"
        COPYONLY)
    set_source_files_properties(
        "${GUI_LL_DEPENDENCY_DIR}/libpng/pngerror.c"
        "${GUI_LL_DEPENDENCY_DIR}/libpng/png.c"
        PROPERTIES COMPILE_OPTIONS "-Wno-maybe-uninitialized") # suppress warnings for libpng
endif()

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${GUI_LL_LIB_DIR}"
    "${GUI_LL_LIB_DIR}/GUI"
    "${GUI_LL_LIB_DIR}/GUI/Fonts"
    "${GUI_LL_LIB_DIR}/Helper"
    "${GUI_LL_LIB_DIR}/LCD/${GUI_LL_LCD_TYPE}"
    "${GUI_LL_PLATFORM_DIR}"
    "${GUI_LL_DEPENDENCY_DIR}/libpng"
    "${GUI_LL_DEPENDENCY_DIR}/zlib")

# Guards against the same gui.ll being included by more than one submodule.
list(REMOVE_DUPLICATES SOURCES)
list(REMOVE_DUPLICATES INCLUDE_DIRS)

include(${GUI_LL_DEPENDENCY_DIR}/fs.ll.cmake)
list(REMOVE_ITEM SOURCES "${FS_LL_PLATFORM_DIR}/HAL.c") # gui.ll's HAL.c supersedes fs.ll's
