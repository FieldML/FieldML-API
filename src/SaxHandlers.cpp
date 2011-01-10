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

#include "SaxHandlers.h"

#include "string_const.h"

using namespace std;


//========================================================================
//
// Utils
//
//========================================================================


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


static FmlObjectHandle getOrCreateObjectHandle( FieldmlRegion *region, const char *name, FieldmlHandleType type )
{
    FmlObjectHandle handle = Fieldml_GetObjectByName( region, name );

    if( handle == FML_INVALID_HANDLE )
    {
        handle = region->addObject( new FieldmlObject( name, VIRTUAL_REGION_HANDLE, type ) );
    }
    
    return handle;
}


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


bool SaxAttributes::getBooleanAttribute( const xmlChar *attribute )
{
    const char *rawAttribute = getAttribute( attribute );
    
    return ( ( rawAttribute != NULL ) && ( strcmp( rawAttribute, STRING_TRUE ) == 0 ) );
}


FmlObjectHandle SaxAttributes::getObjectAttribute( FmlHandle region, const xmlChar *attribute, FieldmlHandleType type )
{
    const char *rawAttribute = getAttribute( attribute );

    if( rawAttribute != NULL )
    {
        return getOrCreateObjectHandle( region, rawAttribute, type );
    }

    return FML_INVALID_HANDLE;
}


SaxHandler::SaxHandler( const xmlChar *_elementName ) :
    elementName( _elementName )
{
}


SaxHandler::~SaxHandler()
{
}


void SaxHandler::onCharacters( const xmlChar *xmlChars, int count )
{
}


RootSaxHandler::RootSaxHandler( const xmlChar *_elementName, SaxContext *_context ) :
    SaxHandler( elementName ),
    context( _context )
{
}


SaxHandler *RootSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, FIELDML_TAG ) == 0 )
    {
        return new FieldmlSaxHandler( elementName, this );
    }
    
    return this;
}


SaxHandler *RootSaxHandler::getParent()
{
    return NULL;
}


FieldmlSaxHandler::FieldmlSaxHandler( const xmlChar *elementName, RootSaxHandler *_parent ) :
    SaxHandler( elementName ),
    parent( _parent )
{
}


SaxHandler *FieldmlSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, REGION_TAG ) == 0 )
    {
        return new RegionSaxHandler( elementName, this, attributes, parent->context ); 
    }
    
    return this;
}


RootSaxHandler *FieldmlSaxHandler::getParent()
{
    return parent;
}



RegionSaxHandler::RegionSaxHandler( const xmlChar *elementName, FieldmlSaxHandler *_parent, SaxAttributes &attributes, SaxContext *context ) :
    SaxHandler( elementName ),
    parent( _parent )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        //HACK Allow nameless regions for now.
        name = "";
    }

    string location = getDirectory( context->source );
    region = Fieldml_Create( location.c_str(), name );
    context->region = region;
}


SaxHandler *RegionSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, ENSEMBLE_TYPE_TAG ) == 0 )
    {
        return new EnsembleTypeSaxHandler( this, elementName, attributes ); 
    }
    if( xmlStrcmp( elementName, CONTINUOUS_TYPE_TAG ) == 0 )
    {
        return new ContinuousTypeSaxHandler( this, elementName, attributes ); 
    }
    if( xmlStrcmp( elementName, MESH_TYPE_TAG ) == 0 )
    {
        return new MeshTypeSaxHandler( this, elementName, attributes );
    }

    if( xmlStrcmp( elementName, ABSTRACT_EVALUATOR_TAG ) == 0 )
    {
        return new AbstractEvaluatorSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, REFERENCE_EVALUATOR_TAG ) == 0 )
    {
        return new ReferenceEvaluatorSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, PARAMETER_EVALUATOR_TAG ) == 0 )
    {
        return new ParametersSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, PIECEWISE_EVALUATOR_TAG ) == 0 )
    {
        return new PiecewiseEvaluatorSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, AGGREGATE_EVALUATOR_TAG ) == 0 )
    {
        return new AggregateEvaluatorSaxHandler( this, elementName, attributes );
    }

    return this;
}


FieldmlSaxHandler *RegionSaxHandler::getParent()
{
    return parent;
}


FmlHandle RegionSaxHandler::getRegion()
{
    return region;
}


FieldmlObjectSaxHandler::FieldmlObjectSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName ) :
    SaxHandler( elementName ),
    parent( _parent ),
    handle( FML_INVALID_HANDLE )
{
}


RegionSaxHandler *FieldmlObjectSaxHandler::getParent()
{
    return parent;
}


FmlHandle FieldmlObjectSaxHandler::getRegion()
{
    return parent->getRegion();
}


ContinuousTypeSaxHandler::ContinuousTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    handle = FML_INVALID_HANDLE;

    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousType has no name" );
        return;
    }

    FmlObjectHandle componentHandle = attributes.getObjectAttribute( getRegion(), COMPONENT_ENSEMBLE_ATTRIB, FHT_UNKNOWN_TYPE );

    handle = Fieldml_CreateContinuousType( getRegion(), name, componentHandle );
}


SaxHandler *ContinuousTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    return this;
}


EnsembleTypeSaxHandler::EnsembleTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    handle = FML_INVALID_HANDLE;
        
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "EnsembleType has no name" );
        return;
    }
    
    const bool isComponentEnsemble = attributes.getBooleanAttribute( IS_COMPONENT_ENSEMBLE_ATTRIB );
    
    handle = Fieldml_CreateEnsembleType( getRegion(), name, isComponentEnsemble ? 1 : 0 );
}


SaxHandler *EnsembleTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
    {
        return new ContiguousEnsembleBoundsHandler( this, elementName, attributes );
    }

    return this;
}


MeshTypeSaxHandler::MeshTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "MeshType has no name" );
        return;
    }

    FmlObjectHandle xiComponent = attributes.getObjectAttribute( getRegion(), XI_COMPONENT_ATTRIB, FHT_UNKNOWN_TYPE );
    if( xiComponent == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "MeshType has no xi component", name );
        return;
    }
    
    handle = Fieldml_CreateMeshType( getRegion(), name, xiComponent );
}


SaxHandler *MeshTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MESH_SHAPES_TAG ) == 0 )
    {
        return new MeshShapesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
    {
        return new ContiguousEnsembleBoundsHandler( this, elementName, attributes );
    }
    
    return this;
}


AbstractEvaluatorSaxHandler::AbstractEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "AbstractEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle valueType = attributes.getObjectAttribute( getRegion(), VALUE_TYPE_ATTRIB, FHT_UNKNOWN_TYPE );
    if( valueType == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "AbstractEvaluator has no value type", name );
        return;
    }
    
    handle = Fieldml_CreateAbstractEvaluator( getRegion(), name, valueType );
}


SaxHandler * AbstractEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDS_TAG ) == 0 )
    {
//        return new BindsSaxHandler( this, elementName, attributes ); //TODO Add binds to abstract evaluators
    }
    
    return this;
}


ReferenceEvaluatorSaxHandler::ReferenceEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ReferenceEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle remoteEvaluator = attributes.getObjectAttribute( getRegion(), EVALUATOR_ATTRIB, FHT_UNKNOWN_EVALUATOR );
    if( remoteEvaluator == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ReferenceEvaluator has no remote evaluator", name );
        return;
    }
    
    handle = Fieldml_CreateReferenceEvaluator( getRegion(), name, remoteEvaluator );
}


SaxHandler * ReferenceEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDS_TAG ) == 0 )
    {
        return new BindsSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


ParametersSaxHandler::ParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "EnsembleParameters has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getRegion(), VALUE_TYPE_ATTRIB, FHT_UNKNOWN_TYPE );
    if( valueType == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "EnsembleParameters has no value type", name );
        return;
    }

    handle = Fieldml_CreateParametersEvaluator( getRegion(), name, valueType );
}


SaxHandler * ParametersSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, SEMI_DENSE_DATA_TAG ) == 0 )
    {
        return new SemidenseSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


PiecewiseEvaluatorSaxHandler::PiecewiseEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "PiecewiseEvaluator has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getRegion(), VALUE_TYPE_ATTRIB, FHT_UNKNOWN_TYPE );
    if( valueType == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "PiecewiseEvaluator has no value domain", name );
        return;
    }

    handle = Fieldml_CreatePiecewiseEvaluator( getRegion(), name, valueType );
}


SaxHandler * PiecewiseEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, ELEMENT_EVALUATORS_TAG ) == 0 )
    {
        FmlObjectHandle defaultHandle = attributes.getObjectAttribute( getRegion(), DEFAULT_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( defaultHandle != FML_INVALID_HANDLE )
        {
            Fieldml_SetDefaultEvaluator( getRegion(), handle, defaultHandle );
        }
        return new IntObjectMapSaxHandler( this, elementName, ELEMENT_TAG, getRegion(), this, 0 );
    }
    else if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDS_TAG ) == 0 )
    {
        return new BindsSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


void PiecewiseEvaluatorSaxHandler::onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId )
{
    if( mapId == 0 )
    {
        if( ( key <= 0 ) || ( value == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( getRegion(), handle );
            getRegion()->logError( "Malformed element evaluator for PiecewiseEvaluator", name );
        }
        else
        {
            Fieldml_SetEvaluator( getRegion(), handle, key, value );
        }
    }
}


AggregateEvaluatorSaxHandler::AggregateEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "AggregateEvaluator has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getRegion(), VALUE_TYPE_ATTRIB, FHT_UNKNOWN_TYPE );
    if( valueType == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "AggregateEvaluator has no value domain", name );
        return;
    }

    handle = Fieldml_CreateAggregateEvaluator( getRegion(), name, valueType );
}


SaxHandler * AggregateEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, COMPONENT_EVALUATORS_TAG ) == 0 )
    {
        FmlObjectHandle defaultHandle = attributes.getObjectAttribute( getRegion(), DEFAULT_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( defaultHandle != FML_INVALID_HANDLE )
        {
            Fieldml_SetDefaultEvaluator( getRegion(), handle, defaultHandle );
        }
        return new IntObjectMapSaxHandler( this, elementName, COMPONENT_TAG, getRegion(), this, 0 );
    }
    else if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDS_TAG ) == 0 )
    {
        return new BindsSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


void AggregateEvaluatorSaxHandler::onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId )
{
    if( mapId == 0 )
    {
        if( ( key <= 0 ) || ( value == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( getRegion(), handle );
            getRegion()->logError( "Malformed element evaluator for AggregateEvaluator", name );
        }
        else
        {
            Fieldml_SetEvaluator( getRegion(), handle, key, value );
        }
    }
}


ObjectMemberSaxHandler::ObjectMemberSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName ) :
    SaxHandler( elementName ),
    parent( _parent )
{
}


FieldmlObjectSaxHandler *ObjectMemberSaxHandler::getParent()
{
    return parent;
}


ContiguousEnsembleBoundsHandler::ContiguousEnsembleBoundsHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
    const char *count = attributes.getAttribute( VALUE_COUNT_ATTRIB );
    if( count == NULL )
    {
        const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
        parent->getRegion()->logError( "Contiguous bounds has no value count", name );
        return;
    }
    
    Fieldml_SetContiguousBoundsCount( parent->getRegion(), parent->handle, atoi( count ) );
}


SaxHandler *ContiguousEnsembleBoundsHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    return this;
}


MeshShapesSaxHandler::MeshShapesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
    const char *defaultValue = attributes.getAttribute( DEFAULT_ATTRIB );
    if( defaultValue != NULL )
    {
        Fieldml_SetMeshDefaultShape( parent->getRegion(), parent->handle, defaultValue );
    }
}


SaxHandler *MeshShapesSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MESH_SHAPE_TAG ) == 0 )
    {
        const char *element = attributes.getAttribute( KEY_ATTRIB );
        const char *shape = attributes.getAttribute( VALUE_ATTRIB );

        if( ( element == NULL ) || ( shape == NULL ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "MeshDomain has malformed shape entry", name );
            return this;
        }
        
        Fieldml_SetMeshElementShape( parent->getRegion(), parent->handle, atoi( element ), shape );
    }
    
    return this;
}


MeshConnectivitySaxHandler::MeshConnectivitySaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
}


SaxHandler *MeshConnectivitySaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MESH_CONNECTIVITY_ENTRY_TAG ) == 0 )
    {
        FmlObjectHandle typeHandle = attributes.getObjectAttribute( parent->getRegion(), VALUE_ATTRIB, FHT_UNKNOWN_TYPE );
        FmlObjectHandle fieldHandle = attributes.getObjectAttribute( parent->getRegion(), KEY_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( ( typeHandle == FML_INVALID_HANDLE ) || ( fieldHandle == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "MeshDomain has malformed connectivity entry", name );
            return this;
        }
        
        Fieldml_SetMeshConnectivity( parent->getRegion(), parent->handle, fieldHandle, typeHandle );
    }
    
    return this;
}


VariablesSaxHandler::VariablesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
}


SaxHandler *VariablesSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLE_TAG ) == 0 )
    {
        FmlObjectHandle variableHandle = attributes.getObjectAttribute( parent->getRegion(), NAME_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( variableHandle == FML_INVALID_HANDLE )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "Evaluator has malformed variable", name );
            return this;
        }
        Fieldml_AddVariable( parent->getRegion(), parent->handle, variableHandle );
    }
    
    return this;
}



BindsSaxHandler::BindsSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
}


SaxHandler *BindsSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, BIND_TAG ) == 0 )
    {
        FmlObjectHandle localHandle = attributes.getObjectAttribute( parent->getRegion(), VARIABLE_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        FmlObjectHandle remoteHandle = attributes.getObjectAttribute( parent->getRegion(), SOURCE_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( ( remoteHandle == FML_INVALID_HANDLE ) || ( localHandle == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "Evaluator has malformed bind", name );
            return this;
        }

        Fieldml_SetBind( parent->getRegion(), parent->handle, remoteHandle, localHandle );
    }
    else if( xmlStrcmp( elementName, BIND_INDEX_TAG ) == 0 )
    {
        FmlObjectHandle indexHandle = attributes.getObjectAttribute( parent->getRegion(), VARIABLE_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        if( indexHandle == FML_INVALID_HANDLE )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "Evaluator has malformed index bind", name );
            return this;
        }

        const char *indexString = attributes.getAttribute( INDEX_NUMBER_ATTRIB );
        int index = -1;
        if( indexString != NULL )
        {
            index = atoi( indexString );
        }

        Fieldml_SetIndexEvaluator( parent->getRegion(), parent->handle, index, indexHandle );
    }
    
    return this;
}



SemidenseSaxHandler::SemidenseSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
    Fieldml_SetParameterDataDescription( parent->getRegion(), parent->handle, DESCRIPTION_SEMIDENSE );
}


void SemidenseSaxHandler::onFileData( SaxAttributes &attributes )
{
    const char *file = attributes.getAttribute( FILE_ATTRIB );
    const char *type = attributes.getAttribute( TYPE_ATTRIB );
    const char *offset = attributes.getAttribute( OFFSET_ATTRIB );
    DataFileType fileType;
    int offsetAmount;
    
    if( file == NULL )
    {
        const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
        parent->getRegion()->logError( "Parameters file data for must have a file name", name );
        return;
    }
    
    if( type == NULL )
    {
        const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
        parent->getRegion()->logError( "Parameters file data for must have a file type", name );
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
        const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
        parent->getRegion()->logError( "Parameters file data for must have a known file type", name );
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
    
    Fieldml_SetParameterDataLocation( parent->getRegion(), parent->handle, LOCATION_FILE );
    Fieldml_SetParameterFileData( parent->getRegion(), parent->handle, file, fileType, offsetAmount );
}


void SemidenseSaxHandler::onObjectListEntry( FmlObjectHandle listEntry, int listId )
{
    if( ( listId == 0 ) || ( listId == 1 ) )
    {
        if( listEntry == FML_INVALID_HANDLE )
        {
            parent->getRegion()->logError( "Invalid index in semi dense data" );
        }
        else
        {
            Fieldml_AddSemidenseIndexEvaluator( parent->getRegion(), parent->handle, listEntry, listId );
        }
    }
}


SaxHandler *SemidenseSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, DENSE_INDEXES_TAG ) == 0 )
    {
        return new ObjectListSaxHandler( this, elementName, parent->getRegion(), this, 0 );
    }
    else if( xmlStrcmp( elementName, SPARSE_INDEXES_TAG ) == 0 )
    {
        return new ObjectListSaxHandler( this, elementName, parent->getRegion(), this, 1 );
    }
    else if( xmlStrcmp( elementName, INLINE_DATA_TAG ) == 0 )
    {
        Fieldml_SetParameterDataLocation( parent->getRegion(), parent->handle, LOCATION_INLINE );
        return new CharacterBufferSaxHandler( this, elementName, this, 0 );
    }
    else if( xmlStrcmp( elementName, FILE_DATA_TAG ) == 0 )
    {
        onFileData( attributes );
    }
    else if( xmlStrcmp( elementName, SWIZZLE_TAG ) == 0 )
    {
        return new CharacterAccumulatorSaxHandler( this, elementName, this, 1 );
    }
    
    return this;
}


void SemidenseSaxHandler::onCharacterBuffer( const char *buffer, int count, int id )
{
    if( id == 0 )
    {
        Fieldml_AddParameterInlineData( parent->getRegion(), parent->handle, buffer, count );
    }
    else if( id == 1 )
    {
        int intCount;
        const int *ints;
        
        intCount = intParserCount( buffer );
        ints = intParserInts( buffer );
        
        Fieldml_SetSwizzle( parent->getRegion(), parent->handle, ints, intCount );
        
        delete[] ints;
    }
}


ObjectListSaxHandler::ObjectListSaxHandler( SaxHandler *_parent, const xmlChar *elementName, FmlHandle _region, ObjectListHandler *_handler, int _listId ) :
    SaxHandler( elementName ),
    parent( _parent ),
    region( _region ),
    handler( _handler ),
    listId( _listId )
{
}


SaxHandler *ObjectListSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, INDEX_TAG ) == 0 )
    {
        FmlObjectHandle handle = attributes.getObjectAttribute( region, EVALUATOR_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        handler->onObjectListEntry( handle, listId );
    }
    
    return this;
}


SaxHandler *ObjectListSaxHandler::getParent()
{
    return parent;
}


CharacterBufferSaxHandler::CharacterBufferSaxHandler( SaxHandler *_parent, const xmlChar *elementName, CharacterBufferHandler *_handler, int _bufferId ) :
    SaxHandler( elementName ),
    parent( _parent ),
    handler( _handler ),
    bufferId( _bufferId )
{
}


SaxHandler *CharacterBufferSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    return this;
}


void CharacterBufferSaxHandler::onCharacters( const xmlChar *xmlChars, int count )
{
    const char *buffer = (const char*)xmlChars;
    handler->onCharacterBuffer( buffer, count, bufferId );
}


SaxHandler *CharacterBufferSaxHandler::getParent()
{
    return parent;
}


CharacterAccumulatorSaxHandler::CharacterAccumulatorSaxHandler( SaxHandler *_parent, const xmlChar *elementName, CharacterBufferHandler *_handler, int _bufferId ) :
    SaxHandler( elementName ),
    parent( _parent ),
    handler( _handler ),
    bufferId( _bufferId )
{
    buffer = NULL;
    count = 0;
}


SaxHandler *CharacterAccumulatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    return this;
}


void CharacterAccumulatorSaxHandler::onCharacters( const xmlChar *xmlChars, int charCount )
{
    const char *characters = (const char*)xmlChars;

    char *newBuffer = new char[ count + charCount + 1 ];
    if( buffer != NULL )
    {
        memcpy( newBuffer, buffer, count );
    }
    memcpy( newBuffer + count, characters, charCount );
    count += charCount;
    newBuffer[ count ] = 0;
    delete[] buffer;
    buffer = newBuffer;
}


SaxHandler *CharacterAccumulatorSaxHandler::getParent()
{
    return parent;
}


CharacterAccumulatorSaxHandler::~CharacterAccumulatorSaxHandler()
{
    handler->onCharacterBuffer( buffer, count, bufferId );
    delete[] buffer;
}


IntObjectMapSaxHandler::IntObjectMapSaxHandler( SaxHandler *_parent, const xmlChar *elementName, const xmlChar *_entryTagName, FmlHandle _region, IntObjectMapHandler *_handler, int _mapId ) :
    SaxHandler( elementName ),
    entryTagName( _entryTagName ),
    region( _region ),
    parent( _parent ),
    handler( _handler ),
    mapId( _mapId )
{
}


SaxHandler *IntObjectMapSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, entryTagName ) == 0 )
    {
        const char *keyString = attributes.getAttribute( NUMBER_ATTRIB );
        int key = -1;
        if( keyString != NULL )
        {
            key = atoi( keyString );
        }

        FmlObjectHandle value = attributes.getObjectAttribute( region, EVALUATOR_ATTRIB, FHT_UNKNOWN_EVALUATOR );
        handler->onIntObjectMapEntry( key, value, mapId );
    }
    
    return this;
}


SaxHandler *IntObjectMapSaxHandler::getParent()
{
    return parent;
}
