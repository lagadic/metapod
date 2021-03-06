# Copyright 2011, 2012, 2013, 2014
#
# Maxime Reis (JRL/LAAS, CNRS/AIST)
# Antonio El Khoury (JRL/LAAS, CNRS/AIST)
# Sébastien Barthélémy (Aldebaran Robotics)
# Nuno Guedelha (LAAS, CNRS)
#
# This file is part of metapod.
# metapod is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# metapod is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Lesser Public License for more details.
# You should have received a copy of the GNU Lesser General Public License
# along with metapod.  If not, see <http://www.gnu.org/licenses/>.
#
# Creation: 14/06/2011

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

INCLUDE(cmake/base.cmake)
INCLUDE(cmake/cpack.cmake)
INCLUDE(cmake/boost.cmake)
INCLUDE(cmake/ros.cmake)
INCLUDE(metapod.cmake)
INCLUDE(CMakeDependentOption)

SET(PROJECT_NAME metapod)
SET(PROJECT_DESCRIPTION "Implementation of a robot dynamics library")
SET(PROJECT_URL "https://github.com/laas/metapod")

SET(CXX_DISABLE_WERROR True)
CMAKE_POLICY(SET CMP0012 NEW)

IF(UNIX OR MINGW)
  ADD_DEFINITIONS("-fvisibility=hidden")
  ADD_DEFINITIONS("-Wno-variadic-macros")
ENDIF()

SETUP_PROJECT()

# Search for Boost.
# Boost.Test is used by the test suite.
# Boost program_options is used by the embedfile utility, which is used
# by metapod_robotbuilder
# Boost filesystem and regex are used by metapod_robotbuilder.
# Boost filesystem depends on Boost system.
SET(BOOST_COMPONENTS
  filesystem system unit_test_framework program_options regex)
SEARCH_FOR_BOOST()
# If Boost is recent enough, we look for Boost timer which can be used by
# by metapod_timer, which is in turn used by the benchmark.
# Boost timer depends on Boost chrono and system.
IF((BUILD_MULTI_MODEL_BENCHMARK OR BUILD_SINGLE_MODEL_BENCHMARKS)
   AND NOT Boost_VERSION LESS 104800)
  SET(BOOST_COMPONENTS ${BOOST_COMPONENTS} timer chrono system)
  SEARCH_FOR_BOOST()
ENDIF()

# Search for Eigen.
ADD_REQUIRED_DEPENDENCY("eigen3 >= 3.0.0")
# Eigen (at least version 3.0.5) makes gcc report conversion warnings
# when one assigns to a block. This floods the compilation results.
# So we include Eigen headers as if they were system headers: the compiler
# won't report warning coming from them.
INCLUDE_DIRECTORIES(SYSTEM ${EIGEN3_INCLUDE_DIRS})

# Set header files.
SET(${PROJECT_NAME}_ROOT_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/include/metapod/config.hh
  include/${PROJECT_NAME}/macro.hh
  )
SET(${PROJECT_NAME}_ALGOS_HEADERS
  include/${PROJECT_NAME}/algos/rnea.hh
  include/${PROJECT_NAME}/algos/crba.hh
  include/${PROJECT_NAME}/algos/jac.hh
  include/${PROJECT_NAME}/algos/jac_point_relative.hh
  include/${PROJECT_NAME}/algos/jac_point_chain.hh
  )
SET(${PROJECT_NAME}_TOOLS_HEADERS
  include/${PROJECT_NAME}/tools/backward.hh
  include/${PROJECT_NAME}/tools/backward_traversal.hh
  include/${PROJECT_NAME}/tools/backward_traversal_prev.hh
  include/${PROJECT_NAME}/tools/bcalc.hh
  include/${PROJECT_NAME}/tools/common.hh
  include/${PROJECT_NAME}/tools/constants.hh
  include/${PROJECT_NAME}/tools/deepest_common_ancestor.hh
  include/${PROJECT_NAME}/tools/depth_first_traversal.hh
  include/${PROJECT_NAME}/tools/fwd.hh
  include/${PROJECT_NAME}/tools/has_parent.hh
  include/${PROJECT_NAME}/tools/initconf.hh
  include/${PROJECT_NAME}/tools/initnodeidconf.hh
  include/${PROJECT_NAME}/tools/initnufwddyn.hh
  include/${PROJECT_NAME}/tools/is_ancestor.hh
  include/${PROJECT_NAME}/tools/jac_point_robot.hh
  include/${PROJECT_NAME}/tools/jcalc.hh
  include/${PROJECT_NAME}/tools/joint.hh
  include/${PROJECT_NAME}/tools/joint-aboutxaxis.hh
  include/${PROJECT_NAME}/tools/joint-aboutyaxis.hh
  include/${PROJECT_NAME}/tools/joint-aboutzaxis.hh
  include/${PROJECT_NAME}/tools/joint-anyaxis.hh
  include/${PROJECT_NAME}/tools/joint-freeflyer.hh
  include/${PROJECT_NAME}/tools/print.hh
  include/${PROJECT_NAME}/tools/qcalc.hh
  include/${PROJECT_NAME}/tools/spatial.hh
  include/${PROJECT_NAME}/tools/static_assert.hh
  )
SET(${PROJECT_NAME}_SPATIAL_HEADERS
  include/${PROJECT_NAME}/tools/spatial/cm-anyaxis.hh
  include/${PROJECT_NAME}/tools/spatial/cm-freeflyer.hh
  include/${PROJECT_NAME}/tools/spatial/cm-oneaxis.hh
  include/${PROJECT_NAME}/tools/spatial/constraintmotion.hh
  include/${PROJECT_NAME}/tools/spatial/force.hh
  include/${PROJECT_NAME}/tools/spatial/inertia.hh
  include/${PROJECT_NAME}/tools/spatial/lti.hh
  include/${PROJECT_NAME}/tools/spatial/motion.hh
  include/${PROJECT_NAME}/tools/spatial/rm-aboutxaxis.hh
  include/${PROJECT_NAME}/tools/spatial/rm-aboutyaxis.hh
  include/${PROJECT_NAME}/tools/spatial/rm-aboutzaxis.hh
  include/${PROJECT_NAME}/tools/spatial/rm-binaryop.hh
  include/${PROJECT_NAME}/tools/spatial/rm-chgaxis.hh
  include/${PROJECT_NAME}/tools/spatial/rm-general.hh
  include/${PROJECT_NAME}/tools/spatial/rm-identity.hh
  include/${PROJECT_NAME}/tools/spatial/rm-mulop.hh
  include/${PROJECT_NAME}/tools/spatial/rotation-matrix.hh
  include/${PROJECT_NAME}/tools/spatial/transform.hh
  )
INSTALL(FILES ${${PROJECT_NAME}_ROOT_HEADERS}
  DESTINATION include/${PROJECT_NAME}
  )
INSTALL(FILES ${${PROJECT_NAME}_ALGOS_HEADERS}
  DESTINATION include/${PROJECT_NAME}/algos
  )
INSTALL(FILES ${${PROJECT_NAME}_TOOLS_HEADERS}
  DESTINATION include/${PROJECT_NAME}/tools
  )
INSTALL(FILES ${${PROJECT_NAME}_SPATIAL_HEADERS}
  DESTINATION include/${PROJECT_NAME}/tools/spatial
  )

INSTALL(FILES metapod.cmake
  DESTINATION share/cmake/
 )


INCLUDE_DIRECTORIES("include" "${CMAKE_CURRENT_BINARY_DIR}/include")

# We build the generator tools only if we are not cross compiling.
# When cross compiling, the generators are searched for on the building host.
IF(NOT CMAKE_CROSSCOMPILING)
  ADD_SUBDIRECTORY(embedfile)
  ADD_SUBDIRECTORY(robotbuilder)
  ADD_SUBDIRECTORY(binarytreemodel)
  SET(WITH_METAPOD_BINARYTREEMODEL TRUE)
ELSE()
  FIND_GENERATOR(METAPOD_BINARYTREEMODEL)
ENDIF()

SET(JOINT_FWD_DYN_VALUES "FROM_URDF;ALL_ON;ALL_OFF")
SET(MODEL_JOINT_FWD_DYN_FROM_URDF_ON_OFF "ALL_OFF" CACHE STRING "Set the joint fwd_dyn type from URDF model or hard value. Possible values are: ${JOINT_FWD_DYN_VALUES}")
SET_PROPERTY(CACHE MODEL_JOINT_FWD_DYN_FROM_URDF_ON_OFF PROPERTY STRINGS ${JOINT_FWD_DYN_VALUES})

# Even when not cross compiling, we cannot always build metapodfromurdf,
# because it depends on ROS bits, which are not always available.
SET(BUILD_METAPODFROMURDF_DEFAULT FALSE)
SET(USE_URDF_FROM_ROS_FUERTE FALSE)
# by default, we build metapodfromurdf if we are not cross compiling and
# we detect urdfdom or ros. Since the detection part is not fool proof,
# we let the user override it.
IF(NOT CMAKE_CROSSCOMPILING)
  FIND_PACKAGE(URDFDOM QUIET)
  IF(URDFDOM_FOUND)
    SET(BUILD_METAPODFROMURDF_DEFAULT TRUE)
    INCLUDE_DIRECTORIES(${urdfdom_INCLUDE_DIRS})
  ELSE()
    # urdfdom is not available alone, but if ros is there, we'll use liburdf
    # instead and assume we are using fuerte.
    # Note: that on fuerte, we'd better use liburdf_parser instead of liburdf.
    # However, fuerte support won't last long, it's not worth changing it now.
    FIND_PROGRAM(ROSPACK_EXECUTABLE rospack)
    IF(ROSPACK_EXECUTABLE)
      SET(BUILD_METAPODFROMURDF_DEFAULT TRUE)
      SET(USE_URDF_FROM_ROS_FUERTE TRUE)
    ENDIF()
  ENDIF()
ENDIF()
CMAKE_DEPENDENT_OPTION(BUILD_METAPODFROMURDF
  "Build the urdf parser"
  ${BUILD_METAPODFROMURDF_DEFAULT}
  "NOT CMAKE_CROSSCOMPILING"
  OFF)
IF(BUILD_METAPODFROMURDF)
  ADD_SUBDIRECTORY(metapodfromurdf)
  SET(WITH_METAPODFROMURDF TRUE)
ELSE()
  FIND_GENERATOR(METAPODFROMURDF)
ENDIF()

CMAKE_DEPENDENT_OPTION(WITH_SIMPLE_HUMANOID
  "Generate and build the simple humanoid example model and the related tests.
  (Only possible if metapodfromurdf is available.)"
  ON
  WITH_METAPODFROMURDF
  OFF)
CMAKE_DEPENDENT_OPTION(WITH_SIMPLE_ARM
  "Generate and build the simple arm example model and the related tests.
  (Only possible if metapodfromurdf is available.)"
  ON
  WITH_METAPODFROMURDF
  OFF)

OPTION(BUILD_MULTI_MODEL_BENCHMARK
    "Generate and build the multi-model benchmark" TRUE)
OPTION(BUILD_SINGLE_MODEL_BENCHMARKS
    "Generate and build the single-model benchmarks" TRUE)


IF(WITH_SIMPLE_ARM)
  ADD_SAMPLEURDFMODEL("simple_arm")
ENDIF()
IF(WITH_SIMPLE_HUMANOID)
  ADD_SAMPLEURDFMODEL("simple_humanoid")
ENDIF()

ADD_SUBDIRECTORY(tests)

IF(BUILD_MULTI_MODEL_BENCHMARK OR BUILD_SINGLE_MODEL_BENCHMARKS)
  ADD_SUBDIRECTORY(timer)
  ADD_SUBDIRECTORY(benchmark)
ENDIF()

SETUP_PROJECT_FINALIZE()
SETUP_PROJECT_CPACK()
