
#
# Library
#

set(SRCS OclUtils.cpp)

add_definitions(-std=c++98)

# Uncomment to use SHA512 checksumming support on data
# add_definitions(-DOpenCLSHA512Checksum)

# Required to find the FindOpenCL.cmake file
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
find_package( OpenCL REQUIRED )
include_directories( ${OPENCL_INCLUDE_DIRS} )
if( OPENCL_HAS_CPP_BINDINGS )
        message( "OpenCL has C++ bindings. Full include is: " ${OPENCL_INCLUDE_DIRS} )
else( OPENCL_HAS_CPP_BINDINGS )
        message( "No OpenCL C++ bindings found. Full include is: " ${OPENCL_INCLUDE_DIRS} )
endif( OPENCL_HAS_CPP_BINDINGS )



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
