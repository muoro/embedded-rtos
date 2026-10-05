cmake_minimum_required(VERSION 3.21)
project(smart-room-ui VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 Network)
qt_standard_project_setup()
add_library(dashboard_core STATIC
    src/network/GatewayConnection.cpp src/network/GatewayConnection.hpp
    src/protocol/GatewayProtocol.cpp src/protocol/GatewayProtocol.hpp
    src/presentation/RoomViewModel.cpp src/presentation/RoomViewModel.hpp)
target_link_libraries(dashboard_core PUBLIC Qt6::Network)
qt_add_executable(smart-room-ui src/main.cpp)
set_target_properties(smart-room-ui PROPERTIES WIN32_EXECUTABLE TRUE)
target_link_libraries(smart-room-ui PRIVATE dashboard_core Qt6::Quick Qt6::QuickControls2)
set(DASHBOARD_QML qml/Main.qml qml/Panel.qml qml/RoomControl.qml
    qml/RoomMap.qml qml/FlowPanel.qml qml/EventPanel.qml
    qml/OverviewPage.qml qml/FlowPage.qml)
qt_add_resources(smart-room-ui qml PREFIX "/" FILES ${DASHBOARD_QML})

option(BUILD_TESTING "Build Qt UI and reconnect tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(ui-tests tests/ui_tests.cpp)
    target_link_libraries(ui-tests PRIVATE dashboard_core Qt6::Quick Qt6::QuickControls2 Qt6::Test)
    qt_add_resources(ui-tests qml-test PREFIX "/" FILES ${DASHBOARD_QML})
    add_test(NAME qt-ui-network COMMAND ui-tests)
    get_filename_component(QT_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
    set_tests_properties(qt-ui-network PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_PLUGIN_PATH=${QT_PREFIX}/plugins" TIMEOUT 30)
endif()
