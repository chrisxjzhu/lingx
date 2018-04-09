macro(_add_core_test name)
  _add_test(${name})
  target_link_libraries(${name} PRIVATE lingx_core)
endmacro()

_add_core_test(strings_test)
_add_core_test(errno_test)
