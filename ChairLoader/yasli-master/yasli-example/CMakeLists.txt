cmake_minimum_required(VERSION 2.8)
project("yasli-example")

include_directories(.)

set(SOURCES Example.cpp)
source_group("" FILES ${SOURCES})
add_executable("yasli-example" ${SOURCES})
set_target_properties("yasli-example" PROPERTIES DEBUG_POSTFIX "-debug")
set_target_properties("yasli-example" PROPERTIES RELWITHDEBINFO_POSTFIX "-relwithdebinfo")
target_link_libraries("yasli-example" "yasli")
