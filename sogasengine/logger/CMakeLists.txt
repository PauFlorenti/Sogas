
add_library(logger STATIC private/logger.cpp)

target_include_directories(logger PRIVATE public)