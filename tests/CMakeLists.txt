# SPDX-FileCopyrightText: 2026 Pascal Klose
# SPDX-License-Identifier: MIT

# Each test compiles the source it checks directly, as the program itself is built from a QML
# module and there is no library to link.
function(add_countdown_test name)
    add_executable(${name} ${name}.cpp ${ARGN})
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_link_libraries(${name} PRIVATE Qt6::Test Qt6::Gui Qt6::Qml Qt6::DBus KF6::ConfigCore KF6::I18n)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_countdown_test(occurrencetest ${SRC}/occurrence.cpp ${SRC}/occurrence.h)
add_countdown_test(worktimetest ${SRC}/worktime.cpp ${SRC}/worktime.h)
add_countdown_test(countdowntimertest ${SRC}/countdowntimer.cpp ${SRC}/countdowntimer.h
    ${SRC}/occurrence.cpp ${SRC}/occurrence.h)
add_countdown_test(countdownmodeltest ${SRC}/countdownmodel.cpp ${SRC}/countdownmodel.h
    ${SRC}/occurrence.cpp ${SRC}/occurrence.h)
