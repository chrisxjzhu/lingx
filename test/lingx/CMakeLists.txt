macro(_add_test name)
  add_executable(${name} ${name}.cc)
  add_test(${name} ${name})
endmacro()

add_compile_options(-Wall -Wextra -Wpointer-arith)
include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

add_subdirectory(core)
