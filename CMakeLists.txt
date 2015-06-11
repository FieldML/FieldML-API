
 # ***** BEGIN LICENSE BLOCK *****
 # Version: MPL 1.1/GPL 2.0/LGPL 2.1
 #
 # The contents of this file are subject to the Mozilla Public License Version
 # 1.1 (the "License"); you may not use this file except in compliance with
 # the License. You may obtain a copy of the License at
 # http://www.mozilla.org/MPL/
 #
 # Software distributed under the License is distributed on an "AS IS" basis,
 # WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 # for the specific language governing rights and limitations under the
 # License.
 #
 # The Original Code is FieldML
 #
 # The Initial Developer of the Original Code is
 # Auckland Uniservices Ltd, Auckland, New Zealand.
 # Portions created by the Initial Developer are Copyright (C) 2011
 # the Initial Developer. All Rights Reserved.
 #
 # Contributor(s): 
 #
 # Alternatively, the contents of this file may be used under the terms of
 # either the GNU General Public License Version 2 or later (the "GPL"), or
 # the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 # in which case the provisions of the GPL or the LGPL are applicable instead
 # of those above. If you wish to allow use of your version of this file only
 # under the terms of either the GPL or the LGPL, and not to allow others to
 # use your version of this file under the terms of the MPL, indicate your
 # decision by deleting the provisions above and replace them with the notice
 # and other provisions required by the GPL or the LGPL. If you do not delete
 # the provisions above, a recipient may use your version of this file under
 # the terms of any one of the MPL, the GPL or the LGPL.
 #
 # ***** END LICENSE BLOCK ***** */

# Set the minimum version of cmake required to 3.0
CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

# Set this namespace to keep these options seperate from other project options
SET( FIELDML_NAMESPACE_NAME FIELDML )
PROJECT(fieldml VERSION 0.5.0 LANGUAGES CXX)

option(USE_HDF5 "Do you want to use netgen (HDF5)?" NO)
option(HDF5_WITH_MPI "Do you want to use MPI with HDF5?" NO)
option(BUILD_TESTS "Build tests" ON)
option(FORTRAN_BINDINGS "Do you want to build the Fortran bindings" YES)
option(JAVA_BINDINGS "Do you want to build the Java bindings" NO)
set(PACKAGE_CONFIG_DIR "lib/cmake" CACHE STRING "Directory for package config files (relative to CMAKE_INSTALL_PREFIX)")
# Further options:
# CMAKE_POSITION_INDEPENDENT_CODE=YES for -fPIC constant (is also passed in from OpenCMISS-main build script)

FIND_PACKAGE(LibXml2 REQUIRED)
if(USE_HDF5)
    # See the FindHDF5 description
    if (BUILD_SHARED_LIBS)
        set(HDF5_USE_STATIC_LIBRARIES NO)
    else()
        set(HDF5_USE_STATIC_LIBRARIES YES)
    endif()
	find_package(HDF5 REQUIRED CXX)
	if(HDF5_WITH_MPI)
		find_package(MPI REQUIRED)
    endif()
endif()

if (NOT CMAKE_DEBUG_POSTFIX)
    SET(CMAKE_DEBUG_POSTFIX d)
endif()

# Add actual fieldml code and io
add_subdirectory(core)
add_subdirectory(io)

if (FORTRAN_BINDINGS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FortranBindings.cmake)
endif()

# Create interface library for easy inclusion
add_library(fieldml-api INTERFACE)
target_link_libraries(fieldml-api INTERFACE fieldml-core fieldml-io)
if (HAVE_Fortran_BINDINGS)
    target_link_libraries(fieldml-api INTERFACE fieldml-fortran)
endif()
install(TARGETS fieldml-api EXPORT fieldml-api-config)

# Tests
if (BUILD_TESTS)
    add_subdirectory(test)
endif()

if(JAVA_BINDINGS)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/JavaBindings.cmake)
endif()

include(CMakePackageConfigHelpers)
install(EXPORT fieldml-api-config
	DESTINATION ${PACKAGE_CONFIG_DIR})
WRITE_BASIC_PACKAGE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-version.cmake
	COMPATIBILITY AnyNewerVersion)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake 
    "include(CMakeFindDependencyMacro)\r\n"
    "set(_OLD_IMPORT_PREFIX \${_IMPORT_PREFIX})\r\n"
)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake "find_dependency(LibXml2 ${LIBXML2_VERSION})\r\n")
if (USE_HDF5)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake "find_dependency(HDF5 ${HDF5_VERSION})\r\n")
endif()
if (JAVA_BINDINGS)
    # TODO
    #file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake "find_dependency(OpenMP)\r\n")
endif()
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake "set(_IMPORT_PREFIX \${_OLD_IMPORT_PREFIX})")
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-version.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/fieldml-api-config-dependencies.cmake
    DESTINATION ${PACKAGE_CONFIG_DIR})