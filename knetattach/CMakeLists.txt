set(knetattach_SRCS main.cpp knetattach.cpp)


ki18n_wrap_ui(knetattach_SRCS knetattach.ui )

# kde4_add_app_icon(knetattach_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-knetattach.png")

add_executable(knetattach ${knetattach_SRCS})

target_link_libraries(knetattach KF5::KIOWidgets KF5::KDELibs4Support)

install(TARGETS knetattach DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS knetattach.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )

