cmake_minimum_required(VERSION 3.6)
project(TruePass-common-tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)

set(PROJECT_SOURCES 
    "UnitTests/kmac.test.cpp"
    "UnitTests/curves.test.cpp"
    "UnitTests/uuid.test.cpp" 
    "UnitTests/BinEncoding.test.cpp")

add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    gtest_main
    TruePass-common
)

# Register test with CTest
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})