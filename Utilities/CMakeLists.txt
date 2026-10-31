cmake_minimum_required(VERSION 3.16)

project(Utilities LANGUAGES CXX)

add_library(Utilities STATIC Utilities.cpp Utf8.cpp)
target_include_directories(Utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(Utilities PUBLIC cxx_std_20)
