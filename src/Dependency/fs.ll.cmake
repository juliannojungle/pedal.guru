# fs.ll.cmake
#
# Copy this file anywhere into your project and include() it from your CMakeLists.txt,
# after project(). It locates fs.ll through FS_LL_PATH, downloading the project when
# that directory is not populated yet.
#
# FS_LL_PATH is cached, so several submodules of the same project (each one carrying its
# own copy of this file) share a single fs.ll checkout: the first one to resolve it wins.
#
# Inputs:
#   FS_LL_PATH     - path to the fs.ll root directory (variable or environment).
#                    Relative paths are resolved against CMAKE_SOURCE_DIR.
#                    Defaults to a 'fs.ll' folder next to this file.
#   PLATFORM_NAME  - Simulator (default), RP2040 or ESP32.
#
# Outputs:
#   FS_LL_PATH     - cached, absolute path to the fs.ll root directory.
#   SOURCES        - appended with the fs.ll and fatfs sources.
#   INCLUDE_DIRS   - appended with the fs.ll and fatfs include directories.

if(DEFINED ENV{FS_LL_PATH} AND (NOT FS_LL_PATH))
    set(FS_LL_PATH $ENV{FS_LL_PATH})
    message("Using FS_LL_PATH from environment ('${FS_LL_PATH}')")
endif()

if(NOT FS_LL_PATH)
    set(FS_LL_PATH "${CMAKE_CURRENT_LIST_DIR}/fs.ll")
endif()

get_filename_component(FS_LL_PATH "${FS_LL_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")

# Sentinel file used to tell a populated checkout from an empty/missing directory.
set(FS_LL_SENTINEL_FILE "${FS_LL_PATH}/src/lib/FileSystem.c")

if(NOT EXISTS "${FS_LL_SENTINEL_FILE}")
    find_package(Git QUIET)
    if(NOT Git_FOUND)
        message(FATAL_ERROR
            "fs.ll was not found at '${FS_LL_PATH}' and git is not available to download it. "
            "Please install git or set FS_LL_PATH to an existing fs.ll checkout.")
    endif()

    message("Downloading fs.ll into '${FS_LL_PATH}'")
    # fatfs is a submodule of fs.ll, hence --recurse-submodules.
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --branch main --depth 1
                --recurse-submodules --shallow-submodules
                https://github.com/juliannojungle/fs.ll.git "${FS_LL_PATH}"
        RESULT_VARIABLE FS_LL_CLONE_RESULT
        ERROR_VARIABLE FS_LL_CLONE_ERROR)

    if(NOT FS_LL_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to download fs.ll into '${FS_LL_PATH}': ${FS_LL_CLONE_ERROR}")
    endif()

    if(NOT EXISTS "${FS_LL_SENTINEL_FILE}")
        message(FATAL_ERROR "Directory '${FS_LL_PATH}' does not appear to contain fs.ll")
    endif()
endif()

set(FS_LL_PATH "${FS_LL_PATH}" CACHE PATH "Path to the fs.ll root directory" FORCE)

if(NOT DEFINED PLATFORM_NAME)
    set(PLATFORM_NAME "Simulator")
endif()

set(FS_LL_PLATFORM_DIR "${FS_LL_PATH}/src/lib/Platform/${PLATFORM_NAME}")

if(NOT EXISTS "${FS_LL_PLATFORM_DIR}")
    message(FATAL_ERROR "fs.ll has no support for platform '${PLATFORM_NAME}' ('${FS_LL_PLATFORM_DIR}' not found)")
endif()

set(SOURCES
    ${SOURCES}
    "${FS_LL_PATH}/src/lib/FileSystem.c"
    "${FS_LL_PLATFORM_DIR}/HAL.c"
    "${FS_LL_PLATFORM_DIR}/RTC.c"
    "${FS_LL_PLATFORM_DIR}/DiskIO.c"
    "${FS_LL_PATH}/src/Dependency/fatfs/source/ff.c"
    "${FS_LL_PATH}/src/Dependency/fatfs/source/ffsystem.c"
    "${FS_LL_PATH}/src/Dependency/fatfs/source/ffunicode.c")

set(INCLUDE_DIRS
    ${INCLUDE_DIRS}
    "${FS_LL_PATH}/src/lib"
    "${FS_LL_PLATFORM_DIR}"
    "${FS_LL_PATH}/src/Dependency/fatfs/source")

# Guards against the same fs.ll being included by more than one submodule.
list(REMOVE_DUPLICATES SOURCES)
list(REMOVE_DUPLICATES INCLUDE_DIRS)

# Apply patch to ffconf.h (FF_FS_RPATH=1, FF_VOLUMES=2, FF_CODE_PAGE=437, FF_USE_LFN=2)
include(${FS_LL_PATH}/src/Dependency/fatfs.ffconf_patch.cmake)
