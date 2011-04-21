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
    
    if( digits > 0 )
    {
        count++;
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
    bool invert;
    char c;
    
    digits = 0;
    count = 0;
    invert = false;
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
            ints[count++] = invert ? -number : number;
            number = 0;
            digits = 0;
            invert = false;
        }
        
        if( c == '-' )
        {
            invert = !invert;
        }
    }
    
    if( digits > 0 )
    {
        ints[count++] = invert ? -number : number;
    }
    
    return ints;
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


int SaxAttributes::getIntAttribute( const xmlChar *attribute, int defaultValue )
{
    const char *rawAttribute = getAttribute( attribute );
    
    return ( rawAttribute != NULL ) ? atoi( rawAttribute ) : defaultValue;
}


FmlObjectHandle SaxAttributes::getObjectAttribute( FmlHandle sessionHandle, const xmlChar *attribute )
{
    const char *rawAttribute = getAttribute( attribute );

    if( rawAttribute != NULL )
    {
        return Fieldml_GetObjectByName( sessionHandle, rawAttribute );
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


FmlHandle SaxHandler::getSessionHandle()
{
    return getParent()->getSessionHandle();
}


FieldmlSession *SaxHandler::getSession()
{
    return getParent()->getSession();
}


RootSaxHandler::RootSaxHandler( const xmlChar *_elementName, SaxContext *_context ) :
    SaxHandler( _elementName ),
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


FmlHandle RootSaxHandler::getSessionHandle()
{
    return context->session->getHandle();
}


FieldmlSession *RootSaxHandler::getSession()
{
    return context->session;
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
        //HACK Ignore region names
        name = "";
    }

    region = context->session->region;
    region->setName( name );
}


SaxHandler *RegionSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, IMPORT_TAG ) == 0 )
    {
        return new ImportSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, DATA_OBJECT_TAG ) == 0 )
    {
        return new DataObjectSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, ENSEMBLE_TYPE_TAG ) == 0 )
    {
        return new EnsembleTypeSaxHandler( this, elementName, attributes, FML_INVALID_HANDLE ); 
    }
    if( xmlStrcmp( elementName, CONTINUOUS_TYPE_TAG ) == 0 )
    {
        return new ContinuousTypeSaxHandler( this, elementName, attributes, FML_INVALID_HANDLE ); 
    }
    if( xmlStrcmp( elementName, MESH_TYPE_TAG ) == 0 )
    {
        return new MeshTypeSaxHandler( this, elementName, attributes );
    }
    
    if( xmlStrcmp( elementName, ELEMENT_SEQUENCE_TAG ) == 0 )
    {
//NYI        return new ElementSequenceSaxHandler( this, elementName, attributes );
    }

    if( xmlStrcmp( elementName, ABSTRACT_EVALUATOR_TAG ) == 0 )
    {
        return new AbstractEvaluatorSaxHandler( this, elementName, attributes );
    }
    if( xmlStrcmp( elementName, EXTERNAL_EVALUATOR_TAG ) == 0 )
    {
        return new ExternalEvaluatorSaxHandler( this, elementName, attributes );
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


FieldmlObjectSaxHandler::FieldmlObjectSaxHandler( SaxHandler *_parent, const xmlChar *elementName ) :
    SaxHandler( elementName ),
    parent( _parent ),
    handle( FML_INVALID_HANDLE )
{
}


SaxHandler *FieldmlObjectSaxHandler::getParent()
{
    return parent;
}


ContinuousTypeSaxHandler::ContinuousTypeSaxHandler( SaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes, FmlObjectHandle mesh ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    handle = FML_INVALID_HANDLE;

    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "ContinuousType has no name" );
        return;
    }

    if( mesh != FML_INVALID_HANDLE )
    {
        handle = Fieldml_CreateMeshXiType( getSessionHandle(), mesh, name );
    }
    else
    {
        handle = Fieldml_CreateContinuousType( getSessionHandle(), name );
    }
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ContinuousType creation failed", name );
    }
}


SaxHandler *ContinuousTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, COMPONENTS_TAG ) == 0 )
    {
        const char *name = attributes.getAttribute( NAME_ATTRIB );
        int count = attributes.getIntAttribute( COUNT_ATTRIB, 0 );
        
        if( ( name == NULL ) || ( count < 0 ) )
        {
            getSession()->logError( "ContinuousType has invalid component specification", name );
        }
        else
        {
            Fieldml_CreateContinuousTypeComponents( getSessionHandle(), handle, name, count );
        }
    }
    
    return this;
}


DataSourceSaxHandler::DataSourceSaxHandler( DataObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    SaxHandler( elementName ),
    parent( _parent )
{
}


void DataSourceSaxHandler::onTextFileSource( SaxAttributes &attributes )
{
    const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
    const char *filename = attributes.getAttribute( FILENAME_ATTRIB );
    int lineCount = attributes.getIntAttribute( FIRST_LINE_ATTRIB, 0 );

    if( filename == NULL )
    {
        parent->getSession()->logError( "FileSource must have a file name", name );
        return;
    }

    Fieldml_SetDataObjectSourceType( parent->getSessionHandle(), parent->handle, SOURCE_TEXT_FILE );
    Fieldml_SetDataObjectTextFileInfo( parent->getSessionHandle(), parent->handle, filename, lineCount );
}


SaxHandler *DataSourceSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, INLINE_SOURCE_TAG ) == 0 )
    {
        Fieldml_SetDataObjectSourceType( parent->getSessionHandle(), parent->handle, SOURCE_INLINE );
        return new CharacterBufferSaxHandler( this, elementName, this, 0 );
    }
    else if( xmlStrcmp( elementName, TEXT_FILE_SOURCE_TAG ) == 0 )
    {
        onTextFileSource( attributes );
    }
    
    return this;
}


void DataSourceSaxHandler::onCharacterBuffer( const char *buffer, int count, int id )
{
    if( id == 0 )
    {
        Fieldml_AddInlineData( parent->getSessionHandle(), parent->handle, buffer, count );
    }
}


DataObjectSaxHandler *DataSourceSaxHandler::getParent()
{
    return parent;
}


DataObjectSaxHandler::DataObjectSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    handle = FML_INVALID_HANDLE;
        
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "DataObject has no name" );
        return;
    }
    
    handle = Fieldml_CreateDataObject( getSessionHandle(), name );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "DataObject creation failed", name );
    }
}


SaxHandler *DataObjectSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, SOURCE_TAG ) == 0 )
    {
        return new DataSourceSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, ENTRIES_TAG ) == 0 )
    {
        const char *countAttrib = attributes.getAttribute( COUNT_ATTRIB );
        const char *lengthAttrib = attributes.getAttribute( LENGTH_ATTRIB );
        int head = attributes.getIntAttribute( HEAD_ATTRIB, 0 );
        int tail = attributes.getIntAttribute( TAIL_ATTRIB, 0 );
        
        if( ( countAttrib == NULL ) || ( lengthAttrib == NULL ) )
        {
            getSession()->logError( "Malformed DataObject entry data" );
            return this;
        }
        
        int count = atoi( countAttrib );
        int length = atoi( lengthAttrib );
        
        Fieldml_SetDataObjectEntryInfo( getSessionHandle(), handle, count, length, head, tail );
    }

    return this;
}


ImportSaxHandler::ImportSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    SaxHandler( elementName ),
    parent( _parent )
{
    const char *location = attributes.getAttribute( LOCATION_ATTRIB );
    const char *region = attributes.getAttribute( REGION_ATTRIB );

    if( location == NULL )
    {
        getSession()->logError( "Import has no region location" );
        return;
    }
    if( region == NULL )
    {
        getSession()->logError( "Import has no region name" );
        return;
    }
    
    importIndex = Fieldml_AddImportSource( getSessionHandle(), location, region );
}


SaxHandler *ImportSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, IMPORT_TYPE_TAG ) == 0 )
    {
        const char *localName = attributes.getAttribute( LOCAL_NAME_ATTRIB );
        const char *remoteName = attributes.getAttribute( REMOTE_NAME_ATTRIB );
        
        Fieldml_AddImport( getSessionHandle(), importIndex, localName, remoteName );
    }
    else if( xmlStrcmp( elementName, IMPORT_EVALUATOR_TAG ) == 0 )
    {
        const char *localName = attributes.getAttribute( LOCAL_NAME_ATTRIB );
        const char *remoteName = attributes.getAttribute( REMOTE_NAME_ATTRIB );
        
        Fieldml_AddImport( getSessionHandle(), importIndex, localName, remoteName );
    }

    return this;
}


SaxHandler *ImportSaxHandler::getParent()
{
    return parent;
}


EnsembleTypeSaxHandler::EnsembleTypeSaxHandler( SaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes, FmlObjectHandle mesh ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    handle = FML_INVALID_HANDLE;
        
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "EnsembleType has no name" );
        return;
    }
    
    if( mesh != FML_INVALID_HANDLE )
    {
        handle = Fieldml_CreateMeshElementsType( getSessionHandle(), mesh, name );
    }
    else
    {
        handle = Fieldml_CreateEnsembleType( getSessionHandle(), name );
    }
    
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "EnsembleType creation failed", name );
    }
}


SaxHandler *EnsembleTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MEMBERS_TAG ) == 0 )
    {
        return new EnsembleElementsHandler( this, elementName, attributes );
    }

    return this;
}


MeshTypeSaxHandler::MeshTypeSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "MeshType has no name" );
        return;
    }

    handle = Fieldml_CreateMeshType( getSessionHandle(), name );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "MeshType creation failed", name );
    }
}


SaxHandler *MeshTypeSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MESH_SHAPES_TAG ) == 0 )
    {
        return new MeshShapesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, XI_TAG ) == 0 )
    {
        return new ContinuousTypeSaxHandler( this, elementName, attributes, handle );
    }
    else if( xmlStrcmp( elementName, ELEMENTS_TAG ) == 0 )
    {
        return new EnsembleTypeSaxHandler( this, elementName, attributes, handle );
    }
    
    return this;
}


ElementSequenceSaxHandler::ElementSequenceSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
#if 0
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "ElementSequence has no name" );
        return;
    }
    
    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ElementSequence has no value type", name );
        return;
    }
    
    handle = Fieldml_CreateEnsembleElementSequence( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ElementSequence creation failed", name );
    }
#endif //NYI
}


SaxHandler *ElementSequenceSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, ELEMENTS_TAG ) == 0 )
    {
//TODO
    }
    
    return this;
}


AbstractEvaluatorSaxHandler::AbstractEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "AbstractEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "AbstractEvaluator has no value type", name );
        return;
    }
    
    handle = Fieldml_CreateAbstractEvaluator( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "Cannot create AbstractEvaluator with given type", name, attributes.getAttribute( VALUE_TYPE_ATTRIB ) );
    }
}


SaxHandler * AbstractEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDINGS_TAG ) == 0 )
    {
//        return new BindsSaxHandler( this, elementName, attributes ); //TODO Add binds to abstract evaluators?
    }
    
    return this;
}


ExternalEvaluatorSaxHandler::ExternalEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "ExternalEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ExternalEvaluator has no value type", name );
        return;
    }
    
    handle = Fieldml_CreateExternalEvaluator( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ExternalEvaluator creation failed", name );
    }
}


SaxHandler * ExternalEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


ReferenceEvaluatorSaxHandler::ReferenceEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "ReferenceEvaluator has no name" );
        return;
    }
    
    FmlObjectHandle sourceEvaluator = attributes.getObjectAttribute( getSessionHandle(), EVALUATOR_ATTRIB );
    if( sourceEvaluator == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ReferenceEvaluator has no source evaluator", name );
        return;
    }
    
    handle = Fieldml_CreateReferenceEvaluator( getSessionHandle(), name, sourceEvaluator );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ReferenceEvaluator creation failed", name );
    }
}


SaxHandler * ReferenceEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDINGS_TAG ) == 0 )
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
        getSession()->logError( "ParametersEvaluator has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ParametersEvaluator has no value type", name );
        return;
    }

    handle = Fieldml_CreateParametersEvaluator( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "ParametersEvaluator creation failed", name );
    }
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
        getSession()->logError( "PiecewiseEvaluator has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "PiecewiseEvaluator has no value domain", name );
        return;
    }

    handle = Fieldml_CreatePiecewiseEvaluator( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "PiecewiseEvaluator creation failed", name );
    }
}


SaxHandler * PiecewiseEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, ELEMENT_EVALUATORS_TAG ) == 0 )
    {
        FmlObjectHandle defaultHandle = attributes.getObjectAttribute( getSessionHandle(), DEFAULT_ATTRIB );
        if( defaultHandle != FML_INVALID_HANDLE )
        {
            Fieldml_SetDefaultEvaluator( getSessionHandle(), handle, defaultHandle );
        }
        return new IntObjectMapSaxHandler( this, elementName, ELEMENT_EVALUATOR_TAG, INDEX_VALUE_ATTRIB, this, 0 );
    }
    else if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDINGS_TAG ) == 0 )
    {
        return new BindsSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, INDEX_EVALUATORS_TAG ) == 0 )
    {
        return new IndexEvaluatorsSaxHandler( this, elementName, attributes );
    }
    
    return this;
}


void PiecewiseEvaluatorSaxHandler::onIntObjectMapEntry( int key, FmlObjectHandle value, int mapId )
{
    if( mapId == 0 )
    {
        if( ( key <= 0 ) || ( value == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( getSessionHandle(), handle );
            getSession()->logError( "Malformed element evaluator for PiecewiseEvaluator", name );
        }
        else
        {
            Fieldml_SetEvaluator( getSessionHandle(), handle, key, value );
        }
    }
}


AggregateEvaluatorSaxHandler::AggregateEvaluatorSaxHandler( RegionSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    FieldmlObjectSaxHandler( _parent, elementName )
{
    const char *name = attributes.getAttribute( NAME_ATTRIB );
    if( name == NULL )
    {
        getSession()->logError( "AggregateEvaluator has no name" );
        return;
    }

    FmlObjectHandle valueType = attributes.getObjectAttribute( getSessionHandle(), VALUE_TYPE_ATTRIB );
    if( valueType == FML_INVALID_HANDLE )
    {
        getSession()->logError( "AggregateEvaluator has no value domain", name );
        return;
    }

    handle = Fieldml_CreateAggregateEvaluator( getSessionHandle(), name, valueType );
    if( handle == FML_INVALID_HANDLE )
    {
        getSession()->logError( "Cannot create AggregateEvaluator with given type.", name, attributes.getAttribute( VALUE_TYPE_ATTRIB ) );
    }
}


SaxHandler * AggregateEvaluatorSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, COMPONENT_EVALUATORS_TAG ) == 0 )
    {
        FmlObjectHandle defaultHandle = attributes.getObjectAttribute( getSessionHandle(), DEFAULT_ATTRIB );
        if( defaultHandle != FML_INVALID_HANDLE )
        {
            Fieldml_SetDefaultEvaluator( getSessionHandle(), handle, defaultHandle );
        }
        return new IntObjectMapSaxHandler( this, elementName, COMPONENT_EVALUATOR_TAG, COMPONENT_ATTRIB, this, 0 );
    }
    else if( xmlStrcmp( elementName, VARIABLES_TAG ) == 0 )
    {
        return new VariablesSaxHandler( this, elementName, attributes );
    }
    else if( xmlStrcmp( elementName, BINDINGS_TAG ) == 0 )
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
            const char * name =  Fieldml_GetObjectName( getSessionHandle(), handle );
            getSession()->logError( "Malformed element evaluator for AggregateEvaluator", name );
        }
        else
        {
            Fieldml_SetEvaluator( getSessionHandle(), handle, key, value );
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


EnsembleElementsHandler::EnsembleElementsHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
}


SaxHandler *EnsembleElementsHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, MEMBER_RANGE_TAG ) == 0 )
    {
        const char *min = attributes.getAttribute( MIN_ATTRIB );
        const char *max = attributes.getAttribute( MAX_ATTRIB );
        int stride = attributes.getIntAttribute( STRIDE_ATTRIB, 1 );
        
        if( ( min == NULL ) || ( max == NULL ) )
        {
            const char *name = Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "member_range is malformed", name );
            return this;
        }
        
        Fieldml_SetEnsembleElementRange( parent->getSessionHandle(), parent->handle, atoi( min ), atoi( max ), stride );
    }
    else if( ( xmlStrcmp( elementName, MEMBER_LIST_DATA_TAG ) == 0 ) ||
        ( xmlStrcmp( elementName, MEMBER_RANGE_DATA_TAG ) == 0 ) ||
        ( xmlStrcmp( elementName, MEMBER_STRIDE_RANGE_DATA_TAG ) == 0 ) )
    {
        EnsembleMembersType type;
        
        if( xmlStrcmp( elementName, MEMBER_LIST_DATA_TAG ) == 0 )
        {
            type = MEMBER_LIST_DATA;
        }
        else if( xmlStrcmp( elementName, MEMBER_RANGE_DATA_TAG ) == 0 )
        {
            type = MEMBER_RANGE_DATA;
        }
        else if( xmlStrcmp( elementName, MEMBER_STRIDE_RANGE_DATA_TAG ) == 0 )
        {
            type = MEMBER_STRIDE_RANGE_DATA;
        }

        FmlObjectHandle dataObject = attributes.getObjectAttribute( getSessionHandle(), DATA_ATTRIB );
        if( dataObject == FML_INVALID_HANDLE )
        {
            getSession()->logError( "EnsembleType member range data has no data object" );
            return this;
        }
        
        int count = attributes.getIntAttribute( COUNT_ATTRIB, -1 );
        if( count < 1 )
        {
            getSession()->logError( "EnsembleType member range data has an invalid count" );
            return this;
        }
        
        Fieldml_SetEnsembleMembersData( parent->getSessionHandle(), parent->handle, type, count, dataObject );
    }

    return this;
}


MeshShapesSaxHandler::MeshShapesSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
    const char *defaultValue = attributes.getAttribute( DEFAULT_ATTRIB );
    if( defaultValue != NULL )
    {
        Fieldml_SetMeshDefaultShape( parent->getSessionHandle(), parent->handle, defaultValue );
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
            const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "MeshDomain has malformed shape entry", name );
            return this;
        }
        
        Fieldml_SetMeshElementShape( parent->getSessionHandle(), parent->handle, atoi( element ), shape );
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
        FmlObjectHandle variableHandle = attributes.getObjectAttribute( parent->getSessionHandle(), NAME_ATTRIB );
        if( variableHandle == FML_INVALID_HANDLE )
        {
            const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "Evaluator has malformed variable", name );
            return this;
        }
        Fieldml_AddVariable( parent->getSessionHandle(), parent->handle, variableHandle );
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
        FmlObjectHandle variableHandle = attributes.getObjectAttribute( parent->getSessionHandle(), VARIABLE_ATTRIB );
        FmlObjectHandle sourceHandle = attributes.getObjectAttribute( parent->getSessionHandle(), SOURCE_ATTRIB );
        if( ( variableHandle == FML_INVALID_HANDLE ) || ( sourceHandle == FML_INVALID_HANDLE ) )
        {
            const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "Evaluator has malformed bind", name );
            return this;
        }

        if( Fieldml_SetBind( parent->getSessionHandle(), parent->handle, variableHandle, sourceHandle ) != FML_ERR_NO_ERROR )
        {
            parent->getSession()->logError( "Incompatible bind",
                attributes.getAttribute( VARIABLE_ATTRIB ),
                attributes.getAttribute( SOURCE_ATTRIB ) );
        }
    }
    else if( ( xmlStrcmp( elementName, BIND_INDEX_TAG ) == 0 ) && ( Fieldml_GetObjectType( parent->getSessionHandle(), parent->handle ) == FHT_AGGREGATE_EVALUATOR ) )
    {
        FmlObjectHandle indexHandle = attributes.getObjectAttribute( parent->getSessionHandle(), VARIABLE_ATTRIB );
        if( indexHandle == FML_INVALID_HANDLE )
        {
            const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "Evaluator has malformed index bind", name );
            return this;
        }

        int index = attributes.getIntAttribute( INDEX_NUMBER_ATTRIB, -1 );

        Fieldml_SetIndexEvaluator( parent->getSessionHandle(), parent->handle, index, indexHandle );
    }
    
    return this;
}


IndexEvaluatorsSaxHandler::IndexEvaluatorsSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
}


SaxHandler *IndexEvaluatorsSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, INDEX_EVALUATOR_TAG ) == 0 )
    {
        FmlObjectHandle indexHandle = attributes.getObjectAttribute( parent->getSessionHandle(), EVALUATOR_ATTRIB );
        if( indexHandle == FML_INVALID_HANDLE )
        {
            const char * name =  Fieldml_GetObjectName( parent->getSessionHandle(), parent->handle );
            parent->getSession()->logError( "Evaluator has malformed index evaluator", name );
            return this;
        }

        int index = attributes.getIntAttribute( INDEX_NUMBER_ATTRIB, -1 );

        Fieldml_SetIndexEvaluator( parent->getSessionHandle(), parent->handle, index, indexHandle );
    }
    
    return this;
}


SemidenseSaxHandler::SemidenseSaxHandler( FieldmlObjectSaxHandler *_parent, const xmlChar *elementName, SaxAttributes &attributes ) :
    ObjectMemberSaxHandler( _parent, elementName )
{
    FmlObjectHandle dataObject = attributes.getObjectAttribute( parent->getSessionHandle(), DATA_ATTRIB );

    Fieldml_SetParameterDataDescription( parent->getSessionHandle(), parent->handle, DESCRIPTION_SEMIDENSE );
    Fieldml_SetDataObject( parent->getSessionHandle(), parent->handle, dataObject );
}


SaxHandler *SemidenseSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, DENSE_INDEXES_TAG ) == 0 )
    {
        return new IndexEvaluatorListSaxHandler( this, elementName, this, 0 );
    }
    else if( xmlStrcmp( elementName, SPARSE_INDEXES_TAG ) == 0 )
    {
        return new IndexEvaluatorListSaxHandler( this, elementName, this, 1 );
    }
    
    return this;
}


IndexEvaluatorListSaxHandler::IndexEvaluatorListSaxHandler( SemidenseSaxHandler *_parent, const xmlChar *elementName, SemidenseSaxHandler *_handler, int _isSparse ) :
    SaxHandler( elementName ),
    parent( _parent ),
    handler( _handler ),
    isSparse( _isSparse )
{
}


SaxHandler *IndexEvaluatorListSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, INDEX_EVALUATOR_TAG ) == 0 )
    {
        FmlObjectHandle handle = attributes.getObjectAttribute( parent->parent->getSessionHandle(), EVALUATOR_ATTRIB );
        FmlObjectHandle orderHandle = attributes.getObjectAttribute( parent->parent->getSessionHandle(), ORDER_ATTRIB );
        if( handle == FML_INVALID_HANDLE )
        {
            parent->parent->getSession()->logError( "Invalid index in semi dense data" );
        }
        else if( isSparse )
        {
            Fieldml_AddSparseIndexEvaluator( parent->parent->getSessionHandle(), parent->parent->handle, handle );
        }
        else
        {
            Fieldml_AddDenseIndexEvaluator( parent->parent->getSessionHandle(), parent->parent->handle, handle, orderHandle );
        }
    }
    
    return this;
}


SaxHandler *IndexEvaluatorListSaxHandler::getParent()
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


IntObjectMapSaxHandler::IntObjectMapSaxHandler( SaxHandler *_parent, const xmlChar *elementName, const xmlChar *_entryTagName, const xmlChar *_entryAttribName, IntObjectMapHandler *_handler, int _mapId ) :
    SaxHandler( elementName ),
    entryTagName( _entryTagName ),
    entryAttribName( _entryAttribName ),
    parent( _parent ),
    handler( _handler ),
    mapId( _mapId )
{
}


SaxHandler *IntObjectMapSaxHandler::onElementStart( const xmlChar *elementName, SaxAttributes &attributes )
{
    if( xmlStrcmp( elementName, entryTagName ) == 0 )
    {
        int key = attributes.getIntAttribute( entryAttribName, -1 );
        FmlObjectHandle value = attributes.getObjectAttribute( getSessionHandle(), EVALUATOR_ATTRIB );
        handler->onIntObjectMapEntry( key, value, mapId );
    }
    
    return this;
}


SaxHandler *IntObjectMapSaxHandler::getParent()
{
    return parent;
}
