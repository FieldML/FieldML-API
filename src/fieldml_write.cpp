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

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include "string_const.h"
#include "fieldml_write.h"
#include "fieldml_api.h"

using namespace std;

const char * MY_ENCODING = "ISO-8859-1";

const int tBufferLength = 256;


static void writeObjectName( xmlTextWriterPtr writer, const xmlChar *attribute, FmlHandle handle, int object )
{
    xmlTextWriterWriteAttribute( writer, attribute, (const xmlChar*)Fieldml_GetObjectName( handle, object ) );
}


static void writeIntArray( xmlTextWriterPtr writer, const xmlChar *tag, int count, const int *array )
{
    int i;

    xmlTextWriterStartElement( writer, tag );
    
    for( i = 0; i < count; i++ )
    {
        xmlTextWriterWriteFormatString( writer, "%d ", array[i] );
    }

    xmlTextWriterEndElement( writer );
}


static void writeStringTableEntry( xmlTextWriterPtr writer, const xmlChar *tagName, const char *key, const char *value )
{
    xmlTextWriterStartElement( writer, tagName );
    xmlTextWriterWriteAttribute( writer, KEY_ATTRIB, (const xmlChar*)key );
    xmlTextWriterWriteAttribute( writer, VALUE_ATTRIB, (const xmlChar*)value );
    xmlTextWriterEndElement( writer );
}


static void writeIntTableEntry( xmlTextWriterPtr writer, const xmlChar *tagName, int key, const char *value )
{
    xmlTextWriterStartElement( writer, tagName );
    xmlTextWriterWriteFormatAttribute( writer, KEY_ATTRIB, "%d", key );
    xmlTextWriterWriteAttribute( writer, VALUE_ATTRIB, (const xmlChar*) value );
    xmlTextWriterEndElement( writer );
}


static void writeComponentEvaluator( xmlTextWriterPtr writer, const xmlChar *tagName, int key, const char *value )
{
    xmlTextWriterStartElement( writer, tagName );
    xmlTextWriterWriteFormatAttribute( writer, NUMBER_ATTRIB, "%d", key );
    xmlTextWriterWriteAttribute( writer, EVALUATOR_ATTRIB, (const xmlChar*) value );
    xmlTextWriterEndElement( writer );
}


static int writeBinds( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    int count = Fieldml_GetBindCount( handle, object );

    //NOTE index binds are currently handled via a dirty hack. Must clean this up.
    int type = Fieldml_GetObjectType( handle, object );
    
    FmlObjectHandle indexEvaluator = FML_INVALID_HANDLE;
    if( ( type == FHT_AGGREGATE_EVALUATOR ) || ( type == FHT_PIECEWISE_EVALUATOR ) )
    {
        indexEvaluator = Fieldml_GetIndexEvaluator( handle, object, 1 );
    }
    
    if( ( count <= 0 ) && ( indexEvaluator == FML_INVALID_HANDLE ) )
    {
        return 0;
    }

    xmlTextWriterStartElement( writer, BINDINGS_TAG );
    
    if( indexEvaluator != FML_INVALID_HANDLE )
    {
        xmlTextWriterStartElement( writer, BIND_INDEX_TAG );
        xmlTextWriterWriteAttribute( writer, VARIABLE_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, indexEvaluator ) );
        xmlTextWriterWriteAttribute( writer, INDEX_NUMBER_ATTRIB, (const xmlChar*)"1" );
        xmlTextWriterEndElement( writer );
    }
    
    for( int i = 1; i <= count; i++ )
    {
        int source = Fieldml_GetBindEvaluator( handle, object, i );
        int variable =  Fieldml_GetBindVariable( handle, object, i );
        if( ( source == FML_INVALID_HANDLE ) || ( variable == FML_INVALID_HANDLE ) )
        {
            continue;
        }

        xmlTextWriterStartElement( writer, BIND_TAG );
        xmlTextWriterWriteAttribute( writer, VARIABLE_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, variable ) );
        xmlTextWriterWriteAttribute( writer, SOURCE_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, source ) );
        xmlTextWriterEndElement( writer );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeVariables( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    int count = Fieldml_GetVariableCount( handle, object );
    if( count <= 0 )
    {
        return 0;
    }

    xmlTextWriterStartElement( writer, VARIABLES_TAG );
    
    for( int i = 1; i <= count; i++ )
    {
        int variable = Fieldml_GetVariable( handle, object, i );
        if( variable == FML_INVALID_HANDLE )
        {
            continue;
        }

        xmlTextWriterStartElement( writer, VARIABLE_TAG );
        xmlTextWriterWriteAttribute( writer, NAME_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, variable ) );
        xmlTextWriterEndElement( writer );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeContinuousType( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, CONTINUOUS_TYPE_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );

    FmlObjectHandle componentType = Fieldml_GetTypeComponentEnsemble( handle, object );
    if( componentType != FML_INVALID_HANDLE )
    {
        writeObjectName( writer, COMPONENT_ENSEMBLE_ATTRIB, handle, componentType );
    }

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static void writeElements( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object, const xmlChar *tagName )
{
    xmlTextWriterStartElement( writer, tagName );
    
    //Output ranges first, then individuals. It's cleaner that way, even if potentially slower.
    int index = 1;
    int firstElement = Fieldml_GetElementEntry( handle, object, index++ );
    int lastElement = firstElement;
    int prevElement;
    bool hasSingles = false; 
    while( firstElement != -1 )
    {
        //TODO Use strides greater than 1 to optimize output.
        prevElement = lastElement;
        lastElement = Fieldml_GetElementEntry( handle, object, index++ );
        while( lastElement == prevElement + 1 )
        {
            prevElement = lastElement;
            lastElement = Fieldml_GetElementEntry( handle, object, index++ );
        }
        
        if( prevElement != firstElement )
        {
            xmlTextWriterStartElement( writer, MEMBER_RANGE_TAG );
            xmlTextWriterWriteFormatAttribute( writer, MIN_ATTRIB, "%d", firstElement );
            xmlTextWriterWriteFormatAttribute( writer, MAX_ATTRIB, "%d", prevElement );
            xmlTextWriterEndElement( writer );
        }
        else
        {
            hasSingles = true;
        }
        
        firstElement = lastElement;
    }
    
    if( !hasSingles )
    {
        xmlTextWriterEndElement( writer );
        return;
    }
    
    
    xmlTextWriterStartElement( writer, MEMBER_LIST_TAG );

    index = 1;
    firstElement = Fieldml_GetElementEntry( handle, object, index++ );
    lastElement = firstElement;
    prevElement;
    while( firstElement != -1 )
    {
        //TODO Use strides greater than 1 to optimize output.
        prevElement = lastElement;
        lastElement = Fieldml_GetElementEntry( handle, object, index++ );
        while( lastElement == prevElement + 1 )
        {
            prevElement = lastElement;
            lastElement = Fieldml_GetElementEntry( handle, object, index++ );
        }
        
        if( prevElement == firstElement )
        {
            xmlTextWriterWriteFormatString( writer, "%d ", firstElement );
        }
        
        firstElement = lastElement;
    }

    xmlTextWriterEndElement( writer );

    xmlTextWriterEndElement( writer );
}


static int writeEnsembleType( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, ENSEMBLE_TYPE_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    if( Fieldml_IsEnsembleComponentType( handle, object ) == 1 )
    {
        xmlTextWriterWriteAttribute( writer, IS_COMPONENT_ENSEMBLE_ATTRIB, (const xmlChar*)STRING_TRUE );
    }
    
    writeElements( writer, handle, object, MEMBERS_TAG );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeMeshType( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, MESH_TYPE_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    
    FmlObjectHandle xiType = Fieldml_GetMeshXiType( handle, object );
    FmlObjectHandle xiComponents = Fieldml_GetTypeComponentEnsemble( handle, xiType );
    if( xiComponents != FML_INVALID_HANDLE )
    {
        writeObjectName( writer, XI_COMPONENT_ATTRIB, handle, xiComponents );
    }
    
    FmlObjectHandle elementType = Fieldml_GetMeshElementType( handle, object );
    int elementCount = Fieldml_GetElementCount( handle, elementType );

    writeElements( writer, handle, Fieldml_GetMeshElementType( handle, object ), ELEMENTS_TAG );

    xmlTextWriterStartElement( writer, MESH_SHAPES_TAG );
    const char *defaultShape = Fieldml_GetMeshDefaultShape( handle, object );
    if( defaultShape != NULL )
    {
        xmlTextWriterWriteFormatAttribute( writer, DEFAULT_ATTRIB, "%s", defaultShape );
    }
    for( int i = 1; i <= elementCount; i++ )
    {
        const char *shape = Fieldml_GetMeshElementShape( handle, object, i, 0 );
        if( shape == NULL )
        {
            continue;
        }
        writeIntTableEntry( writer, MESH_SHAPE_TAG, i, shape );
    }
    xmlTextWriterEndElement( writer );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeElementSequence( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, ELEMENT_SEQUENCE_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    xmlTextWriterStartElement( writer, ELEMENTS_TAG );
    
    int elementCount = Fieldml_GetElementCount( handle, object );
    
    if( elementCount > 0 )
    {
        //TODO This is potentially very inefficient.
        int lastElement = -1;
        int rangeStart = -1;
        int element;
        for( int i = 1; i <= elementCount; i++ )
        {
            element = Fieldml_GetElementEntry( handle, object, i );
            if( element != lastElement + 1 )
            {
                if( rangeStart != -1 )
                {
                    xmlTextWriterWriteFormatString( writer, "%d-%d ", rangeStart, lastElement );
                    rangeStart = -1;
                }
                else if( lastElement != -1 )
                {
                    xmlTextWriterWriteFormatString( writer, "%d ", lastElement );
                }
            }
            else if( rangeStart == -1 )
            {
                rangeStart = lastElement;
            }
            
            lastElement = element;
        }
        
        if( rangeStart == -1 )
        {
            xmlTextWriterWriteFormatString( writer, "%d", lastElement );
        }
        else
        {
            xmlTextWriterWriteFormatString( writer, "%d-%d", rangeStart, lastElement );
        }
    }

    xmlTextWriterEndElement( writer );
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeAbstractEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, ABSTRACT_EVALUATOR_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    writeVariables( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeExternalEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, EXTERNAL_EVALUATOR_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    writeVariables( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeReferenceEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, REFERENCE_EVALUATOR_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, EVALUATOR_ATTRIB, handle, Fieldml_GetReferenceSourceEvaluator( handle, object ) );

    writeVariables( writer, handle, object );
    
    writeBinds( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writePiecewiseEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, PIECEWISE_EVALUATOR_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    writeVariables( writer, handle, object );
    
    writeBinds( writer, handle, object );
    
    int count = Fieldml_GetEvaluatorCount( handle, object );
    int defaultEvaluator = Fieldml_GetDefaultEvaluator( handle, object );
    if( ( count > 0 ) || ( defaultEvaluator != FML_INVALID_HANDLE ) )
    {
        xmlTextWriterStartElement( writer, ELEMENT_EVALUATORS_TAG );
        
        if( defaultEvaluator != FML_INVALID_HANDLE )
        {
            xmlTextWriterWriteFormatAttribute( writer, DEFAULT_ATTRIB, "%s", Fieldml_GetObjectName( handle, defaultEvaluator ) );
        }
    
        for( int i = 1; i <= count; i++ )
        {
            int element = Fieldml_GetEvaluatorElement( handle, object, i );
            int evaluator = Fieldml_GetEvaluator( handle, object, i );
            if( ( element <= 0 ) || ( evaluator == FML_INVALID_HANDLE ) )
            {
                continue;
            }
            writeComponentEvaluator( writer, ELEMENT_EVALUATOR_TAG, element, Fieldml_GetObjectName( handle, evaluator ) );
        }

        xmlTextWriterEndElement( writer );
    }

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static void writeSemidenseIndexes( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object, const int isSparse )
{
    int count = Fieldml_GetSemidenseIndexCount( handle, object, isSparse );
    if( count > 0 )
    {
        if( isSparse )
        {
            xmlTextWriterStartElement( writer, SPARSE_INDEXES_TAG );
        }
        else
        {
            xmlTextWriterStartElement( writer, DENSE_INDEXES_TAG );
        }

        for( int i = 1; i <= count; i++ )
        {
            FmlObjectHandle index = Fieldml_GetSemidenseIndexEvaluator( handle, object, i, isSparse );
            if( index == FML_INVALID_HANDLE )
            {
                continue;
            }
            xmlTextWriterStartElement( writer, INDEX_EVALUATOR_TAG );
            xmlTextWriterWriteAttribute( writer, EVALUATOR_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, index ) );

#if 0
            FmlObjectHandle set = Fieldml_GetSemidenseIndexSet( handle, object, i );
            if( set != FML_INVALID_HANDLE )
            {
                xmlTextWriterWriteAttribute( writer, ELEMENT_SEQUENCE_ATTRIB, (const xmlChar*)Fieldml_GetObjectName( handle, set ) );
            }
#endif //NYI
            xmlTextWriterEndElement( writer );
        }

        xmlTextWriterEndElement( writer );
    }
}

static void writeSemidenseData( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, SEMI_DENSE_DATA_TAG );
    
    writeSemidenseIndexes( writer, handle, object, 1 );
    writeSemidenseIndexes( writer, handle, object, 0 );

    int swizzleCount = Fieldml_GetSwizzleCount( handle, object );
    if( swizzleCount > 0 )
    {
        writeIntArray( writer, SWIZZLE_TAG, swizzleCount, Fieldml_GetSwizzleData( handle, object ) );
    }
    
    xmlTextWriterStartElement( writer, DATA_LOCATION_TAG );
    
    DataLocationType location = Fieldml_GetParameterDataLocation( handle, object );
    if( location == LOCATION_FILE )
    {
        xmlTextWriterStartElement( writer, FILE_DATA_TAG );
        xmlTextWriterWriteAttribute( writer, FILE_ATTRIB, (const xmlChar*)Fieldml_GetParameterDataFilename( handle, object ) );
        
        DataFileType fileType = Fieldml_GetParameterDataFileType( handle, object );
        if( fileType == TYPE_TEXT )
        {
            xmlTextWriterWriteAttribute( writer, TYPE_ATTRIB, (const xmlChar*)STRING_TYPE_TEXT );
        }
        else if( fileType == TYPE_LINES )
        {
            xmlTextWriterWriteAttribute( writer, TYPE_ATTRIB, (const xmlChar*)STRING_TYPE_LINES );
        }

        int offset = Fieldml_GetParameterDataOffset( handle, object );
        xmlTextWriterWriteFormatAttribute( writer, OFFSET_ATTRIB, "%d", offset );
        xmlTextWriterEndElement( writer );
        
    }
    else if( location == LOCATION_INLINE )
    {
        xmlTextWriterStartElement( writer, INLINE_DATA_TAG );
        xmlTextWriterWriteString( writer, (const xmlChar*)Fieldml_GetParameterInlineData( handle, object ) );
        xmlTextWriterEndElement( writer );
    }
    
    xmlTextWriterEndElement( writer );

    xmlTextWriterEndElement( writer );
}


static int writeParameterEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    if( Fieldml_GetObjectType( handle, object ) != FHT_PARAMETER_EVALUATOR )
    {
        return 1;
    }

    xmlTextWriterStartElement( writer, PARAMETER_EVALUATOR_TAG );

    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    writeVariables( writer, handle, object );

    DataDescriptionType description = Fieldml_GetParameterDataDescription( handle, object );
    if( description == DESCRIPTION_SEMIDENSE )
    {
        writeSemidenseData( writer, handle, object );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeAggregateEvaluator( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, AGGREGATE_EVALUATOR_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_TYPE_ATTRIB, handle, Fieldml_GetValueType( handle, object ) );

    writeVariables( writer, handle, object );
    
    writeBinds( writer, handle, object );
    
    int count = Fieldml_GetEvaluatorCount( handle, object );
    FmlObjectHandle defaultEvaluator = Fieldml_GetDefaultEvaluator( handle, object );
    
    if( ( count > 0 ) || ( defaultEvaluator != FML_INVALID_HANDLE ) )
    {
        xmlTextWriterStartElement( writer, COMPONENT_EVALUATORS_TAG );
        
        if( defaultEvaluator != FML_INVALID_HANDLE )
        {
            xmlTextWriterWriteFormatAttribute( writer, DEFAULT_ATTRIB, "%s", Fieldml_GetObjectName( handle, defaultEvaluator ) );
        }
    
        for( int i = 1; i <= count; i++ )
        {
            int element = Fieldml_GetEvaluatorElement( handle, object, i );
            int evaluator = Fieldml_GetEvaluator( handle, object, i );
            if( ( element <= 0 ) || ( evaluator == FML_INVALID_HANDLE ) )
            {
                continue;
            }
            writeComponentEvaluator( writer, COMPONENT_EVALUATOR_TAG, element, Fieldml_GetObjectName( handle, evaluator ) );
        }

        xmlTextWriterEndElement( writer );
    }
    
    xmlTextWriterEndElement( writer );

    return 0;
}


static int writeFieldmlObject( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    switch( Fieldml_GetObjectType( handle, object ) )
    {
    case FHT_CONTINUOUS_TYPE:
        return writeContinuousType( writer, handle, object );
    case FHT_ENSEMBLE_TYPE:
        return writeEnsembleType( writer, handle, object );
    case FHT_MESH_TYPE:
        return writeMeshType( writer, handle, object );
//NYI    case FHT_ELEMENT_SEQUENCE:
//        return writeElementSequence( writer, handle, object );
    case FHT_ABSTRACT_EVALUATOR:
        return writeAbstractEvaluator( writer, handle, object );
    case FHT_EXTERNAL_EVALUATOR:
        return writeExternalEvaluator( writer, handle, object );
    case FHT_REFERENCE_EVALUATOR:
        return writeReferenceEvaluator( writer, handle, object );
    case FHT_PIECEWISE_EVALUATOR:
        return writePiecewiseEvaluator( writer, handle, object );
    case FHT_PARAMETER_EVALUATOR:
        return writeParameterEvaluator( writer, handle, object );
    case FHT_AGGREGATE_EVALUATOR:
        return writeAggregateEvaluator( writer, handle, object );
    default:
        break;
    }
    
    return 0;
}


int writeFieldmlFile( FmlHandle handle, const char *filename )
{
    FmlObjectHandle object;
    int i, count, length;
    int rc = 0;
    char tBuffer[tBufferLength];
    xmlTextWriterPtr writer;

    writer = xmlNewTextWriterFilename( filename, 0 );
    if( writer == NULL )
    {
        printf( "testXmlwriterFilename: Error creating the xml writer\n" );
        return 1;
    }

    xmlTextWriterSetIndent( writer, 1 );
    xmlTextWriterStartDocument( writer, NULL, MY_ENCODING, NULL );

    xmlTextWriterStartElement( writer, FIELDML_TAG );
    xmlTextWriterWriteAttribute( writer, VERSION_ATTRIB, (const xmlChar*)FML_VERSION_STRING );
    xmlTextWriterWriteAttribute( writer, (const xmlChar*)"xsi:noNamespaceSchemaLocation", (const xmlChar*)"Fieldml_0.3.xsd" );
    xmlTextWriterWriteAttribute( writer, (const xmlChar*)"xmlns:xsi", (const xmlChar*)"http://www.w3.org/2001/XMLSchema-instance" );        
    xmlTextWriterStartElement( writer, REGION_TAG );
    
    const char *regionName = Fieldml_GetRegionName( handle );
    if( ( regionName != NULL ) && ( strlen( regionName ) > 0 ) ) 
    {
        xmlTextWriterWriteAttribute( writer, NAME_ATTRIB, (const xmlChar*)regionName );        
    }
    
    
    int importCount = Fieldml_GetImportSourceCount( handle );
    for( int importSourceIndex = 1; importSourceIndex <= importCount; importSourceIndex++ )
    {
        int objectCount = Fieldml_GetImportCount( handle, importSourceIndex );
        if( objectCount <= 0 )
        {
            continue;
        }

        xmlTextWriterStartElement( writer, IMPORT_TAG );
        
        length = Fieldml_CopyImportSourceLocation( handle, importSourceIndex, tBuffer, tBufferLength );
        if( length > 0 )
        {
            xmlTextWriterWriteAttribute( writer, LOCATION_ATTRIB, (const xmlChar*)tBuffer );        
        }
        
        length = Fieldml_CopyImportSourceRegionName( handle, importSourceIndex, tBuffer, tBufferLength );
        if( length > 0 )
        {
            xmlTextWriterWriteAttribute( writer, REGION_ATTRIB, (const xmlChar*)tBuffer );        
        }
        
        for( int importIndex = 1; importIndex <= objectCount; importIndex++ )
        {
            FmlObjectHandle objectHandle = Fieldml_GetImportObject( handle, importSourceIndex, importIndex );
            if( objectHandle == FML_INVALID_HANDLE )
            {
                continue;
            }
            FieldmlHandleType objectType = Fieldml_GetObjectType( handle, objectHandle );
            if( ( objectType == FHT_CONTINUOUS_TYPE ) ||
                ( objectType == FHT_ENSEMBLE_TYPE ) ||
                ( objectType == FHT_MESH_TYPE ) )
            {
                xmlTextWriterStartElement( writer, IMPORT_TYPE_TAG );
            }
            else
            {
                xmlTextWriterStartElement( writer, IMPORT_EVALUATOR_TAG );
            }
            
            
            length = Fieldml_CopyImportLocalName( handle, importSourceIndex, importIndex, tBuffer, tBufferLength );
            if( length > 0 )
            {
                xmlTextWriterWriteAttribute( writer, LOCAL_NAME_ATTRIB, (const xmlChar*)tBuffer );
            }

            length = Fieldml_CopyImportSourceName( handle, importSourceIndex, importIndex, tBuffer, tBufferLength );
            if( length > 0 )
            {
                xmlTextWriterWriteAttribute( writer, SOURCE_NAME_ATTRIB, (const xmlChar*)tBuffer );
            }

            xmlTextWriterEndElement( writer );
        }
        
        xmlTextWriterEndElement( writer );
    }
    
    count = Fieldml_GetTotalObjectCount( handle );
    for( i = 1; i <= count; i++ )
    {
        object = Fieldml_GetObjectByIndex( handle, i );
        if( Fieldml_IsObjectLocal( handle, object ) )
        {
            writeFieldmlObject( writer, handle, object );
        }
        else
        {
            //Not a local object. Don't serialize it.
        }
    }

    rc = xmlTextWriterEndDocument( writer );
    if( rc < 0 )
    {
        printf( "testXmlwriterFilename: Error at xmlTextWriterEndDocument\n" );
        return 1;
    }

    xmlFreeTextWriter( writer );

    return 0;
}
