include_directories(${PROJECT_SOURCE_DIR}/src/libepub)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wlogical-op -Weffc++ -Werror")

add_executable(run_tests
    ${PROJECT_SOURCE_DIR}/src/libepub/path.c
    ${PROJECT_SOURCE_DIR}/src/libepub/path.h
    path_test.cxx
    run_tests.cxx)

target_link_libraries(run_tests CppUTest)
