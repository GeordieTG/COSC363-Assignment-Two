cmake_minimum_required(VERSION 2.8)

project(AssignmentTwo)

add_executable(RayTracer.out RayTracer.cpp Ray.cpp SceneObject.cpp Sphere.cpp Plane.cpp TextureBMP.cpp cylinder.h cylinder.cpp cone.h cone.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} )

target_link_libraries( RayTracer.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLM_LIBRARY} )

