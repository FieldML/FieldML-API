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

#include <algorithm>

#include <string.h>

#include "String_InternalLibrary.h"

#include "fieldml_api.h"
#include "fieldml_sax.h"
#include "fieldml_structs.h"
#include "fieldml_write.h"
#include "string_const.h"

#include "DataReader.h"
#include "DataWriter.h"
#include "FieldmlRegion.h"

using namespace std;

//========================================================================
//
// Utility
//
//========================================================================


static bool checkLocal( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return false;
    }
    
    if( objectHandle == FML_INVALID_HANDLE )
    {
        //Nano-hack. Makes checking legitimate FML_INVALID_HANDLE parameters easier.
        return true;
    }
    
    if( !session->region->hasLocalObject( objectHandle, true, true ) )
    {
        session->setError( FML_ERR_NONLOCAL_OBJECT );
        return false;
    }
    
    return true;
}


static FieldmlObject *getObject( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = session->getObject( objectHandle );
    
    if( object == NULL )
    {
        session->setError( FML_ERR_UNKNOWN_OBJECT );
    }
    
    return object;
}


static FmlObjectHandle addObject( FieldmlSession *session, FieldmlObject *object )
{
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle handle = session->region->getNamedObject( object->name.c_str() );
    
    if( handle == FML_INVALID_HANDLE )
    {
        FmlObjectHandle handle = session->objects->addObject( object );
        session->region->addLocalObject( handle );
        return handle;
    }
    
    FieldmlObject *oldObject = session->objects->getObject( handle );
    
    session->logError( "Handle collision. Cannot replace", object->name.c_str(), oldObject->name.c_str() );
    delete object;
    
    session->setError( FML_ERR_NAME_COLLISION );
    
    return FML_INVALID_HANDLE;
}


static SimpleMap<FmlEnsembleValue, FmlObjectHandle> *getEvaluatorMap( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator *)object;
        return &aggregate->evaluators;
    }
    else if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator *)object;
        return &piecewise->evaluators;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getBindMap( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator *)object;
        return &aggregate->binds;
    }
    else if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator *)object;
        return &piecewise->binds;
    }
    else if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *reference = (ReferenceEvaluator *)object;
        return &reference->binds;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static vector<FmlObjectHandle> *getArgumentList( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return &referenceEvaluator->arguments;
    }
    if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregateEvaluator = (AggregateEvaluator *)object;
        return &aggregateEvaluator->arguments;
    }
    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return &parameterEvaluator->arguments;
    }
    if( object->type == FHT_ARGUMENT_EVALUATOR )
    {
        ArgumentEvaluator *argumentEvaluator = (ArgumentEvaluator *)object;
        return &argumentEvaluator->arguments;
    }
    if( object->type == FHT_EXTERNAL_EVALUATOR )
    {
        ExternalEvaluator *externalEvaluator = (ExternalEvaluator *)object;
        return &externalEvaluator->arguments;
    }
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewiseEvaluator = (PiecewiseEvaluator *)object;
        return &piecewiseEvaluator->arguments;
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SimpleMap<FmlEnsembleValue, string> *getShapeMap( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return &meshType->shapes;
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SemidenseDataDescription *getSemidenseDataDescription( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            return (SemidenseDataDescription *)parameterEvaluator->dataDescription;
        }
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static const char* cstrCopy( const string &s )
{
    if( s.length() == 0 )
    {
        //TODO Change the API so that 'no value' is indicated by an empty string.
        return NULL;
    }

    return strdupS( s.c_str() );
}

static int cappedCopy( const char *source, char *buffer, int bufferLength )
{
    if( ( bufferLength <= 1 ) || ( source == NULL ) )
    {
        return 0;
    }
    
    int length = strlen( source );
    
    if( length >= bufferLength )
    {
        length = ( bufferLength - 1 );
    }
    
    memcpy( buffer, source, length );
    buffer[length] = 0;
    
    return length;
}


static int cappedCopyAndFree( const char *source, char *buffer, int bufferLength )
{
    int length = cappedCopy( source, buffer, bufferLength );
    free( (void*)source );
    return length;
}


static DataSource *getDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type != FHT_DATA_SOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
    
    return (DataSource*)object;
}


static TextDataSource *getTextDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    DataSource *dataSource = getDataSource( session, objectHandle );
    
    if( dataSource == NULL )
    {
        return NULL;
    }

    if( dataSource->type != DATA_SOURCE_TEXT )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    TextDataSource *textSource = (TextDataSource*)dataSource;
    return textSource;
}


static DataResource *getDataResource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type != FHT_DATA_RESOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
    
    return (DataResource*)object;
}


static TextInlineDataResource *getTextInlineDataResource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    DataResource *dataResource = getDataResource( session, objectHandle );
    if( dataResource == NULL )
    {
        return NULL;
    }

    if( dataResource->type != DATA_RESOURCE_TEXT_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    TextInlineDataResource *inlineSource = (TextInlineDataResource*)dataResource;
    return inlineSource;
}


static TextFileDataResource *getTextFileDataResource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    DataResource *dataResource = getDataResource( session, objectHandle );
    
    if( dataResource == NULL )
    {
        return NULL;
    }
    
    if( dataResource->type != DATA_RESOURCE_TEXT_FILE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    TextFileDataResource *fileSource = (TextFileDataResource*)dataResource;
    return fileSource;
}


static bool checkIsValueType( FieldmlSession *session, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble, bool allowMesh )
{
    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return false;
    }
    
    if( object->type == FHT_CONTINUOUS_TYPE && allowContinuous )
    {
        return true;
    }
    else if( object->type == FHT_ENSEMBLE_TYPE && allowEnsemble )
    {
        return true;
    }
    else if( object->type == FHT_MESH_TYPE && allowMesh )
    {
        return true;
    }
    else
    {
        return false;
    }
}


static bool checkIsEvaluatorType( FieldmlSession *session, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble )
{
    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return false;
    }
    
    if( ( object->type != FHT_PARAMETER_EVALUATOR ) &&
        ( object->type != FHT_AGGREGATE_EVALUATOR ) &&
        ( object->type != FHT_REFERENCE_EVALUATOR ) &&
        ( object->type != FHT_PIECEWISE_EVALUATOR ) &&
        ( object->type != FHT_ARGUMENT_EVALUATOR ) &&
        ( object->type != FHT_EXTERNAL_EVALUATOR ) )
    {
        return false;
    }
    
    return checkIsValueType( session, Fieldml_GetValueType( session->getHandle(), objectHandle ), allowContinuous, allowEnsemble, false );
}


static bool checkIsTypeCompatible( FieldmlSession *session, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsValueType( session, objectHandle1, true, true, false ) )
    {
        return false;
    }
    if( !checkIsValueType( session, objectHandle2, true, true, false ) )
    {
        return false;
    }

    FieldmlObject *object1 = getObject( session, objectHandle1 );
    FieldmlObject *object2 = getObject( session, objectHandle2 );
    
    if( ( object1->type == FHT_ENSEMBLE_TYPE ) && ( object2->type == FHT_ENSEMBLE_TYPE ) )
    {
        return objectHandle1 == objectHandle2;
    }
    else if( ( object1->type == FHT_CONTINUOUS_TYPE ) && ( object2->type == FHT_CONTINUOUS_TYPE ) )
    {
        FmlObjectHandle component1 = Fieldml_GetTypeComponentEnsemble( session->getHandle(), objectHandle1 );
        FmlObjectHandle component2 = Fieldml_GetTypeComponentEnsemble( session->getHandle(), objectHandle2 );
        
        if( ( component1 == FML_INVALID_HANDLE ) && ( component2 == FML_INVALID_HANDLE ) )
        {
            return true;
        }
        else if( ( component1 == FML_INVALID_HANDLE ) || ( component2 == FML_INVALID_HANDLE ) )
        {
            return false;
        }
        
        return Fieldml_GetTypeComponentCount( session->getHandle(), objectHandle1 ) == Fieldml_GetTypeComponentCount( session->getHandle(), objectHandle2 );
    }
    else
    {
        return false;
    }
}


static bool checkIsEvaluatorTypeCompatible( FieldmlSession *session, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsEvaluatorType( session, objectHandle1, true, true ) )
    {
        return false;
    }
    if( !checkIsEvaluatorType( session, objectHandle2, true, true ) )
    {
        return false;
    }
    
    FmlObjectHandle typeHandle1 = Fieldml_GetValueType( session->getHandle(), objectHandle1 );
    FmlObjectHandle typeHandle2 = Fieldml_GetValueType( session->getHandle(), objectHandle2 );
    
    return checkIsTypeCompatible( session, typeHandle1, typeHandle2 );
}


//========================================================================
//
// API
//
//========================================================================

FmlSessionHandle Fieldml_CreateFromFile( const char *filename )
{
    FieldmlSession *session = new FieldmlSession();
    
    session->region = session->addResourceRegion( filename, "" );
    if( session->region == NULL )
    {
        session->setError( FML_ERR_IO_READ_ERR );
    }
    else
    {
        session->region->setRoot( getDirectory( filename ) );
        session->region->finalize();
    }
    
    return session->getHandle();
}


FmlSessionHandle Fieldml_Create( const char *location, const char *name )
{
    FieldmlSession *session = new FieldmlSession();
    
    session->region = session->addNewRegion( location, name );
    
    return session->getHandle();
}


FmlErrorNumber Fieldml_SetDebug( FmlSessionHandle handle, const int debug )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->setDebug( debug );
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlErrorNumber Fieldml_GetLastError( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    return session->getLastError();
}


FmlErrorNumber Fieldml_WriteFile( FmlSessionHandle handle, const char *filename )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( session->region == NULL )
    {
        return session->setError( FML_ERR_INVALID_REGION );
    }
        
    session->setError( FML_ERR_NO_ERROR );
    session->region->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( handle, filename );
}


void Fieldml_Destroy( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return;
    }
        
    delete session;
}


const char * Fieldml_GetRegionName( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return cstrCopy( session->region->getName() );
}


FmlErrorNumber Fieldml_FreeString( char *string )
{
    if( string != NULL )
    {
        free( string );
    }
    
    return FML_ERR_NO_ERROR;
}


int Fieldml_CopyRegionName( FmlSessionHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetRegionName( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->getErrorCount();
}


const char * Fieldml_GetError( FmlSessionHandle handle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return cstrCopy( session->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlSessionHandle handle, int index, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetError( handle, index ), buffer, bufferLength );
}


int Fieldml_GetTotalObjectCount( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getCount();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlSessionHandle handle, const int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getObjectByIndex( index );
}


int Fieldml_GetObjectCount( FmlSessionHandle handle, FieldmlHandleType type )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    if( type == FHT_UNKNOWN )
    {
        return -1;
    }

    return session->objects->getCount( type );
}


FmlObjectHandle Fieldml_GetObject( FmlSessionHandle handle, FieldmlHandleType type, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );

    FmlObjectHandle object = session->objects->getObjectByIndex( index, type );
    if( object == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
    }
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByName( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );  
        return FML_INVALID_HANDLE;
    }
        
    FmlObjectHandle object = session->region->getNamedObject( name );
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlSessionHandle handle, const char *name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );  
        return FML_INVALID_HANDLE;
    }
    
    FmlObjectHandle object = session->objects->getObjectByName( name );
    
    return object;
}


FieldmlHandleType Fieldml_GetObjectType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FHT_UNKNOWN;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FHT_UNKNOWN;
    }
    
    return object->type;
}


FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_CONTINUOUS_TYPE )
    {
        ContinuousType *continuousType = (ContinuousType*)object;
        return continuousType->componentType;
    }

    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetTypeComponentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FmlObjectHandle componentTypeHandle = Fieldml_GetTypeComponentEnsemble( handle, objectHandle );
    
    if( componentTypeHandle == FML_INVALID_HANDLE )
    {
        if( session->getLastError() == FML_ERR_NO_ERROR )
        {
            return 1;
        }
        return -1;
    }
    
    return Fieldml_GetElementCount( handle, componentTypeHandle );
}


int Fieldml_GetElementCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->count;
    }
    else if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *sequence = (ElementSequence*)object;
        return sequence->members.getCount();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetElementCount( handle, meshType->elementsType );
    }
        

    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


FmlEnsembleValue Fieldml_GetEnsembleMembersMin( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->min;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersMin( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


FmlEnsembleValue Fieldml_GetEnsembleMembersMax( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->max;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersMax( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_GetEnsembleMembersStride( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->stride;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersStride( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


EnsembleMembersType Fieldml_GetEnsembleMembersType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return MEMBER_UNKNOWN;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return MEMBER_UNKNOWN;
    }
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->type;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetEnsembleMembersType( handle, meshType->elementsType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return MEMBER_UNKNOWN;
}


FmlErrorNumber Fieldml_SetEnsembleMembersDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, EnsembleMembersType type, int count, FmlObjectHandle dataSourceHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    
    if( Fieldml_GetObjectType( handle, dataSourceHandle ) != FHT_DATA_SOURCE )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5 );
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return session->getLastError();
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;

        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        
        ensembleType->type = type;
        ensembleType->count = count;
        ensembleType->dataSource = dataSourceHandle;
        return session->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetEnsembleMembersDataSource( handle, meshType->elementsType, type, count, dataSourceHandle );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT );  
}


FmlBoolean Fieldml_IsEnsembleComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->isComponentEnsemble;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


FmlObjectHandle Fieldml_GetMeshElementsType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->elementsType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


const char * Fieldml_GetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return NULL;
    }

    return cstrCopy( map->get( elementNumber, (allowDefault == 1) ) );
}


int Fieldml_CopyMeshElementShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshElementShape( handle, objectHandle, elementNumber, allowDefault ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetMeshChartType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->chartType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshChartComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetTypeComponentEnsemble( handle, meshType->chartType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlBoolean Fieldml_IsObjectLocal( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return 0;
    }

    if( session->region->hasLocalObject( objectHandle, false, false ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


const char * Fieldml_GetObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return NULL;
    }
    
    string name = session->region->getObjectName( objectHandle );
    if( name == "" )
    {
        return NULL;
    }
    
    return cstrCopy( name.c_str() );
}


int Fieldml_CopyObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectName( handle, objectHandle ), buffer, bufferLength );
}


const char * Fieldml_GetObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    
    FieldmlObject *object = session->objects->getObject( objectHandle );
    if( object == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( object->name.c_str() );
}


int Fieldml_CopyObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectDeclaredName( handle, objectHandle ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_SetObjectInt( FmlSessionHandle handle, FmlObjectHandle objectHandle, int value )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return session->getLastError();
    }

    object->intValue = value;
    return session->getLastError();
}


int Fieldml_GetObjectInt( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return 0;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return 0;
    }

    return object->intValue;
}


FmlObjectHandle Fieldml_GetValueType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    session->setError( FML_ERR_NO_ERROR );
    
    if( object->type == FHT_PARAMETER_EVALUATOR ) 
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return parameterEvaluator->valueType;
    }
    else if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return referenceEvaluator->valueType;
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator *)object;
        return aggregate->valueType;
    }
    else if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator *)object;
        return piecewise->valueType;
    }
    else if( object->type == FHT_ARGUMENT_EVALUATOR )
    {
        ArgumentEvaluator *argumentEvaluator = (ArgumentEvaluator *)object;
        return argumentEvaluator->valueType;
    }
    else if( object->type == FHT_EXTERNAL_EVALUATOR )
    {
        ExternalEvaluator *externalEvaluator = (ExternalEvaluator *)object;
        return externalEvaluator->valueType;
    }
    else if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *sequence = (ElementSequence *)object;
        return sequence->elementType;
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateArgumentEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    //TODO Icky hack to auto-add mesh type subevaluators. Subevaluators need to either be first-class objects, or
    //specified at bind-time.
    if( Fieldml_GetObjectType( handle, valueType ) == FHT_MESH_TYPE )
    {
        FmlObjectHandle chartType = Fieldml_GetMeshChartType( handle, valueType );
        FmlObjectHandle elementsType = Fieldml_GetMeshElementsType( handle, valueType );
        
        string meshName = Fieldml_GetObjectName( handle, valueType );
        meshName += ".";
     
        string argumentName = name;
        string chartComponentName = Fieldml_GetObjectName( handle, chartType );
        string elementsComponentName = Fieldml_GetObjectName( handle, elementsType );
        
        string chartSuffix = "chart";
        if( chartComponentName.compare( 0, meshName.length(), meshName ) == 0 )
        {
            chartSuffix = chartComponentName.substr( meshName.length(), chartComponentName.length() );
        }
        string chartName = argumentName + "." + chartSuffix;
        
        if( Fieldml_GetObjectByName( handle, chartName.c_str() ) != FML_INVALID_HANDLE )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_2 );
            return FML_INVALID_HANDLE;
        }

        string elementsSuffix = "elements";
        if( elementsComponentName.compare( 0, meshName.length(), meshName ) == 0 )
        {
            elementsSuffix = elementsComponentName.substr( meshName.length(), elementsComponentName.length() );
        }
        string elementsName = argumentName + "." + elementsSuffix;
        
        if( Fieldml_GetObjectByName( handle, elementsName.c_str() ) != FML_INVALID_HANDLE )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_2 );
            return FML_INVALID_HANDLE;
        }
        
        ArgumentEvaluator *chartEvaluator = new ArgumentEvaluator( chartName.c_str(), chartType, true );
        addObject( session, chartEvaluator );        
        
        ArgumentEvaluator *elementEvaluator = new ArgumentEvaluator( elementsName.c_str(), elementsType, true );
        addObject( session, elementEvaluator );        
    }
    
    ArgumentEvaluator *argumentEvaluator = new ArgumentEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, argumentEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ExternalEvaluator *externalEvaluator = new ExternalEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, externalEvaluator );
}


FmlObjectHandle Fieldml_CreateParameterEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ParameterEvaluator *parameterEvaluator = new ParameterEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, parameterEvaluator );
}


FmlErrorNumber Fieldml_SetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return session->getLastError();
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType != DESCRIPTION_UNKNOWN )
        {
            return session->setError( FML_ERR_ACCESS_VIOLATION );
        }

        if( description == DESCRIPTION_SEMIDENSE )
        {
            delete parameterEvaluator->dataDescription;
            parameterEvaluator->dataDescription = new SemidenseDataDescription();
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_UNSUPPORTED );  
        }
    }

    return session->setError( FML_ERR_INVALID_OBJECT );
}

DataDescriptionType Fieldml_GetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return DESCRIPTION_UNKNOWN;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return DESCRIPTION_UNKNOWN;
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return parameterEvaluator->dataDescription->descriptionType;
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return DESCRIPTION_UNKNOWN;
}


FmlErrorNumber Fieldml_SetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return session->setError( FML_ERR_UNKNOWN_HANDLE );
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, dataSource ) )
    {
        return session->getLastError();
    }

    if( Fieldml_GetObjectType( handle, dataSource ) != FHT_DATA_SOURCE )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        parameterEvaluator->dataSource = dataSource;
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        EnsembleMembersType type = ensembleType->type;
        
        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        
        ensembleType->dataSource = dataSource;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetDataSource( handle, meshType->elementsType, dataSource );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_AddDenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, indexHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, orderHandle ) )
    {
        return session->getLastError();
    }

    if( !checkIsEvaluatorType( session, indexHandle, false, true ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    if( ( orderHandle != FML_INVALID_HANDLE ) && ( Fieldml_GetObjectType( handle, orderHandle ) != FHT_DATA_SOURCE ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_4 );
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( session, objectHandle );
    if( semidense == NULL )
    {
        return session->getLastError();
    }

    semidense->denseIndexes.push_back( indexHandle );
    semidense->denseOrders.push_back( orderHandle );
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_AddSparseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, indexHandle ) )
    {
        return session->getLastError();
    }


    if( !checkIsEvaluatorType( session, indexHandle, false, true ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
        
    SemidenseDataDescription *semidense = getSemidenseDataDescription( session, objectHandle );
    if( semidense == NULL )
    {
        return session->getLastError();
    }

    semidense->sparseIndexes.push_back( indexHandle );
    
    return session->getLastError();
}


int Fieldml_GetSemidenseIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isSparse )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( session, objectHandle );
    if( semidense == NULL )
    {
        return -1;
    }

    if( isSparse )
    {
        return semidense->sparseIndexes.size();
    }
    else
    {
        return semidense->denseIndexes.size();
    }
}


FmlObjectHandle Fieldml_GetSemidenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlBoolean isSparse )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( session, objectHandle );
    if( semidense == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( isSparse )
    {
        if( ( index > 0 ) && ( index <= semidense->sparseIndexes.size() ) )
        {
            return semidense->sparseIndexes[index - 1];
        }
    }
    else
    {
        if( ( index > 0 ) && ( index <= semidense->denseIndexes.size() ) )
        {
            return semidense->denseIndexes[index - 1];
        }
    }
    
    session->setError( FML_ERR_INVALID_PARAMETER_3 );
    
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    PiecewiseEvaluator *piecewiseEvaluator = new PiecewiseEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, piecewiseEvaluator );
}


FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, false, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    AggregateEvaluator *aggregateEvaluator = new AggregateEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, aggregateEvaluator );
}


FmlErrorNumber Fieldml_SetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, evaluator ) )
    {
        return session->getLastError();
    }

    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( session, evaluator, true, false ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( session, objectHandle, evaluator ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    FieldmlObject *object = getObject( session, objectHandle );
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return session->getLastError();
    }

    if( ( object->type == FHT_PIECEWISE_EVALUATOR ) || ( object->type == FHT_AGGREGATE_EVALUATOR ) )
    {
        map->setDefault( evaluator );
        return session->getLastError();
    }
    else
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
}


FmlObjectHandle Fieldml_GetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( ( object->type == FHT_PIECEWISE_EVALUATOR ) || ( object->type == FHT_AGGREGATE_EVALUATOR ) )
    {
        return map->getDefault();
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
}


FmlErrorNumber Fieldml_SetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue element, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, evaluator ) )
    {
        return session->getLastError();
    }

    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( session, evaluator, true, false ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( session, objectHandle, evaluator ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map != NULL )
    {
        map->set( element, evaluator );
    }

    return session->getLastError();
}


int Fieldml_GetEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->size();
}


FmlEnsembleValue Fieldml_GetEvaluatorElement( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetElementEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->get( elementNumber, allowDefault == 1 );
}


FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle sourceEvaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, sourceEvaluator ) )
    {
        return session->getLastError();
    }

    FmlObjectHandle valueType = Fieldml_GetValueType( handle, sourceEvaluator );

    ReferenceEvaluator *referenceEvaluator = new ReferenceEvaluator( name, sourceEvaluator, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, referenceEvaluator );
}


FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return referenceEvaluator->sourceEvaluator;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetArgumentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    vector<FmlObjectHandle> *arguments = getArgumentList( session, objectHandle );
    if( arguments == NULL )
    {
        return -1;
    }
    
    return arguments->size();
}


FmlObjectHandle Fieldml_GetArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int argumentIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    vector<FmlObjectHandle> *arguments = getArgumentList( session, objectHandle );
    if( arguments == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( argumentIndex < 1 ) || ( argumentIndex > arguments->size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return arguments->at( argumentIndex - 1 );
}


FmlErrorNumber Fieldml_AddArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, evaluatorHandle ) )
    {
        return session->getLastError();
    }

    vector<FmlObjectHandle> *arguments = getArgumentList( session, objectHandle );
    if( arguments == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    if( find( arguments->begin(), arguments->end(), evaluatorHandle ) == arguments->end() )
    {
        arguments->push_back( evaluatorHandle );
    }

    return session->getLastError();
}


int Fieldml_GetBindCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return -1;
    }
    
    return map->size();
}


FmlObjectHandle Fieldml_GetBindArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetBindEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetBindByArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    for( int i = 0; i < map->size(); i++ )
    {
        if( map->getKey( i ) == argumentHandle )
        {
            return map->getValue( i );
        }
    }

    return FML_INVALID_HANDLE;
}


FmlErrorNumber Fieldml_SetBind( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle, FmlObjectHandle sourceHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, argumentHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, sourceHandle ) )
    {
        return session->getLastError();
    }

    if( !checkIsEvaluatorTypeCompatible( session, argumentHandle, sourceHandle ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    map->set( argumentHandle, sourceHandle );
    return session->getLastError();
}



int Fieldml_GetIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return -1;
    }
    
    if( ( object->type == FHT_PIECEWISE_EVALUATOR ) || ( object->type == FHT_AGGREGATE_EVALUATOR ) )
    {
        return 1;
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        int count1, count2;
        
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
            count1 = semidense->sparseIndexes.size();
            count2 = semidense->denseIndexes.size();
            return count1 + count2;
        }
        
        session->setError( FML_ERR_UNSUPPORTED );
        return -1;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );
    return -1;
}


FmlErrorNumber Fieldml_SetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, evaluatorHandle ) )
    {
        return session->getLastError();
    }

    if( !checkIsEvaluatorType( session, evaluatorHandle, false, true ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_4 );
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return session->getLastError();
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            piecewise->indexEvaluator = evaluatorHandle;
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            aggregate->indexEvaluator = evaluatorHandle;
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        int count;
        
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
            count = semidense->sparseIndexes.size();
            
            if( index <= count )
            {
                semidense->sparseIndexes[index - 1] = evaluatorHandle;
                return session->getLastError();
            }

            index -= count;
            count = semidense->denseIndexes.size();

            if( index <= count )
            {
                semidense->denseIndexes[index - 1] = evaluatorHandle;
                return session->getLastError();
            }
            
            session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            session->setError( FML_ERR_UNSUPPORTED );
        }
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }

    return session->getLastError();
}


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            return piecewise->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            return aggregate->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        int count;
        
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
            count = semidense->sparseIndexes.size();
            
            if( index <= count )
            {
                return semidense->sparseIndexes[index - 1];
            }

            index -= count;
            count = semidense->denseIndexes.size();

            if( index <= count )
            {
                return semidense->denseIndexes[index - 1];
            }
            
            session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            session->setError( FML_ERR_UNSUPPORTED );
        }
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }

    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetSemidenseIndexOrder( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
    if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
        int count = semidense->denseIndexes.size();

        if( index <= count )
        {
            return semidense->denseOrders[index - 1];
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else
    {
        session->setError( FML_ERR_UNSUPPORTED );
    }

    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ContinuousType *continuousType = new ContinuousType( name, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, continuousType );
}


FmlObjectHandle Fieldml_CreateContinuousTypeComponents( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char *name, const int count )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        session->setError( FML_ERR_UNKNOWN_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type != FHT_CONTINUOUS_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    ContinuousType *type = (ContinuousType*)object;
    
    if( count < 1 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );
        return FML_INVALID_HANDLE;
    }
    
    string trueName = name;
    
    if( strncmp( name, "~.", 2 ) == 0 )
    {
        trueName = type->name + ( name + 1 );
    }
    
    EnsembleType *ensembleType = new EnsembleType( trueName, true, false );
    FmlObjectHandle componentHandle = addObject( session, ensembleType );
    Fieldml_SetEnsembleElementRange( handle, componentHandle, 1, count, 1 );
    
    type->componentType = componentHandle;
    
    return componentHandle;
}


FmlObjectHandle Fieldml_CreateEnsembleType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    EnsembleType *ensembleType = new EnsembleType( name, false, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    MeshType *meshType = new MeshType( name, false );

    session->setError( FML_ERR_NO_ERROR );

    return addObject( session, meshType );
}


FmlObjectHandle Fieldml_CreateMeshElementsType( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char *name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type != FHT_MESH_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    MeshType *meshType = (MeshType*)object;

    EnsembleType *ensembleType = new EnsembleType( meshType->name + "." + name, false, true );
    FmlObjectHandle elementsHandle = addObject( session, ensembleType );
    
    meshType->elementsType = elementsHandle;
    
    return elementsHandle;
}


FmlObjectHandle Fieldml_CreateMeshChartType( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char *name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type != FHT_MESH_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    MeshType *meshType = (MeshType*)object;

    ContinuousType *chartType = new ContinuousType( meshType->name + "." + name, true );
    FmlObjectHandle chartHandle = addObject( session, chartType );
    
    meshType->chartType = chartHandle;
    
    return chartHandle;
}


FmlErrorNumber Fieldml_SetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char * shape )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    map->setDefault( shape );
    return session->getLastError();
}


const char * Fieldml_GetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( map->getDefault() );
}


int Fieldml_CopyMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshDefaultShape( handle, objectHandle ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_SetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, const char * shape )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }

    map->set( elementNumber, shape );
    return session->getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    DataSource *dataSource = getDataSource( session, objectHandle );

    DataReader *reader = DataReader::create( session, session->region->getRoot().c_str(), dataSource );
    
    if( reader == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return session->addReader( reader );
}


FmlErrorNumber Fieldml_ReadIntValues( FmlSessionHandle handle, FmlReaderHandle readerHandle, int *valueBuffer, int bufferSize )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readIntValues( valueBuffer, bufferSize );
}


FmlErrorNumber Fieldml_ReadDoubleValues( FmlSessionHandle handle, FmlReaderHandle readerHandle, double *valueBuffer, int bufferSize )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readDoubleValues( valueBuffer, bufferSize );
}


FmlErrorNumber Fieldml_CloseReader( FmlSessionHandle handle, FmlReaderHandle readerHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    session->removeReader( readerHandle );
    
    delete reader;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlWriterHandle Fieldml_OpenWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean append )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    DataSource *dataSource = getDataSource( session, objectHandle );

    DataWriter *writer = DataWriter::create( session, session->region->getRoot().c_str(), dataSource, ( append == 1 ));

    if( writer == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return session->addWriter( writer );
}


FmlErrorNumber Fieldml_WriteIntValues( FmlSessionHandle handle, FmlWriterHandle writerHandle, int *valueBuffer, int valueCount )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeIntValues( valueBuffer, valueCount );
}


FmlErrorNumber Fieldml_WriteDoubleValues( FmlSessionHandle handle, FmlWriterHandle writerHandle, double *valueBuffer, int valueCount )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeDoubleValues( valueBuffer, valueCount );
}


FmlErrorNumber Fieldml_CloseWriter( FmlSessionHandle handle, FmlWriterHandle writerHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    session->removeWriter( writerHandle );

    delete writer;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlObjectHandle Fieldml_CreateEnsembleElementSequence( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, false, true, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    ElementSequence *elementSequence = new ElementSequence( name, valueType );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, elementSequence );
}


FmlErrorNumber Fieldml_SetEnsembleElementRange( FmlSessionHandle handle, FmlObjectHandle objectHandle, const FmlEnsembleValue minElement, const FmlEnsembleValue maxElement, const int stride )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return session->getLastError();
    }

    if( ( minElement < 0 ) || ( minElement > maxElement ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    if( stride < 1 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5 );
    }
    
    if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *elementSequence = (ElementSequence*)object;

        for( FmlEnsembleValue i = minElement; i <= maxElement; i += stride )
        {
            elementSequence->members.setBit( i, true );
        }
        
        return session->getLastError();
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        ensemble->type = MEMBER_RANGE;
        ensemble->min = minElement;
        ensemble->max = maxElement;
        ensemble->stride = stride;
        ensemble->count = ( maxElement - minElement ) + 1;

        return session->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_SetEnsembleElementRange( handle, meshType->elementsType, minElement, maxElement, stride );
    }

    return session->setError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_AddImportSource( FmlSessionHandle handle, const char *href, const char *name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    if( href == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );  
        return -1;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }

    FieldmlRegion *importedRegion = session->getRegion( href, name );
    if( importedRegion == NULL )
    {
        importedRegion = session->addResourceRegion( href, name );
        if( importedRegion == NULL )
        {
            return -1;
        }
    }
    
    int index = session->getRegionIndex( href, name );
    if( index < 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }
    
    session->region->addImportSource( index, href, name );
    
    return index + 1;
}


FmlObjectHandle Fieldml_AddImport( FmlSessionHandle handle, int importSourceIndex, const char *localName, const char *remoteName )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }

    if( localName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return FML_INVALID_HANDLE;
    }
    if( remoteName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );
        return FML_INVALID_HANDLE;
    }
    
    FieldmlRegion *region = session->getRegion( importSourceIndex - 1 );
    if( region == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );  
        return FML_INVALID_HANDLE;
    }
    
    FmlObjectHandle remoteObject = region->getNamedObject( remoteName );
    FmlObjectHandle localObject = session->region->getNamedObject( localName );
    
    if( remoteObject == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );  
    }
    else if( localObject != FML_INVALID_HANDLE )
    {
        remoteObject = FML_INVALID_HANDLE;
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
    }
    else
    {
        session->region->addImport( importSourceIndex - 1, localName, remoteName, remoteObject );
    }
    
    return remoteObject;
}


int Fieldml_GetImportSourceCount( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    return session->region->getImportSourceCount();
}


int Fieldml_GetImportCount( FmlSessionHandle handle, int importSourceIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    return session->region->getImportCount( importSourceIndex - 1 );
}


int Fieldml_CopyImportSourceHref( FmlSessionHandle handle, int importSourceIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    string href = session->region->getImportSourceHref( importSourceIndex - 1 );
    if( href == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return -1;
    }
    
    return cappedCopy( href.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportSourceRegionName( FmlSessionHandle handle, int importSourceIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    string regionName = session->region->getImportSourceRegionName( importSourceIndex - 1 );
    if( regionName == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return -1;
    }
    
    return cappedCopy( regionName.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportLocalName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    string localName = session->region->getImportLocalName( importSourceIndex - 1, importIndex );
    if( localName == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return -1;
    }
    
    return cappedCopy( localName.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportRemoteName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return -1;
    }
    
    string remoteName = session->region->getImportRemoteName( importSourceIndex - 1, importIndex );
    if( remoteName == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return -1;
    }
    
    return cappedCopy( remoteName.c_str(), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetImportObject( FmlSessionHandle handle, int importSourceIndex, int importIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }

    return session->region->getImportObject( importSourceIndex - 1, importIndex );
}


FmlObjectHandle Fieldml_CreateTextFileDataResource( FmlSessionHandle handle, const char * name, const char * href )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new TextFileDataResource( name, href );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, dataResource );
}


FmlObjectHandle Fieldml_CreateTextInlineDataResource( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new TextInlineDataResource( name );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, dataResource );
}


DataResourceType Fieldml_GetDataResourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return DATA_RESOURCE_UNKNOWN;
    }
    
    FieldmlObject *object = getObject( session, objectHandle );
    if( object->type != FHT_DATA_RESOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return DATA_RESOURCE_UNKNOWN;
    }
    
    DataResource *resource = (DataResource*)object;
    return resource->type;
}


FmlErrorNumber Fieldml_AddInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char *data, const int length )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    TextInlineDataResource *source = getTextInlineDataResource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    char *newString = new char[source->length + length + 1];
    memcpy( newString, source->inlineString, source->length );
    memcpy( newString + source->length, data, length );

    delete[] source->inlineString;
    source->inlineString = newString;
    
    source->length += length;
    newString[source->length] = 0;
    
    return session->getLastError();
}


int Fieldml_GetInlineDataLength( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    TextInlineDataResource *source = getTextInlineDataResource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->length;
}


const char * Fieldml_GetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    TextInlineDataResource *source = getTextInlineDataResource( session, objectHandle );
    if( source == NULL )
    {
        return NULL;
    }
    
    return strdupS( source->inlineString );
}


int Fieldml_CopyInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    TextInlineDataResource *source = getTextInlineDataResource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    if( offset >= source->length )
    {
        return 0;
    }
    
    return cappedCopy( source->inlineString + offset, buffer, bufferLength );
}


DataSourceType Fieldml_GetDataSourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return DATA_SOURCE_UNKNOWN;
    }
    
    DataSource *dataSource = getDataSource( session, objectHandle );
    if( dataSource == NULL )
    {
        return DATA_SOURCE_UNKNOWN;
    }
    
    return dataSource->type;
}


const char * Fieldml_GetDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    TextFileDataResource *source = getTextFileDataResource( session, objectHandle );
    if( source == NULL )
    {
        return NULL;
    }

    return cstrCopy( source->href );
}


int Fieldml_CopyDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetDataResourceHref( handle, objectHandle ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    
    FmlObjectHandle dataSourceHandle;

    if( object == NULL )
    {
        dataSourceHandle = FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        dataSourceHandle = parameterEvaluator->dataSource;
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        EnsembleMembersType type = ensembleType->type;
        
        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
        
        return ensembleType->dataSource;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetDataSource( handle, meshType->elementsType );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        dataSourceHandle = FML_INVALID_HANDLE;
    }
    
    return dataSourceHandle;;
}


int Fieldml_GetDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return -1;
    }
    
    return resource->dataSources.size();
}


FmlObjectHandle Fieldml_GetDataSourceByIndex( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( index < 0 ) || ( index >= resource->dataSources.size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return resource->dataSources[index];
}


FmlObjectHandle Fieldml_GetDataSourceResource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }
    
    DataSource *source = getDataSource( session, objectHandle );
    if( source == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( source->resource == NULL )
    {
        return FML_ERR_MISCONFIGURED_OBJECT;
    }
    
    return session->region->getNamedObject( source->resource->name );
}


FmlErrorNumber Fieldml_CreateTextDataSource( FmlSessionHandle handle, const char *name, FmlObjectHandle resource, int firstLine, int count, int length, int head, int tail )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    if( !checkLocal( session, resource ) )
    {
        return session->getLastError();
    }
    
    FieldmlObject *object = getObject( session, resource );
    if( object->type != FHT_DATA_RESOURCE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    DataResource *dataResource = (DataResource*)object;
    if( ( dataResource->type != DATA_RESOURCE_TEXT_FILE ) && ( dataResource->type != DATA_RESOURCE_TEXT_INLINE ) )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    if( firstLine <= 0 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    if( count <= 0 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_4 );
    }
    if( length <= 0 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5 );
    }
    if( head <= -1 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_6 );
    }
    if( tail <= -1 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_7 );
    }
    
    DataSource *source = new TextDataSource( name, dataResource, firstLine, count, length, head, tail );

    session->setError( FML_ERR_NO_ERROR );
    FmlObjectHandle sourceHandle = addObject( session, source );
    
    dataResource->dataSources.push_back( sourceHandle );
    
    return sourceHandle;
}


int Fieldml_GetTextDataSourceFirstLine( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    TextDataSource *source = getTextDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }

    return source->firstLine;
}


int Fieldml_GetTextDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    TextDataSource *source = getTextDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->count;
}


int Fieldml_GetTextDataSourceLength( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    TextDataSource *source = getTextDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->length;
}


int Fieldml_GetTextDataSourceHead( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    TextDataSource *source = getTextDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->head;
}


int Fieldml_GetTextDataSourceTail( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    TextDataSource *source = getTextDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->tail;
}
