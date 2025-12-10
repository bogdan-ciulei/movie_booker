cmake_minimum_required(VERSION 3.20)
project(MovieBookingBackend LANGUAGES CXX)

# ======================================================================
# C++ standard
# ======================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif()

# ======================================================================
# Conan integration
# ======================================================================
if(EXISTS "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
    include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)
endif()

find_package(Boost REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(GTest REQUIRED)

# ======================================================================
# Include path
# ======================================================================
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ======================================================================
# server executable
# ======================================================================
add_executable(movie_booker_server
    src/MovieBookerMain.cpp
    src/AsioServer.cpp
    src/MovieBooker.cpp
)

target_link_libraries(movie_booker_server
    PRIVATE Boost::boost
    PRIVATE Boost::system
)

target_include_directories(movie_booker_server PRIVATE ${RapidJSON_INCLUDE_DIRS})

# ======================================================================
# client executable
# ======================================================================
add_executable(movie_booker_client
    src/MovieBookerClient.cpp
    src/AsioClient.cpp
)

target_link_libraries(movie_booker_client
    PRIVATE Boost::boost
    PRIVATE Boost::system
)

target_include_directories(movie_booker_client PRIVATE ${RapidJSON_INCLUDE_DIRS})

# ======================================================================
# Tests
# ======================================================================
add_executable(movie_backend_tests
    tests/tests.cpp
)

target_link_libraries(movie_backend_tests
    PRIVATE GTest::gtest
    PRIVATE GTest::gtest_main
    PRIVATE Boost::boost
    PRIVATE Boost::system
)

target_include_directories(movie_backend_tests PRIVATE ${RapidJSON_INCLUDE_DIRS})

include(GoogleTest)
gtest_discover_tests(movie_backend_tests)

# ======================================================================
# Status message
# ======================================================================
message(STATUS "Configured server, client, and tests successfully.")