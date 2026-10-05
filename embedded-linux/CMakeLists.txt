cmake_minimum_required(VERSION 3.20)
project(embedded_linux LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(room_core STATIC
    src/protocol/line_framer.cpp
    src/protocol/protocol.cpp
    src/gateway/controller.cpp
)
target_include_directories(room_core PUBLIC src)
target_compile_options(room_core PRIVATE -Wall -Wextra -Wpedantic)

add_library(asio_headers INTERFACE)
target_include_directories(asio_headers SYSTEM INTERFACE third_party/asio/include)
target_compile_definitions(asio_headers INTERFACE ASIO_STANDALONE)
target_link_libraries(asio_headers INTERFACE Threads::Threads)

add_executable(device-gateway
    src/main.cpp
    src/logging/logger.cpp
    src/app/gateway_application.cpp
    src/serial/serial_link.cpp
    src/network/server.cpp
    src/network/tcp_session.cpp
)
target_link_libraries(device-gateway PRIVATE room_core asio_headers)
target_compile_options(device-gateway PRIVATE -Wall -Wextra -Wpedantic)
install(TARGETS device-gateway RUNTIME DESTINATION bin)

option(BUILD_TESTING "Build native protocol and controller tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_executable(protocol-tests tests/protocol_tests.cpp)
    target_link_libraries(protocol-tests PRIVATE room_core)
    add_test(NAME protocol-controller COMMAND protocol-tests)
    if(NOT CMAKE_CROSSCOMPILING)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_test(NAME serial-tcp-integration COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration.py $<TARGET_FILE:device-gateway>)
        set_tests_properties(serial-tcp-integration PROPERTIES TIMEOUT 40)
        add_test(NAME logging-integration COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/logging.py $<TARGET_FILE:device-gateway>)
        set_tests_properties(logging-integration PROPERTIES TIMEOUT 30)
    endif()
endif()
