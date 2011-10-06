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

#include <string>
#include <sstream>

#include "fieldml_api.h"

#include "SimpleTest.h"

using namespace std;

/**
 * Ensure that newly created data sources have the correct state.
 */
SIMPLE_TEST( FieldmlDataArrayCreateTest )
{
    const int MAX_STRLEN = 255;
    char *strbuf = (char*)malloc(MAX_STRLEN);

    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );
    
    FmlObjectHandle resource = Fieldml_CreateInlineDataResource( session, "test.resource" );
    FmlObjectHandle source = Fieldml_CreateArrayDataSource( session, "test.source", resource, "1", 3 );
    int sizes[] = { 2, 3, 4 };
    Fieldml_SetArrayDataSourceRawSizes( session, source, sizes );
    const string rawData = "1 2 3 4\n5 6 7 8\n9 10 11 12\n\n13 14 15 16\n17 18 19 20\n21 22 23 24\n";
    int err = Fieldml_AddInlineData( session, resource, rawData.c_str(), rawData.length() );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    
    Fieldml_CopyDataResourceFormat( session, resource, strbuf, MAX_STRLEN );
    SIMPLE_ASSERT_EQUALS( "PLAIN_TEXT", strbuf );
    
    int rank = Fieldml_GetArrayDataSourceRank( session, source );
    SIMPLE_ASSERT_EQUALS( 3, rank );
    
    int realSizes[] = { -1, -1, -1 };
    err = Fieldml_GetArrayDataSourceRawSizes( session, source, realSizes );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    SIMPLE_ASSERT_EQUALS( sizes[0], realSizes[0] );
    SIMPLE_ASSERT_EQUALS( sizes[1], realSizes[1] );
    SIMPLE_ASSERT_EQUALS( sizes[2], realSizes[2] );
    
    unsigned int length = Fieldml_CopyArrayDataSourceLocation( session, source, strbuf, MAX_STRLEN );
    SIMPLE_ASSERT_EQUALS( 1U, length );
    SIMPLE_ASSERT_EQUALS( "1", strbuf );
    
    memset( strbuf, 0, MAX_STRLEN );
    length = Fieldml_CopyInlineData( session, resource, strbuf, MAX_STRLEN, 0 );
    SIMPLE_ASSERT_EQUALS( rawData.length(), length );
    SIMPLE_ASSERT_EQUALS( rawData.c_str(), strbuf );
    
    Fieldml_Destroy( session );
}


/**
 * Ensure that various kinds of reads work with arrays based on inline resources.
 */
SIMPLE_TEST( FieldmlDataArrayReadTest )
{
    FmlSessionHandle session = Fieldml_Create( "test_path", "test" );
    Fieldml_SetDebug( session, 0 );
    SIMPLE_ASSERT( session != FML_INVALID_HANDLE );
    
    FmlObjectHandle resource = Fieldml_CreateInlineDataResource( session, "test.resource" );
    FmlObjectHandle source = Fieldml_CreateArrayDataSource( session, "test.source", resource, "1", 3 );
    int sizes[] = { 2, 3, 4 };
    Fieldml_SetArrayDataSourceRawSizes( session, source, sizes );
    const string rawData = "1 2 3 4\n5 6 7 8\n9 10 11 12\n\n13 14 15 16\n17 18 19 20\n21 22 23 24\n";
    int err = Fieldml_AddInlineData( session, resource, rawData.c_str(), rawData.length() );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );

    int readOffsets[] = { 0, 0, 0 };
    int readSizes[] = { 2, 3, 4 };
    int buffer[24];
    
    FmlObjectHandle reader = Fieldml_OpenReader( session, source );
    SIMPLE_ASSERT( FML_INVALID_HANDLE != reader );
    
    err = Fieldml_ReadIntSlab( session, reader, readOffsets, readSizes, buffer );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    
    stringstream rawStream( rawData );
    int expected; 
    for( int i = 0; i < 24; i++ )
    {
        rawStream >> expected;
        SIMPLE_ASSERT_EQUALS( expected, buffer[i] );
    }
    for( int i = 0; i < 24; i++ ) buffer[i] = -1;
    
    readOffsets[0] = 1;
    readSizes[0] = 1;
    
    err = Fieldml_ReadIntSlab( session, reader, readOffsets, readSizes, buffer );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );
    
    rawStream.seekg( 0 );
    for( int i = 0; i < 24; i++ )
    {
        rawStream >> expected;
        if( i >= 12 )
        {
            SIMPLE_ASSERT_EQUALS( expected, (buffer[i-12]) );
        }
    }
    for( int i = 0; i < 24; i++ ) buffer[i] = -1;
    
    readOffsets[0] = 0;
    readSizes[0] = 2;
    readOffsets[1] = 1;
    readSizes[1] = 2;
    readOffsets[2] = 2;
    readSizes[2] = 1;
    err = Fieldml_ReadIntSlab( session, reader, readOffsets, readSizes, buffer );
    SIMPLE_ASSERT_EQUALS( FML_ERR_NO_ERROR, err );

    SIMPLE_ASSERT_EQUALS( 7, buffer[0] );
    SIMPLE_ASSERT_EQUALS( 11, buffer[1] );
    SIMPLE_ASSERT_EQUALS( 19, buffer[2] );
    SIMPLE_ASSERT_EQUALS( 23, buffer[3] );
    
    Fieldml_CloseReader( session, reader );
    
    Fieldml_Destroy( session );
}
