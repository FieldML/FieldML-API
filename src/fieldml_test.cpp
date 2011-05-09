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

#include <string.h>

#include <libxml/SAX.h>
#include <libxml/globals.h>
#include <libxml/xmlerror.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlschemas.h>

#include "InputStream.h"
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
        
        count2 = Fieldml_GetElementCount( handle, oHandle );
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
        count2 = Fieldml_GetElementCount( handle, oHandle );
        fprintf( stdout, "  %d elements:  \n", count2 );
        //TODO Dirty, dirty hack.
        for( int e = 1; e <= count2; e++ )
        {
            const char *shape = Fieldml_GetMeshElementShape( handle, oHandle, e, 1 );
            if( shape != NULL )
            {
                printf("  %d -> %s\n", e, shape );
            }
        }
        fprintf( stdout, "\n" );
    }

    count = Fieldml_GetObjectCount( handle, FHT_PARAMETER_EVALUATOR );
    fprintf( stdout, "ParametersEvaluators: %d\n", count ); 
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
        count2 = Fieldml_GetSemidenseIndexCount( handle, oHandle, 1 );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "   sparse: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetSemidenseIndexEvaluator( handle, oHandle, j, 1 ) ) );
        }
        count2 = Fieldml_GetSemidenseIndexCount( handle, oHandle, 0 );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    dense: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetSemidenseIndexEvaluator( handle, oHandle, j, 0 ) ) );
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
                Fieldml_GetObjectName( handle, Fieldml_GetBindVariable( handle, oHandle, j ) ),
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
                Fieldml_GetObjectName( handle, Fieldml_GetBindVariable( handle, oHandle, j ) ),
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
                Fieldml_GetObjectName( handle, Fieldml_GetBindVariable( handle, oHandle, j ) ),
                Fieldml_GetObjectName( handle, Fieldml_GetBindEvaluator( handle, oHandle, j ) ) ); 
        }

        count2 = Fieldml_GetEvaluatorCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "   entry %d: %d -> %s\n", j, Fieldml_GetEvaluatorElement( handle, oHandle, j ),
                Fieldml_GetObjectName( handle, Fieldml_GetEvaluator( handle, oHandle, j ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_ABSTRACT_EVALUATOR );
    fprintf( stdout, "AbstractEvaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_ABSTRACT_EVALUATOR, i );
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

    handle = Fieldml_CreateFromFile( filename );
    
    Fieldml_SetDebug( handle, 1 );

    outputFilename = (char*)calloc( 1, strlen( filename ) + 10 );
    strcpy( outputFilename, filename );
    strcat( outputFilename, "_out.xml" );
    
    result = Fieldml_WriteFile( handle, outputFilename );

    Fieldml_Destroy( handle );
    
    return 0;
}


void testStream()
{
    FieldmlInputStream *stream;
    int iActual;
    int iExpected[7] = { 129, 24, 333, 456, -512, 6324, 123 };
    double dExpected[10] = { 129, 24.1, -78.239, -21.34, 65.12, 3.0, 3.2, 0.092, -0.873, 0.005 };
    double dActual; 
    int i;
    bool testOk = true;
    
    stream = FieldmlInputStream::createStringStream( "129 24 ,, 333 .. 456 -512  \n 6324 \r\n asc123asc" );
    for( i = 0; i < 7; i++ )
    {
        iActual = stream->readInt();
        
        if( iActual != iExpected[i] )
        {
            testOk = false;
            fprintf( stderr, "Mismatch at %d: %d != %d\n", i, iExpected[i], iActual );
        }
    }
    
    delete stream;
    
    stream = FieldmlInputStream::createStringStream( "129 ,, 24.1 -78.239 -21.34 65.12,,\r\n\t asf3asf3.2asf.092xxx-.873 0.5e-02" );

    for( i = 0; i < 10; i++ )
    {
        dActual = stream->readDouble();
        
        if( dActual != dExpected[i] )
        {
            testOk = false;
            fprintf( stderr, "Mismatch at %d: %f != %f\n", i, dExpected[i], dActual );
        }
    }

    delete stream;

    if( testOk ) 
    {
        printf("TestStream - ok\n" );
    }
    else
    {
        printf("TestStream - failed\n" );
    }
}


void testMisc()
{
    bool testOk = true;
    int i;
    FmlSessionHandle handle;
    FmlReaderHandle reader;
    FmlWriterHandle writer;
    double values[] = { 45.3, 67.0, -12.8 };
    int indexValues1[] = { 3, 1 };
    double rawValues1[] = { -98.7, -87.6, -76.5, -65.4, -54.3, -43.2, -32.1, -21.0, -10.0 };
    int indexValues2[] = { 2, 2 };
    double rawValues2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int dummy[] = { 0 };
    double readValues[9] = { 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef };
    int readIndexes[2] = { -1, -1 };
    
    handle = Fieldml_Create( "", "test" );
    
    FmlObjectHandle continousType = Fieldml_CreateContinuousType( handle, "example.continuous_type" );
    FmlObjectHandle componentEnsemble = Fieldml_CreateContinuousTypeComponents( handle, continousType, "example.component_ensemble", 3 );
    
    Fieldml_WriteFile( handle, "foo.xml" );
    
    Fieldml_Destroy( handle );
    
    handle = Fieldml_Create( "", "test" );
    
    int importHandle = Fieldml_AddImportSource( handle, "library.xml", "library" );
    FmlObjectHandle rc3Ensemble = Fieldml_AddImport( handle, importHandle, "library.chart.3d.component", "library.chart.3d.component" );
    FmlObjectHandle realType = Fieldml_AddImport( handle, importHandle, "library.real.1d", "library.real.1d" );
    
    FmlObjectHandle parametersData = Fieldml_CreateDataObject( handle, "test.parameters_data" );
    Fieldml_SetDataObjectSourceType( handle, parametersData, SOURCE_INLINE );
    Fieldml_SetDataObjectEntryInfo( handle, parametersData, 1, 3, 0, 0 );
    
    FmlObjectHandle parameters = Fieldml_CreateParametersEvaluator( handle, "test.ensemble_parameters", realType );
    Fieldml_SetParameterDataDescription( handle, parameters, DESCRIPTION_SEMIDENSE );
    Fieldml_SetDataObject( handle, parameters, parametersData );
    
    FmlObjectHandle rc3Index = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters, rc3Index, FML_INVALID_HANDLE );
    
    writer = Fieldml_OpenWriter( handle, parametersData, 1 );
    Fieldml_WriteDoubleValues( handle, writer, values, 3 );
    Fieldml_CloseWriter( handle, writer );
    
    reader = Fieldml_OpenReader( handle, parametersData );
    Fieldml_ReadDoubleValues( handle, reader, readValues, 3 );
    Fieldml_CloseReader( handle, reader );
    
    for( i = 0; i < 3; i++ )
    {
        if( values[i] != readValues[i] ) 
        {
            testOk = false;
            printf("Parameter stream simple test failed: %d %g != %g\n", i, values[i], readValues[i] );
        }
    }
    
    FmlObjectHandle parametersData2 = Fieldml_CreateDataObject( handle, "test.parameters_data2" );
    Fieldml_SetDataObjectSourceType( handle, parametersData2, SOURCE_INLINE );
    Fieldml_SetDataObjectEntryInfo( handle, parametersData2, 2, 11, 0, 0 );
    
    FmlObjectHandle parameters2 = Fieldml_CreateParametersEvaluator( handle, "test.ensemble_parameters.2", realType );
    Fieldml_SetParameterDataDescription( handle, parameters2, DESCRIPTION_SEMIDENSE );
    Fieldml_SetDataObject( handle, parameters2, parametersData2 );
    
    FmlObjectHandle rc3Index1 = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract.21", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters2, rc3Index1, FML_INVALID_HANDLE );
    
    FmlObjectHandle rc3Index2 = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract.22", rc3Ensemble );
    Fieldml_AddDenseIndexEvaluator( handle, parameters2, rc3Index2, FML_INVALID_HANDLE );

    FmlObjectHandle rc3Index3 = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract.23", rc3Ensemble );
    Fieldml_AddSparseIndexEvaluator( handle, parameters2, rc3Index3 );
    
    FmlObjectHandle rc3Index4 = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract.24", rc3Ensemble );
    Fieldml_AddSparseIndexEvaluator( handle, parameters2, rc3Index4 );
    
    writer = Fieldml_OpenWriter( handle, parametersData2, 1 );
    Fieldml_WriteIntValues( handle, writer, indexValues1, 2 );
    Fieldml_WriteDoubleValues( handle, writer, rawValues1, 9 );
    Fieldml_WriteIntValues( handle, writer, indexValues2, 2 );
    Fieldml_WriteDoubleValues( handle, writer, rawValues2, 9 );
    Fieldml_CloseWriter( handle, writer );
    
    reader = Fieldml_OpenReader( handle, parametersData2 );

    Fieldml_ReadIntValues( handle, reader, readIndexes, 2 );
    if( ( indexValues1[0] != readIndexes[0] ) && ( indexValues1[1] != readIndexes[1] ) ) 
    {
        testOk = false;
        printf("Parameter stream semidense first index read failed: index %d %d != %d\n", i, indexValues1[i], readIndexes[i] );
    }

    Fieldml_ReadDoubleValues( handle, reader, readValues, 3 );
    Fieldml_ReadDoubleValues( handle, reader, readValues+3, 6 );

    for( i = 0; i < 9; i++ )
    {
        if( rawValues1[i] != readValues[i] ) 
        {
            testOk = false;
            printf("Parameter stream semidense first values read failed: %d %g != %g\n", i, rawValues1[i], readValues[i] );
        }
    }
    
    Fieldml_ReadIntValues( handle, reader, readIndexes, 2 );
    if( ( indexValues2[0] != readIndexes[0] ) && ( indexValues2[1] != readIndexes[1] ) ) 
    {
        testOk = false;
        printf("Parameter stream test second index read failed: index %d %d != %d\n", i, indexValues2[i], readIndexes[i] );
    }

    Fieldml_ReadDoubleValues( handle, reader, readValues, 9 );

    for( i = 0; i < 9; i++ )
    {
        if( rawValues2[i] != readValues[i] ) 
        {
            testOk = false;
            printf("Parameter stream test second values read failed: %d %g != %g\n", i, rawValues2[i], readValues[i] );
        }
    }
    
    Fieldml_CloseReader( handle, reader );


    Fieldml_Destroy( handle );
    if( testOk ) 
    {
        printf("TestMisc - ok\n" );
    }
    else
    {
        printf("TestMisc - failed\n" );
    }
}


int main( int argc, char **argv )
{
    testRead( argv[1] );
    testWrite( argv[1] );
    testMisc();
    testStream();

    xmlCleanupParser( );
    xmlMemoryDump( );
    
    return 0;
}
