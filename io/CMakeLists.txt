
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

PROJECT( io )

OPTION_WITH_DEFAULT( FIELDML_USE_HDF5 "Do you want to use netgen?" FALSE )

IF( FIELDML_USE_HDF5 )
	FIND_PATH(HDF5_INCLUDE_DIR hdf5.h
		NO_DEFAULT_PATH
	)
	FIND_LIBRARY(HDF5_LIBRARY NAMES hdf5
		NO_DEFAULT_PATH
	)
	ADD_DEFINITIONS( -DFIELDML_HDF5_ARRAY )
	SET( HDF5_INCLUDE_DIR ${HDF5_INCLUDE_DIR} )
	SET( HDF5_LIBRARY ${HDF5_LIBRARY} )
	SET( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${IELDML_HDF5_ARRAY_DEFINE} )
	SET( CMAKE ${CMAKE_C_FLAGS} ${IELDML_HDF5_ARRAY_DEFINE} )
	SET( DEPENDENT_LIBS ${DEPENDENT_LIBS} ${HDF5_LIBRARY} )
	SET( INCLUDE_DIRS ${INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} )
ENDIF( FIELDML_USE_HDF5 )

IF( ${FIELDML_NAMESPACE_NAME}_BUILD_STATIC_LIB )
	SET( LIBRARY_BUILD_TYPE STATIC )
	SET( LIBRARY_INSTALL_TYPE ARCHIVE )
ELSE( ${FIELDML_NAMESPACE_NAME}_BUILD_STATIC_LIB )
	SET( LIBRARY_BUILD_TYPE SHARED )
	SET( LIBRARY_INSTALL_TYPE LIBRARY )
	IF( WIN32 )
		SET( LIBRARY_INSTALL_TYPE RUNTIME )
	ENDIF( WIN32 )
ENDIF( ${FIELDML_NAMESPACE_NAME}_BUILD_STATIC_LIB )

SET( CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} )
FIND_PACKAGE( LibXml2 REQUIRED )

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
	
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
	SET_SOURCE_FILES_PROPERTIES(${FIELDML_IO_API_SRCS}
		PROPERTIES COMPILE_FLAGS "-fPIC")
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )

SET( CMAKE_DEBUG_POSTFIX "d" )
IF( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
	ADD_DEFINITIONS( -DDEBUG )
ENDIF( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
IF( WIN32 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
ENDIF( WIN32 )
INCLUDE_DIRECTORIES( ${FIELDML_API_PUBLIC_HDRS} ${HDF5_INCLUDE_DIR} )

# Create library
ADD_LIBRARY( ${LIBRARY_TARGET_NAME} ${LIBRARY_BUILD_TYPE} ${FIELDML_IO_API_SRCS} ${FIELDML_IO_API_PUBLIC_HDRS} ${FIELDML_IO_API_PRIVATE_HDRS} ${LIBRARY_WIN32_XTRAS} )
TARGET_LINK_LIBRARIES( ${LIBRARY_TARGET_NAME} ${HDF5_LIBRARY} )

# Install targets
IF( WIN32 AND NOT ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_STATIC_LIB )
        SET_TARGET_PROPERTIES(${LIBRARY_TARGET_NAME} PROPERTIES IMPORT_SUFFIX _dll.lib)
        INSTALL( TARGETS ${LIBRARY_TARGET_NAME} ARCHIVE
                DESTINATION lib )
ENDIF( WIN32 AND NOT ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_STATIC_LIB )

# Define install rules
INSTALL( TARGETS ${LIBRARY_TARGET_NAME} EXPORT fieldml-targets ${LIBRARY_INSTALL_TYPE}
        DESTINATION lib )
INSTALL( FILES ${FIELDML_IO_API_PUBLIC_HDRS}
        DESTINATION include )

