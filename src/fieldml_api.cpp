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

#include "fieldml_library_0.3.h"

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


static SimpleMap<int, FmlObjectHandle> *getEvaluatorMap( FieldmlSession *session, FmlObjectHandle objectHandle )
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


static vector<FmlObjectHandle> *getVariableList( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return &referenceEvaluator->variables;
    }
    if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregateEvaluator = (AggregateEvaluator *)object;
        return &aggregateEvaluator->variables;
    }
    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return &parameterEvaluator->variables;
    }
    if( object->type == FHT_ABSTRACT_EVALUATOR )
    {
        AbstractEvaluator *abstractEvaluator = (AbstractEvaluator *)object;
        return &abstractEvaluator->variables;
    }
    if( object->type == FHT_EXTERNAL_EVALUATOR )
    {
        ExternalEvaluator *externalEvaluator = (ExternalEvaluator *)object;
        return &externalEvaluator->variables;
    }
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewiseEvaluator = (PiecewiseEvaluator *)object;
        return &piecewiseEvaluator->variables;
    }

    session->setError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SimpleMap<int, string> *getShapeMap( FieldmlSession *session, FmlObjectHandle objectHandle )
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
    int length;
    
    if( ( bufferLength <= 0 ) || ( source == NULL ) )
    {
        return 0;
    }
    
    length = strlen( source );
    
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


static DataObject *getDataObject( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type != FHT_DATA_OBJECT )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
    
    return (DataObject*)object;
}


static InlineDataSource *getInlineDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    DataObject *dataObject = getDataObject( session, objectHandle );
    
    if( dataObject == NULL )
    {
        return NULL;
    }

    if( dataObject->source->sourceType != SOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    InlineDataSource *inlineSource = (InlineDataSource*)dataObject->source;
    return inlineSource;
}


static TextFileDataSource *getTextFileDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type != FHT_DATA_OBJECT )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
    
    DataObject *dataObject = (DataObject*)object;
    
    if( dataObject->source->sourceType != SOURCE_TEXT_FILE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    TextFileDataSource *fileSource = (TextFileDataSource*)dataObject->source;
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
        ( object->type != FHT_ABSTRACT_EVALUATOR ) &&
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

FmlHandle Fieldml_CreateFromFile( const char *filename )
{
    FieldmlSession *session = new FieldmlSession();
    
    session->region = session->addRegion( filename, "" );
    int err = session->readRegion( session->region );
    session->region->setRoot( getDirectory( filename ) );

    if( err != 0 )
    {
        delete session;
        return FML_INVALID_HANDLE;
    }
    session->region->finalize();
    
    return session->getHandle();
}


FmlHandle Fieldml_Create( const char *location, const char *name )
{
    FieldmlSession *session = new FieldmlSession();
    
    session->region = session->addRegion( location, name );
    
    return session->getHandle();
}


int Fieldml_SetDebug( FmlHandle handle, const int debug )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->setDebug( debug );
    
    return session->setError( FML_ERR_NO_ERROR );
}


int Fieldml_GetLastError( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    return session->getLastError();
}


int Fieldml_WriteFile( FmlHandle handle, const char *filename )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    session->region->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( handle, filename );
}


void Fieldml_Destroy( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return;
    }
        
    delete session;
}


const char * Fieldml_GetRegionName( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return cstrCopy( session->region->getName() );
}


int Fieldml_CopyRegionName( FmlHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetRegionName( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->getErrorCount();
}


const char * Fieldml_GetError( FmlHandle handle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return cstrCopy( session->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlHandle handle, int index, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetError( handle, index ), buffer, bufferLength );
}


int Fieldml_GetTotalObjectCount( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getCount();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlHandle handle, const int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getObjectByIndex( index );
}


int Fieldml_GetObjectCount( FmlHandle handle, FieldmlHandleType type )
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


FmlObjectHandle Fieldml_GetObject( FmlHandle handle, FieldmlHandleType type, int index )
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


FmlObjectHandle Fieldml_GetObjectByName( FmlHandle handle, const char * name )
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
        
    FmlObjectHandle object = session->region->getNamedObject( name );
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlHandle handle, const char *name )
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


FieldmlHandleType Fieldml_GetObjectType( FmlHandle handle, FmlObjectHandle objectHandle )
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


FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle )
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
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        return FML_INVALID_HANDLE;
    }

    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetTypeComponentCount( FmlHandle handle, FmlObjectHandle objectHandle )
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


int Fieldml_GetElementCount( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetElementCount( handle, meshType->elementType );
    }
        

    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_GetEnsembleMembersMin( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetEnsembleMembersMin( handle, meshType->elementType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_GetEnsembleMembersMax( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetEnsembleMembersMax( handle, meshType->elementType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_GetEnsembleMembersStride( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetEnsembleMembersStride( handle, meshType->elementType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


EnsembleMembersType Fieldml_GetEnsembleMembersType( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetEnsembleMembersType( handle, meshType->elementType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return MEMBER_UNKNOWN;
}


int Fieldml_SetEnsembleMembersData( FmlHandle handle, FmlObjectHandle objectHandle, EnsembleMembersType type, int count, FmlObjectHandle dataObjectHandle )
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
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;

        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        
        ensembleType->type = type;
        ensembleType->count = count;
        ensembleType->dataObject = dataObjectHandle;
        return session->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetEnsembleMembersData( handle, meshType->elementType, type, count, dataObjectHandle );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT );  
}


int Fieldml_IsEnsembleComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
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


FmlObjectHandle Fieldml_GetMeshElementType( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return meshType->elementType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


const char * Fieldml_GetMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    SimpleMap<int, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return NULL;
    }

    return cstrCopy( map->get( elementNumber, (allowDefault == 1) ) );
}


int Fieldml_CopyMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshElementShape( handle, objectHandle, elementNumber, allowDefault ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetMeshXiType( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return meshType->xiType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshXiComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
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
        return Fieldml_GetTypeComponentEnsemble( handle, meshType->xiType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_IsObjectLocal( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    if( session->region->hasLocalObject( objectHandle, false ) )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


const char * Fieldml_GetObjectName( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    
    string name = session->region->getObjectName( objectHandle );
    if( name == "" )
    {
        return NULL;
    }
    
    return cstrCopy( name.c_str() );
}


int Fieldml_CopyObjectName( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectName( handle, objectHandle ), buffer, bufferLength );
}


const char * Fieldml_GetObjectDeclaredName( FmlHandle handle, FmlObjectHandle objectHandle )
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


int Fieldml_CopyObjectDeclaredName( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectDeclaredName( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_SetObjectInt( FmlHandle handle, FmlObjectHandle objectHandle, int value )
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


int Fieldml_GetObjectInt( FmlHandle handle, FmlObjectHandle objectHandle )
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


FmlObjectHandle Fieldml_GetValueType( FmlHandle handle, FmlObjectHandle objectHandle )
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
    else if( object->type == FHT_ABSTRACT_EVALUATOR )
    {
        AbstractEvaluator *abstractEvaluator = (AbstractEvaluator *)object;
        return abstractEvaluator->valueType;
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


FmlObjectHandle Fieldml_CreateAbstractEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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
        string xiName, elementName;
        
        xiName = name;
        xiName += ".xi";
        
        elementName = name;
        elementName += ".element";
        
        if( Fieldml_GetObjectByName( handle, xiName.c_str() ) != FML_INVALID_HANDLE )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_2 );
            return FML_INVALID_HANDLE;
        }
        if( Fieldml_GetObjectByName( handle, elementName.c_str() ) != FML_INVALID_HANDLE )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_2 );
            return FML_INVALID_HANDLE;
        }
        
        FmlObjectHandle xiType = Fieldml_GetMeshXiType( handle, valueType );
        AbstractEvaluator *xiEvaluator = new AbstractEvaluator( xiName.c_str(), xiType, true );
        addObject( session, xiEvaluator );        
        
        FmlObjectHandle elementType = Fieldml_GetMeshElementType( handle, valueType );
        AbstractEvaluator *elementEvaluator = new AbstractEvaluator( elementName.c_str(), elementType, true );
        addObject( session, elementEvaluator );        
    }
    
    AbstractEvaluator *abstractEvaluator = new AbstractEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, abstractEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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


FmlObjectHandle Fieldml_CreateParametersEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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


int Fieldml_SetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description )
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

DataDescriptionType Fieldml_GetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle )
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


int Fieldml_SetDataObject( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataObject )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return session->setError( FML_ERR_UNKNOWN_HANDLE );
    }

    if( Fieldml_GetObjectType( handle, dataObject ) != FHT_DATA_OBJECT )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        parameterEvaluator->dataObject = dataObject;
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        EnsembleMembersType type = ensembleType->type;
        
        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
        
        ensembleType->dataObject = dataObject;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetDataObject( handle, meshType->elementType, dataObject );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    return session->getLastError();
}


int Fieldml_AddDenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorType( session, indexHandle, false, true ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    if( ( orderHandle != FML_INVALID_HANDLE ) && ( Fieldml_GetObjectType( handle, orderHandle ) != FHT_DATA_OBJECT ) )
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


int Fieldml_AddSparseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
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


int Fieldml_GetSemidenseIndexCount( FmlHandle handle, FmlObjectHandle objectHandle, int isSparse )
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


FmlObjectHandle Fieldml_GetSemidenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index, int isSparse )
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


FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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


FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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


int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
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
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
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


int Fieldml_GetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
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


int Fieldml_SetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int element, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
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

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map != NULL )
    {
        map->set( element, evaluator );
    }

    return session->getLastError();
}


int Fieldml_GetEvaluatorCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->size();
}


int Fieldml_GetEvaluatorElement( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetElementEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->get( elementNumber, allowDefault == 1 );
}


FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlHandle handle, const char * name, FmlObjectHandle sourceEvaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle valueType = Fieldml_GetValueType( handle, sourceEvaluator );

    ReferenceEvaluator *referenceEvaluator = new ReferenceEvaluator( name, sourceEvaluator, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, referenceEvaluator );
}


FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
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


int Fieldml_GetVariableCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    vector<FmlObjectHandle> *variables = getVariableList( session, objectHandle );
    if( variables == NULL )
    {
        return -1;
    }
    
    return variables->size();
}


FmlObjectHandle Fieldml_GetVariable( FmlHandle handle, FmlObjectHandle objectHandle, int variableIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    vector<FmlObjectHandle> *variables = getVariableList( session, objectHandle );
    if( variables == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( variableIndex < 1 ) || ( variableIndex > variables->size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return variables->at( variableIndex - 1 );
}


int Fieldml_AddVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    vector<FmlObjectHandle> *variables = getVariableList( session, objectHandle );
    if( variables == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    if( find( variables->begin(), variables->end(), evaluatorHandle ) == variables->end() )
    {
        variables->push_back( evaluatorHandle );
    }

    return session->getLastError();
}


int Fieldml_GetBindCount( FmlHandle handle, FmlObjectHandle objectHandle )
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


FmlObjectHandle Fieldml_GetBindVariable( FmlHandle handle, FmlObjectHandle objectHandle, int index )
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


FmlObjectHandle Fieldml_GetBindEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
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


FmlObjectHandle Fieldml_GetBindByVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle )
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
        if( map->getKey( i ) == variableHandle )
        {
            return map->getValue( i );
        }
    }

    return FML_INVALID_HANDLE;
}


int Fieldml_SetBind( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle, FmlObjectHandle sourceHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorTypeCompatible( session, variableHandle, sourceHandle ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    map->set( variableHandle, sourceHandle );
    return session->getLastError();
}



int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle )
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


int Fieldml_SetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
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


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
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


int Fieldml_GetSemidenseIndexOrder( FmlHandle handle, FmlObjectHandle objectHandle, int index )
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


FmlObjectHandle Fieldml_CreateContinuousType( FmlHandle handle, const char * name, FmlObjectHandle componentDescriptionHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( ( componentDescriptionHandle != FML_INVALID_HANDLE ) )
    {
        if( !checkIsValueType( session, componentDescriptionHandle, false, true, false ) )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }
    
    if( componentDescriptionHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_IsEnsembleComponentType( handle, componentDescriptionHandle ) != 1 )
        {
            session->setError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }

    ContinuousType *continuousType = new ContinuousType( name, componentDescriptionHandle, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, continuousType );
}


FmlObjectHandle Fieldml_CreateEnsembleType( FmlHandle handle, const char * name, const int isComponentType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    EnsembleType *ensembleType = new EnsembleType( name, isComponentType == 1, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlHandle handle, const char * name, FmlObjectHandle xiEnsembleDescription )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle xiHandle, elementHandle;
    string xiName, elementsName;

    if( ( xiEnsembleDescription == FML_INVALID_HANDLE ) ||
        ( Fieldml_GetObjectType( handle, xiEnsembleDescription ) != FHT_ENSEMBLE_TYPE ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    xiName = name;
    xiName += ".xi";
    if( Fieldml_GetObjectByName( handle, xiName.c_str() ) != FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    elementsName = name;
    elementsName += ".elements";
    if( Fieldml_GetObjectByName( handle, elementsName.c_str() ) != FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    
    ContinuousType *xiObject = new ContinuousType( xiName.c_str(), xiEnsembleDescription, true );
    xiHandle = addObject( session, xiObject );
    
    EnsembleType *elementObject = new EnsembleType( elementsName.c_str(), false, true );
    elementHandle = addObject( session, elementObject );
    
    MeshType *meshType = new MeshType( name, xiHandle, elementHandle, false );

    session->setError( FML_ERR_NO_ERROR );

    return addObject( session, meshType );
}


int Fieldml_SetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle, const char * shape )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SimpleMap<int, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    map->setDefault( shape );
    return session->getLastError();
}


const char * Fieldml_GetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    SimpleMap<int, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( map->getDefault() );
}


int Fieldml_CopyMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshDefaultShape( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_SetMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, const char * shape )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SimpleMap<int, string> *map = getShapeMap( session, objectHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }

    map->set( elementNumber, shape );
    return session->getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );

    return DataReader::create( session, session->region->getRoot().c_str(), dataObject );
}


int Fieldml_ReadIntValues( FmlHandle handle, FmlReaderHandle readerHandle, int *valueBuffer, int bufferSize )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = DataReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readIntValues( valueBuffer, bufferSize );
}


int Fieldml_ReadDoubleValues( FmlHandle handle, FmlReaderHandle readerHandle, double *valueBuffer, int bufferSize )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = DataReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readDoubleValues( valueBuffer, bufferSize );
}


int Fieldml_CloseReader( FmlHandle handle, FmlReaderHandle readerHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataReader *reader = DataReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    delete reader;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlWriterHandle Fieldml_OpenWriter( FmlHandle handle, FmlObjectHandle objectHandle, int append )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );

    return DataWriter::create( session, session->region->getRoot().c_str(), dataObject, ( append == 1 ));
}


int Fieldml_WriteIntValues( FmlHandle handle, FmlWriterHandle writerHandle, int *valueBuffer, int valueCount )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = DataWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeIntValues( valueBuffer, valueCount );
}


int Fieldml_WriteDoubleValues( FmlHandle handle, FmlWriterHandle writerHandle, double *valueBuffer, int valueCount )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = DataWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeDoubleValues( valueBuffer, valueCount );
}


int Fieldml_CloseWriter( FmlHandle handle, FmlWriterHandle writerHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataWriter *writer = DataWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    delete writer;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlObjectHandle Fieldml_CreateEnsembleElementSequence( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ElementSequence *elementSequence = new ElementSequence( name, valueType );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, elementSequence );
}


int Fieldml_SetEnsembleElementRange( FmlHandle handle, FmlObjectHandle objectHandle, const int minElement, const int maxElement, const int stride )
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

        for( int i = minElement; i <= maxElement; i += stride )
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
        return Fieldml_SetEnsembleElementRange( handle, meshType->elementType, minElement, maxElement, stride );
    }

    return session->setError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_AddImportSource( FmlHandle handle, const char *location, const char *name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    if( location == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );  
        return -1;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }

    FieldmlRegion *importedRegion = session->addRegion( location, name );
    int err = session->readRegion( importedRegion );
    if( err != 0 )
    {
        delete importedRegion;
        return -1;
    }
    
    int index = session->getRegionIndex( location, name );
    if( index < 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }
    
    session->region->addImportSource( index, location, name );
    
    return index + 1;
}


int Fieldml_AddImport( FmlHandle handle, int importSourceIndex, const char *localName, const char *remoteName )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
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
    
    FmlObjectHandle object = region->getNamedObject( remoteName );
    
    if( object == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );  
    }
    else
    {
        session->region->addImport( importSourceIndex - 1, localName, remoteName, object );
    }
    
    return object;
}


int Fieldml_GetImportSourceCount( FmlHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    return session->region->getImportSourceCount();
}


int Fieldml_GetImportCount( FmlHandle handle, int importSourceIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    return session->region->getImportCount( importSourceIndex - 1 );
}


int Fieldml_CopyImportSourceLocation( FmlHandle handle, int importSourceIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    string location = session->region->getImportSourceLocation( importSourceIndex - 1 );
    if( location == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return -1;
    }
    
    return cappedCopy( location.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportSourceRegionName( FmlHandle handle, int importSourceIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
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


int Fieldml_CopyImportLocalName( FmlHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
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


int Fieldml_CopyImportRemoteName( FmlHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
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


int Fieldml_GetImportObject( FmlHandle handle, int importSourceIndex, int importIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    return session->region->getImportObject( importSourceIndex - 1, importIndex );
}


FmlObjectHandle Fieldml_CreateDataObject( FmlHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    DataObject *dataObject = new DataObject( name );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, dataObject );
}


int Fieldml_AddInlineData( FmlHandle handle, FmlObjectHandle objectHandle, const char *data, const int length )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    InlineDataSource *source = getInlineDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    char *newData = new char[source->length + length + 1];
    memcpy( newData, source->data, source->length );
    memcpy( newData + source->length, data, length );

    delete[] source->data;
    source->data = newData;
    
    source->length += length;
    newData[source->length] = 0;
    
    return session->getLastError();
}


int Fieldml_GetInlineDataLength( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    InlineDataSource *source = getInlineDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    return source->length;
}


const char * Fieldml_GetInlineData( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    InlineDataSource *source = getInlineDataSource( session, objectHandle );
    if( source == NULL )
    {
        return NULL;
    }
    
    return strdupS( source->data );
}


int Fieldml_CopyInlineData( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    InlineDataSource *source = getInlineDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }
    
    if( offset >= source->length )
    {
        return 0;
    }
    
    return cappedCopy( source->data + offset, buffer, bufferLength );
}


DataSourceType Fieldml_GetDataObjectSourceType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return SOURCE_UNKNOWN;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return SOURCE_UNKNOWN;
    }
    
    return dataObject->source->sourceType;
}


int Fieldml_SetDataObjectSourceType( FmlHandle handle, FmlObjectHandle objectHandle, DataSourceType source )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return session->getLastError();
    }

    if( source == SOURCE_INLINE )
    {
        delete dataObject->source;
        dataObject->source = new InlineDataSource();
        return session->setError( FML_ERR_NO_ERROR );
    }
    else if( source == SOURCE_TEXT_FILE )
    {
        delete dataObject->source;
        dataObject->source = new TextFileDataSource();
        return session->setError( FML_ERR_NO_ERROR );
    }
    else
    {
        return session->setError( FML_ERR_UNSUPPORTED );
    }
}


int Fieldml_SetDataObjectTextFileInfo( FmlHandle handle, FmlObjectHandle objectHandle, const char * filename, int lineOffset )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    TextFileDataSource *source = getTextFileDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    source->filename = filename;
    source->lineOffset = lineOffset;
    
    return session->getLastError();
}


const char * Fieldml_GetDataObjectFilename( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    TextFileDataSource *source = getTextFileDataSource( session, objectHandle );
    if( source == NULL )
    {
        return NULL;
    }

    return cstrCopy( source->filename );
}


int Fieldml_CopyDataObjectFilename( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetDataObjectFilename( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_GetDataObjectFileOffset( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    TextFileDataSource *source = getTextFileDataSource( session, objectHandle );
    if( source == NULL )
    {
        return -1;
    }

    return source->lineOffset;
}


FmlObjectHandle Fieldml_GetDataObject( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    
    FmlObjectHandle dataObjectHandle;

    if( object == NULL )
    {
        dataObjectHandle = FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        dataObjectHandle = parameterEvaluator->dataObject;
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        EnsembleMembersType type = ensembleType->type;
        
        if( ( type != MEMBER_LIST_DATA ) && ( type != MEMBER_RANGE_DATA ) && ( type != MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
        
        return ensembleType->dataObject;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetDataObject( handle, meshType->elementType );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        dataObjectHandle = FML_INVALID_HANDLE;
    }
    
    return dataObjectHandle;;
}


int Fieldml_SetDataObjectEntryInfo( FmlHandle handle, FmlObjectHandle objectHandle, int count, int length, int head, int tail )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return session->getLastError();
    }
    
    dataObject->entryCount = count;
    dataObject->entryLength = length;
    dataObject->entryHead = head;
    dataObject->entryTail = tail;
    
    return session->getLastError();
}


int Fieldml_GetDataObjectEntryCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return -1;
    }
    
    return dataObject->entryCount;
}


int Fieldml_GetDataObjectEntryLength( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return -1;
    }
    
    return dataObject->entryLength;
}


int Fieldml_GetDataObjectEntryHead( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return -1;
    }
    
    return dataObject->entryHead;
}


int Fieldml_GetDataObjectEntryTail( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    DataObject *dataObject = getDataObject( session, objectHandle );
    if( dataObject == NULL )
    {
        return -1;
    }
    
    return dataObject->entryTail;
}
