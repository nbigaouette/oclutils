
#
# Library
#

set(SRCS OclUtils.cpp)

add_definitions(-std=c++98)

# http://www.vtk.org/Wiki/CMake_FAQ#How_do_I_make_my_shared_and_static_libraries_have_the_same_root_name.2C_but_different_suffixes.3F
add_library(oclutils SHARED ${SRCS})
add_library(oclutils-static STATIC ${SRCS})
set_target_properties(oclutils-static PROPERTIES OUTPUT_NAME "oclutils")
set_target_properties(oclutils-static PROPERTIES PREFIX "lib")

install (FILES OclUtils.hpp DESTINATION include)
install(TARGETS oclutils oclutils-static
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
