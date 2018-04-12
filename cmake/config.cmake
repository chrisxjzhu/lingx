
set(LNX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

if(NOT LNX_PREFIX)
  set(LNX_PREFIX "/usr/local/lingx")
endif()

if(NOT LNX_CONF_PATH)
  set(LNX_CONF_PATH "conf/lingx.conf")
endif()

if(NOT LNX_ERROR_LOG_PATH)
  set(LNX_ERROR_LOG_PATH "logs/error.log")
elseif("${LNX_ERROR_LOG_PATH}" STREQUAL "stderr")
  set(LNX_ERROR_LOG_PATH "")
endif()

if(NOT LNX_PID_PATH)
  set(LNX_PID_PATH "logs/lingx.pid")
endif()
