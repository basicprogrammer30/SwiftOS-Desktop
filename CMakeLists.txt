cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(plasma-desktop)
set(QT_MIN_VERSION "5.15.2")

# Set policies to handle deprecation warnings
cmake_policy(SET CMP0028 NEW)  # Handle double-colon targets properly

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(FeatureSummary)
include(CheckIncludeFiles)

find_package(OpenGL REQUIRED)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Quick
    DBus
    Widgets
    X11Extras
    Svg
    Test
    Concurrent
    OpenGL
)

find_package(KF5 REQUIRED COMPONENTS
    Auth
    Plasma
    PlasmaQuick
    DocTools
    I18n
    KCMUtils
    NewStuff
    KDELibs4Support
    NotifyConfig
    Attica
    Wallet
    Runner
    GlobalAccel
    WindowSystem
    KIO
)

find_package(KDecoration2 REQUIRED CONFIG)
find_package(LibKWorkspace CONFIG REQUIRED)
find_package(LibTaskManager REQUIRED CONFIG)
include_directories(${LibTaskManager_INCLUDE_DIRS})
find_package(KWinDBusInterface CONFIG REQUIRED)
find_package(ScreenSaverDBusInterface CONFIG REQUIRED)
find_package(KRunnerAppDBusInterface CONFIG REQUIRED)
find_package(KSMServerDBusInterface CONFIG REQUIRED)

# Find X11 and related libraries with debug output
message(STATUS "Looking for X11 and components...")

# First find X11 with required components
find_package(X11 REQUIRED)
message(STATUS "X11_FOUND: ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR: ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES: ${X11_LIBRARIES}")

# Now find individual X11 components
find_package(X11 COMPONENTS X11 Xext Xrender Xtst Xkbfile)

# Debug output for X11 components
message(STATUS "X11_X11_FOUND: ${X11_X11_FOUND}")
message(STATUS "X11_Xext_FOUND: ${X11_Xext_FOUND}")
message(STATUS "X11_Xrender_FOUND: ${X11_Xrender_FOUND}")
message(STATUS "X11_Xtst_FOUND: ${X11_Xtst_FOUND}")
message(STATUS "X11_Xkbfile_FOUND: ${X11_Xkbfile_FOUND}")

# Set X11 include directories
set(X11_INCLUDE_DIRS 
    ${X11_INCLUDE_DIR} 
    ${X11_Xext_INCLUDE_PATH} 
    ${X11_Xrender_INCLUDE_PATH} 
    ${X11_Xtst_INCLUDE_PATH} 
    ${X11_Xkbfile_INCLUDE_PATH}
)

# Add additional include directories for KDE components
include_directories(
    /usr/include/kworkspace5
    /usr/include
    /usr/include/KF5/KWorkspace
    /usr/include/KF5/KWindowSystem
    /usr/include/KF5/KI18n
    /usr/include/KF5/KIO
    /usr/include/KF5/KConfig
    /usr/include/KF5/KCoreAddons
    /usr/include/KF5/KWidgetsAddons
    /usr/include/KF5/Plasma
    /usr/include/KF5/PlasmaQuick
)

# Set X11 library variables
set(X11_LIBRARIES 
    ${X11_X11_LIB}
    ${X11_Xext_LIB}
    ${X11_Xrender_LIB}
    ${X11_Xtst_LIB}
    ${X11_Xkbfile_LIB}
    ${OPENGL_LIBRARIES}
)

# Print final X11 variables for debugging
message(STATUS "X11_INCLUDE_DIRS: ${X11_INCLUDE_DIRS}")
message(STATUS "X11_LIBRARIES: ${X11_LIBRARIES}")

# Create imported targets for X11 libraries if they don't exist
if(NOT TARGET X11::X11)
    add_library(X11::X11 UNKNOWN IMPORTED)
    set_target_properties(X11::X11 PROPERTIES
        IMPORTED_LOCATION "${X11_X11_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${X11_INCLUDE_DIR}"
    )
endif()

if(NOT TARGET X11::Xext)
    add_library(X11::Xext UNKNOWN IMPORTED)
    set_target_properties(X11::Xext PROPERTIES
        IMPORTED_LOCATION "${X11_Xext_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${X11_Xext_INCLUDE_PATH}"
    )
endif()

if(NOT TARGET X11::Xrender)
    add_library(X11::Xrender UNKNOWN IMPORTED)
    set_target_properties(X11::Xrender PROPERTIES
        IMPORTED_LOCATION "${X11_Xrender_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${X11_Xrender_INCLUDE_PATH}"
    )
endif()

if(NOT TARGET X11::Xtst)
    add_library(X11::Xtst UNKNOWN IMPORTED)
    set_target_properties(X11::Xtst PROPERTIES
        IMPORTED_LOCATION "${X11_Xtst_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${X11_Xtst_INCLUDE_PATH}"
    )
endif()

if(NOT TARGET X11::Xkbfile)
    add_library(X11::Xkbfile UNKNOWN IMPORTED)
    set_target_properties(X11::Xkbfile PROPERTIES
        IMPORTED_LOCATION "${X11_Xkbfile_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${X11_Xkbfile_INCLUDE_PATH}"
    )
endif()

# Find XCB and XKB
find_package(XCB REQUIRED)
find_library(XCB_XKB_LIBRARY NAMES xcb-xkb)
find_path(XCB_XKB_INCLUDE_DIR NAMES xcb/xkb.h PATH_SUFFIXES xcb)

if(XCB_XKB_INCLUDE_DIR AND XCB_XKB_LIBRARY)
    set(XCB_XKB_FOUND TRUE)
    set(XCB_XKB_LIBRARIES ${XCB_XKB_LIBRARY})
    set(XCB_XKB_INCLUDE_DIRS ${XCB_XKB_INCLUDE_DIR})
else()
    set(XCB_XKB_FOUND FALSE)
    message(WARNING "XCB XKB not found, keyboard module may not work correctly")
endif()

find_package(KF5ItemModels CONFIG REQUIRED)
find_package(KF5Emoticons CONFIG REQUIRED)

find_package(KF5Activities ${KF5_VERSION})
set_package_properties(KF5Activities PROPERTIES DESCRIPTION "management of Plasma activities"
                       TYPE OPTIONAL
                       PURPOSE "Needed by activity related plasmoids and the pager."
                      )

find_package(USB)
set(HAVE_LIBUSB ${LIBUSB_FOUND})
set_package_properties(USB PROPERTIES DESCRIPTION "User level access to USB devices"
                       URL "http://libusb.sourceforge.net"
                       TYPE OPTIONAL
                       PURPOSE "Provides Logitech mouse support in KControl."
                      )

find_package(Fontconfig)
set_package_properties(Fontconfig PROPERTIES DESCRIPTION "Font access configuration library"
                       URL "http://www.freedesktop.org/wiki/Software/fontconfig"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build font configuration and installation tools"
                      )

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "http://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace"
                      )

find_package(XCB
    REQUIRED COMPONENTS
        XCB SHM IMAGE
    OPTIONAL_COMPONENTS
        XKB
)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XCB-XKB" XCB_XKB_FOUND "Required for building kcm/keyboard")
add_feature_info("libxft" X11_Xft_FOUND "X FreeType interface library required for font installation")

include(ConfigureChecks.cmake)

if(${Qt5Gui_OPENGL_IMPLEMENTATION} STREQUAL "GL")
    find_package(OpenGL)
    set_package_properties(OpenGL PROPERTIES DESCRIPTION "The OpenGL libraries"
                        URL "http://www.opengl.org"
                        TYPE REQUIRED
                        )
else()
    find_package(OpenGLES)
    set_package_properties(OpenGLES PROPERTIES DESCRIPTION "The OpenGLES libraries"
                        URL "http://www.khronos.org/opengles"
                        TYPE REQUIRED
                        )
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)
configure_file(config-runtime.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-runtime.h)

set(PLASMADESKTOP_VERSION_MAJOR 5)
set(PLASMADESKTOP_VERSION_MINOR 0)
set(PLASMADESKTOP_VERSION_PATCH 2)
set(PLASMADESKTOP_VERSION ${PLASMADESKTOP_VERSION_MAJOR}.${PLASMADESKTOP_VERSION_MINOR}.${PLASMADESKTOP_VERSION_PATCH} )

set(GENERIC_LIB_SOVERSION ${PLASMADESKTOP_VERSION_MAJOR})
set(GENERIC_LIB_VERSION "${PLASMADESKTOP_VERSION_MAJOR}.${PLASMADESKTOP_VERSION_MINOR}.${PLASMADESKTOP_VERSION_PATCH}")

plasma_install_package(desktoppackage org.kde.plasma.desktop shells shell)

add_subdirectory(doc)
# add_subdirectory(ktouchpadenabler)
# add_subdirectory(solid-actions-kcm)

# add_subdirectory(runners)
add_subdirectory(containments)
add_subdirectory(toolboxes)
add_subdirectory(applets)
add_subdirectory(kcms)
add_subdirectory(knetattach)
add_subdirectory(attica-kde)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
