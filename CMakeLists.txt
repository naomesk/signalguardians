cmake_minimum_required(VERSION 3.16)
project(signalguardians LANGUAGES CXX)

add_library(signalguardians_messages src/message.cpp src/scheduling.cpp)
target_include_directories(signalguardians_messages PUBLIC include)
target_compile_features(signalguardians_messages PUBLIC cxx_std_17)
set_target_properties(signalguardians_messages PROPERTIES CXX_EXTENSIONS OFF)

add_executable(signalguardians src/main.cpp)
target_link_libraries(signalguardians PRIVATE signalguardians_messages)

add_executable(priority_demo examples/priority_demo.cpp)
target_link_libraries(priority_demo PRIVATE signalguardians_messages)

include(CTest)
if(BUILD_TESTING)
    add_executable(message_tests tests/message_tests.cpp)
    target_link_libraries(message_tests PRIVATE signalguardians_messages)
    add_test(NAME message_tests COMMAND message_tests)

    add_executable(scheduling_tests tests/scheduling_tests.cpp)
    target_link_libraries(scheduling_tests PRIVATE signalguardians_messages)
    add_test(NAME scheduling_tests COMMAND scheduling_tests)
endif()
