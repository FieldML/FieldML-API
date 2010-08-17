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
    FmlObjectHandle handle = Fieldml_GetNamedObject( region, name );

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


SaxHandler::SaxHandler( const xmlChar *_tagName ) :
    tagName( _tagName )
{
}


SaxHandler::~SaxHandler()
{
}


void SaxHandler::onCharacters( const xmlChar *xmlChars, int count )
{
}


RootSaxHandler::RootSaxHandler( const xmlChar *_tagName, SaxContext *_context ) :
    SaxHandler( tagName ),
    context( _context )
{
}


SaxHandler *RootSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, FIELDML_TAG ) == 0 )
    {
        return new FieldmlSaxHandler( tagName, this );
    }
    
    return this;
}


SaxHandler *RootSaxHandler::getParent()
{
    return NULL;
}


FieldmlSaxHandler::FieldmlSaxHandler( const xmlChar *tagName, RootSaxHandler *_parent ) :
    SaxHandler( tagName ),
    parent( _parent )
{
}


SaxHandler *FieldmlSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, REGION_TAG ) == 0 )
    {
        return new RegionSaxHandler( tagName, this, attributes, parent->context ); 
    }
    
    return this;
}


RootSaxHandler *FieldmlSaxHandler::getParent()
{
    return parent;
}



RegionSaxHandler::RegionSaxHandler( const xmlChar *tagName, FieldmlSaxHandler *_parent, SaxAttributes &attributes, SaxContext *context ) :
    SaxHandler( tagName ),
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


SaxHandler *RegionSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, ENSEMBLE_DOMAIN_TAG ) == 0 )
    {
        return new EnsembleDomainSaxHandler( this, tagName, attributes ); 
    }
    if( xmlStrcmp( tagName, CONTINUOUS_DOMAIN_TAG ) == 0 )
    {
        return new ContinuousDomainSaxHandler( this, tagName, attributes ); 
    }
    if( xmlStrcmp( tagName, MESH_DOMAIN_TAG ) == 0 )
    {
        return new MeshDomainSaxHandler( this, tagName, attributes );
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


FieldmlObjectSaxHandler::FieldmlObjectSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName ) :
    SaxHandler( tagName ),
    parent( _parent )
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


ContinuousDomainSaxHandler::ContinuousDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    handle = FML_INVALID_HANDLE;

    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousDomain has no name" );
        return;
    }

    FmlObjectHandle componentHandle = attributes.getObjectAttribute( getRegion(), COMPONENT_DOMAIN_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );

    handle = Fieldml_CreateContinuousDomain( getRegion(), name, componentHandle );
}

    
EnsembleDomainSaxHandler::EnsembleDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    handle = FML_INVALID_HANDLE;
        
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "EnsembleDomain has no name" );
        return;
    }
    
    FmlObjectHandle componentHandle = attributes.getObjectAttribute( getRegion(), COMPONENT_DOMAIN_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    const bool isComponentEnsemble = attributes.getBooleanAttribute( IS_COMPONENT_DOMAIN_ATTRIB );
    if( isComponentEnsemble )
    {
        if( componentHandle != FML_INVALID_HANDLE )
        {
            getRegion()->logError( "Component EnsembleDomain cannot be multi-component itself", name );
        }
        else
        {
            handle = Fieldml_CreateComponentEnsembleDomain( getRegion(), name );
        }
    }
    else
    {
        handle = Fieldml_CreateEnsembleDomain( getRegion(), name, componentHandle );
    }
}


SaxHandler *EnsembleDomainSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
    {
        return new ContiguousEnsembleBoundsHandler( this, tagName, attributes );
    }

    return this;
}


MeshDomainSaxHandler::MeshDomainSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "MeshDomain has no name" );
        return;
    }
    
    FmlObjectHandle xiHandle = attributes.getObjectAttribute( getRegion(), XI_COMPONENT_DOMAIN_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    if( xiHandle == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "MeshDomain has no xi components", name );
        return;
    }

    handle = Fieldml_CreateMeshDomain( getRegion(), name, xiHandle );
}


SaxHandler *MeshDomainSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, MESH_SHAPES_TAG ) == 0 )
    {
        return new MeshShapesSaxHandler( this, tagName, attributes );
    }
    else if( xmlStrcmp( tagName, MESH_CONNECTIVITY_TAG ) == 0 )
    {
        return new MeshConnectivitySaxHandler( this, tagName, attributes );
    }
    else if( xmlStrcmp( tagName, CONTIGUOUS_ENSEMBLE_BOUNDS_TAG ) == 0 )
    {
        return new ContiguousEnsembleBoundsHandler( this, tagName, attributes );
    }
}


ContinuousReferenceSaxHandler::ContinuousReferenceSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousReferenceEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle remoteEvaluator = attributes.getObjectAttribute( getRegion(), EVALUATOR_ATTRIB, FHT_UNKNOWN_CONTINUOUS_EVALUATOR );
    if( remoteEvaluator == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousReferenceEvaluator has no remote evaluator", name );
        return;
    }
    
    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), VALUE_DOMAIN_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousReferenceEvaluator has no value domain", name );
        return;
    }

    handle = Fieldml_CreateContinuousReference( getRegion(), name, remoteEvaluator, valueDomain );
}


SaxHandler * ContinuousReferenceSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, ALIASES_TAG ) == 0 )
    {
        return new AliasesSaxHandler( this, tagName, attributes );
    }
    
    return this;
}


EnsembleParametersSaxHandler::EnsembleParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "EnsembleParameters has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), NAME_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "EnsembleParameters has no value domain", name );
        return;
    }

    handle = Fieldml_CreateEnsembleParameters( getRegion(), name, handle );
}


SaxHandler * EnsembleParametersSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, SEMI_DENSE_DATA_TAG ) == 0 )
    {
        return new SemidenseSaxHandler( this, tagName, attributes );
    }
    
    return this;
}


ContinuousParametersSaxHandler::ContinuousParametersSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousParameters has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), NAME_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousParameters has no value domain", name );
        return;
    }

    handle = Fieldml_CreateContinuousParameters( getRegion(), name, handle );
}


SaxHandler * ContinuousParametersSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, SEMI_DENSE_DATA_TAG ) == 0 )
    {
        return new SemidenseSaxHandler( this, tagName, attributes );
    }
    
    return this;
}


ContinuousVariableSaxHandler::ContinuousVariableSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousVariable has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), VALUE_DOMAIN_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousVariable has no value domain", name );
        return;
    }

    handle = Fieldml_CreateContinuousVariable( getRegion(), name, valueDomain );
}


SaxHandler * ContinuousVariableSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    return this;
}


EnsembleVariableSaxHandler::EnsembleVariableSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "EnsembleVariable has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), VALUE_DOMAIN_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "EnsembleVariable has no value domain", name );
        return;
    }

    handle = Fieldml_CreateEnsembleVariable( getRegion(), name, valueDomain );
}


SaxHandler * EnsembleVariableSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    return this;
}


ContinuousPiecewiseSaxHandler::ContinuousPiecewiseSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousPiecewise has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), VALUE_DOMAIN_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousPiecewise has no value domain", name );
        return;
    }

    FmlObjectHandle indexDomain = attributes.getObjectAttribute( getRegion(), INDEX_DOMAIN_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
    if( indexDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousPiecewise has no index domain", name );
        return;
    }
    
    handle = Fieldml_CreateContinuousPiecewise( getRegion(), name, indexDomain, valueDomain );
}


SaxHandler * ContinuousPiecewiseSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, ELEMENT_EVALUATORS_TAG ) == 0 )
    {
        FmlObjectHandle defaultHandle = attributes.getObjectAttribute( getRegion(), DEFAULT_ATTRIB, FHT_UNKNOWN_CONTINUOUS_EVALUATOR );
        if( defaultHandle != FML_INVALID_HANDLE )
        {
            Fieldml_SetDefaultEvaluator( getRegion(), handle, defaultHandle );
        }
        return new IntObjectMapSaxHandler( this, tagName, getRegion(), this, 0 );
    }
    else if( xmlStrcmp( tagName, ALIASES_TAG ) == 0 )
    {
        return new AliasesSaxHandler( this, tagName, attributes );
    }
    
    return this;
}


void ContinuousPiecewiseSaxHandler::onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId )
{
    if( mapId == 0 )
    {
        if( ( key <= 0 ) || ( value == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( getRegion(), handle );
            getRegion()->logError( "Malformed element evaluator for ContinuousPiecewise", name );
        }
        else
        {
            Fieldml_SetEvaluator( getRegion(), handle, key, value );
        }
    }
}


ContinuousAggregateSaxHandler::ContinuousAggregateSaxHandler( RegionSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, tagName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getRegion()->logError( "ContinuousAggregate has no name" );
        return;
    }

    FmlObjectHandle valueDomain = attributes.getObjectAttribute( getRegion(), VALUE_DOMAIN_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
    if( valueDomain == FML_INVALID_HANDLE )
    {
        getRegion()->logError( "ContinuousAggregate has no value domain", name );
        return;
    }

    handle = Fieldml_CreateContinuousAggregate( getRegion(), name, valueDomain );
}


SaxHandler * ContinuousAggregateSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, ELEMENT_EVALUATORS_TAG ) == 0 )
    {
        return new IntObjectMapSaxHandler( this, tagName, getRegion(), this, 0 );
    }
    else if( xmlStrcmp( tagName, ALIASES_TAG ) == 0 )
    {
        return new AliasesSaxHandler( this, tagName, attributes );
    }
    
    return this;
}


void ContinuousAggregateSaxHandler::onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId )
{
    if( mapId == 0 )
    {
        if( ( key <= 0 ) || ( value == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( getRegion(), handle );
            getRegion()->logError( "Malformed element evaluator for ContinuousAggregate", name );
        }
        else
        {
            Fieldml_SetEvaluator( getRegion(), handle, key, value );
        }
    }
}


ObjectMemberSaxHandler::ObjectMemberSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName ) :
    SaxHandler( tagName ),
    parent( _parent )
{
}


FieldmlObjectSaxHandler *ObjectMemberSaxHandler::getParent()
{
    return parent;
}


ContiguousEnsembleBoundsHandler::ContiguousEnsembleBoundsHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, tagName )
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


SaxHandler *ContiguousEnsembleBoundsHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    return this;
}


MeshShapesSaxHandler::MeshShapesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, tagName )
{
    const char *defaultValue = attributes.getAttribute( DEFAULT_ATTRIB );
    if( defaultValue != NULL )
    {
        Fieldml_SetMeshDefaultShape( parent->getRegion(), parent->handle, defaultValue );
    }
}


SaxHandler *MeshShapesSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, MAP_ENTRY_TAG ) == 0 )
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


MeshConnectivitySaxHandler::MeshConnectivitySaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, tagName )
{
}


SaxHandler *MeshConnectivitySaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, MAP_ENTRY_TAG ) == 0 )
    {
        FmlObjectHandle domainHandle = attributes.getObjectAttribute( parent->getRegion(), VALUE_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN );
        FmlObjectHandle fieldHandle = attributes.getObjectAttribute( parent->getRegion(), KEY_ATTRIB, FHT_UNKNOWN_ENSEMBLE_SOURCE );
        if( ( domainHandle == FML_INVALID_HANDLE ) || ( fieldHandle == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "MeshDomain has malformed connectivity entry", name );
            return this;
        }
        
        Fieldml_SetMeshConnectivity( parent->getRegion(), parent->handle, fieldHandle, domainHandle );
    }
    
    return this;
}


AliasesSaxHandler::AliasesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, tagName )
{
}


SaxHandler *AliasesSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, MAP_ENTRY_TAG ) == 0 )
    {
        FmlObjectHandle localHandle = attributes.getObjectAttribute( parent->getRegion(), VALUE_ATTRIB, FHT_UNKNOWN_CONTINUOUS_SOURCE );
        FmlObjectHandle remoteHandle = attributes.getObjectAttribute( parent->getRegion(), KEY_ATTRIB, FHT_UNKNOWN_CONTINUOUS_DOMAIN );
        if( ( remoteHandle == FML_INVALID_HANDLE ) || ( localHandle == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getRegion(), parent->handle );
            parent->getRegion()->logError( "Evaluator has malformed alias", name );
            return this;
        }

        Fieldml_SetAlias( parent->getRegion(), parent->handle, remoteHandle, localHandle );
    }
    
    return this;
}



SemidenseSaxHandler::SemidenseSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *tagName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, tagName )
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
            Fieldml_AddSemidenseIndex( parent->getRegion(), parent->handle, listEntry, listId );
        }
    }
}


SaxHandler *SemidenseSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, DENSE_INDEXES_TAG ) == 0 )
    {
        return new ObjectListSaxHandler( this, tagName, parent->getRegion(), this, 0 );
    }
    else if( xmlStrcmp( tagName, SPARSE_INDEXES_TAG ) == 0 )
    {
        return new ObjectListSaxHandler( this, tagName, parent->getRegion(), this, 1 );
    }
    else if( xmlStrcmp( tagName, INLINE_DATA_TAG ) == 0 )
    {
        Fieldml_SetParameterDataLocation( parent->getRegion(), parent->handle, LOCATION_INLINE );
        return new CharacterBufferSaxHandler( this, tagName, this, 0 );
    }
    else if( xmlStrcmp( tagName, FILE_DATA_TAG ) == 0 )
    {
        onFileData( attributes );
    }
    else if( xmlStrcmp( tagName, SWIZZLE_TAG ) == 0 )
    {
        return new CharacterAccumulatorSaxHandler( this, tagName, this, 0 );
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


ObjectListSaxHandler::ObjectListSaxHandler( SaxHandler *_parent, const xmlChar *tagName, FmlHandle _region, ObjectListHandler *_handler, int _listId ) :
    SaxHandler( tagName ),
    parent( _parent ),
    region( _region ),
    handler( _handler ),
    listId( _listId )
{
}


SaxHandler *ObjectListSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, ENTRY_TAG ) == 0 )
    {
        FmlObjectHandle handle = attributes.getObjectAttribute( region, VALUE_ATTRIB, FHT_UNKNOWN_ENSEMBLE_DOMAIN);
        handler->onObjectListEntry( handle, listId );
    }
    
    return this;
}


CharacterBufferSaxHandler::CharacterBufferSaxHandler( SaxHandler *_parent, const xmlChar *tagName, CharacterBufferHandler *_handler, int _bufferId ) :
    SaxHandler( tagName ),
    parent( _parent ),
    handler( _handler ),
    bufferId( _bufferId )
{
}


SaxHandler *CharacterBufferSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    return this;
}


void CharacterBufferSaxHandler::onCharacters( const xmlChar *xmlChars, int count )
{
    const char *buffer = (const char*)xmlChars;
    handler->onCharacterBuffer( buffer, count, bufferId );
}


CharacterAccumulatorSaxHandler::CharacterAccumulatorSaxHandler( SaxHandler *_parent, const xmlChar *tagName, CharacterBufferHandler *_handler, int _bufferId ) :
    SaxHandler( tagName ),
    parent( _parent ),
    handler( _handler ),
    bufferId( _bufferId )
{
    buffer = NULL;
    count = 0;
}


SaxHandler *CharacterAccumulatorSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
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


CharacterAccumulatorSaxHandler::~CharacterAccumulatorSaxHandler()
{
    handler->onCharacterBuffer( buffer, count, bufferId );
    delete[] buffer;
}


IntObjectMapSaxHandler::IntObjectMapSaxHandler( SaxHandler *_parent, const xmlChar *tagName, FmlHandle _region, IntObjectMapHandler *_handler, int _mapId ) :
    SaxHandler( tagName ),
    region( _region ),
    parent( _parent ),
    handler( _handler ),
    mapId( _mapId )
{
}


SaxHandler *IntObjectMapSaxHandler::onElementStart( const xmlChar *tagName, SaxAttributes &attributes )
{
    if( xmlStrcmp( tagName, MAP_ENTRY_TAG ) == 0 )
    {
        const char *keyString = attributes.getAttribute( KEY_ATTRIB );
        int key = -1;
        if( keyString != NULL )
        {
            key = atoi( keyString );
        }
        //TODO FHT_UNKNOWN_CONTINUOUS_SOURCE should be specified in the construct, to allow for piecewise/aggregate ensemble evaluators.
        FmlObjectHandle value = attributes.getObjectAttribute( region, VALUE_ATTRIB, FHT_UNKNOWN_CONTINUOUS_SOURCE );
        handler->onIntObjectMapEntry( key, value, mapId );
    }
    
    return this;
}
