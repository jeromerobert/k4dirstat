project(k4dirstat)

SET(QT_USE_QT3SUPPORT true)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
find_package(LibKonq REQUIRED)
find_package(ZLIB)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${QT_GUI_INCLUDES} ${QT_QT3SUPPORT_INCLUDES} )

ADD_DEFINITIONS(-DQT3_SUPPORT -D_LARGE_FILES -D_FILE_OFFSET_BITS=64)
REMOVE_DEFINITIONS(-DQT_NO_DEBUG)

add_subdirectory( doc )
add_subdirectory( src )
add_subdirectory( icons )
