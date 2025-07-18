add_definitions(-DTRANSLATION_DOMAIN=\"kcmkeyboard\")

set(CMAKE_REQUIRED_LIBRARIES -lXi)
check_function_exists(_XiGetDevicePresenceNotifyEvent XiGetDevicePresenceNotifyEvent_EXISTS)

if( X11_Xinput_FOUND AND XiGetDevicePresenceNotifyEvent_EXISTS )
    set(XiGetDevicePresence_FOUND 1)
    add_definitions(-DHAVE_XINPUT_AND_DEVICE_NOTIFY=1)
else()
    message(STATUS "_XiGetDevicePresenceNotifyEvent is not found, libXi version 1.2.0 or later is required to keep layouts with keyboard hotplugging")
endif()



### kded daemon ###

set( kded_keyboard_SRCS
    keyboard_daemon.cpp
    layout_memory.cpp
    layout_memory_persister.cpp
    x11_helper.cpp
    xinput_helper.cpp
    xkb_helper.cpp
    keyboard_config.cpp
    keyboard_hardware.cpp
    numlockx.c
    bindings.cpp
# next are temporary for sys tray UI
    layout_tray_icon.cpp
    layouts_menu.cpp
    flags.cpp
    xkb_rules.cpp
)

add_library(kded_keyboard MODULE ${kded_keyboard_SRCS})

target_link_libraries(kded_keyboard
    Qt5::Concurrent
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::Notifications
    KF5::Service
    KF5::WindowSystem
    KF5::Plasma
    KF5::XmlGui
    KF5::KDELibs4Support
    X11::X11
    X11::Xi
    X11::Xtst
    XCB::XCB
    ${XCB_XKB_LIBRARIES}
    X11::X11
    X11::Xkbfile
    X11::Xrender
    X11::Xext
 )

if(X11_XTest_FOUND)
  target_link_libraries(kded_keyboard ${X11_XTest_LIB})
endif()

if(XiGetDevicePresence_FOUND)
  target_link_libraries(kded_keyboard ${X11_Xinput_LIB})
endif()

install( TARGETS kded_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


### plasma applet ###
#
# set( plasma_applet_keyboard_SRCS
#     keyboard_applet.cpp
#     flags.cpp
#     keyboard_config.cpp
#     xkb_rules.cpp
#     x11_helper.cpp
#     xkb_helper.cpp
#     layouts_menu.cpp
# )
#
# add_library(plasma_applet_keyboard MODULE ${plasma_applet_keyboard_SRCS})
#
# target_link_libraries(plasma_applet_keyboard
#      KF5::Plasma ${KDE4_KDEUI_LIBS} Qt5::Xml ${X11_Xkbfile_LIB} ${X11_LIBRARIES}
#      ${text_paint_LIB}
# )
#
# install( TARGETS plasma_applet_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )
# install( FILES plasma_applet_keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
#
#
### widget lib ###

set( keyboard_layout_widget_SRCS
    layout_widget.cpp
    keyboard_config.cpp
    flags.cpp
    x11_helper.cpp
    xkb_helper.cpp
)

# MODULE?
add_library(keyboard_layout_widget MODULE ${keyboard_layout_widget_SRCS})

target_link_libraries(keyboard_layout_widget
    Qt5::X11Extras
    Qt5::Xml
    KF5::I18n
    KF5::Service
    KF5::Plasma
    XCB::XCB
    XCB::XKB
    KF5::KDELibs4Support
    ${X11_LIBRARIES}
    ${X11_Xkbfile_LIB}
)

install( TARGETS keyboard_layout_widget DESTINATION ${PLUGIN_INSTALL_DIR} )


### kcm keyboard ###
include_directories("preview")

set(kcm_keyboard_PART_SRCS 
  kcm_keyboard.cpp
  kcm_keyboard_widget.cpp
  kcm_view_models.cpp
  kcm_add_layout_dialog.cpp
  keyboard_config.cpp
  x11_helper.cpp
  xkb_helper.cpp
  xkb_rules.cpp
  flags.cpp
  iso_codes.cpp
  kcmmisc.cpp
  bindings.cpp
  preview/keyaliases.cpp
  preview/keyboardlayout.cpp
  preview/keyboardpainter.cpp
  preview/keysymbols.cpp
  preview/keysymhelper.cpp
  preview/kbpreviewframe.cpp
  preview/keysym2ucs.cpp
)


ki18n_wrap_ui(kcm_keyboard_PART_SRCS kcm_keyboard.ui kcm_add_layout_dialog.ui kcmmiscwidget.ui)

add_library(kcm_keyboard MODULE ${kcm_keyboard_PART_SRCS})

target_link_libraries(kcm_keyboard
    Qt5::Concurrent
    Qt5::Widgets
    Qt5::X11Extras
    Qt5::Xml
    KF5::KCMUtils
    KF5::GlobalAccel
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WindowSystem
    KF5::Plasma
    KF5::XmlGui
    XCB::XCB
    XCB::XKB
    KF5::KDELibs4Support
    ${X11_Xkbfile_LIB}
    ${X11_LIBRARIES}
    ${text_paint_LIB}
)

install(TARGETS kcm_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kcm_keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

# Images

add_subdirectory( pics )


# Unit tests

add_subdirectory( tests )
