cmake_minimum_required(VERSION 2.8)
 
PROJECT(mesh2depth)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Find Opencv
FIND_PACKAGE(OpenCV 2.4.8 REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
MESSAGE(STATUS "Compiling with OpenCV")
MESSAGE(STATUS "\tOpenCV_INCLUDE_DIRS: " ${OpenCV_INCLUDE_DIRS})
MESSAGE(STATUS "\tOpenCV_LIBS: " ${OpenCV_LIBS})
 
add_executable(mesh2depth MACOSX_BUNDLE mesh2depth.cxx)
 
if(VTK_LIBRARIES)
  target_link_libraries(mesh2depth ${VTK_LIBRARIES} ${OpenCV_LIBS})
else()
  target_link_libraries(mesh2depth vtkHybrid vtkWidgets)
endif()
