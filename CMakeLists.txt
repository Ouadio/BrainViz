PROJECT(BRAINVIZ LANGUAGES CXX)
cmake_minimum_required(VERSION 3.0)


FIND_PACKAGE(VTK 9.0.20210317 EXACT REQUIRED ) 
set(VTK_FOUND 1)


include_directories(
  ${VTK_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}
)


SET(FILES "app") 
FOREACH(f ${FILES})
  ADD_EXECUTABLE(${f} ${f}.cpp)
  TARGET_LINK_LIBRARIES(${f} ${VTK_LIBRARIES}) 
  target_include_directories(${f} PUBLIC "${VTK_DIR}")

ENDFOREACH(f)


