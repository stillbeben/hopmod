cmake_minimum_required(VERSION 2.6)
project(Lua)
add_subdirectory(src)
set(LUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src PARENT_SCOPE)
