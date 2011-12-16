/* \file
 * $Id$
 * \author Caton Little
 * \brief 
 *
 * \section LICENSE
 *
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is FieldML
 *
 * The Initial Developer of the Original Code is Auckland Uniservices Ltd,
 * Auckland, New Zealand. Portions created by the Initial Developer are
 * Copyright (C) 2010 the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 */
#include <cstdlib>

#include "fieldml_api.h"

#include "SimpleTest.h"

/**
 * Ensure that newly created sessions have the correct state.
 */
SIMPLE_TEST( FieldmlCreateTest )
{
    const int MAX_STRLEN = 255;
    char *strbuf = (char*)malloc(MAX_STRLEN);

    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );
    
    int length = Fieldml_CopyRegionName( session, strbuf, MAX_STRLEN );
    SIMPLE_ASSERT_EQUALS( "test", strbuf );
    SIMPLE_ASSERT_EQUALS( 4, length );
    
    int count = Fieldml_GetTotalObjectCount( session );
    SIMPLE_ASSERT_EQUALS( 0, count );

    count = Fieldml_GetImportSourceCount( session );
    SIMPLE_ASSERT_EQUALS( 0, count );

    Fieldml_Destroy( session );
}


SIMPLE_TEST( FieldmlCreateContinuousTypeTest )
{
    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );

    int count = Fieldml_GetObjectCount( session, FHT_CONTINUOUS_TYPE );
    SIMPLE_ASSERT_EQUALS( 0, count );
    
    const char *name = "test.continuous";
    FmlObjectHandle typeHandle = Fieldml_CreateContinuousType( session, NULL );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    typeHandle = Fieldml_CreateContinuousType( -1, name );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    
    //Check the uninitialized state of the continuous type.
    typeHandle = Fieldml_CreateContinuousType( session, name );
    SIMPLE_ASSERT( typeHandle != FML_INVALID_HANDLE );
    
    FmlObjectHandle components = Fieldml_GetTypeComponentEnsemble( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( FML_INVALID_HANDLE, components );
    
    count = Fieldml_GetTypeComponentCount( session, typeHandle );
    //NOTE: Componentless continuous types are scalars, so they have one component, but no index.
    SIMPLE_ASSERT_EQUALS( 1, count );
    
    
    //Give the continuous type some components. 
    const char *componentName = "test.continuous.component";
    const int COMPONENT_COUNT = 3;
    components = Fieldml_CreateContinuousTypeComponents( session, typeHandle, componentName, COMPONENT_COUNT );
    SIMPLE_ASSERT( components != FML_INVALID_HANDLE );
    
    count = Fieldml_GetTypeComponentCount( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( COMPONENT_COUNT, count );
    
    FmlObjectHandle componentType = Fieldml_GetTypeComponentEnsemble( session, typeHandle );
    SIMPLE_ASSERT( componentType == components );
    
    
    //Attempt to give the continuous type some new components.
    const int NEW_COMPONENT_COUNT = 8;
    const char *newComponentName = "test.continuous.new_component";
    FmlObjectHandle newComponents = Fieldml_CreateContinuousTypeComponents( session, typeHandle, newComponentName, NEW_COMPONENT_COUNT );
    SIMPLE_ASSERT( newComponents == FML_INVALID_HANDLE );
    
    componentType = Fieldml_GetTypeComponentEnsemble( session, typeHandle );
    SIMPLE_ASSERT( componentType == components );

    
    //Change the component count.
    FmlErrorNumber err = Fieldml_SetEnsembleMembersRange( session, components, 1, NEW_COMPONENT_COUNT, 1 );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    
    count = Fieldml_GetTypeComponentCount( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( NEW_COMPONENT_COUNT, count );
    
    count = Fieldml_GetMemberCount( session, components );
    SIMPLE_ASSERT_EQUALS( NEW_COMPONENT_COUNT, count );

    count = Fieldml_GetObjectCount( session, FHT_CONTINUOUS_TYPE );
    SIMPLE_ASSERT_EQUALS( 1, count );
    
    FmlObjectHandle altHandle = Fieldml_GetObject( session, FHT_CONTINUOUS_TYPE, 1 );
    SIMPLE_ASSERT_EQUALS( typeHandle, altHandle );
    
    Fieldml_Destroy( session );
}


SIMPLE_TEST( FieldmlCreateEnsembleTypeTest )
{
    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );

    int count = Fieldml_GetObjectCount( session, FHT_ENSEMBLE_TYPE );
    SIMPLE_ASSERT_EQUALS( 0, count );
    
    const char *name = "test.ensemble";
    FmlObjectHandle typeHandle = Fieldml_CreateEnsembleType( session, NULL );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    typeHandle = Fieldml_CreateEnsembleType( -1, name );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    
    //Check the uninitialized state of the ensemble type.
    typeHandle = Fieldml_CreateEnsembleType( session, name );
    SIMPLE_ASSERT( typeHandle != FML_INVALID_HANDLE );
    
    count = Fieldml_GetMemberCount( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( 0, count );

    FmlErrorNumber err = Fieldml_SetEnsembleMembersRange( session, typeHandle, -1, -8, 1 );
    SIMPLE_ASSERT( err != FML_ERR_NO_ERROR );

    const int COMPONENT_MAX = 10;
    const int COMPONENT_MIN = 3;
    const int COMPONENT_STRIDE = 2;
    err = Fieldml_SetEnsembleMembersRange( session, typeHandle, COMPONENT_MIN, COMPONENT_MAX, COMPONENT_STRIDE );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    
    count = Fieldml_GetMemberCount( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( (COMPONENT_MAX - COMPONENT_MIN + 1)/2, count );
    
    count = Fieldml_GetEnsembleMembersMin( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( COMPONENT_MIN, count );
    
    count = Fieldml_GetEnsembleMembersMax( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( COMPONENT_MAX, count );
    
    count = Fieldml_GetEnsembleMembersStride( session, typeHandle );
    SIMPLE_ASSERT_EQUALS( COMPONENT_STRIDE, count );


    count = Fieldml_GetObjectCount( session, FHT_ENSEMBLE_TYPE );
    SIMPLE_ASSERT_EQUALS( 1, count );
    
    FmlObjectHandle altHandle = Fieldml_GetObject( session, FHT_ENSEMBLE_TYPE, 1 );
    SIMPLE_ASSERT_EQUALS( typeHandle, altHandle );
    
    Fieldml_Destroy( session );
}


SIMPLE_TEST( FieldmlCreateBooleanTypeTest )
{
    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );

    int count = Fieldml_GetObjectCount( session, FHT_BOOLEAN_TYPE );
    SIMPLE_ASSERT_EQUALS( 0, count );
    
    const char *name = "test.boolean";
    FmlObjectHandle typeHandle = Fieldml_CreateBooleanType( session, NULL );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    typeHandle = Fieldml_CreateBooleanType( -1, name );
    SIMPLE_ASSERT( typeHandle == FML_INVALID_HANDLE );

    
    typeHandle = Fieldml_CreateBooleanType( session, name );
    SIMPLE_ASSERT( typeHandle != FML_INVALID_HANDLE );
    
    count = Fieldml_GetObjectCount( session, FHT_BOOLEAN_TYPE );
    SIMPLE_ASSERT_EQUALS( 1, count );
    
    FmlObjectHandle altHandle = Fieldml_GetObject( session, FHT_BOOLEAN_TYPE, 1 );
    SIMPLE_ASSERT_EQUALS( typeHandle, altHandle );
    
    Fieldml_Destroy( session );
}


/**
 * Ensure that destroyed sessions are inaccessible.
 */
SIMPLE_TEST( FieldmlDestroyTest )
{
    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );
    
    Fieldml_Destroy( session );

    int count = Fieldml_GetTotalObjectCount( session );
    SIMPLE_ASSERT_EQUALS( -1, count );
    
    count = Fieldml_GetImportSourceCount( session );
    SIMPLE_ASSERT_EQUALS( -1, count );
}
