cmake_minimum_required(VERSION 3.10)
project(GoCoPP LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

# Boost installation policy
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# ---------- Eigen ----------

# Method 1: Try CMake's config mode first (preferred for conda/pixi)
find_package(Eigen3 3.4 QUIET CONFIG)

if(NOT Eigen3_FOUND)
    # Method 2: Try module mode
    find_package(Eigen3 3.4 QUIET NO_MODULE)
endif()

if(NOT Eigen3_FOUND)
    # Method 3: Manual search for pixi/conda environments
    set(EIGEN3_POSSIBLE_PATHS
        "${CMAKE_SOURCE_DIR}/.pixi/envs/default/include/eigen3"  # pixi local env
        "$ENV{PIXI_ENV_PREFIX}/include/eigen3"                      # pixi env prefix
        "$ENV{PREFIX}/include/eigen3"                              # conda env
        "/usr/include/eigen3"
        "/usr/local/include/eigen3"
    )

    foreach(path IN LISTS EIGEN3_POSSIBLE_PATHS)
        if(EXISTS "${path}/Eigen/Core")
            set(EIGEN3_INCLUDE_DIRS "${path}")
            set(EIGEN3_FOUND TRUE)
            break()
        endif()
    endforeach()
endif()

if(NOT Eigen3_FOUND)
    message(FATAL_ERROR "Eigen3 not found. Install with: pixi add eigen")
endif()

message(STATUS "Using Eigen3 from: ${EIGEN3_INCLUDE_DIRS}")
include_directories(${EIGEN3_INCLUDE_DIRS})

# Prevent CGAL from searching for Eigen3 again
set(EIGEN3_FOUND TRUE)
set(EIGEN3_VERSION "3.4.0")

# ---------- Boost ----------

# Find Boost
find_package(Boost 1.76 REQUIRED COMPONENTS filesystem system)
if(Boost_FOUND)
    message(STATUS "Found Boost: ${Boost_VERSION}")
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else()
    message(FATAL_ERROR "Boost not found")
endif()

# ---------- CLI11 ----------

# Set up CLI11 paths - for pixi environments, it's typically in .pixi/envs/default/include
# Since CMAKE_SOURCE_DIR points to GoCoPP/GoCoPP, we need to go up one level to get to the project root
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)
set(CLI11_INCLUDE_DIR "${PROJECT_ROOT_DIR}/.pixi/envs/default/include")

# Check if CLI/CLI.hpp exists in the expected location
if(EXISTS "${CLI11_INCLUDE_DIR}/CLI/CLI.hpp")
    include_directories(${CLI11_INCLUDE_DIR})
    message(STATUS "Using CLI11")
    set(cli11_FOUND TRUE)
else()
    # Try alternative paths
    set(CLI11_ALTERNATIVE_PATHS
        "$ENV{PIXI_ENV_PREFIX}/include"
        "$ENV{PREFIX}/include"
        "/usr/include"
        "/usr/local/include"
    )
    
    foreach(path IN LISTS CLI11_ALTERNATIVE_PATHS)
        if(EXISTS "${path}/CLI/CLI.hpp")
            set(CLI11_INCLUDE_DIR "${path}")
            include_directories(${CLI11_INCLUDE_DIR})
            message(STATUS "Using CLI11")
            set(cli11_FOUND TRUE)
            break()
        endif()
    endforeach()
    
    if(NOT cli11_FOUND)
        message(FATAL_ERROR "CLI11 not found at ${CLI11_INCLUDE_DIR}/CLI/CLI.hpp or alternative paths. Install with: pixi add cli11")
    endif()
endif()

# ---------- CGAL ----------

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
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME gocopp)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

### For clangd
# Libraries installed by pixi
set(PIXI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../.pixi/envs/default/include)
target_compile_options(${PROJECT_NAME} PRIVATE -I${PIXI_INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -I${PIXI_INCLUDE_DIR}/eigen3)
