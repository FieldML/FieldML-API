
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

# Create library
add_library(fieldml-core ${FIELDML_API_SRCS} ${FIELDML_API_PUBLIC_HDRS}
    ${FIELDML_API_PRIVATE_HDRS} ${LIBRARY_WIN32_XTRAS})
target_link_libraries(fieldml-core xml2)
target_compile_definitions(fieldml-core PRIVATE $<$<CONFIG:Debug>:DEBUG>)
if (WIN32)
    target_compile_definitions(fieldml-core PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Install targets
if(WIN32 AND BUILD_SHARED_LIBS)
    SET_TARGET_PROPERTIES(fieldml-core PROPERTIES IMPORT_SUFFIX _dll.lib)
endif()
install(TARGETS fieldml-core
    EXPORT fieldml-api-config
    DESTINATION lib)
install(FILES ${FIELDML_API_PUBLIC_HDRS} DESTINATION include)

