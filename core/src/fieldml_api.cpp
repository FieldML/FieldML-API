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
#include "ErrorContextAutostack.h"
#include "fieldml_structs.h"
#include "Evaluators.h"
#include "fieldml_write.h"
#include "string_const.h"
#include "Util.h"

#include "FieldmlRegion.h"

using namespace std;

//========================================================================
//
// Utility
//
// NOTE: A number of these methods are just wrappers around FieldmlSession
// methods, but which also set error codes. Methods such as FieldmlSession::getObject
// should not set error codes themselves, as failure is not necessarily an error.
//
//========================================================================

static FieldmlObject *getObject( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = session->getObject( objectHandle );
    
    if( object == NULL )
    {
        session->setError( FML_ERR_UNKNOWN_OBJECT, "Invalid object handle." );
    }
    
    return object;
}


static bool checkLocal( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region." );
        return false;
    }
    
    if( objectHandle == FML_INVALID_HANDLE )
    {
        //Nano-hack. Makes checking legitimate FML_INVALID_HANDLE parameters easier.
        return true;
    }
    
    if( !session->region->hasLocalObject( objectHandle, true, true ) )
    {
        session->setError( FML_ERR_NONLOCAL_OBJECT, objectHandle, "Not a local object." );
        return false;
    }
    
    return true;
}


static FmlObjectHandle addObject( FieldmlSession *session, FieldmlObject *object )
{
    ERROR_AUTOSTACK( session );

    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle handle = session->region->getNamedObject( object->name.c_str() );
    
    if( handle == FML_INVALID_HANDLE )
    {
        FmlObjectHandle handle = session->objects.addObject( object );
        session->region->addLocalObject( handle );
        return handle;
    }
    
    FieldmlObject *oldObject = session->objects.getObject( handle );
    
    session->logError( "Handle collision. Cannot replace", object->name.c_str(), oldObject->name.c_str() );
    delete object;
    
    session->setError( FML_ERR_NAME_COLLISION, "There is already an object named " + object->name + " in this scope." );
    
    return FML_INVALID_HANDLE;
}


static SimpleMap<FmlEnsembleValue, FmlObjectHandle> *getEvaluatorMap( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return NULL;
    }

    AggregateEvaluator *aggregate = AggregateEvaluator::checkedCast( session, objectHandle );
    if( aggregate != NULL )
    {
        return &aggregate->evaluators;
    }
    
    PiecewiseEvaluator *piecewise = PiecewiseEvaluator::checkedCast( session, objectHandle );
    if( piecewise != NULL )
    {
        return &piecewise->evaluators;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must either be a Piecewise or Aggregate evaluator." );
    return NULL;
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getBindMap( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return NULL;
    }
    
    AggregateEvaluator *aggregate = AggregateEvaluator::checkedCast( session, objectHandle );
    if( aggregate != NULL )
    {
        return &aggregate->binds;
    }

    PiecewiseEvaluator *piecewise = PiecewiseEvaluator::checkedCast( session, objectHandle );
    if( piecewise != NULL )
    {
        return &piecewise->binds;
    }

    ReferenceEvaluator *reference = ReferenceEvaluator::checkedCast( session, objectHandle );
    if( reference != NULL )
    {
        return &reference->binds;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an Aggregate, Piecewise or Reference evaluator." );
    return NULL;
}


static vector<FmlObjectHandle> getArgumentList( FieldmlSession *session, FmlObjectHandle objectHandle, bool isBound, bool isUsed )
{
    vector<FmlObjectHandle> args;

    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return args;
    }

    Evaluator *evaluator = Evaluator::checkedCast( session, objectHandle );
    
    if( evaluator == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get arguments. Must be an evalator." );
        return args;
    }

    if ( isBound && !isUsed )
    {
        //Always an empty set with the current algorithm, as it only tracks unbound or used arguments.
        return args;
    }
    if( !isBound && !isUsed )
    {
        //Always an empty set with the current algorithm, as it assumes that arguments of arguments are used.
        return args;
    }
    
    set<FmlObjectHandle> unbound, used;
    session->getArguments( objectHandle, unbound, used, false );
    
    if( isBound && isUsed )
    {
        for( set<FmlObjectHandle>::const_iterator i = used.begin(); i != used.end(); i++ )
        {
						//	Performance wise, I am not sure I should remove all elems from
						//	unbound in used or using find as shown below.
						if (unbound.end() == unbound.find(*i))
								args.push_back( *i );
        }
        return args;
    }
    else //if( !isBound && isUsed )
    {
        //In used, and in unbound. Unbound is always is a subset of used with the current algorithm.
        for( set<FmlObjectHandle>::const_iterator i = unbound.begin(); i != unbound.end(); i++ )
        {
            args.push_back( *i );
        }
        return args;
    }
}


static bool checkCyclicDependency( FieldmlSession *session, FmlObjectHandle objectHandle, FmlObjectHandle objectDependancy )
{
    ERROR_AUTOSTACK( session );

    set<FmlObjectHandle> delegates;
    session->getDelegateEvaluators( objectDependancy, delegates );
    if( FmlUtil::contains( delegates, objectHandle ) )
    {
        session->setError( FML_ERR_CYCLIC_DEPENDENCY, objectHandle, "Cyclic dependancy." );
        return false;
    }
    
    return true;
}


static char* cstrCopy( const string &s )
{
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
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->objectType != FHT_DATA_SOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, "Must be a data source." );
        return NULL;
    }
    
    return (DataSource*)object;
}


static ArrayDataSource *getArrayDataSource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    
    if( dataSource == NULL )
    {
        return NULL;
    }

    if( dataSource->sourceType != FML_DATA_SOURCE_ARRAY )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an array data source." );
        return NULL;
    }

    ArrayDataSource *arraySource = (ArrayDataSource*)dataSource;
    return arraySource;
}


static DataResource *getDataResource( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->objectType != FHT_DATA_RESOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a data resource." );
        return NULL;
    }
    
    return (DataResource*)object;
}


static bool checkIsValueType( FieldmlSession *session, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble, bool allowMesh, bool allowBoolean )
{
    ERROR_AUTOSTACK( session );

    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return false;
    }

    switch( object->objectType )
    {
    case FHT_CONTINUOUS_TYPE:
        return allowContinuous;
    case FHT_ENSEMBLE_TYPE:
        return allowEnsemble;
    case FHT_MESH_TYPE:
        return allowMesh;
    case FHT_BOOLEAN_TYPE:
        return allowBoolean;
    default:
        return false;
    }
}


static bool checkIsEvaluatorType( FieldmlSession *session, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble, bool allowBoolean )
{
    ERROR_AUTOSTACK( session );

    Evaluator *evaluator = Evaluator::checkedCast( session, objectHandle );
    if( evaluator == NULL )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Is not an evaluator." );
        return false;
    }
    
    return checkIsValueType( session, evaluator->valueType, allowContinuous, allowEnsemble, false, allowBoolean );
}


static bool checkIsTypeCompatible( FieldmlSession *session, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    ERROR_AUTOSTACK( session );

    if( !checkIsValueType( session, objectHandle1, true, true, false, true ) )
    {
        return false;
    }
    if( !checkIsValueType( session, objectHandle2, true, true, false, true ) )
    {
        return false;
    }

    FieldmlObject *object1 = getObject( session, objectHandle1 );
    FieldmlObject *object2 = getObject( session, objectHandle2 );

    if( object1->objectType != object2->objectType )
    {
        return false;
    }
    else if( object1->objectType == FHT_BOOLEAN_TYPE )
    {
        return true;
    }
    else if( object1->objectType == FHT_ENSEMBLE_TYPE )
    {
        return objectHandle1 == objectHandle2;
    }
    else if( object1->objectType == FHT_CONTINUOUS_TYPE )
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
    ERROR_AUTOSTACK( session );

    if( !checkIsEvaluatorType( session, objectHandle1, true, true, true ) )
    {
        return false;
    }
    if( !checkIsEvaluatorType( session, objectHandle2, true, true, true ) )
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
    ErrorContextAutostack bob( session, __FILE__, __LINE__, __ECA_FUNC__ );
    
    if( filename == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_1, "Cannot create FieldML session. Invalid filename." );
    }
    else
    {
        session->region = session->addResourceRegion( filename, "" );
        if( session->region == NULL )
        {
            session->setError( FML_ERR_READ_ERR, "Cannot create FieldML session. Invalid document or read error." );
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
    ERROR_AUTOSTACK( session );
    
    if( location == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_1, "Cannot create FieldML session. Invalid location." );
    }
    else if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create FieldML session. Invalid name." );
    }
    else
    {
        session->region = session->addNewRegion( location, name );
    }
    
    return session->getSessionHandle();
}


FmlErrorNumber Fieldml_SetDebug( FmlSessionHandle handle, int debug )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );
    
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->setDebug( debug );
    
    return session->setError( FML_ERR_NO_ERROR, "" );
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


FmlErrorNumber Fieldml_WriteFile( FmlSessionHandle handle, const char * filename )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );
    
    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( session->region == NULL )
    {
        return session->setError( FML_ERR_INVALID_REGION, "Cannot write FieldML file. FieldML session has no region." );
    }
    if( filename == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot write FieldML file. Invalid filename." );
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    session->region->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( session, handle, filename );
}


void Fieldml_Destroy( FmlSessionHandle handle )
{
    FieldmlSession::removeSession( handle );    
}


char * Fieldml_GetRegionName( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );
    
    if( session == NULL )
    {
        return NULL;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "Cannot get region name. FieldML session has no region." );
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
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


char * Fieldml_GetRegionRoot( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );
    
    if( session == NULL )
    {
        return NULL;
    }
    
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "Cannot get region root. FieldML session has no region." );
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    return cstrCopy( session->region->getRoot() );
}


int Fieldml_CopyRegionRoot( FmlSessionHandle handle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetRegionRoot( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    return session->getErrorCount();
}


char * Fieldml_GetError( FmlSessionHandle handle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    return cstrCopy( session->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlSessionHandle handle, int errorIndex, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetError( handle, errorIndex ), buffer, bufferLength );
}


FmlErrorNumber Fieldml_ClearErrors( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    session->clearErrors();
    return session->setError( FML_ERR_NO_ERROR, "" );
}


int Fieldml_GetTotalObjectCount( FmlSessionHandle handle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    return session->objects.getCount();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlSessionHandle handle, const int objectIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    return session->objects.getObjectByIndex( objectIndex );
}


int Fieldml_GetObjectCount( FmlSessionHandle handle, FieldmlHandleType type )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );
    if( type == FHT_UNKNOWN )
    {
        return -1;
    }

    return session->objects.getCount( type );
}


FmlObjectHandle Fieldml_GetObject( FmlSessionHandle handle, FieldmlHandleType objectType, int objectIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    session->setError( FML_ERR_NO_ERROR, "" );

    FmlObjectHandle object = session->objects.getObjectByIndex( objectIndex, objectType );
    if( object == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot get object by type. Invalid index." );  
    }
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByName( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "Cannot get object by local name. FieldML session has no region." );
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot get object by local name. Invalid name." );  
        return FML_INVALID_HANDLE;
    }
        
    FmlObjectHandle object = session->region->getNamedObject( name );
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot get object by declared name. Invalid name." );  
        return FML_INVALID_HANDLE;
    }
    
    FmlObjectHandle object = session->objects.getObjectByName( name );
    
    return object;
}


FieldmlHandleType Fieldml_GetObjectType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FHT_UNKNOWN;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FHT_UNKNOWN;
    }
    
    return object->objectType;
}


FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->objectType == FHT_CONTINUOUS_TYPE )
    {
        ContinuousType *continuousType = (ContinuousType*)object;
        return continuousType->componentType;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get component ensemble. Must be a type with components." );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetTypeComponentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->count;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetMemberCount( handle, meshType->elementsType );
    }
        

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get member count. Must be a type with members/elements." );  
    return -1;
}


FmlEnsembleValue Fieldml_GetEnsembleMembersMin( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->min;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersMin( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble or mesh type." );  
    return -1;
}


FmlEnsembleValue Fieldml_GetEnsembleMembersMax( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->max;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersMax( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble or mesh type." );  
    return -1;
}


int Fieldml_GetEnsembleMembersStride( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->stride;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetEnsembleMembersStride( handle, meshType->elementsType );
    }
        
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble or mesh type." );  
    return -1;
}


FieldmlEnsembleMembersType Fieldml_GetEnsembleMembersType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ENSEMBLE_MEMBER_UNKNOWN;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return FML_ENSEMBLE_MEMBER_UNKNOWN;
    }
    
    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->membersType;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetEnsembleMembersType( handle, meshType->elementsType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble or mesh type." );  
    return FML_ENSEMBLE_MEMBER_UNKNOWN;
}


FmlErrorNumber Fieldml_SetEnsembleMembersDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FieldmlEnsembleMembersType type, int count, FmlObjectHandle dataSourceHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
        return session->setError( FML_ERR_INVALID_PARAMETER_5, dataSourceHandle, "Must be a data source to be used for member labels." );
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return session->getLastError();
    }
    else if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;

        if( ( type != FML_ENSEMBLE_MEMBER_LIST_DATA ) && ( type != FML_ENSEMBLE_MEMBER_RANGE_DATA ) && ( type != FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Has a member type which cannot be used with a data source." );
        }
        
        ensembleType->membersType = type;
        ensembleType->count = count;
        ensembleType->dataSource = dataSourceHandle;
        return session->getLastError();
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetEnsembleMembersDataSource( handle, meshType->elementsType, type, count, dataSourceHandle );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble or mesh type." );  
}


FmlBoolean Fieldml_IsEnsembleComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    
    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->isComponentEnsemble;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an ensemble type." );  
    return -1;
}


FmlObjectHandle Fieldml_GetMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->objectType == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->elementsType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Must be a mesh type." );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshShapes( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }
    FieldmlObject *object = getObject( session, meshHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->objectType == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->shapes;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Must be a mesh type." );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->objectType == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->chartType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Must be a mesh type." );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshChartComponentType( FmlSessionHandle handle, FmlObjectHandle meshHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = getObject( session, meshHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->objectType == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetTypeComponentEnsemble( handle, meshType->chartType );
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Must be a mesh type." );  
    return FML_INVALID_HANDLE;
}


FmlBoolean Fieldml_IsObjectLocal( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isDeclaredOnly )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return 0;
    }

    bool allowVirtual = ( isDeclaredOnly != 1 );
    bool allowImport = ( isDeclaredOnly != 1 );
    if( session->region->hasLocalObject( objectHandle, allowVirtual, allowImport ) )
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }
    
    FieldmlObject *object = session->objects.getObject( objectHandle );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    ERROR_AUTOSTACK( session );

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
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    Evaluator *evaluator = Evaluator::checkedCast( session, objectHandle );
    if( evaluator != NULL )
    {
        return evaluator->valueType;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an evaluator." );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateArgumentEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Invalid argument evaluator name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, true, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, valueType, "Invalid value type for argument evaluator " + string( name ) );
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
            session->setError( FML_ERR_INVALID_PARAMETER_2, valueType, "Cannot create mesh argument evaluator. " + chartName + " already exists."  );
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
            session->setError( FML_ERR_INVALID_PARAMETER_2, valueType, "Cannot create mesh argument evaluator. " + elementsName + " already exists."  );
            return FML_INVALID_HANDLE;
        }
        
        //Shouldn't need to check for name-collision, as we already have.
        ArgumentEvaluator *chartEvaluator = new ArgumentEvaluator( chartName.c_str(), chartType, true );
        addObject( session, chartEvaluator );        
        
        ArgumentEvaluator *elementEvaluator = new ArgumentEvaluator( elementsName.c_str(), elementsType, true );
        addObject( session, elementEvaluator );        
    }
    
    ArgumentEvaluator *argumentEvaluator = new ArgumentEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, argumentEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create external evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, valueType, "Cannot create external evaluator of this type." );
        return FML_INVALID_HANDLE;
    }
        
    ExternalEvaluator *externalEvaluator = new ExternalEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, externalEvaluator );
}


FmlObjectHandle Fieldml_CreateParameterEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create parameter evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, valueType, "Cannot create parameter evaluator of this type." );
        return FML_INVALID_HANDLE;
    }
        
    ParameterEvaluator *parameterEvaluator = new ParameterEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, parameterEvaluator );
}


FmlErrorNumber Fieldml_SetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle, FieldmlDataDescriptionType description )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, objectHandle ) )
    {
        return session->getLastError();
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        if( parameter->dataDescription->descriptionType != FML_DATA_DESCRIPTION_UNKNOWN )
        {
            return session->setError( FML_ERR_ACCESS_VIOLATION, objectHandle, "Parameter evaluator already has a data description." );
        }

        if( description == FML_DATA_DESCRIPTION_DOK_ARRAY )
        {
            delete parameter->dataDescription;
            parameter->dataDescription = new DokArrayDataDescription();
            return session->getLastError();
        }
        else if( description == FML_DATA_DESCRIPTION_DENSE_ARRAY )
        {
            delete parameter->dataDescription;
            parameter->dataDescription = new DenseArrayDataDescription();
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_UNSUPPORTED, objectHandle, "Unsupported/invalid data description." );  
        }
    }

    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
}


FieldmlDataDescriptionType Fieldml_GetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_DATA_DESCRIPTION_UNKNOWN;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        return parameter->dataDescription->descriptionType;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
    return FML_DATA_DESCRIPTION_UNKNOWN;
}


FmlErrorNumber Fieldml_SetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    if( !checkLocal( session, dataSource ) )
    {
        return session->getLastError();
    }
    if( Fieldml_GetObjectType( handle, dataSource ) != FHT_DATA_SOURCE )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, dataSource, "Must be a data source." );
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DENSE_ARRAY )
        {
            //TODO Check that the rank of the data source is equal to the number of dense indexes.
            DenseArrayDataDescription *denseArray = (DenseArrayDataDescription*)parameter->dataDescription;
            denseArray->dataSource = dataSource;
        }
        else if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DOK_ARRAY )
        {
            //TODO Check that the rank of the data source is equal to the number of dense indexes plus one.
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameter->dataDescription;
            dokArray->valueSource = dataSource;
        }
        else
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must have a data description that uses a data source." );
        }
        return session->getLastError();
    }

    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        FieldmlEnsembleMembersType type = ensembleType->membersType;
        
        if( ( type != FML_ENSEMBLE_MEMBER_LIST_DATA ) && ( type != FML_ENSEMBLE_MEMBER_RANGE_DATA ) && ( type != FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA ) )
        {
            return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Ensemble type does not require a data source." );
        }
        
        ensembleType->dataSource = dataSource;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_SetDataSource( handle, meshType->elementsType, dataSource );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator, mesh type or ensemble type." );
    }
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_SetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
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
        return session->setError( FML_ERR_INVALID_PARAMETER_3, dataSource, "Must be a data source." );
    }
    
    ArrayDataSource *source = getArrayDataSource( session, dataSource );
    if( source == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, dataSource, "Must be an array data source." );
    }
    else if( source->rank != 2 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, dataSource, "Key data source must be rank 2." );
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameter->dataDescription;
            dokArray->keySource = dataSource;
        }
        else
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must have dictionary-of-keys data description." );
        }
        return session->getLastError();
    }

    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
}


FmlErrorNumber Fieldml_AddDenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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

    if( !checkIsEvaluatorType( session, indexHandle, false, true, false ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, indexHandle, "Must be an ensemble-valued evaluator to be used for ordering." );
    }
    
    if( orderHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_GetObjectType( handle, orderHandle ) != FHT_DATA_SOURCE )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4, orderHandle, "Must be a data source to be used for ordering." );
        }
            
        ArrayDataSource *orderSource = getArrayDataSource( session, orderHandle );
        if( orderSource == NULL )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4, orderHandle, "Must be an array data source to be used for ordering." );
        }
        else if( orderSource->rank != 1 )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_4, orderHandle, "Must be a rank 1 array data source to be used for ordering." );
        }
    }
    
    if( !checkCyclicDependency( session, objectHandle, indexHandle ) )
    {
        return session->getLastError();
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlErrorNumber error = parameter->dataDescription->addIndexEvaluator( false, indexHandle, orderHandle );
        return session->setError( error, objectHandle, "Cannot set dense index evaluator." );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot add dense index evaluator. Must be a parameter evaluator." );
}


FmlErrorNumber Fieldml_AddSparseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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

    if( !checkIsEvaluatorType( session, indexHandle, false, true, false ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, indexHandle, "Must be an ensemble-valued evaluator to be used for an index evaluator." );
    }
        
    if( !checkCyclicDependency( session, objectHandle, indexHandle ) )
    {
        return session->getLastError();
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlErrorNumber error = parameter->dataDescription->addIndexEvaluator( true, indexHandle, FML_INVALID_HANDLE );
        return session->setError( error, objectHandle, "Cannot set sparse index evaluator." );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot add sparse index evaluator. Must be a parameter evaluator." );
}


int Fieldml_GetParameterIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isSparse )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        int count = parameter->dataDescription->getIndexCount( isSparse != 0 );
        if( count == -1 )
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get parameter index count." );
        }
        
        return count;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
    return -1;
}


FmlObjectHandle Fieldml_GetParameterIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlBoolean isSparse )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlObjectHandle evaluator;
        FmlErrorNumber error = parameter->dataDescription->getIndexEvaluator( index-1, isSparse != 0, evaluator );
        session->setError( error, objectHandle, "Cannot get parameter index evaluator." );
        
        return evaluator;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create piecewise evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, valueType, "Invalid type for piecewise evaluator." );
        return FML_INVALID_HANDLE;
    }
        
    PiecewiseEvaluator *piecewiseEvaluator = new PiecewiseEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, piecewiseEvaluator );
}


FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create aggregate evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, false, false, false ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, valueType, "Invalid type for aggregate evaluator." );
        return FML_INVALID_HANDLE;
    }
        
    AggregateEvaluator *aggregateEvaluator = new AggregateEvaluator( name, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, aggregateEvaluator );
}


FmlErrorNumber Fieldml_SetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
        if( !checkIsEvaluatorType( session, evaluator, true, false, false ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3, evaluator, "Invalid type for aggregator delegate." );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( session, objectHandle, evaluator ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Incompatible type for delegate evaluator." );
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

    map->setDefault( evaluator );
    return session->getLastError();
}


FmlObjectHandle Fieldml_GetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlEnsembleValue, FmlObjectHandle> *map = getEvaluatorMap( session, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getDefault();
}


FmlErrorNumber Fieldml_SetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue element, FmlObjectHandle evaluator )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
        if( !checkIsEvaluatorType( session, evaluator, true, false, false ) )
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3, evaluator, "Invalid type for aggregator delegate." );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( session, objectHandle, evaluator ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Incompatible type for delegate evaluator." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create reference evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, sourceEvaluator ) )
    {
        return session->getLastError();
    }

    FmlObjectHandle valueType = Fieldml_GetValueType( handle, sourceEvaluator );

    ReferenceEvaluator *referenceEvaluator = new ReferenceEvaluator( name, sourceEvaluator, valueType, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, referenceEvaluator );
}


FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ReferenceEvaluator *reference = ReferenceEvaluator::checkedCast( session, objectHandle );
    if( reference != NULL )
    {
        return reference->sourceEvaluator;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a reference evaluator." );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetArgumentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isBound, FmlBoolean isUsed )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    
    vector<FmlObjectHandle> args = getArgumentList( session, objectHandle, isBound != 0, isUsed != 0 );
    if( session->getLastError() != FML_ERR_NO_ERROR )
    {
        return -1;
    }
    return args.size();
}


FmlObjectHandle Fieldml_GetArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int argumentIndex, FmlBoolean isBound, FmlBoolean isUsed )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    vector<FmlObjectHandle> args = getArgumentList( session, objectHandle, isBound != 0, isUsed != 0 );
    if( session->getLastError() != FML_ERR_NO_ERROR )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( argumentIndex < 1 ) || ( argumentIndex > (int)args.size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index number." );
        return FML_INVALID_HANDLE;
    }
    
    return args.at( argumentIndex - 1 );
}


FmlErrorNumber Fieldml_AddArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    
    if( Fieldml_GetObjectType( handle, evaluatorHandle ) != FHT_ARGUMENT_EVALUATOR )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Wrong type evaluator for argument evaluator." );
    }
    
    ArgumentEvaluator *argumentEvaluator = ArgumentEvaluator::checkedCast( session, objectHandle );
    if( argumentEvaluator != NULL )
    {
        argumentEvaluator->arguments.insert( evaluatorHandle );
        return session->getLastError();
    }
    
    ExternalEvaluator *externalEvaluator = ExternalEvaluator::checkedCast( session, objectHandle );
    if( externalEvaluator != NULL )
    {
        externalEvaluator->arguments.insert( evaluatorHandle );
        return session->getLastError();
    }

    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be an argument evaluator or external evaluator." );
}


int Fieldml_GetBindCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Incompatible bind for " + string( Fieldml_GetObjectName( handle, argumentHandle ) ) );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }

    PiecewiseEvaluator *piecewise = PiecewiseEvaluator::checkedCast( session, objectHandle );
    if( piecewise != NULL )
    {
        return 1;
    }
    
    AggregateEvaluator *aggregate = AggregateEvaluator::checkedCast( session, objectHandle );
    if( aggregate != NULL )
    {
        return 1;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        int denseCount = parameter->dataDescription->getIndexCount( false );
        int sparseCount = parameter->dataDescription->getIndexCount( true );
        
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
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get index evaluator count." );
            return -1;
        }
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
    return -1;
}


FmlErrorNumber Fieldml_SetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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

    if( !checkIsEvaluatorType( session, evaluatorHandle, false, true, false ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_4, evaluatorHandle, "Must be ensemble-valued to be used as an index evaluator." );
    }

    if( !checkCyclicDependency( session, objectHandle, evaluatorHandle ) )
    {
        return session->getLastError();
    }

    PiecewiseEvaluator *piecewise = PiecewiseEvaluator::checkedCast( session, objectHandle );
    if( piecewise != NULL )
    {
        if( index == 1 )
        {
            piecewise->indexEvaluator = evaluatorHandle;
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index for piecewise index evaluator." );
        }
    }
    
    AggregateEvaluator *aggregate = AggregateEvaluator::checkedCast( session, objectHandle );
    if( aggregate != NULL )
    {
        if( index == 1 )
        {
            aggregate->indexEvaluator = evaluatorHandle;
            return session->getLastError();
        }
        else
        {
            return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index for aggregate index evaluator." );
        }
    }
    
    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlErrorNumber error = parameter->dataDescription->setIndexEvaluator( index-1, evaluatorHandle, FML_INVALID_HANDLE );
        return session->setError( error, objectHandle, "Cannot set index evaluator." );
    }
    
    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be piecewise or aggregate to set an index evaluator." );
}


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int indexNumber )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( indexNumber <= 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index number." );
        return FML_INVALID_HANDLE;
    }

    PiecewiseEvaluator *piecewise = PiecewiseEvaluator::checkedCast( session, objectHandle );
    if( piecewise != NULL )
    {
        if( indexNumber == 1 )
        {
            return piecewise->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index number." );
        return FML_INVALID_HANDLE;
    }

    AggregateEvaluator *aggregate = AggregateEvaluator::checkedCast( session, objectHandle );
    if( aggregate != NULL )
    {
        if( indexNumber == 1 )
        {
            return aggregate->indexEvaluator;
        }
        
        session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Invalid index number." );
        return FML_INVALID_HANDLE;
    }
    
    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlObjectHandle evaluator;
        FmlErrorNumber error = parameter->dataDescription->getIndexEvaluator( indexNumber-1, evaluator );
        session->setError( error, objectHandle, "Cannot get index evaluator." );
        
        return evaluator;
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Much be an aggregate, piecewise or parameter evaluator." );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetParameterIndexOrder( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        FmlObjectHandle order;
        FmlErrorNumber error = parameter->dataDescription->getIndexOrder( index-1, order );
        session->setError( error, objectHandle, "Cannot get index order." );
     
        return order;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Must be a parameter evaluator." );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateBooleanType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create boolean type. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    BooleanType *booleanType = new BooleanType( name, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, booleanType );
}


FmlObjectHandle Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create continuous type. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    ContinuousType *continuousType = new ContinuousType( name, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, continuousType );
}


FmlObjectHandle Fieldml_CreateContinuousTypeComponents( FmlSessionHandle handle, FmlObjectHandle typeHandle, const char * name, const int count )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, typeHandle, "Cannot create components. Invalid name." );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, typeHandle ) )
    {
        return session->getLastError();
    }

    FieldmlObject *object = getObject( session, typeHandle );
    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( object->objectType != FHT_CONTINUOUS_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, typeHandle, "Cannot create components. Must be a continuous type." );
        return FML_INVALID_HANDLE;
    }
    
    ContinuousType *type = (ContinuousType*)object;
    if( type->componentType != FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, typeHandle, "Cannot create new components. Components have already been created." );
        return FML_INVALID_HANDLE;
    }
    
    if( count < 1 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, typeHandle, "Cannot create components. Invalid count." );
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, typeHandle, "Cannot create components. Invalid name." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create ensemble type. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    EnsembleType *ensembleType = new EnsembleType( name, false, false );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create mesh type. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    MeshType *meshType = new MeshType( name, false );

    session->setError( FML_ERR_NO_ERROR, "" );

    return addObject( session, meshType );
}


FmlObjectHandle Fieldml_CreateMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, meshHandle, "Cannot create mesh elements. Invalid name." );
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

    if( object->objectType != FHT_MESH_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Cannot create mesh elements. Must be a mesh type." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, meshHandle, "Cannot create mesh chart. Invalid name." );
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

    if( object->objectType != FHT_MESH_TYPE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Cannot create mesh chart. Must be a mesh type." );
        return FML_INVALID_HANDLE;
    }
    
    MeshType *meshType = (MeshType*)object;

    ContinuousType *chartType = new ContinuousType( meshType->name + "." + name, true );
    FmlObjectHandle chartHandle = addObject( session, chartType );
    
    meshType->chartType = chartHandle;
    
    return chartHandle;
}


FmlErrorNumber Fieldml_SetMeshShapes( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlObjectHandle shapesHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkLocal( session, meshHandle ) )
    {
        return session->getLastError();
    }
    if( !checkLocal( session, shapesHandle ) )
    {
        return session->getLastError();
    }

    if( !checkIsEvaluatorType( session, shapesHandle, false, false, true ) )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, shapesHandle, "Cannot set mesh shapes. Must be a boolean-valued evaluator." );
    }

    FieldmlObject *object = getObject( session, meshHandle );

    if( object == NULL )
    {
    }
    else if( object->objectType == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        meshType->shapes = shapesHandle;
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT, meshHandle, "Cannot set mesh shapes. Must be a mesh type." );
    }
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_SetEnsembleMembersRange( FmlSessionHandle handle, FmlObjectHandle objectHandle, const FmlEnsembleValue minElement, const FmlEnsembleValue maxElement, const int stride )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Cannot set ensemble members range. Invalid range." );
    }

    if( stride < 1 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5, objectHandle, "Cannot set ensemble members range. Invalid stride." );
    }
    
    if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        ensemble->membersType = FML_ENSEMBLE_MEMBER_RANGE;
        ensemble->min = minElement;
        ensemble->max = maxElement;
        ensemble->stride = stride;
        ensemble->count = ( ( maxElement - minElement ) / stride ) + 1;

        return session->getLastError();
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_SetEnsembleMembersRange( handle, meshType->elementsType, minElement, maxElement, stride );
    }

    return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot set ensemble members range. Must be a mesh type or ensemble type." );
}


int Fieldml_AddImportSource( FmlSessionHandle handle, const char * href, const char * regionName )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    if( href == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot add import. Invalid href." );  
        return -1;
    }
    if( regionName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot add import. Invalid region name." );  
        return -1;
    }

    FieldmlRegion *importedRegion = session->getRegion( href, regionName );
    if( importedRegion == NULL )
    {
        importedRegion = session->addResourceRegion( href, regionName );
        if( importedRegion == NULL )
        {
            //TODO Get a more descriptive reason.
            session->setError( FML_ERR_READ_ERR, "Cannot add import." );
            return -1;
        }
    }
    
    int index = session->getRegionIndex( href, regionName );
    if( index < 0 )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, string( "Cannot get index for import " ) + string( href ) + "." );  
        return -1;
    }
    
    session->region->addImportSource( index, href, regionName );
    
    return index + 1;
}


FmlObjectHandle Fieldml_AddImport( FmlSessionHandle handle, int importSourceIndex, const char * localName, const char * remoteName )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return FML_INVALID_HANDLE;
    }

    if( localName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Import has invalid local name." );  
        return FML_INVALID_HANDLE;
    }
    if( remoteName == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, "Import has invalid remote name." );
        return FML_INVALID_HANDLE;
    }
    
    FieldmlRegion *region = session->getRegion( importSourceIndex - 1 );
    if( region == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Invalid import source index." );  
        return FML_INVALID_HANDLE;
    }
    
    FmlObjectHandle remoteObject = region->getNamedObject( remoteName );
    FmlObjectHandle localObject = session->region->getNamedObject( localName );
    
    if( remoteObject == FML_INVALID_HANDLE )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, string( "Invalid import. Unknown remote object " ) + remoteName + "." );  
    }
    else if( localObject != FML_INVALID_HANDLE )
    {
        remoteObject = FML_INVALID_HANDLE;
        session->setError( FML_ERR_INVALID_PARAMETER_3, string( "Invalid import. Local name " ) + string( localName ) + " already used." );  
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
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    return session->region->getImportSourceCount();
}


int Fieldml_GetImportCount( FmlSessionHandle handle, int importSourceIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    return session->region->getImportCount( importSourceIndex - 1 );
}


int Fieldml_CopyImportSourceHref( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    string href = session->region->getImportSourceHref( importSourceIndex - 1 );
    if( href == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Invalid import source index." );
        return -1;
    }
    
    return cappedCopy( href.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportSourceRegionName( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    string regionName = session->region->getImportSourceRegionName( importSourceIndex - 1 );
    if( regionName == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Invalid import source index." );
        return -1;
    }
    
    return cappedCopy( regionName.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportLocalName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    string localName = session->region->getImportLocalName( importSourceIndex - 1, importIndex );
    if( localName == "" )
    {
        //TODO Report exactly which parameter was bad
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Invalid import index or source index." );
        return -1;
    }
    
    return cappedCopy( localName.c_str(), buffer, bufferLength );
}


int Fieldml_CopyImportRemoteName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return -1;
    }
    
    string remoteName = session->region->getImportRemoteName( importSourceIndex - 1, importIndex );
    if( remoteName == "" )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Invalid import or import source index." );
        return -1;
    }
    
    return cappedCopy( remoteName.c_str(), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetImportObject( FmlSessionHandle handle, int importSourceIndex, int importIndex )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
        return FML_INVALID_HANDLE;
    }

    return session->region->getImportObject( importSourceIndex - 1, importIndex );
}


FmlObjectHandle Fieldml_CreateHrefDataResource( FmlSessionHandle handle, const char * name, const char * format, const char * href )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create href data resource. Invalid name." );
        return FML_INVALID_HANDLE;
    }
    if( href == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot create href data resource. Invalid href." );
        return FML_INVALID_HANDLE;
    }
    if( format == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, "Cannot create href data resource. Invalid format." );
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new DataResource( name, FML_DATA_RESOURCE_HREF, format, href );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, dataResource );
}


FmlObjectHandle Fieldml_CreateInlineDataResource( FmlSessionHandle handle, const char * name )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create inline data resource. Invalid name." );
        return FML_INVALID_HANDLE;
    }

    DataResource *dataResource = new DataResource( name, FML_DATA_RESOURCE_INLINE, PLAIN_TEXT_NAME, "" );
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, dataResource );
}


FieldmlDataResourceType Fieldml_GetDataResourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_DATA_RESOURCE_UNKNOWN;
    }
    
    FieldmlObject *object = getObject( session, objectHandle );
    if( object->objectType != FHT_DATA_RESOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get data resource type. Must be a data resource." );
        return FML_DATA_RESOURCE_UNKNOWN;
    }
    
    DataResource *resource = (DataResource*)object;
    return resource->resourceType;
}


FmlErrorNumber Fieldml_AddInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char * data, const int length )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( data == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Cannot add inline data. Invalid data." );
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
    if( resource->resourceType != FML_DATA_RESOURCE_INLINE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot add inline data. Must be inline data resource." );
    }
    
    resource->description = resource->description + string( data, length );
    
    return session->getLastError();
}


FmlErrorNumber Fieldml_SetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char * data, const int length )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( data == NULL )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot set inline data. Invalid data." );
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
    if( resource->resourceType != FML_DATA_RESOURCE_INLINE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot set inline data. Must be inline data resource." );
    }
    
    resource->description = string( data, length );
    
    return session->getLastError();
}


int Fieldml_GetInlineDataLength( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return -1;
    }
    if( resource->resourceType != FML_DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get inline data length. Must be inline data resource." );
        return -1;
    }
    
    return resource->description.length();
}


char * Fieldml_GetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return NULL;
    }
    if( resource->resourceType != FML_DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get inline data. Must be inline data resource." );
        return NULL;
    }
    
    return cstrCopy( resource->description );
}


int Fieldml_CopyInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength, int offset )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }

    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return -1;
    }
    if( resource->resourceType != FML_DATA_RESOURCE_INLINE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot copy inline data. Must be inline data resource." );
        return -1;
    }
    
    if( offset >= (int)resource->description.length() )
    {
        return 0;
    }
    
    //This is probably not the best way to do this
    return cappedCopy( resource->description.c_str() + offset, buffer, bufferLength );
}


FieldmlDataSourceType Fieldml_GetDataSourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_DATA_SOURCE_UNKNOWN;
    }
    
    DataSource *dataSource = objectAsDataSource( session, objectHandle );
    if( dataSource == NULL )
    {
        return FML_DATA_SOURCE_UNKNOWN;
    }
    
    return dataSource->sourceType;
}


char * Fieldml_GetDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }

    DataResource *dataResource = getDataResource( session, objectHandle );
    if( dataResource == NULL )
    {
        return NULL;
    }
    
    if( dataResource->resourceType == FML_DATA_RESOURCE_HREF )
    {
        return cstrCopy( dataResource->description );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get data resource href. Must be href data resource." );
        return NULL;
    }
}


int Fieldml_CopyDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetDataResourceHref( handle, objectHandle ), buffer, bufferLength );
}


char * Fieldml_GetDataResourceFormat( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DENSE_ARRAY )
        {
            DenseArrayDataDescription *denseArray = (DenseArrayDataDescription*)parameter->dataDescription;
            return denseArray->dataSource;
        }
        else if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameter->dataDescription;
            return dokArray->valueSource;
        }
        else
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get data source. Invalid data description." );
            return FML_INVALID_HANDLE;
        }
    }

    FieldmlObject *object = getObject( session, objectHandle );

    if( object == NULL )
    {
        //Error has already been set.
    }
    else if( object->objectType == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        FieldmlEnsembleMembersType type = ensembleType->membersType;
        
        if( ( type != FML_ENSEMBLE_MEMBER_LIST_DATA ) && ( type != FML_ENSEMBLE_MEMBER_RANGE_DATA ) && ( type != FML_ENSEMBLE_MEMBER_STRIDE_RANGE_DATA ) )
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get data source. Invalid member description." );
            return FML_INVALID_HANDLE;
        }
        
        return ensembleType->dataSource;
    }
    else if( object->objectType == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetDataSource( handle, meshType->elementsType );
    }
    else
    {
        session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get data source. Invalid object." );
        return FML_INVALID_HANDLE;
    }
    
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }

    ParameterEvaluator *parameter = ParameterEvaluator::checkedCast( session, objectHandle );
    if( parameter != NULL )
    {
        if( parameter->dataDescription->descriptionType == FML_DATA_DESCRIPTION_DOK_ARRAY )
        {
            DokArrayDataDescription *dokArray = (DokArrayDataDescription*)parameter->dataDescription;
            return dokArray->keySource;
        }
        else
        {
            session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get key data source. Invalid data description." );
        }
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get key data source. Invalid object." );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    DataResource *resource = getDataResource( session, objectHandle );
    if( resource == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( index < 0 ) || (  (unsigned int)index >= resource->dataSources.size() ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Cannot get data source. Invalid index." );
        return FML_INVALID_HANDLE;
    }
    
    return resource->dataSources[index];
}


FmlObjectHandle Fieldml_GetDataSourceResource( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( session->region == NULL )
    {
        session->setError( FML_ERR_INVALID_REGION, "FieldML session has no region" );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return -1;
    }
    
    FieldmlObject *object = getObject( session, objectHandle );
    if( object == NULL )
    {
        return -1;
    }
    if( object->objectType != FHT_DATA_SOURCE )
    {
        session->setError( FML_ERR_INVALID_OBJECT, "Cannot get array data source rank. Must be a data source." );
        return -1;
    }
    
    DataSource *source = (DataSource*)object;
    if( source->sourceType == FML_DATA_SOURCE_ARRAY )
    {
        ArrayDataSource *arraySource = (ArrayDataSource*)source;
        return arraySource->rank;
    }
    
    session->setError( FML_ERR_INVALID_OBJECT, "Cannot get array data source rank. Must be an array data source." );
    return -1;
}


FmlErrorNumber Fieldml_GetArrayDataSourceSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
            return session->setError( FML_ERR_INVALID_PARAMETER_3, objectHandle, "Cannot set array data sizes. Invalid size." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
            return session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot set array data raw size. Invalid size." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

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
            return session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot set array data offset. Invalid offset." );
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
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create array data source. Invalid name." );
        return FML_INVALID_HANDLE;
    }
    if( location == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, "Cannot create array data source. Invalid location." );
        return FML_INVALID_HANDLE;
    }
    
    if( !checkLocal( session, resourceHandle ) )
    {
        return session->getLastError();
    }

    if( rank <= 0 )
    {
        return session->setError( FML_ERR_INVALID_PARAMETER_5, "Cannot create array data source. Invalid rank." );
    }
    
    FieldmlObject *object = getObject( session, resourceHandle );
    if( object == NULL )
    {
        return session->getLastError();
    }
    if( object->objectType != FHT_DATA_RESOURCE )
    {
        return session->setError( FML_ERR_INVALID_OBJECT, resourceHandle, "Cannot create array data source. Must be a data resource." );
    }
    
    DataResource *dataResource = getDataResource( session, resourceHandle );

    ArrayDataSource *source = new ArrayDataSource( name, dataResource, location, rank );

    session->setError( FML_ERR_NO_ERROR, "" );
    FmlObjectHandle sourceHandle = addObject( session, source );
    
    dataResource->dataSources.push_back( sourceHandle );
    
    return sourceHandle;
}


int Fieldml_CreateConstantEvaluator( FmlSessionHandle handle, const char * name, const char * literal, FmlObjectHandle valueType )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( name == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_2, "Cannot create constant evaluator. Invalid name." );
        return FML_INVALID_HANDLE;
    }
    if( literal == NULL )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_3, "Cannot create constant evaluator. Invalid value." );
        return FML_INVALID_HANDLE;
    }

    if( !checkLocal( session, valueType ) )
    {
        return session->getLastError();
    }

    if( !checkIsValueType( session, valueType, true, true, false, true ) )
    {
        session->setError( FML_ERR_INVALID_PARAMETER_4, valueType, "Cannot create constant evaluator. Invalid type." );
        return FML_INVALID_HANDLE;
    }

    ConstantEvaluator *evaluator = new ConstantEvaluator( name, literal, valueType );
    
    session->setError( FML_ERR_NO_ERROR, "" );
    return addObject( session, evaluator );
}


char * Fieldml_GetConstantEvaluatorValueString( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlSession *session = FieldmlSession::handleToSession( handle );
    ERROR_AUTOSTACK( session );

    if( session == NULL )
    {
        return NULL;
    }
    
    ConstantEvaluator *evaluator = ConstantEvaluator::checkedCast( session, objectHandle );
    if( evaluator != NULL )
    {
        return cstrCopy( evaluator->valueString );
    }

    session->setError( FML_ERR_INVALID_OBJECT, objectHandle, "Cannot get constant evaluator value. Invalid object." );
    return NULL;
}


int Fieldml_CopyConstantEvaluatorValueString( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetConstantEvaluatorValueString( handle, objectHandle ), buffer, bufferLength );
}
