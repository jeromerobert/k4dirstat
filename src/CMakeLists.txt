qt5_add_resources(UI_RCC k4dirstatui.qrc)
set(k4dirstat_SRCS
   k4dirstat.cpp
   ${UI_RCC}
   main.cpp
   ktreemapview.cpp
   kcleanupcollection.cpp
   kfileinfo.cpp
   kdirtreeview.cpp
   ktreemaptile.cpp
   kstdcleanup.cpp
   kcleanup.cpp
   kdirtree.cpp
   kexcluderules.cpp
   kdirreadjob.cpp
   kdirinfo.cpp
   kdirtreecache.cpp
   kdirtreeiterators.cpp
   kdirstatsettings.cpp
 )

kconfig_add_kcfg_files(k4dirstat_SRCS settings.kcfgc )

add_executable(k4dirstat ${k4dirstat_SRCS})

target_link_libraries(k4dirstat KF5::XmlGui KF5::KIOCore
    KF5::KIOWidgets KF5::I18n KF5::IconThemes
    ${QT_QTGUI_LIBS}
    ${LIBKONQ_LIBRARY}
    ${ZLIB_LIBRARIES})

install(TARGETS k4dirstat ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES k4dirstat.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES k4dirstat.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
