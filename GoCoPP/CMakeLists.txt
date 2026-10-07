cmake_minimum_required(VERSION 3.10)
project(GoCoPP LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/${CMAKE_BUILD_TYPE})

# Boost imported targets
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

# Make pixi/conda prefixes discoverable by find_package() in config mode.
# Covers local builds (pixi run, CONDA_PREFIX) and rattler-build (pixi publish,
# PREFIX). The relative path is a fallback for the pixi dev environment.
foreach(env_prefix
        "$ENV{CONDA_PREFIX}"
        "$ENV{PIXI_ENV_PREFIX}"
        "$ENV{PREFIX}"
        "${CMAKE_SOURCE_DIR}/../.pixi/envs/default")
    if(EXISTS "${env_prefix}")
        list(APPEND CMAKE_PREFIX_PATH "${env_prefix}")
    endif()
endforeach()

# ---------- Dependencies ----------

find_package(Eigen3 3.4 REQUIRED CONFIG)
find_package(Boost 1.76 REQUIRED COMPONENTS filesystem system)
find_package(CLI11 2.4 REQUIRED CONFIG)
find_package(CGAL 5.6 REQUIRED)

message(STATUS "Eigen3 version: ${Eigen3_VERSION}")
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "CLI11 version: ${CLI11_VERSION}")
message(STATUS "CGAL version: ${CGAL_VERSION}")

# ---------- Target ----------

file(GLOB_RECURSE psd_HDRS include/*)
file(GLOB_RECURSE psd_SRCS src/*)

add_executable(${PROJECT_NAME} ${psd_HDRS} ${psd_SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE include)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CONSOLE _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    CGAL::CGAL
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    CLI11::CLI11
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME gocopp)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

### For clangd
# Libraries installed by pixi
set(PIXI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../.pixi/envs/default/include)
target_compile_options(${PROJECT_NAME} PRIVATE -I${CMAKE_SOURCE_DIR}/include)
target_compile_options(${PROJECT_NAME} PRIVATE -I${PIXI_INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -I${PIXI_INCLUDE_DIR}/eigen3)