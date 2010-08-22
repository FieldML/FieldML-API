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


static void writeStringTableEntry( xmlTextWriterPtr writer, const char *key, const char *value )
{
    xmlTextWriterStartElement( writer, MAP_ENTRY_TAG );
    xmlTextWriterWriteAttribute( writer, KEY_ATTRIB, (const xmlChar*)key );
    xmlTextWriterWriteAttribute( writer, VALUE_ATTRIB, (const xmlChar*)value );
    xmlTextWriterEndElement( writer );
}


static void writeListEntry( xmlTextWriterPtr writer, const char *value )
{
    xmlTextWriterStartElement( writer, ENTRY_TAG );
    xmlTextWriterWriteAttribute( writer, VALUE_ATTRIB, (const xmlChar*)value );
    xmlTextWriterEndElement( writer );
}


static void writeIntTableEntry( xmlTextWriterPtr writer, int key, const char *value )
{
    xmlTextWriterStartElement( writer, MAP_ENTRY_TAG );
    xmlTextWriterWriteFormatAttribute( writer, KEY_ATTRIB, "%d", key );
    xmlTextWriterWriteAttribute( writer, VALUE_ATTRIB, (const xmlChar*) value );
    xmlTextWriterEndElement( writer );
}


static int writeMarkup( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    int i, count;
    
    count = Fieldml_GetMarkupCount( handle, object );
    if( count <= 0 )
    {
        return 0;
    }

    xmlTextWriterStartElement( writer, MARKUP_TAG );

    for( i = 1; i <= count; i++ )
    {
        const char *attribute = Fieldml_GetMarkupAttribute( handle, object, i );
        const char *value = Fieldml_GetMarkupValue( handle, object, i );
        if( ( attribute == NULL ) || ( value == NULL ) )
        {
            continue;
        }
        
        writeStringTableEntry( writer, attribute, value );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeAliases( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    int count = Fieldml_GetAliasCount( handle, object );
    if( count == 0 )
    {
        return 0;
    }

    xmlTextWriterStartElement( writer, ALIASES_TAG );
    
    for( int i = 1; i <= count; i++ )
    {
        int remote = Fieldml_GetAliasRemote( handle, object, i );
        int local =  Fieldml_GetAliasLocal( handle, object, i );
        if( ( remote == FML_INVALID_HANDLE ) || ( local == FML_INVALID_HANDLE ) )
        {
            continue;
        }
        
        writeStringTableEntry( writer,
            Fieldml_GetObjectName( handle, remote ),
            Fieldml_GetObjectName( handle, local )
            );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeContinuousDomain( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, CONTINUOUS_DOMAIN_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );

    FmlObjectHandle componentDomain = Fieldml_GetDomainComponentEnsemble( handle, object );
    if( componentDomain != FML_INVALID_HANDLE )
    {
        writeObjectName( writer, COMPONENT_DOMAIN_ATTRIB, handle, componentDomain );
    }

    writeMarkup( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static void writeBounds( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    DomainBoundsType boundsType = Fieldml_GetDomainBoundsType( handle, object );
    
    if( boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
    {
        xmlTextWriterStartElement( writer, BOUNDS_TAG );
        xmlTextWriterStartElement( writer, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG );
        xmlTextWriterWriteFormatAttribute( writer, VALUE_COUNT_ATTRIB, "%d", Fieldml_GetContiguousBoundsCount( handle, object ) );
        xmlTextWriterEndElement( writer );
        xmlTextWriterEndElement( writer );
    }
}


static int writeEnsembleDomain( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, ENSEMBLE_DOMAIN_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    if( Fieldml_IsEnsembleComponentDomain( handle, object ) == 1 )
    {
        xmlTextWriterWriteAttribute( writer, IS_COMPONENT_DOMAIN_ATTRIB, (const xmlChar*)STRING_TRUE );
    }
    
    FmlObjectHandle componentDomain = Fieldml_GetDomainComponentEnsemble( handle, object );
    if( componentDomain != FML_INVALID_HANDLE )
    {
        writeObjectName( writer, COMPONENT_DOMAIN_ATTRIB, handle, componentDomain );
    }
    
    writeMarkup( writer, handle, object );
        
    writeBounds( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeMeshDomain( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, MESH_DOMAIN_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    
    FmlObjectHandle xiDomain = Fieldml_GetMeshXiDomain( handle, object );
    FmlObjectHandle xiComponents = Fieldml_GetDomainComponentEnsemble( handle, xiDomain );
    if( xiComponents != FML_INVALID_HANDLE )
    {
        writeObjectName( writer, XI_COMPONENT_DOMAIN_ATTRIB, handle, xiComponents );
    }
    
    writeMarkup( writer, handle, object );
    
    FmlObjectHandle elementDomain = Fieldml_GetMeshElementDomain( handle, object );
    int elementCount = Fieldml_GetEnsembleDomainElementCount( handle, elementDomain );

    writeBounds( writer, handle, Fieldml_GetMeshElementDomain( handle, object ) );

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
        writeIntTableEntry( writer, i, shape );
    }
    xmlTextWriterEndElement( writer );

    int count = Fieldml_GetMeshConnectivityCount( handle, object );
    if( count > 0 )
    {
        xmlTextWriterStartElement( writer, MESH_CONNECTIVITY_TAG );

        for( int i = 1; i <= count; i++ )
        {
            FmlObjectHandle source = Fieldml_GetMeshConnectivitySource( handle, object, i );
            FmlObjectHandle domain = Fieldml_GetMeshConnectivityDomain( handle, object, i );
            
            if( ( source == FML_INVALID_HANDLE ) || ( domain == FML_INVALID_HANDLE ) )
            {
                continue;
            }
            
            writeStringTableEntry( writer, Fieldml_GetObjectName( handle, source ), Fieldml_GetObjectName( handle, domain ) );
        }
        xmlTextWriterEndElement( writer );
    }

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeVariable( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    if( Fieldml_GetObjectType( handle, object ) == FHT_CONTINUOUS_VARIABLE )
    {
        xmlTextWriterStartElement( writer, CONTINUOUS_VARIABLE_TAG );
    }
    else if( Fieldml_GetObjectType( handle, object ) == FHT_ENSEMBLE_VARIABLE )
    {
        xmlTextWriterStartElement( writer, ENSEMBLE_VARIABLE_TAG );
    }
    else
    {
        return 1;
    }
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_DOMAIN_ATTRIB, handle, Fieldml_GetValueDomain( handle, object ) );

    writeMarkup( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeContinuousReference( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, CONTINUOUS_REFERENCE_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, EVALUATOR_ATTRIB, handle, Fieldml_GetReferenceRemoteEvaluator( handle, object ) );
    writeObjectName( writer, VALUE_DOMAIN_ATTRIB, handle, Fieldml_GetValueDomain( handle, object ) );

    writeMarkup( writer, handle, object );
    
    writeAliases( writer, handle, object );

    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeContinuousPiecewise( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, CONTINUOUS_PIECEWISE_TAG );
    
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, INDEX_DOMAIN_ATTRIB, handle, Fieldml_GetIndexDomain( handle, object, 1 ) );
    writeObjectName( writer, VALUE_DOMAIN_ATTRIB, handle, Fieldml_GetValueDomain( handle, object ) );

    writeMarkup( writer, handle, object );
    
    writeAliases( writer, handle, object );
    
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
            writeIntTableEntry( writer, element, Fieldml_GetObjectName( handle, object ) );
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
            FmlObjectHandle index = Fieldml_GetSemidenseIndex( handle, object, i, isSparse );
            if( index == FML_INVALID_HANDLE )
            {
                continue;
            }
            writeListEntry( writer, Fieldml_GetObjectName( handle, index ) );
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


static int writeParameters( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    if( Fieldml_GetObjectType( handle, object ) == FHT_CONTINUOUS_PARAMETERS )
    {
        xmlTextWriterStartElement( writer, CONTINUOUS_PARAMETERS_TAG );
    }
    else if( Fieldml_GetObjectType( handle, object ) == FHT_ENSEMBLE_PARAMETERS )
    {
        xmlTextWriterStartElement( writer, ENSEMBLE_PARAMETERS_TAG );
    }
    else
    {
        return 1;
    }

    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_DOMAIN_ATTRIB, handle, Fieldml_GetValueDomain( handle, object ) );

    writeMarkup( writer, handle, object );
    
    DataDescriptionType description = Fieldml_GetParameterDataDescription( handle, object );
    if( description == DESCRIPTION_SEMIDENSE )
    {
        writeSemidenseData( writer, handle, object );
    }
    
    xmlTextWriterEndElement( writer );
    
    return 0;
}


static int writeAggregate( xmlTextWriterPtr writer, FmlHandle handle, FmlObjectHandle object )
{
    xmlTextWriterStartElement( writer, CONTINUOUS_AGGREGATE_TAG );
    writeObjectName( writer, NAME_ATTRIB, handle, object );
    writeObjectName( writer, VALUE_DOMAIN_ATTRIB, handle, Fieldml_GetValueDomain( handle, object ) );

    writeMarkup( writer, handle, object );
    
    writeAliases( writer, handle, object );
    
    int count = Fieldml_GetEvaluatorCount( handle, object );
    
    if( count > 0 )
    {
        xmlTextWriterStartElement( writer, SOURCE_FIELDS_TAG );
    
        for( int i = 1; i <= count; i++ )
        {
            int element = Fieldml_GetEvaluatorElement( handle, object, i );
            int evaluator = Fieldml_GetEvaluator( handle, object, i );
            if( ( element <= 0 ) || ( evaluator == FML_INVALID_HANDLE ) )
            {
                continue;
            }
            writeIntTableEntry( writer, element, Fieldml_GetObjectName( handle, evaluator ) );
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
    case FHT_CONTINUOUS_DOMAIN:
        return writeContinuousDomain( writer, handle, object );
    case FHT_ENSEMBLE_DOMAIN:
        return writeEnsembleDomain( writer, handle, object );
    case FHT_MESH_DOMAIN:
        return writeMeshDomain( writer, handle, object );
    case FHT_CONTINUOUS_VARIABLE:
    case FHT_ENSEMBLE_VARIABLE:
        return writeVariable( writer, handle, object );
    case FHT_CONTINUOUS_REFERENCE:
        return writeContinuousReference( writer, handle, object );
    case FHT_CONTINUOUS_PIECEWISE:
        return writeContinuousPiecewise( writer, handle, object );
    case FHT_CONTINUOUS_PARAMETERS:
    case FHT_ENSEMBLE_PARAMETERS:
        return writeParameters( writer, handle, object );
    case FHT_CONTINUOUS_AGGREGATE:
        return writeAggregate( writer, handle, object );
    default:
        break;
    }
    
    return 0;
}


int writeFieldmlFile( FmlHandle handle, const char *filename )
{
    FmlObjectHandle object;
    int i, count;
    int rc = 0;
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
    xmlTextWriterWriteAttribute( writer, (const xmlChar*)"xsi:noNamespaceSchemaLocation", (const xmlChar*)"Fieldml_0.2.xsd" );
    xmlTextWriterWriteAttribute( writer, (const xmlChar*)"xmlns:xsi", (const xmlChar*)"http://www.w3.org/2001/XMLSchema-instance" );        
    xmlTextWriterStartElement( writer, REGION_TAG );
    
    const char *regionName = Fieldml_GetName( handle );
    if( ( regionName != NULL ) && ( strlen( regionName ) > 0 ) ) 
    {
        xmlTextWriterWriteAttribute( writer, NAME_TAG, (const xmlChar*)regionName );        
    }
    
    count = Fieldml_GetTotalObjectCount( handle );
    for( i = 1; i <= count; i++ )
    {
        object = Fieldml_GetObjectByIndex( handle, i );
        if( Fieldml_GetRegion( handle, object ) == FILE_REGION_HANDLE )
        {
            writeFieldmlObject( writer, handle, object );
        }
        else
        {
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
