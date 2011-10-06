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

#include <cstring>

#include "String_InternalLibrary.h"

#include "fieldml_api.h"
#include "FieldmlSession.h"
#include "fieldml_structs.h"
#include "fieldml_write.h"
#include "string_const.h"
#include "Util.h"

#include "ArrayDataReader.h"

#include "ArrayDataWriter.h"
#include "FieldmlRegion.h"

using namespace std;

//========================================================================
//
// Utility
//
//========================================================================

#define getSession( handle ) getSessionAndSetContext( handle, __FILE__, __LINE__ )

static FieldmlSession *getSessionAndSetContext( FmlSessionHandle handle, char * file, int line )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session != NULL )
    {
        session->setErrorContext( file, line );
        session->setError( FML_ERR_NO_ERROR );
    }
    return session;
}


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


static vector<FmlObjectHandle> getArgumentList( FieldmlSession *session, FmlObjectHandle objectHandle, bool isUnbound, bool isUsed )
{
    FieldmlObject *object = getObject( session, objectHandle );

    vector<FmlObjectHandle> args;

    if( object == NULL )
    {
        session->setError( FML_ERR_UNKNOWN_OBJECT );
        return args;
    }
    
    if( ( object->type != FHT_AGGREGATE_EVALUATOR ) &&
        ( object->type != FHT_ARGUMENT_EVALUATOR ) &&
        ( object->type != FHT_EXTERNAL_EVALUATOR ) &&
        ( object->type != FHT_PARAMETER_EVALUATOR ) &&
        ( object->type != FHT_PIECEWISE_EVALUATOR ) &&
        ( object->type != FHT_REFERENCE_EVALUATOR ) )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return args;
    }
        

    if ( !isUnbound && !isUsed )
    {
        //Always an empty set with the current algorithm, as it only tracks unbound or used arguments.
        return args;
    }
    if( isUnbound && !isUsed )
    {
        //Always an empty set with the current algorithm, as it assumes that arguments of arguments are used.
        return args;
    }
    
    set<FmlObjectHandle> unbound, used;
    session->getArguments( objectHandle, unbound, used, false );
    
    if( !isUnbound && isUsed )
    {
        used.erase( unbound.begin(), unbound.end() );
        for( set<FmlObjectHandle>::const_iterator i = used.begin(); i != used.end(); i++ )
        {
            args.push_back( *i );
        }
        return args;
    }
    else //if( isUnbound && isUsed )
    {
        //In used, and in unbound. Unbound is always is a subset of used with the current algorithm.
        for( set<FmlObjectHandle>::const_iterator i = unbound.begin(); i != unbound.end(); i++ )
        {
            args.push_back( *i );
        }
        return args;
    }
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


static bool checkCyclicDependency( FieldmlSession *session, FmlObjectHandle objectHandle, FmlObjectHandle objectDependancy )
{
    set<FmlObjectHandle> delegates;
    session->getDelegateEvaluators( objectDependancy, delegates );
    if( FmlUtil::contains( delegates, objectHandle ) )
    {
        session->setError( FML_ERR_CYCLIC_DEPENDENCY );
        return false;
    }
    
    return true;
}


static char* cstrCopy( const string &s )
{
    if( s.length() == 0 )
    {
        //TODO Change the API so that 'no value' is indicated by an empty string.
        return NULL;
    }

    return strdupS( s.c_str() );
}

static int cappedCopy( const char * source, char * buffer, int bufferLength )
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


static int cappedCopyAndFree( const char * source, char * buffer, int bufferLength )
{
    int length = cappedCopy( source, buffer, bufferLength );
    free( (void*)source );
    return length;
}


static DataSource *objectAsDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
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


static ArrayDataSource *getArrayDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    
    if( dataSource == NULL )
    {
        return NULL;
    }

    if( dataSource->type != DATA_SOURCE_ARRAY )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    ArrayDataSource *arraySource = (ArrayDataSource*)dataSource;
    return arraySource;
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
    
    return checkIsValueType( session, Fieldml_GetValueType( session->getSessionHandle(), objectHandle ), allowContinuous, allowEnsemble, false );
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
        FmlObjectHandle component1 = Fieldml_GetTypeComponentEnsemble( session->getSessionHandle(), objectHandle1 );
        FmlObjectHandle component2 = Fieldml_GetTypeComponentEnsemble( session->getSessionHandle(), objectHandle2 );
        
        if( ( component1 == FML_INVALID_HANDLE ) && ( component2 == FML_INVALID_HANDLE ) )
        {
            return true;
        }
        else if( ( component1 == FML_INVALID_HANDLE ) || ( component2 == FML_INVALID_HANDLE ) )
        {
            return false;
        }
        
        return Fieldml_GetTypeComponentCount( session->getSessionHandle(), objectHandle1 ) == Fieldml_GetTypeComponentCount( session->getSessionHandle(), objectHandle2 );
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
    
    FmlObjectHandle typeHandle1 = Fieldml_GetValueType( session->getSessionHandle(), objectHandle1 );
    FmlObjectHandle typeHandle2 = Fieldml_GetValueType( session->getSessionHandle(), objectHandle2 );
    
    return checkIsTypeCompatible( session, typeHandle1, typeHandle2 );
}


//========================================================================
//
// API
//
//========================================================================

FmlSessionHandle Fieldml_CreateFromFile( const char * filename )
{
    FieldmlSession *session = new FieldmlSession();
    session->setErrorContext( __FILE__, __LINE__ );
    session->setError( FML_ERR_NO_ERROR );
    
    if( filename == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_1 );
    }
    else
    {
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
    }
    
    return session->getSessionHandle();
}


FmlSessionHandle Fieldml_Create( const char * location, const char * name )
{
    FieldmlSession *session = new FieldmlSession();
    session->setErrorContext( __FILE__, __LINE__ );
    session->setError( FML_ERR_NO_ERROR );
    
    if( location == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_1 );
    }
    else if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
    }
    else
    {
        session->region = session->addNewRegion( location, name );
    }
    
    return session->getSessionHandle();
}


FmlErrorNumber Fieldml_SetDebug( FmlSessionHandle handle, int debug )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->setDebug( debug );
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlErrorNumber Fieldml_GetLastError( FmlSessionHandle handle )
{
    //Bypass the error state reset in getSession.
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    return session->getLastError();
}


FmlErrorNumber Fieldml_WriteFile( FmlSessionHandle handle, const char * filename )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( session->region == NULL )
    {
        return session->setError( FML_ERR_INVALID_REGION );
    }
    if( filename == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_2 );
    }
        
    session->setError( FML_ERR_NO_ERROR );
    session->region->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( session, handle, filename );
}


void Fieldml_Destroy( FmlSessionHandle handle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return;
    }
        
    delete session;
}


char * Fieldml_GetRegionName( FmlSessionHandle handle )
{
    FieldmlSession *session = getSession( handle );
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


FmlErrorNumber Fieldml_FreeString( char * string )
{
    if( string != NULL )
    {
        free( string );
    }
    
    return FML_ERR_NO_ERROR;
}


int Fieldml_CopyRegionName( FmlSessionHandle handle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetRegionName( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlSessionHandle handle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->getErrorCount();
}


char * Fieldml_GetError( FmlSessionHandle handle, int index )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return cstrCopy( session->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlSessionHandle handle, int errorIndex, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetError( handle, errorIndex ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_ClearErrors( FmlSessionHandle handle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->clearErrors();
    return session->setError( FML_ERR_NO_ERROR );
}


int Fieldml_GetTotalObjectCount( FmlSessionHandle handle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getCount();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlSessionHandle handle, const int objectIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );
    return session->objects->getObjectByIndex( objectIndex );
}


int Fieldml_GetObjectCount( FmlSessionHandle handle, FieldmlHandleType type )
{
    FieldmlSession *session = getSession( handle );
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


FmlObjectHandle Fieldml_GetObject( FmlSessionHandle handle, FieldmlHandleType objectType, int objectIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR );

    FmlObjectHandle object = session->objects->getObjectByIndex( objectIndex, objectType );
    if( object == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
    }
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByName( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
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


FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    
    return Fieldml_GetMemberCount( handle, componentTypeHandle );
}


int Fieldml_GetMemberCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetMemberCount( handle, meshType->elementsType );
    }
        

    session->setError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


FmlEnsembleValue Fieldml_GetEnsembleMembersMin( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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


FmlObjectHandle Fieldml_GetMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

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


char * Fieldml_GetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }
        
    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, meshHandle ); 
    if( map == NULL )
    {
        return NULL;
    }

    return cstrCopy( map->get( elementNumber, (allowDefault == 1) ) );
}


int Fieldml_CopyMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshElementShape( handle, meshHandle, elementNumber, allowDefault ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

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


FmlObjectHandle Fieldml_GetMeshChartComponentType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

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
    FieldmlSession *session = getSession( handle );
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


char * Fieldml_GetObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectName( handle, objectHandle ), buffer, bufferLength );
}


char * Fieldml_GetObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectDeclaredName( handle, objectHandle ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_SetObjectInt( FmlSessionHandle handle, FmlObjectHandle objectHandle, int value )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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

    session->setError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateArgumentEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = getSession( handle );
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
        
        string meshName = session->region->getObjectName( valueType );
        meshName += ".";
     
        string argumentName = name;
        string chartComponentName = session->region->getObjectName( chartType );
        string elementsComponentName = session->region->getObjectName( elementsType );
        
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
        
        //Shouldn't need to check for name-collision, as we already have.
        ArgumentEvaluator *chartEvaluator = new ArgumentEvaluator( chartName.c_str(), chartType, true );
        addObject( session, chartEvaluator );        
        
        ArgumentEvaluator *elementEvaluator = new ArgumentEvaluator( elementsName.c_str(), elementsType, true );
        addObject( session, elementEvaluator );        
    }
    
    ArgumentEvaluator *argumentEvaluator = new ArgumentEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, argumentEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
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


FmlObjectHandle Fieldml_CreateParameterEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
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
    FieldmlSession *session = getSession( handle );
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

        if( description == DESCRIPTION_DOK_ARRAY )
        {
            delete parameterEvaluator->dataDescription;
            parameterEvaluator->dataDescription = new DokArrayDataDescription();
            return session->getLastError();
        }
        else if( description == DESCRIPTION_DENSE_ARRAY )
        {
            delete parameterEvaluator->dataDescription;
            parameterEvaluator->dataDescription = new DenseArrayDataDescription();
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DENSE_ARRAY )
        {
            DenseArrayDataDescription *denseArray = (DenseArrayDataDescription*)parameterEvaluator->dataDescription;
            denseArray->dataSource = dataSource;
        }
        else if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameterEvaluator->dataDescription;
            dokArray->valueSource = dataSource;
        }
        else
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
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


FmlErrorNumber Fieldml_SetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource )
{
    FieldmlSession *session = getSession( handle );
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
    
    ArrayDataSource *source = getArrayDataSource( session, dataSource );
    if( source == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else if( source->rank != 2 )
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
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameterEvaluator->dataDescription;
            dokArray->keySource = dataSource;
        }
        else
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_AddDenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return session->getLastError();
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
    
    if( orderHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_GetObjectType( handle, orderHandle ) != FHT_DATA_SOURCE )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4 );
        }
            
        ArrayDataSource *orderSource = getArrayDataSource( session, orderHandle );
        if( orderSource == NULL )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4 );
        }
        else if( orderSource->rank != 1 )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4 );
        }
    }
    
    if( !checkCyclicDependency( session, objectHandle, indexHandle ) )
    {
        return session->getLastError();
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        session->setError( parameterEvaluator->dataDescription->addIndexEvaluator( false, indexHandle, orderHandle ) );
    }
    else
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_AddSparseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return session->getLastError();
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
        
    if( !checkCyclicDependency( session, objectHandle, indexHandle ) )
    {
        return session->getLastError();
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        session->setError( parameterEvaluator->dataDescription->addIndexEvaluator( true, indexHandle, FML_INVALID_HANDLE ) );
    }
    else
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    return session->getLastError();
}


int Fieldml_GetParameterIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isSparse )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return -1;
    }
    
    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        int count = parameterEvaluator->dataDescription->getIndexCount( isSparse != 0 );
        if( count == -1 )
        {
            session->setError( FML_ERR_INVALID_OBJECT );
        }
        
        return count;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }
}


FmlObjectHandle Fieldml_GetParameterIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlBoolean isSparse )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        FmlObjectHandle evaluator;
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        session->setError( parameterEvaluator->dataDescription->getIndexEvaluator( index-1, isSparse != 0, evaluator ) );
        
        return evaluator;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
}


FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
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
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
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
    FieldmlSession *session = getSession( handle );
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

    if( !checkCyclicDependency( session, objectHandle, evaluator ) )
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
 
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    if( !checkCyclicDependency( session, objectHandle, evaluator ) )
    {
        return session->getLastError();
    }
    
    map->set( element, evaluator );
    return session->getLastError();
}


int Fieldml_GetEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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


FmlEnsembleValue Fieldml_GetEvaluatorElement( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->getKey( evaluatorIndex - 1 );
}


FmlObjectHandle Fieldml_GetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getValue( evaluatorIndex - 1 );
}


FmlObjectHandle Fieldml_GetElementEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
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
    FieldmlSession *session = getSession( handle );
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


int Fieldml_GetArgumentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isUnbound, FmlBoolean isUsed )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    vector<FmlObjectHandle> args = getArgumentList( session, objectHandle, isUnbound != 0, isUsed != 0 );
    if( session->getLastError() != FML_ERR_NO_ERROR )
    {
        return -1;
    }
    return args.size();
}


FmlObjectHandle Fieldml_GetArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int argumentIndex, FmlBoolean isUnbound, FmlBoolean isUsed )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    vector<FmlObjectHandle> args = getArgumentList( session, objectHandle, isUnbound != 0, isUsed != 0 );
    if( session->getLastError() != FML_ERR_NO_ERROR )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( argumentIndex < 1 ) || ( argumentIndex > args.size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return args.at( argumentIndex - 1 );
}


FmlErrorNumber Fieldml_AddArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return session->getLastError();
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, evaluatorHandle ) )
    {
        return session->getLastError();
    }
    
    if( Fieldml_GetObjectType( handle, evaluatorHandle ) != FHT_ARGUMENT_EVALUATOR )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    if( object->type == FHT_ARGUMENT_EVALUATOR )
    {
        ArgumentEvaluator *evaluator = (ArgumentEvaluator*)object;
        evaluator->arguments.insert( evaluatorHandle );
    }
    else if( object->type == FHT_EXTERNAL_EVALUATOR )
    {
        ExternalEvaluator *evaluator = (ExternalEvaluator*)object;
        evaluator->arguments.insert( evaluatorHandle );
    }
    else
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return session->getLastError();
}


int Fieldml_GetBindCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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


FmlObjectHandle Fieldml_GetBindArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getKey( bindIndex - 1 );
}


FmlObjectHandle Fieldml_GetBindEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( bindIndex - 1 );
}


FmlObjectHandle Fieldml_GetBindByArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( session, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->get( argumentHandle, false );
}


FmlErrorNumber Fieldml_SetBind( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle, FmlObjectHandle sourceHandle )
{
    FieldmlSession *session = getSession( handle );
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
    
    if( !checkCyclicDependency( session, objectHandle, sourceHandle ) )
    {
        return session->getLastError();
    }
    
    map->set( argumentHandle, sourceHandle );
    return session->getLastError();
}



int Fieldml_GetIndexEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        int denseCount = parameterEvaluator->dataDescription->getIndexCount( false );
        int sparseCount = parameterEvaluator->dataDescription->getIndexCount( true );
        
        if( ( denseCount > -1 ) && ( sparseCount > -1 ) )
        {
            return denseCount + sparseCount;
        }
        else if( denseCount > -1 )
        {
            return denseCount;
        }
        else if( sparseCount > -1 )
        {
            return sparseCount;
        }
        else
        {
            session->setError( FML_ERR_INVALID_OBJECT );
            return -1;
        }
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );
    return -1;
}


FmlErrorNumber Fieldml_SetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = getSession( handle );
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

    if( !checkCyclicDependency( session, objectHandle, evaluatorHandle ) )
    {
        return session->getLastError();
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
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        session->setError( parameterEvaluator->dataDescription->setIndexEvaluator( index-1, evaluatorHandle, FML_INVALID_HANDLE ) );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }

    return session->getLastError();
}


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int indexNumber )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( indexNumber <= 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( indexNumber == 1 )
        {
            return piecewise->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( indexNumber == 1 )
        {
            return aggregate->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        FmlObjectHandle evaluator;
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        session->setError( parameterEvaluator->dataDescription->getIndexEvaluator( indexNumber-1, evaluator ) );
        
        return evaluator;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
    }

    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetParameterIndexOrder( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;

    FmlObjectHandle order;
    FmlErrorNumber err = parameterEvaluator->dataDescription->getIndexOrder( index-1, order );
    session->setError( err );
    
    return order;
}


FmlObjectHandle Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    ContinuousType *continuousType = new ContinuousType( name, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, continuousType );
}


FmlObjectHandle Fieldml_CreateContinuousTypeComponents( FmlSessionHandle handle, FmlObjectHandle typeHandle, const char * name, const int count )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, typeHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, typeHandle );
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
    if( type->componentType != FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    if( count < 1 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    string trueName = name;
    
    if( strncmp( name, "~.", 2 ) == 0 )
    {
        trueName = type->name + ( name + 1 );
    }
    
    EnsembleType *ensembleType = new EnsembleType( trueName, true, false );
    FmlObjectHandle componentHandle = addObject( session, ensembleType );
    Fieldml_SetEnsembleMembersRange( handle, componentHandle, 1, count, 1 );
    
    type->componentType = componentHandle;
    
    return componentHandle;
}


FmlObjectHandle Fieldml_CreateEnsembleType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    EnsembleType *ensembleType = new EnsembleType( name, false, false );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    MeshType *meshType = new MeshType( name, false );

    session->setError( FML_ERR_NO_ERROR );

    return addObject( session, meshType );
}


FmlObjectHandle Fieldml_CreateMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, meshHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, meshHandle );
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


FmlObjectHandle Fieldml_CreateMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, meshHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, meshHandle );
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


FmlErrorNumber Fieldml_SetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * shape )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, meshHandle ) )
    {
        return session->getLastError();
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, meshHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }
    
    map->setDefault( shape );
    return session->getLastError();
}


char * Fieldml_GetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, meshHandle ); 
    if( map == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( map->getDefault() );
}


int Fieldml_CopyMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMeshDefaultShape( handle, meshHandle ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_SetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, const char * shape )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, meshHandle ) )
    {
        return session->getLastError();
    }

    SimpleMap<FmlEnsembleValue, string> *map = getShapeMap( session, meshHandle ); 
    if( map == NULL )
    {
        return session->getLastError();
    }

    map->set( elementNumber, shape );
    return session->getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
        return FML_INVALID_HANDLE;
    }

    ArrayDataReader *reader = NULL;
    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    if( dataSource->type == DATA_SOURCE_ARRAY )
    {
        ArrayDataSource *arraySource = (ArrayDataSource*)dataSource;
        reader = ArrayDataReader::create( session, session->region->getRoot().c_str(), arraySource );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    if( reader == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return session->addReader( reader );
}


FmlErrorNumber Fieldml_ReadIntSlab( FmlSessionHandle handle, FmlReaderHandle readerHandle, int *offsets, int *sizes, int *valueBuffer )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readIntSlab( offsets, sizes, valueBuffer );
}


FmlErrorNumber Fieldml_ReadDoubleSlab( FmlSessionHandle handle, FmlReaderHandle readerHandle, int *offsets, int *sizes, double *valueBuffer )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readDoubleSlab( offsets, sizes, valueBuffer );
}


FmlErrorNumber Fieldml_CloseReader( FmlSessionHandle handle, FmlReaderHandle readerHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataReader *reader = session->handleToReader( readerHandle );
    if( reader == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    session->removeReader( readerHandle );
    
    delete reader;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlWriterHandle Fieldml_OpenWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle typeHandle, FmlBoolean append, int *sizes, int rank )
{
    FieldmlSession *session = getSession( handle );
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
        return FML_INVALID_HANDLE;
    }
    if( !checkLocal( session, objectHandle ) )
    {
        return FML_INVALID_HANDLE;
    }
    
    //NOTE: Currently, IO only supports scalar ensemble or continuous values, so this boolean is sufficient. 
    bool isDouble;
    
    FieldmlObject *typeObject = getObject( session, typeHandle );
    if( typeObject == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    if( typeObject->type == FHT_ENSEMBLE_TYPE )
    {
        isDouble = false;
    }
    else if( typeObject->type == FHT_CONTINUOUS_TYPE )
    {
        ContinuousType *continuousType = (ContinuousType*)typeObject;
        if( continuousType->componentType == FML_INVALID_HANDLE )
        {
            isDouble = true;
        }
        else
        {
            session->setError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }
    else
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    ArrayDataWriter *writer = NULL;
    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    if( dataSource->type == DATA_SOURCE_ARRAY )
    {
        ArrayDataSource *arraySource = (ArrayDataSource*)dataSource;
        writer = ArrayDataWriter::create( session, session->region->getRoot().c_str(), arraySource, isDouble, ( append == 1 ), sizes, rank );
    }
    

    if( writer == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return session->addWriter( writer );
}


FmlErrorNumber Fieldml_WriteIntSlab( FmlSessionHandle handle, FmlWriterHandle writerHandle, int *offsets, int *sizes, int *valueBuffer )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return writer->writeIntSlab( offsets, sizes, valueBuffer );
}


FmlErrorNumber Fieldml_WriteDoubleSlab( FmlSessionHandle handle, FmlWriterHandle writerHandle, int *offsets, int *sizes, double *valueBuffer )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    return writer->writeDoubleSlab( offsets, sizes, valueBuffer );
}


FmlErrorNumber Fieldml_CloseWriter( FmlSessionHandle handle, FmlWriterHandle writerHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ArrayDataWriter *writer = session->handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }

    session->removeWriter( writerHandle );
    
    delete writer;
    
    return session->setError( FML_ERR_NO_ERROR );
}


FmlErrorNumber Fieldml_SetEnsembleMembersRange( FmlSessionHandle handle, FmlObjectHandle objectHandle, const FmlEnsembleValue minElement, const FmlEnsembleValue maxElement, const int stride )
{
    FieldmlSession *session = getSession( handle );
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
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        ensemble->type = MEMBER_RANGE;
        ensemble->min = minElement;
        ensemble->max = maxElement;
        ensemble->stride = stride;
        ensemble->count = ( ( maxElement - minElement ) / stride ) + 1;

        return session->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_SetEnsembleMembersRange( handle, meshType->elementsType, minElement, maxElement, stride );
    }

    return session->setError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_AddImportSource( FmlSessionHandle handle, const char * href, const char * regionName )
{
    FieldmlSession *session = getSession( handle );
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
    if( regionName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }

    FieldmlRegion *importedRegion = session->getRegion( href, regionName );
    if( importedRegion == NULL )
    {
        importedRegion = session->addResourceRegion( href, regionName );
        if( importedRegion == NULL )
        {
            session->setError( FML_ERR_IO_READ_ERR );
            return -1;
        }
    }
    
    int index = session->getRegionIndex( href, regionName );
    if( index < 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );  
        return -1;
    }
    
    session->region->addImportSource( index, href, regionName );
    
    return index + 1;
}


FmlObjectHandle Fieldml_AddImport( FmlSessionHandle handle, int importSourceIndex, const char * localName, const char * remoteName )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyImportSourceHref( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyImportSourceRegionName( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyImportLocalName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = getSession( handle );
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


int Fieldml_CopyImportRemoteName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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


FmlObjectHandle Fieldml_CreateHrefDataResource( FmlSessionHandle handle, const char * name, const char * format, const char * href )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    if( href == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    if( format == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new DataResource( name, DATA_RESOURCE_HREF, format, href );
    
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, dataResource );
}


FmlObjectHandle Fieldml_CreateInlineDataResource( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new DataResource( name, DATA_RESOURCE_INLINE, PLAIN_TEXT_NAME, "" );
    session->setError( FML_ERR_NO_ERROR );
    return addObject( session, dataResource );
}


DataResourceType Fieldml_GetDataResourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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


FmlErrorNumber Fieldml_AddInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char * data, const int length )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( data == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3 );
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return session->getLastError();
    }
    if( resource->type != DATA_RESOURCE_INLINE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    resource->description = resource->description + string( data, length );
    
    return session->getLastError();
}


int Fieldml_GetInlineDataLength( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return -1;
    }
    if( resource->type != DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }
    
    return resource->description.length();
}


char * Fieldml_GetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return NULL;
    }
    if( resource->type != DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
    
    return cstrCopy( resource->description );
}


int Fieldml_CopyInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength, int offset )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return -1;
    }
    if( resource->type != DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }
    
    if( offset >= resource->description.length() )
    {
        return 0;
    }
    
    //This is probably not the best way to do this
    return cappedCopy( resource->description.c_str() + offset, buffer, bufferLength );
}


DataSourceType Fieldml_GetDataSourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return DATA_SOURCE_UNKNOWN;
    }
    
    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    if( dataSource == NULL )
    {
        return DATA_SOURCE_UNKNOWN;
    }
    
    return dataSource->type;
}


char * Fieldml_GetDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    DataResource *dataResource = getDataResource( session, objectHandle );
    if( dataResource == NULL )
    {
        return NULL;
    }
    
    if( dataResource->type == DATA_RESOURCE_HREF )
    {
        return cstrCopy( dataResource->description );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


int Fieldml_CopyDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetDataResourceHref( handle, objectHandle ), buffer, bufferLength );
}


char * Fieldml_GetDataResourceFormat( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    DataResource *dataResource = getDataResource( session, objectHandle );
    if( dataResource == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( dataResource->format );
}


int Fieldml_CopyDataResourceFormat( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetDataResourceFormat( handle, objectHandle ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
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
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DENSE_ARRAY )
        {
            DenseArrayDataDescription *denseArray = (DenseArrayDataDescription*)parameterEvaluator->dataDescription;
            return denseArray->dataSource;
        }
        else if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameterEvaluator->dataDescription;
            return dokArray->valueSource;
        }
        else
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
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
    
    return dataSourceHandle;
}


FmlObjectHandle Fieldml_GetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
        if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameterEvaluator->dataDescription;
            return dokArray->keySource;
        }
        else
        {
            return session->setError( FML_ERR_INVALID_OBJECT );
        }
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        dataSourceHandle = FML_INVALID_HANDLE;
    }
    
    return dataSourceHandle;
}


int Fieldml_GetDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
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
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION );
        return FML_INVALID_HANDLE;
    }
    
    DataSource *source = objectAsDataSource( session, objectHandle );
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


const char * Fieldml_GetArrayDataSourceLocation( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return NULL;
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return NULL;
    }

    return cstrCopy( source->location );
}


int Fieldml_CopyArrayDataSourceLocation( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetArrayDataSourceLocation( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_GetArrayDataSourceRank( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return -1;
    }
    
    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return -1;
    }
    if( object->type != FHT_DATA_SOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT );
        return -1;
    }
    
    DataSource *source = (DataSource*)object;
    if( source->type == DATA_SOURCE_ARRAY )
    {
        ArrayDataSource *arraySource = (ArrayDataSource*)source;
        return arraySource->rank;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT );
    return -1;
}


FmlErrorNumber Fieldml_GetArrayDataSourceSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        sizes[i] = source->sizes[i];
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber Fieldml_SetArrayDataSourceSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        if( sizes[i] < 0 )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    
    source->sizes.clear();
    for( int i = 0; i < source->rank; i++ )
    {
        source->sizes.push_back( sizes[i] );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber Fieldml_GetArrayDataSourceRawSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        sizes[i] = source->rawSizes[i];
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber Fieldml_SetArrayDataSourceRawSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        if( sizes[i] <= 0 )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    
    source->rawSizes.clear();
    for( int i = 0; i < source->rank; i++ )
    {
        source->rawSizes.push_back( sizes[i] );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber Fieldml_GetArrayDataSourceOffsets( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *offsets )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        offsets[i] = source->offsets[i];
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber Fieldml_SetArrayDataSourceOffsets( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *offsets )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return session->getLastError();
    }

    ArrayDataSource *source = getArrayDataSource( session, objectHandle );
    if( source == NULL )
    {
        return session->getLastError();
    }

    for( int i = 0; i < source->rank; i++ )
    {
        if( offsets[i] < 0 )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    
    source->offsets.clear();
    for( int i = 0; i < source->rank; i++ )
    {
        source->offsets.push_back( offsets[i] );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlObjectHandle Fieldml_CreateArrayDataSource( FmlSessionHandle handle, const char * name, FmlObjectHandle resourceHandle, const char * location, int rank )
{
    FieldmlSession *session = getSession( handle );
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    if( location == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4 );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, resourceHandle ) )
    {
        return session->getLastError();
    }

    if( rank <= 0 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5 );
    }
    
    FieldmlObject *object = getObject( session, resourceHandle );
    if( object == NULL )
    {
        return session->getLastError();
    }
    if( object->type != FHT_DATA_RESOURCE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT );
    }
    
    DataResource *dataResource = getDataResource( session, resourceHandle );

    ArrayDataSource *source = new ArrayDataSource( name, dataResource, location, rank );

    session->setError( FML_ERR_NO_ERROR );
    FmlObjectHandle sourceHandle = addObject( session, source );
    
    dataResource->dataSources.push_back( sourceHandle );
    
    return sourceHandle;
}
