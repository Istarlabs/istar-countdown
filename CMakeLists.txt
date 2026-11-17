# SPDX-FileCopyrightText: 2026 Pascal Klose
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

project(istar-countdown VERSION 0.1.0 LANGUAGES CXX)

set(QT_MIN_VERSION 6.7.0)
set(KF_MIN_VERSION 6.0.0)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

# Deliberately no include(KDEClangFormat): it rewrites .clang-format at every configure with ECM's
# copy, and this repository's style differs from it - see the file itself.

# KDECompilerSettings pins the standard to 17, so this has to come after it.
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
if(NOT ANDROID)
    # The task manager progress bar is a D-Bus signal; the tray icon's context menu is a QMenu.
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS DBus Widgets)
    # One instance per session, the tray icon, raising the window from a second launch on Wayland, and the
    # notification when a countdown is over.
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS DBusAddons Notifications StatusNotifierItem WindowSystem)
endif()
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config I18n Kirigami)

add_subdirectory(src)

if(BUILD_TESTING AND NOT ANDROID)
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
    add_subdirectory(tests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# Compiles every po/<lang>/istar-countdown.po to a .mo and installs it where KLocalizedString
# looks. The domain it is found under is the one main() passes to setApplicationDomain(). On Android
# androiddeployqt packs share/locale of the staging root, which is where `cpp.py android build` puts it
# (the frameworks' own translations get into the APK the same way).
ki18n_install(po)

# The RPM is built by devscripts' rpm.py from `cmake --install` with prefix /usr: [rpm] in Scripts/project.toml.
