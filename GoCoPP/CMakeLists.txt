cmake_minimum_required(VERSION 3.10)
project(GoCoPP LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

# Find Eigen3 FIRST (before CGAL and Boost)
# Check common conda/pixi locations for Eigen3
set(EIGEN3_POSSIBLE_PATHS
    "$ENV{PREFIX}/include/eigen3"
    "$ENV{PIXI_ENV_PREFIX}/include/eigen3"
    "/usr/include/eigen3"
    "/usr/local/include/eigen3"
)

# Find the first existing path
foreach(path IN LISTS EIGEN3_POSSIBLE_PATHS)
    if(EXISTS "${path}/Eigen/Core")
        set(EIGEN3_INCLUDE_DIR "${path}")
        break()
    endif()
endforeach()

if(NOT EIGEN3_INCLUDE_DIR)
    message(FATAL_ERROR "Eigen3 not found in any standard location")
endif()

message(STATUS "Using Eigen3 from: ${EIGEN3_INCLUDE_DIR}")
include_directories(${EIGEN3_INCLUDE_DIR})

# Set this so CGAL's finder doesn't try to find Eigen again
set(EIGEN3_FOUND TRUE)
set(EIGEN3_VERSION "3.4.0")  # Explicit version to satisfy CGAL

# Find Boost
find_package(Boost 1.76 REQUIRED COMPONENTS filesystem system)
if(Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost not found")
endif()

# Find CGAL - it will use the already-found Eigen3
find_package(CGAL REQUIRED)
if(CGAL_FOUND)
    include(${CGAL_USE_FILE})
    message(STATUS "Found CGAL: ${CGAL_VERSION}")
else()
    message(FATAL_ERROR "CGAL not found")
endif()

add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE
    psd_HDRS
    include/*)

file(GLOB_RECURSE
    psd_SRCS
    src/*)

include_directories(include)

if(CGAL_FOUND)
    include(AddFileDependencies)
    add_executable(${PROJECT_NAME} ${psd_HDRS} ${psd_SRCS})
    target_link_libraries(${PROJECT_NAME} ${CGAL_LIBRARIES} Boost::filesystem)
else()
    message(FATAL_ERROR "CGAL is required to build this project")
endif()

# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION bin)