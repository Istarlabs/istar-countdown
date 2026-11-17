# SPDX-FileCopyrightText: 2026 Pascal Klose
# SPDX-License-Identifier: MIT

# QML modules of an executable go under :/qt/qml, which is where loadFromModule() looks. Without the
# policy they land under :/ and the module is found but empty: 'contains no type named "Main"'.
qt_policy(SET QTP0001 NEW)

qt_add_executable(istar-countdown main.cpp)

qt_add_qml_module(istar-countdown
    URI net.kurokyuu.istarcountdown
    VERSION 1.0
    QML_FILES
        qml/AboutAppPage.qml
        qml/CountdownDialog.qml
        qml/CountdownsPage.qml
        qml/GradientText.qml
        qml/Main.qml
        qml/TimerPage.qml
        qml/WorkTimePage.qml
    SOURCES
        about.h
        countdownmodel.cpp
        countdownmodel.h
        countdowntimer.cpp
        countdowntimer.h
        occurrence.cpp
        occurrence.h
        worktime.cpp
        worktime.h
)

target_compile_definitions(istar-countdown PRIVATE ISTAR_COUNTDOWN_VERSION="${PROJECT_VERSION}")

target_link_libraries(istar-countdown PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::I18nQml
    KF6::Kirigami
)

# An APK is made from the built library by create-apk, not from an install, and has no menu entry (its
# manifest takes that role). The only install rule left on Android is the icons below.
if(NOT ANDROID)
    target_link_libraries(istar-countdown PRIVATE
        Qt6::DBus
        Qt6::Widgets
        KF6::DBusAddons
        KF6::Notifications
        KF6::StatusNotifierItem
        KF6::WindowSystem
    )

    # Named after KAboutData's component name; without it KNotification::event() shows nothing.
    install(FILES istar-countdown.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
    install(TARGETS istar-countdown ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

    # The full path, not just the name: a session started before ~/.local/bin existed does not have it in
    # its PATH, and a menu entry that cannot find its program is the one failure nobody sees an error for.
    configure_file(net.kurokyuu.istar-countdown.desktop.in
                   ${CMAKE_CURRENT_BINARY_DIR}/net.kurokyuu.istar-countdown.desktop @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/net.kurokyuu.istar-countdown.desktop DESTINATION ${KDE_INSTALL_APPDIR})
else()
    # Android has no icon theme to look the names in the QML up in; this packs the Breeze icons the app
    # names (Kirigami adds its own) as a theme Kirigami finds, "breeze-internal". Installing it into the
    # staging root is a step of `cpp.py android build`. BREEZEICONS_DIR points at a breeze-icons
    # checkout; without it the macro clones one into the build directory.
    # The first four are the ones the Breeze style draws its spin boxes and arrows with (see
    # qqc2-breeze-style's SpinBoxIndicator); without them the buttons beside the number are empty.
    kirigami_package_breeze_icons(ICONS
        arrow-down
        arrow-left
        arrow-right
        arrow-up
        appointment-new
        chronometer
        chronometer-pause
        chronometer-start
        document-edit
        edit-delete
        help-about
        list-add
        view-calendar
        view-calendar-birthday
        view-calendar-day
        view-calendar-month
        view-calendar-week
    )

    include(ECMAddAndroidApk)
    ecm_add_android_apk(istar-countdown ANDROID_DIR ${CMAKE_SOURCE_DIR}/android)
endif()
