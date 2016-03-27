cmake_minimum_required(VERSION 3.4)
project(image_viewer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

set(SOURCE_FILES src/main.cpp src/DirectoryScanner.cc src/DirectoryScanner.hh)
add_executable(image_viewer ${SOURCE_FILES})

set_property(TARGET image_viewer PROPERTY CXX_STANDARD 14)
set_property(TARGET image_viewer PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(image_viewer ${SDL2_LIBRARIES})
target_include_directories(image_viewer PUBLIC ${SDL2_INCLUDE_DIR})

target_link_libraries(image_viewer ${SDL2_image_LIBRARIES})
target_include_directories(image_viewer PUBLIC ${SDL2_image_INCLUDE_DIR})

foreach(BINARY IN LISTS SDL2_BINARIES SDL2_image_BINARIES)
    configure_file(${BINARY} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()
