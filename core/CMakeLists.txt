
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

PROJECT( core )

STRING( TOUPPER ${LIBRARY_TARGET_NAME} UPPERCASE_LIBRARY_TARGET_NAME )
SET( FIELDML_API_MAJOR_VERSION "0" )
SET( FIELDML_API_MINOR_VERSION "4" )
SET( FIELDML_API_PATCH_VERSION "1" )

SET( CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX} )
FIND_PACKAGE( LibXml2 REQUIRED )

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

SET( FIELDML_API_SRCS
	src/ErrorContextAutostack.cpp
	src/Evaluators.cpp
	src/fieldml_api.cpp
	src/FieldmlDOM.cpp
	src/FieldmlRegion.cpp
	src/FieldmlSession.cpp
	src/fieldml_structs.cpp
	src/fieldml_write.cpp
	src/ImportInfo.cpp
	src/ObjectStore.cpp
	src/SimpleBitset.cpp
	src/string_const.cpp
	src/String_InternalLibrary.cpp
	src/String_InternalXSD.cpp )
SET( FIELDML_API_PRIVATE_HDRS
	src/ErrorContextAutostack.h
	src/Evaluators.h
	src/FieldmlDOM.h
	src/FieldmlErrorHandler.h
	src/FieldmlRegion.h
	src/FieldmlSession.h
	src/fieldml_structs.h
	src/fieldml_write.h
	src/ImportInfo.h
	src/ObjectStore.h
	src/SimpleBitset.h
	src/SimpleMap.h
	src/string_const.h
	src/String_InternalLibrary.h
	src/String_InternalXSD.h
	src/Util.h )
SET( FIELDML_API_PUBLIC_HDRS
	src/fieldml_api.h )

SET( CMAKE_DEBUG_POSTFIX "d" )
IF( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
	ADD_DEFINITIONS( -DDEBUG )
ENDIF( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
IF( WIN32 )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
ENDIF( WIN32 )
INCLUDE_DIRECTORIES( ${LIBXML2_INCLUDE_DIR} )

# Create library
ADD_LIBRARY( ${LIBRARY_TARGET_NAME} ${LIBRARY_BUILD_TYPE} ${FIELDML_API_SRCS} ${FIELDML_API_PUBLIC_HDRS} ${FIELDML_API_PRIVATE_HDRS} ${LIBRARY_WIN32_XTRAS} )
TARGET_LINK_LIBRARIES( ${LIBRARY_TARGET_NAME} ${LIBXML2_LIBRARIES} )

# Install targets
IF( WIN32 AND NOT ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_STATIC_LIB )
        SET_TARGET_PROPERTIES(${LIBRARY_TARGET_NAME} PROPERTIES IMPORT_SUFFIX _dll.lib)
        INSTALL( TARGETS ${LIBRARY_TARGET_NAME} ARCHIVE
                DESTINATION lib )
ENDIF( WIN32 AND NOT ${UPPERCASE_LIBRARY_TARGET_NAME}_BUILD_STATIC_LIB )

# Define install rules
INSTALL( TARGETS ${LIBRARY_TARGET_NAME} EXPORT ${LIBRARY_TARGET_NAME}-targets ${LIBRARY_INSTALL_TYPE}
        DESTINATION lib )
INSTALL( FILES ${FIELDML_API_PUBLIC_HDRS}
        DESTINATION include )
INSTALL( EXPORT ${LIBRARY_TARGET_NAME}-targets DESTINATION lib/cmake )

