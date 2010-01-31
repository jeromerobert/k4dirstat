set(k4dirstat_SRCS
   k4dirstat.cpp
   main.cpp
   k4dirstatview.cpp
 )

kde4_add_ui_files(k4dirstat_SRCS k4dirstatview_base.ui prefs_base.ui)

kde4_add_kcfg_files(k4dirstat_SRCS settings.kcfgc )

kde4_add_executable(k4dirstat ${k4dirstat_SRCS})

target_link_libraries(k4dirstat ${KDE4_KDEUI_LIBS} )

install(TARGETS k4dirstat ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES k4dirstat.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES k4dirstat.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES k4dirstatui.rc  DESTINATION  ${DATA_INSTALL_DIR}/k4dirstat )
