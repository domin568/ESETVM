include(FetchContent)

set(CMAKE_CXX_FLAGS "-DGTEST")

add_compile_definitions("TEST_FOLDER=${CMAKE_CURRENT_SOURCE_DIR}")

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

find_library(EsetVMLibrary EsetVM)
add_executable(esetvm_test test.cpp)

target_link_libraries(esetvm_test
 PRIVATE
  GTest::GTest
  EsetVMLibrary
)

add_test(esetvm_gtests esetvm_test)
