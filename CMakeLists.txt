cmake_minimum_required(VERSION 3.20)
project(dag-sched CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -Wno-deprecated-declarations -Wno-unused-variable")

#-------------------------------------------------------------------------------
# Options
#-------------------------------------------------------------------------------

option(WITH_ZAHAF "Compiling also for Zahaf2019 test (if you have the rights to access the repo)" OFF)

#-------------------------------------------------------------------------------
# External Libraries
#-------------------------------------------------------------------------------
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package(yaml-cpp REQUIRED)

find_package(Python3 COMPONENTS Interpreter Development)
message("Python_FOUND:${Python3_FOUND}")
# message("Python_VERSION:${Python3_VERSION}")
# message("Python_Development_FOUND:${Python3_Development_FOUND}")
message("Python_LIBRARIES:${Python3_LIBRARIES}")
message("Python_INCLUDE_DIRS:${Python3_INCLUDE_DIRS}")
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("NUMPY_INCLUDE_DIR:${NUMPY_INCLUDE_DIR}")
include_directories(${NUMPY_INCLUDE_DIR})

#-------------------------------------------------------------------------------
# Submodules
#-------------------------------------------------------------------------------

add_subdirectory(np-schedulability-analysis)

if(WITH_ZAHAF)
    add_compile_definitions(ZAHAF2019)
    add_subdirectory(rt_compiler)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rt_compiler ${CMAKE_CURRENT_SOURCE_DIR}/rt_compiler/build)
endif()

#-------------------------------------------------------------------------------
# Build libraries
#-------------------------------------------------------------------------------


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include matplotlib-cpp 
                    ${CMAKE_CURRENT_SOURCE_DIR}/np-schedulability-analysis/include 
                    ${Python3_INCLUDE_DIRS})

file(GLOB dag-sched-SRC "src/*/*.cpp" "src/*.cpp")

if(WITH_ZAHAF)
    set(dag-sched-LIBS yaml-cpp ${Python3_LIBRARIES} tbb rt_compiler)
else()
    set(dag-sched-LIBS yaml-cpp ${Python3_LIBRARIES} tbb)
endif()

add_library(dag-sched SHARED ${dag-sched-SRC})
target_link_libraries(dag-sched ${dag-sched-LIBS} )

#-------------------------------------------------------------------------------
# Build executables
#-------------------------------------------------------------------------------
add_executable(demo demo/main.cpp)
target_link_libraries(demo dag-sched)

add_executable(eval demo/evaluate.cpp)
target_link_libraries(eval dag-sched)
