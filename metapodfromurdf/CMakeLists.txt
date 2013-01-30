## Copyright (c) 2012, 2013 Aldebaran Robotics. All rights reserved
## Use of this source code is governed by a BSD-style license that can be
## found in the COPYING.bsd file
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/robotbuilder/include
    ${PROJECT_BINARY_DIR}/robotbuilder/include
    ${Boost_INCLUDE_DIRS}) # Eigen is already in the include path

ADD_EXECUTABLE(metapodfromurdf
  src/metapodfromurdf.cpp)
TARGET_LINK_LIBRARIES(metapodfromurdf
  ${Boost_LIBRARIES}
  metapod_robotbuilder)
FOREACH(dep rosconsole urdf)
  ADD_ROSPACK_DEPENDENCY(${dep})
  ROSPACK_USE_DEPENDENCY(metapodfromurdf ${dep})
  # the following command is a workaround
  # see https://github.com/jrl-umi3218/jrl-cmakemodules/issues/20
  TARGET_LINK_LIBRARIES(metapodfromurdf ${dep})
ENDFOREACH()

GET_PROPERTY(METAPODFROMURDF_EXECUTABLE
  TARGET metapodfromurdf PROPERTY LOCATION)
SET(METAPODFROMURDF_EXECUTABLE ${METAPODFROMURDF_EXECUTABLE} CACHE
  STRING "Set by cmake if metapodfromurdf is built, by the user otherwise")