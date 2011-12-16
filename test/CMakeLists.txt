
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

SET( CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} )
FIND_PACKAGE( ZLIB QUIET )

SET( TEST_EXE_SRCS src/fieldml_test.cpp )
SET( TEST_EXE_TARGET_NAME fieldml_test )

SET( SIMPLE_TEST_SRCS src/SimpleTest.cpp )
SET( SIMPLE_TEST_HDRS src )

SET( TEST_ARRAY_READING_EXE_SRCS src/FieldmlTestArrayReading.cpp )
SET( TEST_ARRAY_READING_EXE_TARGET_NAME fieldml_test_array_reading )

SET( TEST_CREATE_EXE_SRCS src/FieldmlTestCreate.cpp )
SET( TEST_CREATE_EXE_TARGET_NAME fieldml_test_create )

SET( FIELDML_API_PUBLIC_HDRS ../core/src ) 
SET( FIELDML_IO_API_PUBLIC_HDRS ../io/src )
SET( INPUT_RESOURCES input/I16BE.h5 )

IF( ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_TEST )
	OPTION_WITH_DEFAULT( BUILD_TEST_WITH_SZLIB "Build test application with SZLIB" FALSE )
	IF( BUILD_TEST_WITH_SZLIB )
		FIND_PATH(SZIP_INCLUDE_DIR szlib.h
			NO_DEFAULT_PATH
		)
		FIND_LIBRARY(SZIP_LIBRARY NAMES szip
			NO_DEFAULT_PATH
		)
		SET( SZIP_INCLUDE_DIR ${SZIP_INCLUDE_DIR} )
		SET( SZIP_LIBRARY ${SZIP_LIBRARY} )
	ENDIF( BUILD_TEST_WITH_SZLIB )

	INCLUDE_DIRECTORIES( ${FIELDML_API_PUBLIC_HDRS} ${FIELDML_IO_API_PUBLIC_HDRS} ${SIMPLE_TEST_HDRS})

	ADD_EXECUTABLE( ${TEST_EXE_TARGET_NAME} ${TEST_EXE_SRCS} )
	IF( WIN32 )
		ADD_EXECUTABLE( ${TEST_ARRAY_READING_EXE_TARGET_NAME} ${SIMPLE_TEST_SRCS} ${TEST_ARRAY_READING_EXE_SRCS} )
		ADD_EXECUTABLE( ${TEST_CREATE_EXE_TARGET_NAME} ${SIMPLE_TEST_SRCS} ${TEST_CREATE_EXE_SRCS} )
	ELSE( WIN32 )
		ADD_EXECUTABLE( ${TEST_ARRAY_READING_EXE_TARGET_NAME} ${TEST_ARRAY_READING_EXE_SRCS} ${SIMPLE_TEST_SRCS} )
		ADD_EXECUTABLE( ${TEST_CREATE_EXE_TARGET_NAME} ${TEST_CREATE_EXE_SRCS} ${SIMPLE_TEST_SRCS} )
	ENDIF( WIN32 )
	TARGET_LINK_LIBRARIES( ${TEST_EXE_TARGET_NAME} ${FIELDML_API_LIBRARY_TARGET_NAME} ${FIELDML_IO_API_LIBRARY_TARGET_NAME} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES} ${HDF5_LIBRARY} ${SZIP_LIBRARY} )
	TARGET_LINK_LIBRARIES( ${TEST_ARRAY_READING_EXE_TARGET_NAME} ${FIELDML_API_LIBRARY_TARGET_NAME} ${FIELDML_IO_API_LIBRARY_TARGET_NAME} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES} ${HDF5_LIBRARY} ${SZIP_LIBRARY} )
	TARGET_LINK_LIBRARIES( ${TEST_CREATE_EXE_TARGET_NAME} ${FIELDML_API_LIBRARY_TARGET_NAME} ${FIELDML_IO_API_LIBRARY_TARGET_NAME} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES} ${HDF5_LIBRARY} ${SZIP_LIBRARY} )

	INSTALL( TARGETS ${TEST_EXE_TARGET_NAME} EXPORT ${LIBRARY_TARGET_NAME}-targets
			${LIBRARY_INSTALL_TYPE}
        	DESTINATION test )
	INSTALL( TARGETS ${TEST_ARRAY_READING_EXE_TARGET_NAME} EXPORT ${LIBRARY_TARGET_NAME}-targets ${LIBRARY_INSTALL_TYPE}
        	DESTINATION test )
	INSTALL( TARGETS ${TEST_CREATE_EXE_TARGET_NAME} EXPORT ${LIBRARY_TARGET_NAME}-targets ${LIBRARY_INSTALL_TYPE}
        	DESTINATION test )


	INSTALL( FILES ${INPUT_RESOURCES} DESTINATION test/input )


ENDIF( ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_TEST )
