
#
# Example
#


add_definitions(-std=c++98)

# Required to find the FindOpenCL.cmake file
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

find_package( OpenCL REQUIRED )

include_directories( ${OPENCL_INCLUDE_DIRS} )

if( OPENCL_HAS_CPP_BINDINGS )
        message( "OpenCL has C++ bindings. Full include is: " ${OPENCL_INCLUDE_DIRS} )
else( OPENCL_HAS_CPP_BINDINGS )
        message( "No OpenCL C++ bindings found. Full include is: " ${OPENCL_INCLUDE_DIRS} )
endif( OPENCL_HAS_CPP_BINDINGS )




include_directories("${PROJECT_SOURCE_DIR}/src")
add_executable(OclUtilsExample Example.cpp)

target_link_libraries(OclUtilsExample oclutils ${OPENCL_LIBRARIES})
