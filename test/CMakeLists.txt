
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

#find_package(ZLIB ${ZLIB_VERSION} QUIET)

add_executable(fieldml_test src/fieldml_test.cpp)
target_link_libraries(fieldml_test fieldml-api)
if (USE_HDF5)
    target_link_libraries(fieldml_test ${HDF5_CXX_LIBRARIES})
    
    if(HDF5_WITH_MPI)
        add_executable(fieldml_test_phdf5 src/fieldml_phdf5_test.cpp)
        target_link_libraries(fieldml_test_phdf5 fieldml-api ${HDF5_CXX_LIBRARIES})
    endif()
endif()

add_executable(fieldml_simpletest src/SimpleTest.cpp)	
target_link_libraries(fieldml_simpletest fieldml-api)

add_executable(fieldml_test_array_reading src/FieldmlTestArrayReading.cpp src/SimpleTest.cpp)
target_link_libraries(fieldml_test_array_reading fieldml-api)

add_executable(fieldml_test_create src/FieldmlTestCreate.cpp src/SimpleTest.cpp)
target_link_libraries(fieldml_test_create fieldml-api)

option(BUILD_TEST_WITH_SZLIB "Build test application with SZLIB" FALSE)
if(BUILD_TEST_WITH_SZLIB)
	FIND_PATH(SZIP_INCLUDE_DIR szlib.h
		NO_DEFAULT_PATH
	)
	FIND_LIBRARY(SZIP_LIBRARY NAMES szip
		NO_DEFAULT_PATH
	)
	SET( SZIP_INCLUDE_DIR ${SZIP_INCLUDE_DIR} )
	SET( SZIP_LIBRARY ${SZIP_LIBRARY} )
endif()

install(TARGETS fieldml_test fieldml_simpletest fieldml_test_array_reading fieldml_test_create
    DESTINATION test)
install(DIRECTORY input DESTINATION test)
install(DIRECTORY output DESTINATION test)    