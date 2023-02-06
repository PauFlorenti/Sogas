project(Sandbox LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp)

if(MSVC)
    target_compile_options(${PROJECT_NAME}
        PRIVATE
        -W4 -WX)
else()
    add_compile_options(${PROJECT_NAME}
        PRIVATE
        -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SogasEngine)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sogasengine/public
    ${CMAKE_SOURCE_DIR}/sogasengine/external)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(Sandbox PROPERTIES LINK_FLAGS "/WHOLEARCHIVE:sogasengine")