cmake_minimum_required(VERSION 3.16.0)
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

include(CheckCCompilerFlag)
check_c_compiler_flag(-Wl,--as-needed has_as_needed)
if (has_as_needed)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(KF5 REQUIRED COMPONENTS CoreAddons I18n DocTools XmlGui KIO JobWidgets IconThemes)
find_package(ZLIB)

ADD_DEFINITIONS(-D_LARGE_FILES -D_FILE_OFFSET_BITS=64)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)

git_version(K4DIRSTAT_VERSION 3.2.2 k4dirstat-)
add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( po )

install(FILES k4dirstat.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
