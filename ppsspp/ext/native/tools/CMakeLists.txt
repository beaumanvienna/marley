cmake_minimum_required(VERSION 2.6)

project (Tools)

add_definitions(-O2)
add_definitions(-Wall)
add_definitions(-DSDL)
add_definitions(-Wno-multichar)
add_definitions(-fno-strict-aliasing)

if(IOS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
elseif(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -mmacosx-version-min=10.7")
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7.0)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(prebuilt)
include_directories(../../)
include_directories(../../../)

add_subdirectory(../../libpng17 png17)


add_executable(atlastool atlastool.cpp)
target_link_libraries(atlastool freetype util png17 z ${CMAKE_SOURCE_DIR}/../../../build/lib/libCommon.a png17 z)

add_executable(zimtool zimtool.cpp)
target_link_libraries(zimtool freetype png17 z ${CMAKE_SOURCE_DIR}/../../../build/lib/libCommon.a png17 z)
