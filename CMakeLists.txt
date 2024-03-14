cmake_minimum_required(VERSION 3.16)
set (PROJECT_NAME "EsetVM")
project (${PROJECT_NAME} VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILER_FLAGS}")

enable_testing()

set (EXECUTABLE_NAME ${PROJECT_NAME})
set (SOURCE_FILES src/EVMDisasm.cpp src/EVMFile.cpp src/utils.cpp src/ESETVM.cpp src/CLIArgParser.cpp src/BitStreamReader.cpp src/EVMExecutionUnit.cpp)
set (HEADER_FILES src/EVMDisasm.h src/EVMFile.h src/utils.h src/ESETVM.h src/CLIArgParser.h src/BitStreamReader.h src/EVMTypes.h src/EVMExecutionUnit.h)

# Create a library containing common source files
add_library(EsetVMLibrary STATIC ${SOURCE_FILES} ${HEADER_FILES})

add_executable (${EXECUTABLE_NAME} src/main.cpp)

# Link the library to the main executable
target_link_libraries(${EXECUTABLE_NAME} PRIVATE EsetVMLibrary)

add_subdirectory(test)