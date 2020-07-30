cmake_minimum_required(VERSION 3.7)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

ExternalProject_Add(
        git_catch
        PREFIX ${CMAKE_BINARY_DIR}/catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        TIMEOUT 10
        UPDATE_COMMAND ${GIT_EXECUTABLE} pull
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(git_catch source_dir)

add_library(catch2_main STATIC catch-main.cpp)
add_dependencies(catch2_main git_catch)
target_include_directories(catch2_main PUBLIC ${source_dir}/single_include  ${source_dir}/single_include/catch2)

set(TEST_LIST
        expression.cpp cnf.cpp solver.cpp model.cpp)

set(TARGET_NAME tests)

add_executable(${TARGET_NAME}
        ${TEST_LIST})

target_link_libraries(${TARGET_NAME} catch2_main)
target_link_libraries(${TARGET_NAME} complyer-sat)

add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})