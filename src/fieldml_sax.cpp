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

#include <libxml/SAX.h>
#include <libxml/globals.h>
#include <libxml/xmlerror.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlschemas.h>

#include <stack>
#include <iostream>

#include "fieldml_sax.h"
#include "string_const.h"
#include "fieldml_structs.h"
#include "fieldml_api.h"

using namespace std;

//========================================================================
//
// Structs
//
//========================================================================

enum SaxState
{
    FML_ROOT,
    FML_FIELDML,
    FML_REGION,

    FML_ENSEMBLE_DOMAIN,
    FML_ENSEMBLE_DOMAIN_BOUNDS,
    
    FML_CONTINUOUS_DOMAIN,

    FML_MESH_DOMAIN,
    FML_MESH_SHAPES,
    FML_MESH_CONNECTIVITY,
    
    FML_CONTINUOUS_REFERENCE,
    FML_ALIASES,

    FML_ENSEMBLE_PARAMETERS,
    FML_CONTINUOUS_PARAMETERS,

    FML_CONTINUOUS_VARIABLE,

    FML_ENSEMBLE_VARIABLE,

    FML_SEMI_DENSE,
    FML_DENSE_INDEXES,
    FML_SPARSE_INDEXES,
    FML_INLINE_DATA,
    FML_SWIZZLE_DATA,
    FML_FILE_DATA,
    
    FML_CONTINUOUS_PIECEWISE,
    FML_ELEMENT_EVALUATORS,
    
    FML_CONTINUOUS_AGGREGATE,
    FML_SOURCE_FIELDS,
    
    FML_MARKUP,
};

class SaxAttribute
{
public:
    const xmlChar *attribute;
    const char *prefix;
    const char *URI;
    const char *value;
};


class SaxAttributes
{
public:
    SaxAttributes( const int attributeCount, const xmlChar ** attributes );
    ~SaxAttributes();
    
    const char *getAttribute( const xmlChar *name );
    
private:
    SaxAttribute *attributes;
    int count;
};


struct SaxContext
{
    stack<SaxState> state;

    FmlObjectHandle currentObject;
    
    int bufferLength;
    char *buffer;
    string source;
    
    FieldmlRegion *region;
};


//========================================================================
//
// Utils
//
//========================================================================


SaxAttributes::SaxAttributes( const int attributeCount, const xmlChar ** rawXmlAttributes )
{
    int i, attributeNumber;
    
    const char ** rawAttributes = (const char**)(rawXmlAttributes);

    count = attributeCount;
    attributes = new SaxAttribute[attributeCount];
    
    attributeNumber = 0;
    for( i = 0; i < attributeCount * 5; i += 5 )
    {
        attributes[attributeNumber].attribute = (const xmlChar*)strdupS( rawAttributes[i + 0] );
        attributes[attributeNumber].prefix = strdupS( rawAttributes[i + 1] );
        attributes[attributeNumber].URI = strdupS( rawAttributes[i + 2] );
        attributes[attributeNumber].value = strdupN( rawAttributes[i + 3], rawAttributes[i + 4] - rawAttributes[i + 3] );
        attributeNumber++;
    }
}


SaxAttributes::~SaxAttributes()
{
    int i;

    for( i = 0; i < count; i++ )
    {
        free( (void*)attributes[i].attribute );
        free( (void*)attributes[i].prefix );
        free( (void*)attributes[i].URI );
        free( (void*)attributes[i].value );
    }
    delete[] attributes;
}


const char * SaxAttributes::getAttribute( const xmlChar *attribute )
{
    int i;
    for( i = 0; i < count; i++ )
    {
        if( xmlStrcmp( attribute, attributes[i].attribute ) == 0 )
        {
            return attributes[i].value;
        }
    }

    return NULL;
}


static int intParserCount( const char *buffer )
{
    const char *p = buffer;
    int digits = 0;
    int count = 0;
    char c;
    
    while( *p != 0 )
    {
        c = *p++;
        if( isdigit( c ) )
        {
            digits++;
            continue;
        }
        
        if( digits > 0 )
        {
            count++;
            digits = 0;
        }
    }
    
    return count;
}


static const int *intParserInts( const char *buffer )
{
    int count = intParserCount( buffer );
    int *ints = new int[count];
    const char *p = buffer;
    int number = 0;
    int digits;
    char c;
    
    digits = 0;
    count = 0;
    while( *p != 0 )
    {
        c = *p++;
        if( isdigit( c ) )
        {
            number *= 10;
            number += ( c - '0' );
            digits++;
            continue;
        }
        
        if( digits > 0 )
        {
            ints[count++] = number;
            number = 0;
            digits = 0;
        }
    }
    
    return ints;
}


//========================================================================
//
// SAX -> FieldmlRegion glue
//
//========================================================================


static FmlObjectHandle getOrCreateObjectHandle( FieldmlRegion *region, const char *name, FieldmlHandleType type )
{
    FmlObjectHandle handle = Fieldml_GetNamedObject( region, name );

    if( handle == FML_INVALID_HANDLE )
    {
        handle = region->addObject( new FieldmlObject( name, VIRTUAL_REGION_HANDLE, type ) );
    }
    
    return handle;
}


static void startRegion( SaxContext *context, SaxAttributes &attributes )
{
    string location;
    const char *name;
    
    name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        //NOTE Allow nameless regions for now.
        name = "";
    }

    location = getDirectory( context->source );
    context->region = Fieldml_Create( location.c_str(), name );
}


static void startEnsembleDomain( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *componentEnsemble;
    const char *isComponentEnsemble;
    FmlObjectHandle handle;
        
    name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        context->region->logError( "EnsembleDomain has no name" );
        return;
    }
    
    componentEnsemble = attributes.getAttribute( COMPONENT_DOMAIN_ATTRIB );
    if( componentEnsemble != NULL )
    {
        handle = getOrCreateObjectHandle( context->region, componentEnsemble, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    }
    else
    {
        handle = FML_INVALID_HANDLE;
    }
    
    isComponentEnsemble = attributes.getAttribute( IS_COMPONENT_DOMAIN_ATTRIB );
    if( ( isComponentEnsemble != NULL ) && ( strcmp( isComponentEnsemble, STRING_TRUE ) == 0 ) )
    {
        if( handle != FML_INVALID_HANDLE )
        {
            context->region->logError( "Component EnsembleDomain cannot be multi-component itself", name );
        }
        else
        {
            context->currentObject = Fieldml_CreateComponentEnsembleDomain( context->region, name );
        }
    }
    else
    {
        context->currentObject = Fieldml_CreateEnsembleDomain( context->region, name, handle );
    }
}


static void endEnsembleDomain( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startContiguousBounds( SaxContext *context, SaxAttributes &attributes )
{
    const char *count;
    
    count = attributes.getAttribute( VALUE_COUNT_ATTRIB );
    if( count == NULL )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Contiguous bounds has no value count", name );
        return;
    }
    
    Fieldml_SetContiguousBoundsCount( context->region, context->currentObject, atoi( count ) );
}


static void startContinuousDomain( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *componentEnsemble;
    FmlObjectHandle handle;
        
    name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        context->region->logError( "ContinuousDomain has no name", name );
        return;
    }
    
    componentEnsemble = attributes.getAttribute( COMPONENT_DOMAIN_ATTRIB );
    if( componentEnsemble != NULL )
    {
        handle = getOrCreateObjectHandle( context->region, componentEnsemble, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    }
    else
    {
        handle = FML_INVALID_HANDLE;
    }

    context->currentObject = Fieldml_CreateContinuousDomain( context->region, name, handle );
}


static void endContinuousDomain( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startMeshDomain( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *xiEnsemble;
    FmlObjectHandle xiHandle;
    char *subName;
    
    name = attributes.getAttribute( NAME_ATTRIB );
    xiEnsemble = attributes.getAttribute( XI_COMPONENT_DOMAIN_ATTRIB );
    
    if( name == NULL )
    {
        context->region->logError( "MeshDomain has no name" );
        return;
    }
    
    if( xiEnsemble == NULL )
    {
        context->region->logError( "MeshDomain has no xi components", name );
        return;
    }

    subName = (char *)calloc( 1, strlen( name ) + 12 );
    
    xiHandle = getOrCreateObjectHandle( context->region, xiEnsemble, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    
    context->currentObject = Fieldml_CreateMeshDomain( context->region, name, xiHandle );
}


static void startMeshShapes( SaxContext *context, SaxAttributes &attributes )
{
    const char *defaultValue = attributes.getAttribute( DEFAULT_ATTRIB );

    if( defaultValue != NULL )
    {
        Fieldml_SetMeshDefaultShape( context->region, context->currentObject, defaultValue );
    }
}


static void onMeshShape( SaxContext *context, SaxAttributes &attributes )
{
    const char *element = attributes.getAttribute( KEY_ATTRIB );
    const char *shape = attributes.getAttribute( VALUE_ATTRIB );

    if( ( element == NULL ) || ( shape == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "MeshDomain has malformed shape entry", name );
        return;
    }
    
    Fieldml_SetMeshElementShape( context->region, context->currentObject, atoi( element ), shape );
}


static void onMeshConnectivity( SaxContext *context, SaxAttributes &attributes )
{
    const char *field = attributes.getAttribute( KEY_ATTRIB );
    const char *type = attributes.getAttribute( VALUE_ATTRIB );
    FmlObjectHandle fieldHandle, domainHandle;

    if( ( type == NULL ) || ( field == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "MeshDomain has malformed connectivity entry", name );
        return;
    }
    
    domainHandle = getOrCreateObjectHandle( context->region, type, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    fieldHandle = getOrCreateObjectHandle( context->region, field, FHT_UNKNOWN_ENSEMBLE_SOURCE );
    
    Fieldml_SetMeshConnectivity( context->region, context->currentObject, fieldHandle, domainHandle );
}


static void endMeshDomain( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startContinuousReference( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *remoteName;
    const char *valueDomain;
    FmlObjectHandle handle, remoteHandle;
        
    name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        context->region->logError( "ContinuousReferenceEvaluator has no name" );
        return;
    }
    
    remoteName = attributes.getAttribute( EVALUATOR_ATTRIB );
    if( remoteName == NULL )
    {
        context->region->logError( "ContinuousReferenceEvaluator has no remote name", name );
        return;
    }
    
    valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    if( valueDomain == NULL )
    {
        context->region->logError( "ContinuousReferenceEvaluator has no value domain", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    remoteHandle = getOrCreateObjectHandle( context->region, remoteName, FHT_UNKNOWN_CONTINUOUS_EVALUATOR );

    context->currentObject = Fieldml_CreateContinuousReference( context->region, name, remoteHandle, handle );
}


static void onAlias( SaxContext *context, SaxAttributes &attributes )
{
    const char *remote = attributes.getAttribute( KEY_ATTRIB );
    const char *local = attributes.getAttribute( VALUE_ATTRIB );
    FmlObjectHandle localHandle, remoteHandle;

    if( ( remote == NULL ) || ( local == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Evaluator has malformed alias", name );
        return;
    }

    localHandle = getOrCreateObjectHandle( context->region, local, FHT_UNKNOWN_CONTINUOUS_SOURCE );

    remoteHandle = getOrCreateObjectHandle( context->region, remote, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    
    Fieldml_SetAlias( context->region, context->currentObject, remoteHandle, localHandle );
}


static void endContinuousReference( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startEnsembleParameters( SaxContext *context, SaxAttributes &attributes )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    const char *valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    FmlObjectHandle handle;

    if( name == NULL )
    {
        context->region->logError( "EnsembleParameters has no name" );
        return;
    }

    if( valueDomain == NULL )
    {
        context->region->logError( "EnsembleParameters has no value domain", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_ENSEMBLE_DOMAIN );

    context->currentObject = Fieldml_CreateEnsembleParameters( context->region, name, handle );
}


static void endEnsembleParameters( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startContinuousParameters( SaxContext *context, SaxAttributes &attributes )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    const char *valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    FmlObjectHandle handle;

    if( name == NULL )
    {
        context->region->logError( "ContinuousParameters has no name" );
        return;
    }

    if( valueDomain == NULL )
    {
        context->region->logError( "ContinuousParameters has no value domain", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_CONTINUOUS_DOMAIN );

    context->currentObject = Fieldml_CreateContinuousParameters( context->region, name, handle );
}


static void endContinuousParameters( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startInlineData( SaxContext *context, SaxAttributes &attributes )
{
    Fieldml_SetParameterDataLocation( context->region, context->currentObject, LOCATION_INLINE );
}


static void onInlineData( SaxContext *context, const char *const characters, const int length )
{
    Fieldml_AddParameterInlineData( context->region, context->currentObject, characters, length );
}


static void onFileData( SaxContext *context, SaxAttributes &attributes )
{
    const char *file = attributes.getAttribute( FILE_ATTRIB );
    const char *type = attributes.getAttribute( TYPE_ATTRIB );
    const char *offset = attributes.getAttribute( OFFSET_ATTRIB );
    DataFileType fileType;
    int offsetAmount;
    
    if( file == NULL )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Parameters file data for must have a file name", name );
        return;
    }
    
    if( type == NULL )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Parameters file data for must have a file type", name );
        return;
    }
    else if( strcmp( type, STRING_TYPE_TEXT ) == 0 )
    {
        fileType = TYPE_TEXT;
    }
    else if( strcmp( type, STRING_TYPE_LINES ) == 0 )
    {
        fileType = TYPE_LINES;
    }
    else 
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Parameters file data for must have a known file type", name );
        return;
    }
    
    if( offset == NULL )
    {
        offsetAmount = 0;
    }
    else
    {
        offsetAmount = atoi( offset );
    }
    
    Fieldml_SetParameterDataLocation( context->region, context->currentObject, LOCATION_FILE );
    Fieldml_SetParameterFileData( context->region, context->currentObject, file, fileType, offsetAmount );
}


static void startSwizzleData( SaxContext *context, SaxAttributes &attributes )
{
}


static void onSwizzleData( SaxContext *context, const char *const characters, const int length )
{
    char *newString;
    
    newString = (char *)malloc( context->bufferLength + length + 1 );
    if( context->buffer != NULL )
    {
        memcpy( newString, context->buffer, context->bufferLength );
    }
    memcpy( newString + context->bufferLength, characters, length );
    context->bufferLength += length;
    newString[ context->bufferLength ] = 0;
    free( context->buffer );
    context->buffer = newString;
}


static void endSwizzleData( SaxContext *context )
{
    int intCount;
    const int *ints;
    
    intCount = intParserCount( context->buffer );
    ints = intParserInts( context->buffer );
    
    Fieldml_SetSwizzle( context->region, context->currentObject, ints, intCount );
    
    delete[] ints;
    free( context->buffer );
    context->buffer = NULL;
    context->bufferLength = 0;
}


static void startSemidenseData( SaxContext *context, SaxAttributes &attributes )
{
    Fieldml_SetParameterDataDescription( context->region, context->currentObject, DESCRIPTION_SEMIDENSE );
}


static void onSemidenseSparseIndex( SaxContext *context, SaxAttributes &attributes )
{
    const char *index;
    FmlObjectHandle handle;
    
    index = attributes.getAttribute( VALUE_ATTRIB );
    if( index == NULL )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Missing index in semi dense data", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, index, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    
    Fieldml_AddSemidenseIndex( context->region, context->currentObject, handle, 1 );
}


static void onSemidenseDenseIndex( SaxContext *context, SaxAttributes &attributes )
{
    const char *index;
    FmlObjectHandle handle;
    
    index = attributes.getAttribute( VALUE_ATTRIB );
    if( index == NULL )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Missing index in semi dense data", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, index, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    
    Fieldml_AddSemidenseIndex( context->region, context->currentObject, handle, 0 );
}


static void endSemidenseData( SaxContext *context )
{
}


static void startContinuousPiecewise( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *valueDomain;
    const char *indexDomain;
    FmlObjectHandle valueHandle, indexHandle;
    
    name = attributes.getAttribute( NAME_ATTRIB );
    valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    indexDomain = attributes.getAttribute( INDEX_DOMAIN_ATTRIB );
    
    if( name == NULL )
    {
        context->region->logError( "ContinuousPiecewise has no name" );
        return;
    }
    
    if( valueDomain == NULL )
    {
        context->region->logError( "ContinuousPiecewise has no value domain", name );
        return;
    }
    
    if( indexDomain == NULL )
    {
        context->region->logError( "ContinuousPiecewise has no index domain", name );
        return;
    }
    
    valueHandle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    indexHandle = getOrCreateObjectHandle( context->region, indexDomain, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    
    context->currentObject = Fieldml_CreateContinuousPiecewise( context->region, name, indexHandle, valueHandle );
}


static void onContinuousPiecewiseEvaluators( SaxContext *context, SaxAttributes &attributes )
{
    const char *defaultValue;
    
    defaultValue = attributes.getAttribute( DEFAULT_ATTRIB );
    if( defaultValue != NULL )
    {
        int defaultHandle = getOrCreateObjectHandle( context->region, defaultValue, FHT_UNKNOWN_CONTINUOUS_EVALUATOR );
        Fieldml_SetDefaultEvaluator( context->region, context->currentObject, defaultHandle );
    }
}

static void onContinuousPiecewiseEntry( SaxContext *context, SaxAttributes &attributes )
{
    const char *key;
    const char *value;
    FmlObjectHandle handle;
    
    key = attributes.getAttribute( KEY_ATTRIB );
    value = attributes.getAttribute( VALUE_ATTRIB );
    
    if( ( key == NULL ) || ( value == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Malformed element evaluator for ContinuousPiecewise", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, value, FHT_UNKNOWN_CONTINUOUS_SOURCE );
    
    Fieldml_SetEvaluator( context->region, context->currentObject, atoi( key ), handle );
}


static void endContinuousPiecewise( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startContinuousAggregate( SaxContext *context, SaxAttributes &attributes )
{
    const char *name;
    const char *valueDomain;
    FmlObjectHandle valueHandle;
    
    name = attributes.getAttribute( NAME_ATTRIB );
    valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    
    if( name == NULL )
    {
        context->region->logError( "ContinuousAggregate has no name" );
        return;
    }
    
    if( valueDomain == NULL )
    {
        context->region->logError( "ContinuousAggregate has no value domain", name );
        return;
    }
    
    valueHandle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    
    context->currentObject = Fieldml_CreateContinuousAggregate( context->region, name, valueHandle );
}


static void onContinuousAggregateEntry( SaxContext *context, SaxAttributes &attributes )
{
    const char *key;
    const char *value;
    FmlObjectHandle handle;
    
    key = attributes.getAttribute( KEY_ATTRIB );
    value = attributes.getAttribute( VALUE_ATTRIB );
    
    if( ( key == NULL ) || ( value == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Malformed element evaluator for ContinuousAggregate", name );
        return;
    }
    
    handle = getOrCreateObjectHandle( context->region, value, FHT_UNKNOWN_CONTINUOUS_SOURCE );
    
    Fieldml_SetEvaluator( context->region, context->currentObject, atoi( key ), handle );
}


static void endContinuousAggregate( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void startContinuousVariable( SaxContext *context, SaxAttributes &attributes )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    const char *valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    FmlObjectHandle valueHandle;

    if( name == NULL )
    {
        context->region->logError( "ContinuousVariable has no name" );
        return;
    }
    if( valueDomain == NULL )
    {
        context->region->logError( "ContinuousVariable has no value domain", name );
        return;
    }

    valueHandle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_CONTINUOUS_DOMAIN );

    context->currentObject = Fieldml_CreateContinuousVariable( context->region, name, valueHandle );
}


static void startEnsembleVariable( SaxContext *context, SaxAttributes &attributes )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    const char *valueDomain = attributes.getAttribute( VALUE_DOMAIN_ATTRIB );
    FmlObjectHandle valueHandle;
    
    if( name == NULL )
    {
        context->region->logError( "EnsembleVariable has no name" );
        return;
    }
    if( valueDomain == NULL )
    {
        context->region->logError( "EnsembleVariable has no value domain", name );
        return;
    }

    valueHandle = getOrCreateObjectHandle( context->region, valueDomain, FHT_UNKNOWN_ENSEMBLE_DOMAIN );

    context->currentObject = Fieldml_CreateEnsembleVariable( context->region, name, valueHandle );
}


static void endVariable( SaxContext *context )
{
    Fieldml_ValidateObject( context->region, context->currentObject );
    
    context->currentObject = FML_INVALID_HANDLE;
}


static void onMarkupEntry( SaxContext *context, SaxAttributes &attributes )
{
    const char *key;
    const char *value;
    
    key = attributes.getAttribute( KEY_ATTRIB );
    value = attributes.getAttribute( VALUE_ATTRIB );
    
    if( ( key == NULL ) || ( value == NULL ) )
    {
        const char * name =  Fieldml_GetObjectName( context->region, context->currentObject );
        context->region->logError( "Malformed markup", name );
        return;
    }
    
    Fieldml_SetMarkup( context->region, context->currentObject, key, value );
}

//========================================================================
//
// SAX handlers
//
//========================================================================

int isStandalone( void *context )
{
    return 0;
}


int hasInternalSubset( void *context )
{
    return 0;
}


int hasExternalSubset( void *context )
{
    return 0;
}


void onInternalSubset( void *context, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID )
{
}


void externalSubset( void *context, const xmlChar *name, const xmlChar *externalID, const xmlChar *systemID )
{
}


xmlParserInputPtr resolveEntity( void *context, const xmlChar *publicId, const xmlChar *systemId )
{
    return NULL;
}


xmlEntityPtr getEntity( void *context, const xmlChar *name )
{
    return NULL;
}


xmlEntityPtr getParameterEntity( void *context, const xmlChar *name )
{
    return NULL;
}


void onEntityDecl( void *context, const xmlChar *name, int type, const xmlChar *publicId, const xmlChar *systemId, xmlChar *content )
{
}


static void onAttributeDecl( void *context, const xmlChar * elem, const xmlChar * name, int type, int def, const xmlChar * defaultValue, xmlEnumerationPtr tree )
{
    xmlFreeEnumeration( tree );
}


static void onElementDecl( void *context, const xmlChar *name, int type, xmlElementContentPtr content )
{
}


static void onNotationDecl( void *context, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId )
{
}

static void onUnparsedEntityDecl( void *context, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId, const xmlChar *notationName )
{
}


void setDocumentLocator( void *context, xmlSAXLocatorPtr loc )
{
    /*
     At the moment (libxml 2.7.2), this is worse than useless.
     The locator only wraps functions which require the library's internal
     parsing context, which is only passed into this function if you pass
     in 'NULL' as the user-data which initiating the SAX parse...
     which is exactly what we don't want to do.
     */
}


static void onStartDocument( void *context )
{
}


static void onEndDocument( void *context )
{
}


static void onCharacters( void *context, const xmlChar *xmlChars, int len )
{
    SaxContext *saxContext = (SaxContext*)context;
    
    const char *chars = (const char*)(xmlChars);

    switch( saxContext->state.top() )
    {
    case FML_INLINE_DATA:
        onInlineData( saxContext, chars, len );
        break;
    case FML_SWIZZLE_DATA:
        onSwizzleData( saxContext, chars, len );
        break;
    default:
        break;
    }
}


static void onReference( void *context, const xmlChar *name )
{
}


static void onIgnorableWhitespace( void *context, const xmlChar *ch, int len )
{
}


static void onProcessingInstruction( void *context, const xmlChar *target, const xmlChar *data )
{
}


static void onCdataBlock( void *context, const xmlChar *value, int len )
{
}


void comment( void *context, const xmlChar *value )
{
}


static void XMLCDECL warning( void *context, const char *msg, ... )
{
    va_list args;

    va_start( args, msg );
    fprintf( stdout, "SAX.warning: " );
    vfprintf( stdout, msg, args );
    va_end( args );
}


static void XMLCDECL error( void *context, const char *msg, ... )
{
    va_list args;

    va_start( args, msg );
    fprintf( stdout, "SAX.error: " );
    vfprintf( stdout, msg, args );
    va_end( args );
}


static void XMLCDECL fatalError( void *context, const char *msg, ... )
{
    va_list args;

    va_start( args, msg );
    fprintf( stdout, "SAX.fatalError: " );
    vfprintf( stdout, msg, args );
    va_end( args );
}


static void onStartElementNs( void *context, const xmlChar *name, const xmlChar *prefix, const xmlChar *URI, int nb_namespaces, const xmlChar **namespaces,
    int nb_attributes, int nb_defaulted, const xmlChar **attributes )
{
    SaxAttributes saxAttributes = SaxAttributes( nb_attributes, attributes );
    SaxContext *saxContext = (SaxContext*)context;

    SaxState state;

    state = saxContext->state.top();

    if( ( state != FML_ROOT ) && ( state != FML_FIELDML ) && ( state != FML_REGION ) )
    {
        if( xmlStrcmp( name, MARKUP_TAG ) == 0 )
        {
            saxContext->state.push( FML_MARKUP );
            return;
        }
    }

    switch( state )
    {
    case FML_ROOT:
        if( xmlStrcmp( name, FIELDML_TAG ) == 0 )
        {
            saxContext->state.push( FML_FIELDML );
        }
        break;
    case FML_FIELDML:
        if( xmlStrcmp( name, REGION_TAG ) == 0 )
        {
            startRegion( saxContext, saxAttributes );
            saxContext->state.push( FML_REGION );
        }
        break;
    case FML_ENSEMBLE_DOMAIN:
        if( xmlStrcmp( name, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
        {
            startContiguousBounds( saxContext, saxAttributes );
            saxContext->state.push( FML_ENSEMBLE_DOMAIN_BOUNDS );
        }
        break;
    case FML_MESH_DOMAIN:
        if( xmlStrcmp( name, MESH_SHAPES_TAG ) == 0 )
        {
            startMeshShapes( saxContext, saxAttributes );
            saxContext->state.push( FML_MESH_SHAPES );
        }
        else if( xmlStrcmp( name, MESH_CONNECTIVITY_TAG ) == 0 )
        {
            saxContext->state.push( FML_MESH_CONNECTIVITY );
        }
        else if( xmlStrcmp( name, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
        {
            startContiguousBounds( saxContext, saxAttributes );
            saxContext->state.push( FML_ENSEMBLE_DOMAIN_BOUNDS );
        }
        break;
    case FML_MESH_SHAPES:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onMeshShape( saxContext, saxAttributes );
        }
        break;
    case FML_MESH_CONNECTIVITY:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onMeshConnectivity( saxContext, saxAttributes );
        }
        break;
    case FML_CONTINUOUS_REFERENCE:
        if( xmlStrcmp( name, ALIASES_TAG ) == 0 )
        {
            saxContext->state.push( FML_ALIASES );
        }
        break;
    case FML_ENSEMBLE_PARAMETERS:
    case FML_CONTINUOUS_PARAMETERS:
        if( xmlStrcmp( name, SEMI_DENSE_DATA_TAG ) == 0 )
        {
            startSemidenseData( saxContext, saxAttributes );
            saxContext->state.push( FML_SEMI_DENSE );
        }
        break;
    case FML_CONTINUOUS_PIECEWISE:
        if( xmlStrcmp( name, ELEMENT_EVALUATORS_TAG ) == 0 )
        {
            saxContext->state.push( FML_ELEMENT_EVALUATORS );
            onContinuousPiecewiseEvaluators( saxContext, saxAttributes );
        }
        else if( xmlStrcmp( name, ALIASES_TAG ) == 0 )
        {
            saxContext->state.push( FML_ALIASES );
        }
        break;
    case FML_CONTINUOUS_AGGREGATE:
        if( xmlStrcmp( name, SOURCE_FIELDS_TAG ) == 0 )
        {
            saxContext->state.push( FML_SOURCE_FIELDS );
        }
        else if( xmlStrcmp( name, ALIASES_TAG ) == 0 )
        {
            saxContext->state.push( FML_ALIASES );
        }
        break;
    case FML_MARKUP:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onMarkupEntry( saxContext, saxAttributes );
        }
        break;
    case FML_SOURCE_FIELDS:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onContinuousAggregateEntry( saxContext, saxAttributes );
        }
        break;
    case FML_ELEMENT_EVALUATORS:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onContinuousPiecewiseEntry( saxContext, saxAttributes );
        }
        break;
    case FML_SEMI_DENSE:
        if( xmlStrcmp( name, DENSE_INDEXES_TAG ) == 0 )
        {
            saxContext->state.push( FML_DENSE_INDEXES );
        }
        else if( xmlStrcmp( name, SPARSE_INDEXES_TAG ) == 0 )
        {
            saxContext->state.push( FML_SPARSE_INDEXES );
        }
        else if( xmlStrcmp( name, INLINE_DATA_TAG ) == 0 )
        {
            startInlineData( saxContext, saxAttributes );
            saxContext->state.push( FML_INLINE_DATA );
        }
        else if( xmlStrcmp( name, FILE_DATA_TAG ) == 0 )
        {
            onFileData( saxContext, saxAttributes );
        }
        else if( xmlStrcmp( name, SWIZZLE_TAG ) == 0 )
        {
            startSwizzleData( saxContext, saxAttributes );
            saxContext->state.push( FML_SWIZZLE_DATA );
        }
        break;
    case FML_DENSE_INDEXES:
        if( xmlStrcmp( name, ENTRY_TAG ) == 0 )
        {
            onSemidenseDenseIndex( saxContext, saxAttributes );
        }
        break;
    case FML_SPARSE_INDEXES:
        if( xmlStrcmp( name, ENTRY_TAG ) == 0 )
        {
            onSemidenseSparseIndex( saxContext, saxAttributes );
        }
        break;
    case FML_ALIASES:
        if( xmlStrcmp( name, MAP_ENTRY_TAG ) == 0 )
        {
            onAlias( saxContext, saxAttributes );
        }
        break;
    case FML_REGION:
        if( xmlStrcmp( name, ENSEMBLE_DOMAIN_TAG ) == 0 )
        {
            startEnsembleDomain( saxContext, saxAttributes );
            saxContext->state.push( FML_ENSEMBLE_DOMAIN );
        }
        else if( xmlStrcmp( name, CONTINUOUS_DOMAIN_TAG ) == 0 )
        {
            startContinuousDomain( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_DOMAIN );
        }
        else if( xmlStrcmp( name, MESH_DOMAIN_TAG ) == 0 )
        {
            startMeshDomain( saxContext, saxAttributes );
            saxContext->state.push( FML_MESH_DOMAIN );
        }
        else if( xmlStrcmp( name, CONTINUOUS_REFERENCE_TAG ) == 0 )
        {
            startContinuousReference( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_REFERENCE );
        }
        else if( xmlStrcmp( name, ENSEMBLE_PARAMETERS_TAG ) == 0 )
        {
            startEnsembleParameters( saxContext, saxAttributes );
            saxContext->state.push( FML_ENSEMBLE_PARAMETERS );
        }
        else if( xmlStrcmp( name, CONTINUOUS_PARAMETERS_TAG ) == 0 )
        {
            startContinuousParameters( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_PARAMETERS );
        }
        else if( xmlStrcmp( name, CONTINUOUS_PIECEWISE_TAG ) == 0 )
        {
            startContinuousPiecewise( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_PIECEWISE );
        }
        else if( xmlStrcmp( name, CONTINUOUS_VARIABLE_TAG ) == 0 )
        {
            startContinuousVariable( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_VARIABLE );
        }
        else if( xmlStrcmp( name, CONTINUOUS_AGGREGATE_TAG ) == 0 )
        {
            startContinuousAggregate( saxContext, saxAttributes );
            saxContext->state.push( FML_CONTINUOUS_AGGREGATE );
        }
        else if( xmlStrcmp( name, ENSEMBLE_VARIABLE_TAG ) == 0 )
        {
            startEnsembleVariable( saxContext, saxAttributes );
            saxContext->state.push( FML_ENSEMBLE_VARIABLE );
        }
        break;
        //FALLTHROUGH
    default:
        break;
    }
}


static void onEndElementNs( void *context, const xmlChar *name, const xmlChar *prefix, const xmlChar *URI )
{
    SaxContext *saxContext = (SaxContext*)context;

    switch( saxContext->state.top() )
    {
    case FML_FIELDML:
        if( xmlStrcmp( name, FIELDML_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_ENSEMBLE_DOMAIN_BOUNDS:
        if( xmlStrcmp( name, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_MESH_DOMAIN:
        if( xmlStrcmp( name, MESH_DOMAIN_TAG ) == 0 )
        {
            endMeshDomain( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_MESH_SHAPES:
        if( xmlStrcmp( name, MESH_SHAPES_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_MESH_CONNECTIVITY:
        if( xmlStrcmp( name, MESH_CONNECTIVITY_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_ENSEMBLE_DOMAIN:
        if( xmlStrcmp( name, ENSEMBLE_DOMAIN_TAG ) == 0 )
        {
            endEnsembleDomain( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_DOMAIN:
        if( xmlStrcmp( name, CONTINUOUS_DOMAIN_TAG ) == 0 )
        {
            endContinuousDomain( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_ALIASES:
        if( xmlStrcmp( name, ALIASES_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_REFERENCE:
        if( xmlStrcmp( name, CONTINUOUS_REFERENCE_TAG ) == 0 )
        {
            endContinuousReference( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_ENSEMBLE_PARAMETERS:
        if( xmlStrcmp( name, ENSEMBLE_PARAMETERS_TAG ) == 0 )
        {
            endEnsembleParameters( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_PIECEWISE:
        if( xmlStrcmp( name, CONTINUOUS_PIECEWISE_TAG ) == 0 )
        {
            endContinuousPiecewise( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_SOURCE_FIELDS:
        if( xmlStrcmp( name, SOURCE_FIELDS_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_MARKUP:
        if( xmlStrcmp( name, MARKUP_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_AGGREGATE:
        if( xmlStrcmp( name, CONTINUOUS_AGGREGATE_TAG ) == 0 )
        {
            endContinuousAggregate( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_PARAMETERS:
        if( xmlStrcmp( name, CONTINUOUS_PARAMETERS_TAG ) == 0 )
        {
            endContinuousParameters( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_SEMI_DENSE:
        if( xmlStrcmp( name, SEMI_DENSE_DATA_TAG ) == 0 )
        {
            endSemidenseData( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_ELEMENT_EVALUATORS:
        if( xmlStrcmp( name, ELEMENT_EVALUATORS_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_SPARSE_INDEXES:
        if( xmlStrcmp( name, SPARSE_INDEXES_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_DENSE_INDEXES:
        if( xmlStrcmp( name, DENSE_INDEXES_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_INLINE_DATA:
        if( xmlStrcmp( name, INLINE_DATA_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_SWIZZLE_DATA:
        if( xmlStrcmp( name, SWIZZLE_TAG ) == 0 )
        {
            endSwizzleData( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_FILE_DATA:
        if( xmlStrcmp( name, FILE_DATA_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        break;
    case FML_CONTINUOUS_VARIABLE:
        if( xmlStrcmp( name, CONTINUOUS_VARIABLE_TAG ) == 0 )
        {
            endVariable( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_ENSEMBLE_VARIABLE:
        if( xmlStrcmp( name, ENSEMBLE_VARIABLE_TAG ) == 0 )
        {
            endVariable( saxContext );
            saxContext->state.pop();
        }
        break;
    case FML_REGION:
        if( xmlStrcmp( name, REGION_TAG ) == 0 )
        {
            saxContext->state.pop();
        }
        //FALLTHROUGH
    default:
        break;
    }
}


static xmlSAXHandler SAX2HandlerStruct =
{
    onInternalSubset,
    isStandalone,
    hasInternalSubset,
    hasExternalSubset,
    resolveEntity,
    getEntity,
    onEntityDecl,
    onNotationDecl,
    onAttributeDecl,
    onElementDecl,
    onUnparsedEntityDecl,
    setDocumentLocator,
    onStartDocument,
    onEndDocument,
    NULL,
    NULL,
    onReference,
    onCharacters,
    onIgnorableWhitespace,
    onProcessingInstruction,
    comment,
    warning,
    error,
    fatalError,
    getParameterEntity,
    onCdataBlock,
    externalSubset,
    XML_SAX2_MAGIC,
    NULL,
    onStartElementNs,
    onEndElementNs,
    NULL
};

static xmlSAXHandlerPtr SAX2Handler = &SAX2HandlerStruct;

//========================================================================
//
// Main
//
//========================================================================

FieldmlRegion *parseFieldmlFile( const char *filename )
{
    int res, state;
    SaxContext context;

    context.region = NULL;
    context.currentObject = FML_INVALID_HANDLE;
    context.bufferLength = 0;
    context.buffer = NULL;
    context.source = filename;

    context.state.push( FML_ROOT );

    LIBXML_TEST_VERSION

    xmlSubstituteEntitiesDefault( 1 );
    
    res = xmlSAXUserParseFile( SAX2Handler, &context, filename );
    if( res != 0 )
    {
        context.region->logError( "xmlSAXUserParseFile returned error" );
    }

    state = context.state.top();
    if( state != FML_ROOT )
    {
        context.region->logError( "Parser state not empty" );
    }

    xmlCleanupParser();
    xmlMemoryDump();
    
    context.region->finalize();

    return context.region;
}
