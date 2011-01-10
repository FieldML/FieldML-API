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

static int validate( char *filename )
{
    int res;
    xmlParserInputBufferPtr buffer;
    char *schema;
    xmlSchemaPtr schemas = NULL;
    xmlSchemaParserCtxtPtr sctxt;
    xmlSchemaValidCtxtPtr vctxt;
    
    LIBXML_TEST_VERSION

    xmlSubstituteEntitiesDefault( 1 );

    schema = "Fieldml_0.3.xsd";

    buffer = xmlParserInputBufferCreateFilename( filename, XML_CHAR_ENCODING_NONE );
    if( buffer == NULL )
    {
        return 0;
    }

    sctxt = xmlSchemaNewParserCtxt( schema );
    xmlSchemaSetParserErrors( sctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr );
    schemas = xmlSchemaParse( sctxt );
    if( schemas == NULL )
    {
        xmlGenericError( xmlGenericErrorContext, "WXS schema %s failed to compile\n", schema );
        schema = NULL;
    }
    xmlSchemaFreeParserCtxt( sctxt );

    vctxt = xmlSchemaNewValidCtxt( schemas );
    xmlSchemaSetValidErrors( vctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr );

    res = xmlSchemaValidateStream( vctxt, buffer, (xmlCharEncoding)0, NULL, NULL );
    if( res == 0 )
    {
        fprintf( stderr, "%s validates\n", filename );
    }
    else if( res > 0 )
    {
        fprintf( stderr, "%s fails to validate\n", filename );
    }
    else
    {
        fprintf( stderr, "%s validation generated an internal error\n", filename );
    }

    xmlSchemaFreeValidCtxt( vctxt );

    xmlSchemaFree( schemas );
    
    xmlCleanupParser( );
    xmlMemoryDump( );
    
    return res == 0;
}


void testRead( const char * filename )
{
    int i, j, count, count2;
    FmlObjectHandle oHandle;
    FmlHandle handle;
    TypeBoundsType boundsType;
    const int *swizzle;
    const char *shape;

    handle = Fieldml_CreateFromFile( filename );
    
    Fieldml_SetDebug( handle, 1 );

    count = Fieldml_GetObjectCount( handle, FHT_CONTINUOUS_TYPE );
    fprintf( stdout, "ContinuousTypes: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_CONTINUOUS_TYPE, i );
        
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
        
        fprintf( stdout, "  %d: %s\n", i, Fieldml_GetObjectName( handle, oHandle ) );
        
        boundsType = Fieldml_GetBoundsType( handle, oHandle );
        if( boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            fprintf( stdout, "    1...%d\n", Fieldml_GetContiguousBoundsCount( handle, oHandle ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_MESH_TYPE );
    fprintf( stdout, "MeshTypes: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_MESH_TYPE, i );
        
        fprintf( stdout, "  %d: %s (%s, %s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetMeshElementType( handle, oHandle ) ),
            Fieldml_GetObjectName( handle, Fieldml_GetMeshXiType( handle, oHandle ) ) );
        boundsType = Fieldml_GetBoundsType( handle, Fieldml_GetMeshElementType( handle, oHandle ) );
        if( boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            int bounds = Fieldml_GetContiguousBoundsCount( handle, Fieldml_GetMeshElementType( handle, oHandle ) );
            fprintf( stdout, "    1...%d\n", bounds );
            for( j = 1; j <= bounds; j++ )
            {
                shape = Fieldml_GetMeshElementShape( handle, oHandle, j, 0 );
                if( shape != NULL )
                {
                    fprintf( stdout, "    %d: %s\n", j, shape );
                }
            }
        }
        
        count2 = Fieldml_GetMeshConnectivityCount( handle, oHandle );
        for( j = 1; j <= count2; j++ )
        {
            fprintf( stdout, "    %s: %s\n",
                Fieldml_GetObjectName( handle, Fieldml_GetMeshConnectivityType( handle, oHandle, j ) ),
                Fieldml_GetObjectName( handle, Fieldml_GetMeshConnectivitySource( handle, oHandle, j ) ) );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_PARAMETER_EVALUATOR );
    fprintf( stdout, "ParametersEvaluators: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_PARAMETER_EVALUATOR, i );
        
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
        
        count2 = Fieldml_GetSwizzleCount( handle, oHandle );
        if( count2 > 0 )
        {
            swizzle = Fieldml_GetSwizzleData( handle, oHandle );
            fprintf( stdout, "    swizzle: " );
            for( j = 0; j < count2; j++ )
            {
                fprintf( stdout, "%d ", swizzle[j] );
            }
            fprintf( stdout, "\n" );
        }
    }

    count = Fieldml_GetObjectCount( handle, FHT_REFERENCE_EVALUATOR );
    fprintf( stdout, "ReferenceEvaluators: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_REFERENCE_EVALUATOR, i );
        
        fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );
        fprintf( stdout, "    Remote name: %s\n", Fieldml_GetObjectName( handle, Fieldml_GetReferenceRemoteEvaluator( handle, oHandle ) ) );
        
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
        
        fprintf( stdout, "  %d: %s (%s)\n", i, Fieldml_GetObjectName( handle, oHandle ),
            Fieldml_GetObjectName( handle, Fieldml_GetValueType( handle, oHandle ) ) );
    }

    count = Fieldml_GetObjectCount( handle, FHT_REMOTE_TYPE );
    fprintf( stdout, "External type: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_REMOTE_TYPE, i );
        
        fprintf( stdout, "  %d: %s\n", i, Fieldml_GetObjectName( handle, oHandle ) );
    }

    count = Fieldml_GetObjectCount( handle, FHT_REMOTE_EVALUATOR );
    fprintf( stdout, "External evaluator: %d\n", count ); 
    for( i = 1; i <= count; i++ )
    {
        oHandle = Fieldml_GetObject( handle, FHT_REMOTE_EVALUATOR, i );
        
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
    FmlHandle handle;
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
    FmlInputStream stream;
    int iActual;
    int iExpected[7] = { 129, 24, 333, 456, -512, 6324, 123 };
    double dExpected[10] = { 129, 24.1, -78.239, -21.34, 65.12, 3.0, 3.2, 0.092, -0.873, 0.005 };
    double dActual; 
    int i;
    
    stream = FieldmlInputStream::create( "129 24 ,, 333 .. 456 -512  \n 6324 \r\n asc123asc" );
    for( i = 0; i < 7; i++ )
    {
        iActual = stream->readInt();
        
        if( iActual != iExpected[i] )
        {
            fprintf( stderr, "Mismatch at %d: %d != %d\n", i, iExpected[i], iActual );
        }
    }
    
    delete stream;
    
    stream = FieldmlInputStream::create( "129 ,, 24.1 -78.239 -21.34 65.12,,\r\n\t asf3asf3.2asf.092xxx-.873 0.5e-02" );

    for( i = 0; i < 10; i++ )
    {
        dActual = stream->readDouble();
        
        if( dActual != dExpected[i] )
        {
            fprintf( stderr, "Mismatch at %d: %f != %f\n", i, dExpected[i], dActual );
        }
    }

    delete stream;
}


void testMisc()
{
    int i;
    FmlHandle handle;
    FmlObjectHandle o1, o2, o3;
    FmlReaderHandle reader;
//    FmlWriterHandle writer;
    double values[] = { 45.3, 67.0, -12.8 };
    int dummy[] = { 0 };
    double readValues[3] = { 0xdeadbeef, 0xdeadbeef, 0xdeadbeef };
    
    handle = Fieldml_Create( "", "test" );
    
    o1 = Fieldml_CreateEnsembleType( handle, "example.component_ensemble", true );
    Fieldml_SetContiguousBoundsCount( handle, o1, 3 );
    
    o2 = Fieldml_CreateContinuousType( handle, "example.continuous_type", o1 );
    
    Fieldml_WriteFile( handle, "foo.xml" );
    
    Fieldml_Destroy( handle );
    
    handle = Fieldml_Create( "", "test" );
    
    o1 = Fieldml_GetObjectByName( handle, "library.ensemble.rc.3d" );
    o2 = Fieldml_GetObjectByName( handle, "library.real.1d" );
    
    o3 = Fieldml_CreateParametersEvaluator( handle, "test.ensemble_parameters", o2 );
    Fieldml_SetParameterDataDescription( handle, o3, DESCRIPTION_SEMIDENSE );
    Fieldml_SetParameterDataLocation( handle, o3, LOCATION_INLINE );
    
    o1 = Fieldml_CreateAbstractEvaluator( handle, "test.rc_3d.abstract", o1 );
    Fieldml_AddSemidenseIndexEvaluator( handle, o3, o1, 0 );
    
    Fieldml_AddParameterInlineData( handle, o3, "45.3 67.0 -12.8", 15 );
//    writer = Fieldml_OpenWriter( handle, o3, 1 );
//    Fieldml_WriteDoubleSlice( handle, writer, dummy, values );
//    Fieldml_CloseWriter( handle, writer );
    
    reader = Fieldml_OpenReader( handle, o3 );
    Fieldml_ReadDoubleValues( handle, reader, readValues, 3 );
    Fieldml_CloseReader( handle, reader );
    
    for( i = 0; i < 3; i++ )
    {
        if( values[i] != readValues[i] ) 
        {
            printf("Parameter stream test failed: %d %g != %g\n", i, values[i], readValues[i] );
        }
    }
    
    Fieldml_Destroy( handle );
}


int main( int argc, char **argv )
{
    if( !validate( argv[1] ) )
    {
        return 1;
    }
    testRead( argv[1] );
    testWrite( argv[1] );
    testMisc();
//    testStream();
    return 0;
}
