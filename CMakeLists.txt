cmake_minimum_required (VERSION 2.6)
include(FindPkgConfig)

############################
# General project settings #
############################

PROJECT(3D_uvc)
SET(APPLICATIONS_VERSION_MAJOR 0)
SET(APPLICATIONS_VERSION_MINOR 1)

FIND_PACKAGE(VTK REQUIRED)

INCLUDE(${VTK_USE_FILE})

PKG_CHECK_MODULES(XML libxml++-2.6)



INCLUDE_DIRECTORIES($ENV{OBVIOUSLY_ROOT}
                   )

LINK_DIRECTORIES($ENV{OBVIOUSLY_ROOT}/build/release/obcore $ENV{OBVIOUSLY_ROOT}/build/release/obvision $ENV{OBVIOUSLY_ROOT}/build/release/obgraphic $ENV{OBVIOUSLY_ROOT}/build/release/obdevice)

ADD_EXECUTABLE(3D_uvc src/3D_uvc.cpp)


SET(CORELIBS obcore gsl gslcblas)
SET(DEVICELIBS obdevice OpenNI v4l2 udev)
SET(GRAPHICLIBS obgraphic vtkHybrid glut GL jpeg)
SET(VISIONLIBS obvision ann flann)


TARGET_LINK_LIBRARIES(3D_uvc ${GRAPHICLIBS} ${DEVICELIBS} ${CORELIBS} ${OPENCV})

