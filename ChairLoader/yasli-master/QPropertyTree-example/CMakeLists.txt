# makefile
cmake_minimum_required(VERSION 2.8)
set(CMAKE_AUTOMOC true)

project("QPropertyTreeTest")

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif (MSVC)

find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif (UNIX)

#file(GLOB SOURCES *.h *.cpp)
set(SOURCES 
	main.cpp
	ExampleControlCharacters.cpp
	ExampleCustomRows.cpp
	ExamplePolymorphicTree.cpp
	ExampleTable.cpp
	ExampleTwoTrees.h
	ExampleTwoTrees.cpp
	MainWindow.cpp MainWindow.h
  )
source_group("" FILES ${SOURCES})

add_executable(qpropertytree-example ${SOURCES})

target_link_libraries(qpropertytree-example yasli qpropertytree)
set_target_properties(qpropertytree-example PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties(qpropertytree-example PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")

if (MSVC_IDE)
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR})
    set_target_properties(qpropertytree-example PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PROJECT_SOURCE_DIR})
endif (MSVC_IDE)
