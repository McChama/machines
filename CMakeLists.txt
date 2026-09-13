##
# Main CMake project file
# Contains global options and definitions
##

cmake_minimum_required(VERSION 3.20)

project(machines C CXX)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

##
# Project version
##
set(MACHINES_VERSION_CODENAME "Patch 1.5")
set(MACHINES_VERSION_MAJOR    0)
set(MACHINES_VERSION_MINOR    1)
set(MACHINES_VERSION_REVISION 1.5)

message(STATUS "Building Machines \"${MACHINES_VERSION_CODENAME}\" (${MACHINES_VERSION_FULL})")

set(BUILD_NUMBER 0)
if(NOT "$ENV{BUILD_NUMBER}" STREQUAL "")
    set(BUILD_NUMBER "$ENV{BUILD_NUMBER}")
    message(STATUS "CI build #${BUILD_NUMBER}")
endif()


##
# Platform detection and some related checks
##
if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    message(STATUS "Build for Windows system")
    set(PLATFORM_WINDOWS 1)
    set(PLATFORM_GNU     0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   0)

    # Platform-dependent implementation of system.h
    #set(SYSTEM_CPP_MODULE "system_windows.cpp")
    #set(SYSTEM_H_MODULE "system_windows.h")
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    message(STATUS "Build for Linux system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   1)
    set(PLATFORM_GNU     1)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   0)

    set(OpenGL_GL_PREFERENCE "GLVND")
    # Platform-dependent implementation of system.h
    #set(SYSTEM_CPP_MODULE "system_linux.cpp")
    #set(SYSTEM_H_MODULE "system_linux.h")
else()
    message(STATUS "Build for other system")
    set(PLATFORM_WINDOWS 0)
    set(PLATFORM_LINUX   0)
    set(PLATFORM_GNU     0)
    set(PLATFORM_MACOSX  0)
    set(PLATFORM_OTHER   1)

    # Platform-dependent implementation of system.h
    set(SYSTEM_CPP_MODULE "system_other.cpp")
    set(SYSTEM_H_MODULE "system_other.h")
endif()


##
# Build options
##
# Include cmake directory with some additional scripts
set(CMAKE_MODULE_PATH "${machines_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Build targets should be placed in the root build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# 32 bit build configuration
option(BUILD_32 "Builds 32 bit executable" OFF)
# Include cmake directory with some additions

# Compiler detection
#set(CMAKE_CXX_COMPILER "${OMPP_CXX}" CACHE PATH "Set compiler used")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
        message(FATAL_ERROR "${PROJECT_NAME} requires GCC 4.7 or greater.")
    endif()

    message(STATUS "Detected GCC version 4.7+")

    #set(NORMAL_CXX_FLAGS "-std=gnu++11 -Wall -Werror -Wold-style-cast -pedantic-errors -Wmissing-declarations")
    if(BUILD_32)
        set(MACHINES_COMPILER_BIT_MODE -m32)
    else()
        set(MACHINES_COMPILER_BIT_MODE -m64)
    endif()
    check_cxx_compiler_flag("${MACHINES_COMPILER_BIT_MODE}" CXX_COMPILER_SUPPORTS_BIT_MODE)
    check_c_compiler_flag("${MACHINES_COMPILER_BIT_MODE}" C_COMPILER_SUPPORTS_BIT_MODE)
    if(NOT C_COMPILER_SUPPORTS_BIT_MODE OR NOT CXX_COMPILER_SUPPORTS_BIT_MODE)
        set(MACHINES_COMPILER_BIT_MODE "")
        message(WARNING "${PROJECT_NAME} will be built for compiler default target architecture.")
    endif()
        
    set(NORMAL_CXX_FLAGS "-std=gnu++11 ${MACHINES_COMPILER_BIT_MODE} -Wall -Wold-style-cast -Wmissing-declarations")
    set(NORMAL_C_FLAGS " ${MACHINES_COMPILER_BIT_MODE} -Wall")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958

    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wsuggest-override")
    endif()

    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-gstabs -O0")
    set(TEST_CXX_FLAGS "-pthread")
    add_definitions(-DNOEXCEPT=noexcept -DHAVE_DEMANGLE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
        message(FATAL_ERROR "${PROJECT_NAME} requires Clang 3.1 or greater.")
    endif()

    message(STATUS "Detected Clang version 3.1+")

    # No -Werror/-pedantic-errors: same call Phase 0 made for GCC (see the GCC
    # branch above) - this 1999 codebase trips plenty of legacy-style warnings
    # (old-style casts, extra semicolons, multi-line comments) that aren't worth
    # hard-failing the build over, on AppleClang or any other Clang.
    set(NORMAL_CXX_FLAGS "-std=c++11 ${MACHINES_COMPILER_BIT_MODE} -Wall -Wold-style-cast -Wmissing-prototypes")
    set(NORMAL_CXX_FLAGS "${NORMAL_CXX_FLAGS} -Wno-error=deprecated-declarations") # updated version of physfs is not available on some platforms so we keep using deprecated functions, see #958
    set(RELEASE_CXX_FLAGS "-O2")
    set(DEBUG_CXX_FLAGS "-g -O0")
    set(TEST_CXX_FLAGS "-pthread")
    add_definitions(-DNOEXCEPT=noexcept -DHAVE_DEMANGLE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Detected MSVC compiler")

    set(NORMAL_CXX_FLAGS "/wd\"4244\" /wd\"4309\" /wd\"4800\" /wd\"4996\" /wd\"4351\" /EHsc") # disable some useless warnings
    set(RELEASE_CXX_FLAGS "/MD")
    set(DEBUG_CXX_FLAGS "/MDd /ZI")
    set(TEST_CXX_FLAGS "")
    add_definitions(-DNOEXCEPT= -DHAS_MSVC_EXCEPTION_BUG)

    # Needed for Debug information (it's set to "No" by default for some reason)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG")
else()
    message(FATAL_ERROR "Your C++ compiler doesn't seem to be supported.")
endif()

# Compiler flags
set(MACHINES_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${NORMAL_CXX_FLAGS}")
set(MACHINES_C_FLAGS "${CMAKE_C_FLAGS} ${NORMAL_C_FLAGS}")
set(MACHINES_CXX_FLAGS_RELEASE "${RELEASE_CXX_FLAGS}")
set(MACHINES_CXX_FLAGS_DEBUG "${DEBUG_CXX_FLAGS}")

# Asserts can be enabled/disabled regardless of build type
option(DEV_BUILD "Enable assert()s and other debug checks" ON)

# Minimal automated tests (see src/tests/). Fetches Catch2 via FetchContent when on,
# so -DBUILD_TESTING=OFF is available for offline builds.
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    enable_testing()
endif()

# Warn about development build
if(DEV_BUILD)
    message(STATUS "Building with development extensions")
endif()

##
# Searching for packages
##
#set_target_properties(machines PROPERTIES LINK_SEARCH_START_STATIC 1)
#set_target_properties(machines PROPERTIES LINK_SEARCH_END_STATIC 1)
if(BUILD_32)
    # Prefer static libraries
    #set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS OFF)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS ON)
else()
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS ON)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS OFF)
endif()

find_package(OpenGL 2.1 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenAL REQUIRED)

# Classic Alure 1.x has no vcpkg port or Homebrew formula, so it isn't findable
# out of the box on MSVC/macOS (Linux apt and the bundled windows/ MinGW libs
# already satisfy this REQUIRED find just fine). When the system copy isn't
# found, fetch and build alure 1.2 from source as a fallback.
find_package(Alure QUIET)
if(NOT ALURE_FOUND)
    message(STATUS "System Alure not found - fetching and building alure 1.2 from source")

    include(FetchContent)
    set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(alure_src
        URL https://archive.ubuntu.com/ubuntu/pool/universe/a/alure/alure_1.2.orig.tar.bz2
        URL_HASH SHA256=465e6adae68927be3a023903764662d64404e40c4c152d160e3a8838b1d70f71
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_GetProperties(alure_src)
    if(NOT alure_src_POPULATED)
        FetchContent_Populate(alure_src)

        # alure 1.2's CMakeLists.txt and cmake/CheckFileOffsetBits.cmake both
        # look up alure's own bundled cmake/ helper files via CMAKE_SOURCE_DIR,
        # which as a FetchContent subproject means *our* top-level source dir,
        # not alure's own - patch both to CMAKE_CURRENT_SOURCE_DIR.
        foreach(_alure_file_to_patch "CMakeLists.txt" "cmake/CheckFileOffsetBits.cmake")
            file(READ "${alure_src_SOURCE_DIR}/${_alure_file_to_patch}" _alure_file_content)
            string(REPLACE "\${CMAKE_SOURCE_DIR}" "\${CMAKE_CURRENT_SOURCE_DIR}" _alure_file_content "${_alure_file_content}")
            file(WRITE "${alure_src_SOURCE_DIR}/${_alure_file_to_patch}" "${_alure_file_content}")
        endforeach()

        # alure 1.2's CMakeLists.txt predates CMake 3.5; CMake 4.x removed
        # support for policies below 3.5 outright, so raise the floor just
        # for processing this subdirectory.
        set(_alure_saved_policy_min "${CMAKE_POLICY_VERSION_MINIMUM}")
        set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
        add_subdirectory(${alure_src_SOURCE_DIR} ${alure_src_BINARY_DIR})
        set(CMAKE_POLICY_VERSION_MINIMUM "${_alure_saved_policy_min}")
    endif()

    # alure-static's own CMakeLists.txt never links OpenAL into the static
    # target (only its shared counterpart does), so consumers must do so
    # themselves - src/CMakeLists.txt already links ${OPENAL_LIBRARY}
    # alongside ${ALURE_LIBRARY} independently, so this matches.
    set(ALURE_LIBRARY alure-static)
    set(ALURE_INCLUDE_DIR ${alure_src_SOURCE_DIR}/include)
    set(ALURE_FOUND TRUE)
endif()

find_package(ENet REQUIRED)
find_package(Freetype REQUIRED)


if(NOT DEV_BUILD)
    add_definitions(-DNDEBUG)
    add_definitions(-DPRODUCTION)
endif()

if(PLATFORM_WINDOWS)
  if(DEFINED WINDOWS_DEPENDENCIES_DIR)
    # MinGW cross build (see cmake/Toolchain-mingw64.cmake): links directly against
    # the prebuilt windows/lib64/libfreetype.a, which was statically built bundling
    # harfbuzz/graphite2/bz2/png16/zlib - none of those are otherwise on the link
    # line, so they have to be resolved and linked explicitly here.
    if(NOT BUILD_32)
      find_library(MSVCRT_LIBRARY msvcrt HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(FREETYPE_LIBRARY freetype HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(HARFBUZZ_LIBRARY harfbuzz HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(BZ2_LIBRARY bz2 HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(PNG16_LIBRARY png16 HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(GRAPHITE2_LIBRARY graphite2 HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(Z_LIBRARY z HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(RPCRT4_LIBRARY rpcrt4 HINTS ${WINDOWS_DEPENDENCIES_DIR})
      find_library(USP10_LIBRARY usp10 HINTS ${WINDOWS_DEPENDENCIES_DIR})
      # Windows 64-bit libraries
      set(LIBS
          ${MSVCRT_LIBRARY}
          ${FREETYPE_LIBRARY}
          ${HARFBUZZ_LIBRARY}
          ${BZ2_LIBRARY}
          ${PNG16_LIBRARY}
          ${GRAPHITE2_LIBRARY}
          ${Z_LIBRARY}
          ${RPCRT4_LIBRARY}
          ${USP10_LIBRARY}
      )
    endif()
  else()
    # Native MSVC build (vcpkg toolchain, no WINDOWS_DEPENDENCIES_DIR set): the
    # find_library() calls above hunted for standalone harfbuzz/graphite2/msvcrt/
    # rpcrt4/usp10 libs that don't exist in this toolchain (vcpkg's freetype port
    # doesn't build/export those, and find_library() can't see the Windows SDK/CRT
    # library dirs the MSVC linker already knows about) and hard-failed CMake
    # configure with NOTFOUND. find_package(Freetype REQUIRED) above already
    # resolved the real Freetype::Freetype target for this toolchain (including
    # whatever it actually needs transitively), so just use that.
    if(TARGET Freetype::Freetype)
      set(FREETYPE_LIBRARY Freetype::Freetype)
    else()
      set(FREETYPE_LIBRARY ${FREETYPE_LIBRARIES})
    endif()
  endif()
endif()

##
# Targets
##

# Subdirectory with sources
add_subdirectory(src)



