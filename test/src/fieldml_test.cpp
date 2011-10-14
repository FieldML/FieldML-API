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

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "fieldml_api.h"


//========================================================================
//
// Main
//
//========================================================================

void testRead( const char * filename )
{
    int i, j, count, count2;
    FmlObjectHandle oHandle;
    FmlSessionHandle handle;

    printf( "Test read...\n" );
    
   handle = Fieldml_CreateFromFile( filename );
    
    Fieldml_SetDebug( handle, 1 );

    count = Fieldml_GetObjectCount( handle, FHT_CONTINUOUS_TYPE );
    fprintf( stdout, "ContinuousTypes: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_CONTINUOUS_TYPE, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        if( Fieldml_GetTypeComponentEnsemble( handle, oHandle ) == FML_INVALID_HANDLE )
        {
            fprintf( stdout, "  %d: %s\n", i, Fieldml_GetObjectName( handle, oHandle ) );
        }
        else
        {
            fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
                Fieldml_GetObjectName( handle, Fieldml_GetTypeComponentEnsemble( handle, oHandle ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_ENSEMBLE_TYPE );
    fprintf( stdout, "EnsembleTypes: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_ENSEMBLE_TYPE, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s\n", i, Fieldml_GetObjectName( handle, oHandle ) );
        
        count2 = Fieldml_GetMemberCount( handle, oHandle );
        fprintf( stdout, "  %d elements: ", count2 );
        fprintf( stdout, "\n" );
    }

    count = Fieldml_GetObjectCount( handle, FHT_MESH_TYPE );
    fprintf( stdout, "MeshTypes: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_MESH_TYPE, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s (%s, %s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetMeshElementsType( handle, oHandle ) ),
            Fieldml_GetObjectName( handle, Fieldml_GetMeshChartType( handle, oHandle ) ) );
        fprintf( stdout, "    " );
        count2 = Fieldml_GetMemberCount( handle, oHandle );
        fprintf( stdout, "  %d elements:  \n", count2 );
    }

    count = Fieldml_GetObjectCount( handle, FHT_PARAMETER_EVALUATOR );
    fprintf( stdout, "ParameterEvaluators: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_PARAMETER_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %d %s (%s)\n", i, Fieldml_GetParameterDataDescription( handle, oHandle ),
            Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );
        count2 = Fieldml_GetParameterIndexCount( handle, oHandle, 1 );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "   sparse: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetParameterIndexEvaluator( handle, oHandle, j, 1 ) ) );
        }
        count2 = Fieldml_GetParameterIndexCount( handle, oHandle, 0 );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    dense: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetParameterIndexEvaluator( handle, oHandle, j, 0 ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_REFERENCE_EVALUATOR );
    fprintf( stdout, "ReferenceEvaluators: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_REFERENCE_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );
        fprintf( stdout, "    Source name: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetReferenceSourceEvaluator( handle, oHandle ) ) );
        
        count2 = Fieldml_GetBindCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    %s  -->  %s\n",
                Fieldml_GetObjectName( handle, Fieldml_GetBindArgument( handle, oHandle, j ) ),
                Fieldml_GetObjectName( handle, Fieldml_GetBindEvaluator( handle, oHandle, j ) ) ); 
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_PIECEWISE_EVALUATOR );
    fprintf( stdout, "PiecewiseEvaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_PIECEWISE_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s over %s (%s)\n", i,
            Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetIndexEvaluator( handle, oHandle, 1 ) ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );

        count2 = Fieldml_GetBindCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    %s  -->  %s\n",
                Fieldml_GetObjectName( handle, Fieldml_GetBindArgument( handle, oHandle, j ) ),
                Fieldml_GetObjectName( handle, Fieldml_GetBindEvaluator( handle, oHandle, j ) ) ); 
        }

        count2 = Fieldml_GetEvaluatorCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "   entry %d: %d -> %s\n", j, Fieldml_GetEvaluatorElement( handle, oHandle, j ),
                Fieldml_GetObjectName( handle, Fieldml_GetEvaluator( handle, oHandle, j ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_AGGREGATE_EVALUATOR );
    fprintf( stdout, "AggregateEvaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_AGGREGATE_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );

        count2 = Fieldml_GetBindCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    %s  -->  %s\n",
                Fieldml_GetObjectName( handle, Fieldml_GetBindArgument( handle, oHandle, j ) ),
                Fieldml_GetObjectName( handle, Fieldml_GetBindEvaluator( handle, oHandle, j ) ) ); 
        }

        count2 = Fieldml_GetEvaluatorCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "   entry %d: %d -> %s\n", j, Fieldml_GetEvaluatorElement( handle, oHandle, j ),
                Fieldml_GetObjectName( handle, Fieldml_GetEvaluator( handle, oHandle, j ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_ARGUMENT_EVALUATOR );
    fprintf( stdout, "ArgumentEvaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_ARGUMENT_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );
    }

    count = Fieldml_GetObjectCount( handle, FHT_EXTERNAL_EVALUATOR );
    fprintf( stdout, "External evaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_EXTERNAL_EVALUATOR, i );
        if( !Fieldml_IsObjectLocal( handle, oHandle ) )
        {
            continue;
        }
        
        fprintf( stdout, "  %d: %s\n", i, Fieldml_GetObjectName( handle, oHandle ) );
    }
    
    
    fprintf( stdout, "*******************************\n" );
    
    count = Fieldml_GetErrorCount( handle );
    if( count <= 0 )
    {
        fprintf( stdout, "No Errors\n" );
    }
    else
    {
        for( i = 1; i <= count; i++ )
        {
            fprintf( stdout, "   %s\n", Fieldml_GetError( handle, i ) );
        }
    }

    fprintf( stdout, "*******************************\n" );
    
    Fieldml_Destroy( handle );
}


int testWrite( const char *filename )
{
    FmlSessionHandle handle;
    char *outputFilename;
    int result;

    printf( "Test write...\n" );
    
    handle = Fieldml_CreateFromFile( filename );
    
    Fieldml_SetDebug( handle, 1 );

    outputFilename = (char*)calloc( 1, strlen( filename ) + 10 );
    strcpy( outputFilename, filename );
    strcat( outputFilename, "_out.xml" );
    
    result = Fieldml_WriteFile( handle, outputFilename );

    Fieldml_Destroy( handle );
    
    return 0;
}


void testMisc()
{
    bool testOk = true;
    FmlSessionHandle handle;
    FmlReaderHandle reader;
    FmlWriterHandle writer;
    double values[] = { 45.3, 67.0, -12.8 };
    int indexValues1[] = { 3, 1 };
    double rawValues1[] = { -98.7, -87.6, -76.5, -65.4, -54.3, -43.2, -32.1, -21.0, -10.0 };
    int indexValues2[] = { 2, 2 };
    double rawValues2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    double readValues[9] = { 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef };
    int readIndexes[2] = { -1, -1 };
    int sizes[10], offsets[10];
    
    printf( "Test misc...\n" );
    
    handle = Fieldml_Create( "", "test" );
    
    FmlObjectHandle continousType = Fieldml_CreateContinuousType( handle, "example.continuous_type" );
    Fieldml_CreateContinuousTypeComponents( handle, continousType, "example.component_ensemble", 3 );
    
    Fieldml_WriteFile( handle, ".\\output\\foo.xml" );
    
    Fieldml_Destroy( handle );
    
    handle = Fieldml_Create( "", "test" );
    
    int importHandle = Fieldml_AddImportSource( handle, "http://www.fieldml.org/resources/xml/0.4/FieldML_Library_0.4.xml", "library" );
    FmlObjectHandle rc3Ensemble = Fieldml_AddImport( handle, importHandle, "chart.3d.component", "chart.3d.component" );
    FmlObjectHandle realType = Fieldml_AddImport( handle, importHandle, "real.1d", "real.1d" );
    
    FmlObjectHandle parametersResource = Fieldml_CreateInlineDataResource( handle, "test.resource.parameters_data" );
    FmlObjectHandle parametersData = Fieldml_CreateArrayDataSource( handle, "test.parameters_data", parametersResource, "1", 1 );

    sizes[0] = 3;
    Fieldml_SetArrayDataSourceRawSizes( handle, parametersData, sizes );
    Fieldml_SetArrayDataSourceSizes( handle, parametersData, sizes );
    
    FmlObjectHandle parameters = Fieldml_CreateParameterEvaluator( handle, "test.ensemble_parameters", realType );
    Fieldml_SetParameterDataDescription( handle, parameters, DESCRIPTION_DENSE_ARRAY );
    Fieldml_SetDataSource( handle, parameters, parametersData );
    
    FmlObjectHandle rc3Index = Fieldml_CreateArgumentEvaluator( handle, "test.rc_3d.argument", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters, rc3Index, FML_INVALID_HANDLE );
    
    sizes[0] = 3;
    offsets[0] = 0;
    writer = Fieldml_OpenArrayWriter( handle, parametersData, realType, 0, sizes, 1 );
    Fieldml_WriteDoubleSlab( handle, writer, offsets, sizes, values );
    Fieldml_CloseWriter( handle, writer );
    
    reader = Fieldml_OpenReader( handle, parametersData );
    Fieldml_ReadDoubleSlab( handle, reader, offsets, sizes, readValues );
    Fieldml_CloseReader( handle, reader );

    for( int i = 0; i < 3; i++ )
    {
        if( values[i] != readValues[i] ) 
        {
            testOk = false;
            printf( "Parameter stream simple test failed: %d %g != %g\n", i, values[i], readValues[i] );
        }
    }
    
    FmlObjectHandle parametersResource2 = Fieldml_CreateInlineDataResource( handle, "test.resource.parameters_data2" );
    FmlObjectHandle parameters2KeyData = Fieldml_CreateArrayDataSource( handle, "test.parameters2_keydata", parametersResource2, "1", 2 );
    FmlObjectHandle parameters2ValueData = Fieldml_CreateArrayDataSource( handle, "test.parameters2_valuedata", parametersResource2, "2", 3 );
    
    sizes[0] = 2;
    sizes[1] = 2;
    Fieldml_SetArrayDataSourceRawSizes( handle, parameters2KeyData, sizes );
    Fieldml_SetArrayDataSourceSizes( handle, parameters2KeyData, sizes );

    sizes[0] = 2;
    sizes[1] = 3;
    sizes[2] = 3;
    Fieldml_SetArrayDataSourceRawSizes( handle, parameters2ValueData, sizes );
    Fieldml_SetArrayDataSourceSizes( handle, parameters2ValueData, sizes );
    
    FmlObjectHandle parameters2 = Fieldml_CreateParameterEvaluator( handle, "test.ensemble_parameters.2", realType );
    Fieldml_SetParameterDataDescription( handle, parameters2, DESCRIPTION_DOK_ARRAY );
    Fieldml_SetKeyDataSource( handle, parameters2, parameters2KeyData );
    Fieldml_SetDataSource( handle, parameters2, parameters2ValueData );
    
    FmlObjectHandle rc3Index1 = Fieldml_CreateArgumentEvaluator( handle, "test.rc_3d.argument.21", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters2, rc3Index1, FML_INVALID_HANDLE );

    FmlObjectHandle rc3Index2 = Fieldml_CreateArgumentEvaluator( handle, "test.rc_3d.argument.22", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters2, rc3Index2, FML_INVALID_HANDLE );

    FmlObjectHandle rc3Index3 = Fieldml_CreateArgumentEvaluator( handle, "test.rc_3d.argument.23", rc3Ensemble );
    Fieldml_AddSparseIndexEvaluator( handle, parameters2, rc3Index3 );
    
    FmlObjectHandle rc3Index4 = Fieldml_CreateArgumentEvaluator( handle, "test.rc_3d.argument.24", rc3Ensemble );
    Fieldml_AddSparseIndexEvaluator( handle, parameters2, rc3Index4 );

    sizes[0] = 2;
    sizes[1] = 2;
    offsets[0] = 0;
    offsets[1] = 0;
    writer = Fieldml_OpenArrayWriter( handle, parameters2KeyData, rc3Ensemble, 0, sizes, 2 );
    
    sizes[0] = 1;
    sizes[1] = 2;
    Fieldml_WriteIntSlab( handle, writer, offsets, sizes, indexValues1 );
    offsets[0] = 1;
    Fieldml_WriteIntSlab( handle, writer, offsets, sizes, indexValues2 );
    Fieldml_CloseWriter( handle, writer );
    
    sizes[0] = 2;
    sizes[1] = 3;
    sizes[2] = 3;
    writer = Fieldml_OpenArrayWriter( handle, parameters2ValueData, realType, 1, sizes, 3 );
    
    sizes[0] = 1;
    sizes[1] = 3;
    sizes[2] = 3;
    offsets[0] = 0;
    offsets[1] = 0;
    offsets[2] = 0;
    Fieldml_WriteDoubleSlab( handle, writer, offsets, sizes, rawValues1 );
    offsets[0] = 1;
    Fieldml_WriteDoubleSlab( handle, writer, offsets, sizes, rawValues2 );
    Fieldml_CloseWriter( handle, writer );
    
    reader = Fieldml_OpenReader( handle, parameters2KeyData );

    offsets[0] = 0;
    offsets[1] = 0;
    sizes[0] = 1;
    sizes[1] = 2;
    Fieldml_ReadIntSlab( handle, reader, offsets, sizes, readIndexes );
    for( int i = 0; i < 2; i++ )
    {
        if( indexValues1[i] != readIndexes[i] )
        {
            testOk = false;
            printf( "Parameter stream DOK test first index read failed: index %d %d != %d\n", i, indexValues1[i], readIndexes[i] );
        }
    }
    
    offsets[0] = 1;
    Fieldml_ReadIntSlab( handle, reader, offsets, sizes, readIndexes );
    for( int i = 0; i < 2; i++ )
    {
        if( indexValues2[i] != readIndexes[i] )
        {
            testOk = false;
            printf( "Parameter stream DOK test second index read failed: index %d %d != %d\n", i, indexValues2[i], readIndexes[i] );
        }
    }
    
    Fieldml_CloseReader( handle, reader );
    
    reader = Fieldml_OpenReader( handle, parameters2ValueData );
    
    offsets[0] = 0;
    offsets[1] = 0;
    offsets[2] = 0;
    sizes[0] = 1;
    sizes[1] = 3;
    sizes[2] = 3;
    Fieldml_ReadDoubleSlab( handle, reader, offsets, sizes, readValues );

    for( int i = 0; i < 9; i++ )
    {
        if( rawValues1[i] != readValues[i] ) 
        {
            testOk = false;
            printf( "Parameter stream DOK test first values read failed: %d %g != %g\n", i, rawValues1[i], readValues[i] );
        }
    }
    
    offsets[0] = 1;
    Fieldml_ReadDoubleSlab( handle, reader, offsets, sizes, readValues );

    for( int i = 0; i < 9; i++ )
    {
        if( rawValues2[i] != readValues[i] ) 
        {
            testOk = false;
            printf( "Parameter stream DOK test second values read failed: %d %g != %g\n", i, rawValues2[i], readValues[i] );
        }
    }
    
    Fieldml_CloseReader( handle, reader );

    Fieldml_Destroy( handle );
    if( testOk ) 
    {
        printf( "TestMisc - ok\n" );
    }
    else
    {
        printf( "TestMisc - failed\n" );
    }
}


int testCycles()
{
    bool testOk = true;
    
    printf( "Test cycles...\n" );
    
    FmlSessionHandle session = Fieldml_Create( "test", "test" );
    
    FmlObjectHandle type = Fieldml_CreateContinuousType( session, "test.type" );
    
    FmlObjectHandle ensemble = Fieldml_CreateEnsembleType( session, "test.ensemble" );
    Fieldml_SetEnsembleMembersRange( session, ensemble, 1, 20, 1 );
    
    FmlObjectHandle arg1 = Fieldml_CreateArgumentEvaluator( session, "test.arg1", type );
    
    FmlObjectHandle external = Fieldml_CreateExternalEvaluator( session, "test.external", type );
    
    FmlObjectHandle ref1 = Fieldml_CreateReferenceEvaluator( session, "test.reference1", external );
    FmlObjectHandle ref2 = Fieldml_CreateReferenceEvaluator( session, "test.reference2", ref1 );

    if( Fieldml_SetBind( session, ref1, arg1, ref2 ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - ReferenceEvaluator test failed\n" );
        testOk = false;
    }
    
    FmlObjectHandle param = Fieldml_CreateParameterEvaluator( session, "test.parameter", ensemble );
    Fieldml_SetParameterDataDescription( session, param, DESCRIPTION_DOK_ARRAY );
    
    FmlObjectHandle ref3 = Fieldml_CreateReferenceEvaluator( session, "test.reference3", param );
    
    if( Fieldml_AddDenseIndexEvaluator( session, param, ref3, FML_INVALID_HANDLE ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - ParameterEvaluator dense test failed\n" );
        testOk = false;
    }
    if( Fieldml_AddSparseIndexEvaluator( session, param, ref3 ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - ParameterEvaluator sparse test failed\n" );
        testOk = false;
    }
    
    FmlObjectHandle piece = Fieldml_CreatePiecewiseEvaluator( session, "test.piecewise", ensemble );
    Fieldml_AddDenseIndexEvaluator( session, param, piece, FML_INVALID_HANDLE );
    
    if( Fieldml_SetDefaultEvaluator( session, piece, param ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - PiecewiseEvaluator default test failed\n" );
        testOk = false;
    }
    
    if( Fieldml_SetEvaluator( session, piece, 1, param ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - PiecewiseEvaluator evaluator test failed\n" );
    }
    
    if( Fieldml_SetIndexEvaluator( session, piece, 1, param ) != FML_ERR_CYCLIC_DEPENDENCY )
    {
        printf( "TestCycles - PiecewiseEvaluator index evaluator test failed\n" );
    }
    
    Fieldml_Destroy( session );
    
    if( testOk ) 
    {
        printf( "TestCycles - ok\n" );
    }
    else
    {
        printf( "TestCycles - failed\n" );
    }
    
    return 0;
}


int testHdf5Read()
{
    bool testOk = true;
    
    printf("Testing HDF5 array read\n");

    FmlSessionHandle session = Fieldml_Create( "test", "test" );
    
    FmlObjectHandle resource = Fieldml_CreateHrefDataResource( session, "test.resource2", "HDF5", "./input/I16BE.h5" );
    FmlObjectHandle sourceI = Fieldml_CreateArrayDataSource( session, "test.source2_int", resource, "I16BE", 2 );
    FmlObjectHandle sourceD = Fieldml_CreateArrayDataSource( session, "test.source2_double", resource, "DOUBLE", 2 );
    
    FmlObjectHandle reader = Fieldml_OpenReader( session, sourceI );
    
    const int INT_FILL = 12345;
    const double DOUBLE_FILL = 0.12345f;
    const double TOLERANCE = 1e-10;
    
    int offsets[1];
    offsets[0] = 2;
    
    int sizes[1];
    sizes[0] = 3;
    
    int dataI[20];
    for( int i = 0; i < 20; i++ )
    {
        dataI[i] = INT_FILL;
    }

    Fieldml_ReadIntSlab( session, reader, offsets, sizes, dataI );
        
    Fieldml_CloseReader( session, reader );

    for( int i = 0; i < sizes[0]; i++ )
    {
        const int index = i + offsets[0];
        const int expected = (index * 100) + index;
        if( dataI[i] != expected )
        {
            printf("Data mismatch in I16BE.h5/I16BE. Expecting %d, got %d\n", expected, dataI[i] );
            testOk = false;
            break;
        }
    }
    for( int i = sizes[0]; i < 20; i++ )
    {
        if( dataI[i] != INT_FILL )
        {
            printf("Data over-read in I16BE.h5/I16BE. Expecting %d, got %d\n", INT_FILL, dataI[i] );
            testOk = false;
            break;
        }
    }
    
    reader = Fieldml_OpenReader( session, sourceD );
    
    offsets[0] = 4;
    sizes[0] = 6;
    
    double dataD[20];
    for( int i = 0; i < 20; i++ )
    {
        dataD[i] = DOUBLE_FILL;
    }
    
    Fieldml_ReadDoubleSlab( session, reader, offsets, sizes, dataD );
    
    reader = Fieldml_OpenReader( session, sourceD );
    
    Fieldml_CloseReader( session, reader );

    for( int i = 0; i < sizes[0]; i++ )
    {
        const int index = (i + offsets[0])%6;
        const double expected = (index * 100) + ( index *0.001 );
        double delta = dataD[i] - expected;
        if( delta > TOLERANCE )
        {
            printf("Data mismatch in I16BE.h5/I16BE. Expecting %g got %g (%g)\n", expected, dataD[i], delta );
            testOk = false;
            break;
        }
    }
    for( int i = sizes[0]; i < 20; i++ )
    {
        if( dataD[i] != DOUBLE_FILL )
        {
            printf("Data over-read in I16BE.h5/I16BE. Expecting %g, got %g\n", DOUBLE_FILL, dataD[i] );
            testOk = false;
            break;
        }
    }
    
    Fieldml_Destroy( session );
    
    if( testOk ) 
    {
        printf( "TestHdf5Read - ok\n" );
    }
    else
    {
        printf( "TestHdf5Read - failed\n" );
    }
    
    return 0;
}


int testHdf5Write()
{
    bool testOk = true;
    
    printf("Testing HDF5 array write\n");

    FmlSessionHandle session = Fieldml_Create( "test", "test" );
    
    FmlObjectHandle cType = Fieldml_CreateContinuousType( session, "test.scalar_real" );
    
    FmlObjectHandle resource = Fieldml_CreateHrefDataResource( session, "test.resource", "HDF5", "./output/test.h5" );
    FmlObjectHandle sourceD = Fieldml_CreateArrayDataSource( session, "test.source_double", resource, "foo2", 2 );
    
    double dataD[20];
    const double DOUBLE_FILL = 0.12345;
    for( int i = 0; i < 20; i++ )
    {
        dataD[i] = DOUBLE_FILL;
    }

    for( int i = 0; i < 6; i++ )
    {
        dataD[i] = ( 200 * i ) + ( 0.002 * i );
    }
    
    int offsets[1];
    int sizes[1];

    sizes[0] = 12;
    FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, cType, 1, sizes, 1 );

    offsets[0] = 0;
    sizes[0] = 6;
    Fieldml_WriteDoubleSlab( session, writer, offsets, sizes, dataD );
    
    offsets[0] = 6;
    sizes[0] = 6;
    Fieldml_WriteDoubleSlab( session, writer, offsets, sizes, dataD );
    
    Fieldml_CloseWriter( session, writer );
    
    Fieldml_Destroy( session );
    
    if( testOk ) 
    {
        printf( "TestHdf5Write - ok\n" );
    }
    else
    {
        printf( "TestHdf5Write - failed\n" );
    }
    
    return 0;
}


int main( int argc, char **argv )
{
    if( argc > 1 )
    {
        testRead( argv[1] );
        testWrite( argv[1] );
    }
    testMisc();
    
    testCycles();
    
    testHdf5Read();
    
    testHdf5Write();
    
    return 0;
}
