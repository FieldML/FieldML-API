
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
 # Portions created by the Initial Developer are Copyright (C) 2005
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

#SET( HDF5_INCLUDE_DIRS "" )
#SET( HDF5_MINE_LIBRARIES "" )
#SET( MPI_INCLUDE_DIRS "" )
#SET( MPI_MINE_LIBRARIES "" )

SET( FIELDML_IO_API_SRCS
	src/ArrayDataReader.cpp
	src/ArrayDataWriter.cpp
	src/FieldmlIoApi.cpp
	src/FieldmlIoSession.cpp
	src/Hdf5ArrayDataReader.cpp
	src/Hdf5ArrayDataWriter.cpp
	src/InputStream.cpp
	src/OutputStream.cpp
	src/StringUtil.cpp
	src/TextArrayDataReader.cpp
	src/TextArrayDataWriter.cpp )
SET( FIELDML_IO_API_PRIVATE_HDRS
	src/ArrayDataReader.h
	src/ArrayDataWriter.h
	src/FieldmlIoContext.h
	src/FieldmlIoSession.h
	src/Hdf5ArrayDataReader.h
	src/Hdf5ArrayDataWriter.h
	src/InputStream.h
	src/OutputStream.h
	src/StringUtil.h
	src/TextArrayDataReader.h
	src/TextArrayDataWriter.h )
SET( FIELDML_IO_API_PUBLIC_HDRS
	src/FieldmlIoApi.h )
SET( FIELDML_API_PUBLIC_HDRS
	../core/src )

#INCLUDE_DIRECTORIES( ${FIELDML_API_PUBLIC_HDRS} ${HDF5_INCLUDE_DIRS} ${MPI_INCLUDE_DIRS} )

# Create library
add_library(fieldml-io ${LIBRARY_BUILD_TYPE} ${FIELDML_IO_API_SRCS} ${FIELDML_IO_API_PUBLIC_HDRS} ${FIELDML_IO_API_PRIVATE_HDRS} ${LIBRARY_WIN32_XTRAS} )
target_include_directories(fieldml-io PUBLIC     
    $<BUILD_INTERFACE:${fieldml_SOURCE_DIR}/core/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
target_compile_definitions(fieldml-io PRIVATE $<$<CONFIG:Debug>:DEBUG>)
if (WIN32)
    target_compile_definitions(fieldml-io PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(USE_HDF5)
	target_compile_definitions(fieldml-io PUBLIC FIELDML_HDF5_ARRAY)
	target_link_libraries(fieldml-io PUBLIC ${HDF5_CXX_LIBRARIES})
	SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${FIELDML_HDF5_ARRAY_DEFINE})
	#SET(HDF5_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
	#SET(HDF5_MINE_LIBRARIES ${HDF5_LIBRARIES})
	#SET(DEPENDENT_LIBS ${DEPENDENT_LIBS} ${HDF5_MINE_LIBRARIES})
	#SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} )
	if(HDF5_WITH_MPI)
		target_compile_definitions(fieldml-io PUBLIC FIELDML_PHDF5_ARRAY)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FIELDML_PHDF5_ARRAY_DEFINE}")
		#SET( MPI_INCLUDE_DIRS ${MPI_INCLUDE_PATH} )
		#SET( MPI_MINE_LIBRARIES ${MPI_LIBRARIES} )
		#SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${MPI_MINE_LIBRARIES} )
		#SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${MPI_INCLUDE_DIRS} )
	endif()
endif()

# Install targets
if(WIN32 AND BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES(fieldml PROPERTIES IMPORT_SUFFIX _dll.lib)
endif()

INSTALL(TARGETS fieldml-io
    EXPORT fieldml-api-config
    DESTINATION lib)
INSTALL(FILES ${FIELDML_IO_API_PUBLIC_HDRS} DESTINATION include)

