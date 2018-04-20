add_executable(lingx lingx.cc)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_compile_options(-Wall -Wextra -Wpointer-arith)

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

target_link_libraries(lingx PRIVATE lingx_core)

add_subdirectory(core)
