cmake_minimum_required(VERSION 3.20)
project(MovieBookingBackend LANGUAGES CXX)

enable_testing()

# ======================================================================
# C++ standard
# ======================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x1000 -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING )
endif()

# ======================================================================
# Conan packages (CMakeDeps)
# ======================================================================
find_package(Boost REQUIRED COMPONENTS system)
find_package(RapidJSON REQUIRED)
find_package(GTest REQUIRED)

# ======================================================================
# server executable
# ======================================================================
add_executable(movie_booker
    src/MovieBookerMain.cpp
    src/AsioServer.cpp
    src/MovieBooker.cpp
)

target_include_directories(movie_booker PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(movie_booker
    PRIVATE Boost::boost
    PRIVATE rapidjson
)
target_compile_options(movie_booker PRIVATE
        -fpermissive
        -Wno-error
)

# ======================================================================
# client executable
# ======================================================================
add_executable(movie_booker_client
    src/movie_booker_client.cpp
    src/AsioClient.cpp
)

target_include_directories(movie_booker_client PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(movie_booker_client
    PRIVATE Boost::boost
)

# ======================================================================
# Tests
# ======================================================================
add_executable(movie_booker_tests
    tests/tests.cpp
	src/MovieBooker.cpp
	src/AsioServer.cpp
	tests/AsioServer_tests.cpp

)

target_include_directories(movie_booker_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(movie_booker_tests
    PRIVATE Boost::boost
	PRIVATE GTest::gtest
	PRIVATE GTest::gmock
    PRIVATE GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(movie_booker_tests)

# ======================================================================
# Status message
# ======================================================================
message(STATUS "Server, client, and tests configured successfully.")