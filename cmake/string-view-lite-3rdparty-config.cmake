get_filename_component(_IMPORT_PREFIX "${PROJECT_SOURCE_DIR}/3rdparty/string-view-lite/" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

set_and_check(STRING_VIEW_LITE_INCLUDE_DIR ${_IMPORT_PREFIX}/include)
set(STRING_VIEW_LITE_INCLUDE_DIRS "${STRING_VIEW_LITE_INCLUDE_DIR}")

add_library(string-view-lite INTERFACE)

target_include_directories(string-view-lite
  INTERFACE
    ${STRING_VIEW_LITE_INCLUDE_DIRS}
)
