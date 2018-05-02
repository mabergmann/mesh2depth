cmake_minimum_required(VERSION 2.8)
 
PROJECT(ReadOBJ)
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Find Opencv
FIND_PACKAGE(OpenCV 2.4.8 REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
MESSAGE(STATUS "Compiling with OpenCV")
MESSAGE(STATUS "\tOpenCV_INCLUDE_DIRS: " ${OpenCV_INCLUDE_DIRS})
MESSAGE(STATUS "\tOpenCV_LIBS: " ${OpenCV_LIBS})
 
add_executable(ReadOBJ MACOSX_BUNDLE ReadOBJ.cxx)
 
if(VTK_LIBRARIES)
  target_link_libraries(ReadOBJ ${VTK_LIBRARIES} ${OpenCV_LIBS})
else()
  target_link_libraries(ReadOBJ vtkHybrid vtkWidgets)
endif()
