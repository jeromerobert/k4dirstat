set(k4dirstat_SRCS
   k4dirstat.cpp
   main.cpp
   k4dirstatview.cpp
   ktreemapview.cpp
   kcleanupcollection.cpp
   kfileinfo.cpp
   kdirtreeview.cpp
   ktreemaptile.cpp
   kstdcleanup.cpp
   kcleanup.cpp
   kdirsaver.cpp
   kdirtree.cpp
   kexcluderules.cpp
   kdirreadjob.cpp
   kdirinfo.cpp
   kdirtreecache.cpp
   kdirtreeiterators.cpp
   kpacman.cpp
   #kpacman_dummy.cpp
   kdirstatsettings.cpp
 )

kde4_add_ui_files(k4dirstat_SRCS k4dirstatview_base.ui prefs_base.ui)

kde4_add_kcfg_files(k4dirstat_SRCS settings.kcfgc )

kde4_add_executable(k4dirstat ${k4dirstat_SRCS})

target_link_libraries(k4dirstat ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBS}
    ${QT_QT3SUPPORT_LIBRARY}
    ${KDE4_KDE3SUPPORT_LIBRARY})

install(TARGETS k4dirstat ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES k4dirstat.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES k4dirstat.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES k4dirstatui.rc  DESTINATION  ${DATA_INSTALL_DIR}/k4dirstat )
