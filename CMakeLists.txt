cmake_minimum_required(VERSION 3.20)

project(PomoLyth VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_DISABLE_FIND_PACKAGE_WrapVulkanHeaders TRUE)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Concurrent)

qt_add_executable(PomoLyth
    app/main.cpp
    app/Application.h
    app/Application.cpp
    models/AppEvent.h
    models/DistractionEvent.h
    models/FocusSession.h
    models/FocusTask.h
    models/PetProfile.h
    core/EventBus.h
    core/EventBus.cpp
    core/PomodoroTimer.h
    core/PomodoroTimer.cpp
    core/PetStateMachine.h
    core/PetStateMachine.cpp
    core/AchievementSystem.h
    core/AchievementSystem.cpp
    core/FocusSessionManager.h
    core/FocusSessionManager.cpp
    ai/IAiClient.h
    ai/MockAiClient.h
    ai/MockAiClient.cpp
    ai/OpenAiClient.h
    ai/OpenAiClient.cpp
    ai/PromptBuilder.h
    ai/PromptBuilder.cpp
    ai/TaskPlanner.h
    ai/TaskPlanner.cpp
    ai/ReviewGenerator.h
    ai/ReviewGenerator.cpp
    monitor/IWindowMonitor.h
    monitor/WindowsWindowMonitor.h
    monitor/WindowsWindowMonitor.cpp
    monitor/FocusMonitor.h
    monitor/FocusMonitor.cpp
    storage/SQLiteStorage.h
    storage/SQLiteStorage.cpp
    storage/JsonStorage.h
    storage/JsonStorage.cpp
    storage/UserMemory.h
    storage/UserMemory.cpp
    ui/TimerPanel.h
    ui/TimerPanel.cpp
    ui/PetWidget.h
    ui/PetWidget.cpp
    ui/PetWindow.h
    ui/PetWindow.cpp
    ui/DesktopCalendarWindow.h
    ui/DesktopCalendarWindow.cpp
    ui/SettingsDialog.h
    ui/SettingsDialog.cpp
    ui/ReviewDialog.h
    ui/ReviewDialog.cpp
    ui/DashboardWindow.h
    ui/DashboardWindow.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
)

target_include_directories(PomoLyth PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(PomoLyth PRIVATE Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Concurrent)

add_custom_command(TARGET PomoLyth POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PomoLyth>/ui"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/pink_theme.qss"
        "$<TARGET_FILE_DIR:PomoLyth>/ui/pink_theme.qss")

if(WIN32)
    target_link_libraries(PomoLyth PRIVATE psapi)
endif()

if(MSVC)
    target_compile_options(PomoLyth PRIVATE /W4)
else()
    target_compile_options(PomoLyth PRIVATE -Wall -Wextra -Wpedantic)
endif()
