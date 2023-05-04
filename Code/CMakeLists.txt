cmake_minimum_required(VERSION 2.8)

project(lab7)

add_executable(RayTracer.out RayTracer.cpp Ray.cpp SceneObject.cpp Sphere.cpp Cylinder.cpp Cone.cpp Plane.cpp TextureBMP.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} )

target_link_libraries( RayTracer.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLM_LIBRARY} )

