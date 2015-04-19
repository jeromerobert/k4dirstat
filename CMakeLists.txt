cmake_minimum_required(VERSION 2.8.0)
project(k4dirstat)

function(git_version output_variable default_version strip_string)
    option(K4DIRSTAT_GIT_VERSION "Get the version string from git describe" ON)
    if(K4DIRSTAT_GIT_VERSION)
        find_package(Git)
        if(GIT_FOUND)
            execute_process(COMMAND "${GIT_EXECUTABLE}"
                describe --dirty=-dirty --always --tags
                OUTPUT_VARIABLE _GIT_DESCRIBE ERROR_QUIET)
            if(_GIT_DESCRIBE)
                string(REPLACE ${strip_string} "" ${output_variable} ${_GIT_DESCRIBE})
                string(STRIP ${${output_variable}} ${output_variable})
            endif()
        endif()
    endif()
    if(NOT ${output_variable})
        # It would be better use only git and add a make dist target for Linux distro
        # but this would require to upload dist tarball on Bitbucket and I prefere using
        # the Bitbucket download tag feature.
        set(${output_variable} ${default_version})
    endif()
    message(STATUS "Version string is ${${output_variable}}")
    add_definitions(-D${output_variable}=${${output_variable}})
endfunction()

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
find_package(LibKonq REQUIRED)
find_package(ZLIB)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${QT_GUI_INCLUDES})

ADD_DEFINITIONS(-D_LARGE_FILES -D_FILE_OFFSET_BITS=64)
REMOVE_DEFINITIONS(-DQT_NO_DEBUG)

git_version(K4DIRSTAT_VERSION 2.7.8 k4dirstat-)
add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )
