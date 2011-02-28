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

#include "fieldml_api.h"
#include "fieldml_sax.h"
#include "fieldml_structs.h"
#include "fieldml_write.h"
#include "fieldml_validate.h"
#include "string_const.h"

#include "ParameterReader.h"
#include "ParameterWriter.h"
#include "Region.h"

using namespace std;

//========================================================================
//
// Utility
//
//========================================================================


static SimpleMap<int, FmlObjectHandle> *getEvaluatorMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

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
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getBindMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

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
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static std::vector<FmlObjectHandle> *getVariableList( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

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

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SimpleMap<int, string> *getShapeMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return &meshType->shapes;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SemidenseDataDescription *getSemidenseDataDescription( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

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

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
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


static InlineDataLocation *getInlineDataLocation( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle ); 

    if( semidense->dataLocation->locationType == LOCATION_INLINE )
    {
        return (InlineDataLocation*)semidense->dataLocation;
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static FileDataLocation *getFileDataLocation( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle ); 

    if( semidense->dataLocation->locationType == LOCATION_FILE )
    {
        return (FileDataLocation*)semidense->dataLocation;
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static bool checkIsValueType( FmlHandle handle, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble, bool allowMesh )
{
    FieldmlObject *object = handle->getObject( objectHandle );
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


static bool checkIsEvaluatorType( FmlHandle handle, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble )
{
    FieldmlObject *object = handle->getObject( objectHandle );
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
    
    return checkIsValueType( handle, Fieldml_GetValueType( handle, objectHandle ), allowContinuous, allowEnsemble, false );
}


static bool checkIsTypeCompatible( FmlHandle handle, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsValueType( handle, objectHandle1, true, true, false ) )
    {
        return false;
    }
    if( !checkIsValueType( handle, objectHandle2, true, true, false ) )
    {
        return false;
    }

    FieldmlObject *object1 = handle->getObject( objectHandle1 );
    FieldmlObject *object2 = handle->getObject( objectHandle2 );
    
    if( ( object1->type == FHT_ENSEMBLE_TYPE ) && ( object2->type == FHT_ENSEMBLE_TYPE ) )
    {
        return objectHandle1 == objectHandle2;
    }
    else if( ( object1->type == FHT_CONTINUOUS_TYPE ) && ( object2->type == FHT_CONTINUOUS_TYPE ) )
    {
        return Fieldml_GetTypeComponentCount( handle, objectHandle1 ) == Fieldml_GetTypeComponentCount( handle, objectHandle2 );
    }
    else
    {
        return false;
    }
}


static bool checkIsEvaluatorTypeCompatible( FmlHandle handle, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsEvaluatorType( handle, objectHandle1, true, true ) )
    {
        return false;
    }
    if( !checkIsEvaluatorType( handle, objectHandle2, true, true ) )
    {
        return false;
    }
    
    FmlObjectHandle typeHandle1 = Fieldml_GetValueType( handle, objectHandle1 );
    FmlObjectHandle typeHandle2 = Fieldml_GetValueType( handle, objectHandle2 );
    
    return checkIsTypeCompatible( handle, typeHandle1, typeHandle2 );
}


//========================================================================
//
// API
//
//========================================================================

FmlHandle Fieldml_CreateFromFile( const char *filename )
{
    FmlHandle region = parseFieldmlFile( filename, LOCAL_LOCATION_HANDLE, NULL );
    
    region->finalize();
    
    return region;
}


FmlHandle Fieldml_Create( const char *location, const char *name, const char *libraryLocation )
{
    if( libraryLocation == NULL )
    {
        libraryLocation = "";
    }
    
    return new FieldmlRegion( location, name, libraryLocation );
}


int Fieldml_SetDebug( FmlHandle handle, const int debug )
{
    handle->setDebug( debug );
    
    return handle->setRegionError( FML_ERR_NO_ERROR );
}


int Fieldml_GetLastError( FmlHandle handle )
{
    return handle->getLastError();
}


int Fieldml_WriteFile( FmlHandle handle, const char *filename )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    handle->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( handle, filename );
}


void Fieldml_Destroy( FmlHandle handle )
{
    delete handle;
}


const char * Fieldml_GetName( FmlHandle handle )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( handle->getName() );
}


int Fieldml_CopyName( FmlHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetName( handle ), buffer, bufferLength );
}


const char * Fieldml_GetLibraryName( FmlHandle handle )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( handle->getLibraryName() );
}


int Fieldml_CopyLibraryName( FmlHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetLibraryName( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlHandle handle )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->getErrorCount();
}


const char * Fieldml_GetError( FmlHandle handle, int index )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( handle->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlHandle handle, int index, char *buffer, int bufferLength )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return cappedCopyAndFree( Fieldml_GetError( handle, index ), buffer, bufferLength );
}


int Fieldml_GetTotalObjectCount( FmlHandle handle )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->getTotal();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlHandle handle, const int index )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->getObjectByIndex( index );
}


int Fieldml_GetObjectCount( FmlHandle handle, FieldmlHandleType type )
{
    handle->setRegionError( FML_ERR_NO_ERROR );
    if( type == FHT_UNKNOWN )
    {
        return -1;
    }

    return handle->getTotal( type );
}


FmlObjectHandle Fieldml_GetObject( FmlHandle handle, FieldmlHandleType type, int index )
{
    handle->setRegionError( FML_ERR_NO_ERROR );

    FmlObjectHandle object = handle->getNthHandle( type, index );
    
    if( object == FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
    }
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByName( FmlHandle handle, const char * name )
{
    return handle->getNamedHandle( name );
}


FieldmlHandleType Fieldml_GetObjectType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FHT_UNKNOWN;
    }
    
    return object->type;
}


int Fieldml_ValidateObject( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }
    
    return validateFieldmlObject( handle, object );
}


FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

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

    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetTypeComponentCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FmlObjectHandle componentTypeHandle = Fieldml_GetTypeComponentEnsemble( handle, objectHandle );
    
    if( componentTypeHandle == FML_INVALID_HANDLE )
    {
        if( handle->getLastError() == FML_ERR_NO_ERROR )
        {
            return 1;
        }
        return -1;
    }
    
    return Fieldml_GetEnsembleTypeElementCount( handle, componentTypeHandle );
}


TypeBoundsType Fieldml_GetBoundsType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return BOUNDS_UNKNOWN;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->bounds->boundsType;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return BOUNDS_UNKNOWN;
}


int Fieldml_GetEnsembleTypeElementCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        if( ensembleType->bounds->boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            ContiguousBounds *contiguous = (ContiguousBounds *)ensembleType->bounds;
            return contiguous->count;
        }
        
        handle->setRegionError( FML_ERR_UNSUPPORTED );  
        return -1;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_IsEnsembleComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->isComponentEnsemble;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_GetContiguousBoundsCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return -1;
    }
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        if( ensembleType->bounds->boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            ContiguousBounds *contiguous = (ContiguousBounds *)ensembleType->bounds;
            handle->setRegionError( FML_ERR_NO_ERROR );  
            return contiguous->count;
        }
        
        handle->setRegionError( FML_ERR_INVALID_OBJECT );  
        return -1;
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        FieldmlObject *subObject;
        
        subObject = handle->getObject( meshType->elementType );
        
        if( ( subObject == NULL ) || ( subObject->type != FHT_ENSEMBLE_TYPE ) )
        {
            return handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
        }
        
        return Fieldml_GetContiguousBoundsCount( handle, meshType->elementType );
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );  
        return -1;
    }


}


int Fieldml_SetContiguousBoundsCount( FmlHandle handle, FmlObjectHandle objectHandle, int count )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }
    
    if( count < 1 )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        delete ensembleType->bounds;
        ensembleType->bounds = new ContiguousBounds( count );
        
        return handle->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;

        FieldmlObject *subObject;
        
        subObject = handle->getObject( meshType->elementType );
        
        if( ( subObject == NULL ) || ( subObject->type != FHT_ENSEMBLE_TYPE ) )
        {
            return handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
        }
        
        EnsembleType *ensembleType = (EnsembleType*)subObject;
        delete ensembleType->bounds;
        ensembleType->bounds = new ContiguousBounds( count );
        
        return handle->getLastError();
    }

    return handle->setRegionError( FML_ERR_INVALID_OBJECT );
}


FmlObjectHandle Fieldml_GetMeshElementType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->elementType;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


const char * Fieldml_GetMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    SimpleMap<int, string> *map = getShapeMap( handle, objectHandle ); 
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
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->xiType;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshXiComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetTypeComponentEnsemble( handle, meshType->xiType );
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


const char * Fieldml_GetObjectName( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    return cstrCopy( object->name );
}


int Fieldml_CopyObjectName( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetObjectName( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_SetObjectInt( FmlHandle handle, FmlObjectHandle objectHandle, int value )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }

    object->intValue = value;
    return handle->getLastError();
}


int Fieldml_GetObjectInt( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return 0;
    }

    return object->intValue;
}


FmlObjectHandle Fieldml_GetValueType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    handle->setRegionError( FML_ERR_NO_ERROR );
    
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
    else if( object->type == FHT_ELEMENT_SET )
    {
        ElementSet *set = (ElementSet *)object;
        return set->valueType;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateAbstractEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    if( !checkIsValueType( handle, valueType, true, true, true ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    AbstractEvaluator *abstractEvaluator = new AbstractEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( abstractEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    if( !checkIsValueType( handle, valueType, true, true, false ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ExternalEvaluator *externalEvaluator = new ExternalEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( externalEvaluator );
}


FmlObjectHandle Fieldml_CreateParametersEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    if( !checkIsValueType( handle, valueType, true, true, false ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ParameterEvaluator *parameterEvaluator = new ParameterEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( parameterEvaluator );
}


int Fieldml_SetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType != DESCRIPTION_UNKNOWN )
        {
            return handle->setRegionError( FML_ERR_ACCESS_VIOLATION );
        }

        if( description == DESCRIPTION_SEMIDENSE )
        {
            delete parameterEvaluator->dataDescription;
            parameterEvaluator->dataDescription = new SemidenseDataDescription();
            return handle->getLastError();
        }
        else
        {
            return handle->setRegionError( FML_ERR_UNSUPPORTED );  
        }
    }

    return handle->setRegionError( FML_ERR_INVALID_OBJECT );
}

DataDescriptionType Fieldml_GetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return DESCRIPTION_UNKNOWN;
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return parameterEvaluator->dataDescription->descriptionType;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return DESCRIPTION_UNKNOWN;
}


DataLocationType Fieldml_GetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    
    if( semidense == NULL )
    {
        return LOCATION_UNKNOWN;
    }

    return semidense->dataLocation->locationType;
}


int Fieldml_SetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle, DataLocationType location )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );

    if( semidense->dataLocation->locationType != LOCATION_UNKNOWN )
    {
        return handle->setRegionError( FML_ERR_ACCESS_VIOLATION );
    }

    if( location == LOCATION_INLINE )
    {
        delete semidense->dataLocation;
        semidense->dataLocation = new InlineDataLocation();
        return handle->setRegionError( FML_ERR_NO_ERROR );
    }
    else if( location == LOCATION_FILE )
    {
        delete semidense->dataLocation;
        semidense->dataLocation = new FileDataLocation();
        return handle->setRegionError( FML_ERR_NO_ERROR );
    }
    else
    {
        return handle->setRegionError( FML_ERR_UNSUPPORTED );
    }
}


int Fieldml_AddParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle, const char *data, const int length )
{
    InlineDataLocation *location = getInlineDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return handle->getLastError();
    }

    char *newData = new char[location->length + length + 1];
    memcpy( newData, location->data, location->length );
    memcpy( newData + location->length, data, length );

    delete[] location->data;
    location->data = newData;
    
    location->length += length;
    newData[location->length] = 0;
    
    return handle->getLastError();
}


int Fieldml_GetParameterInlineDataLength( FmlHandle handle, FmlObjectHandle objectHandle )
{
    InlineDataLocation *location = getInlineDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return -1;
    }
    
    return location->length;
}


const char * Fieldml_GetParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle )
{
    InlineDataLocation *location = getInlineDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return NULL;
    }
    
    return strdupS( location->data );
}


int Fieldml_CopyInlineParameterData( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset )
{
    InlineDataLocation *location = getInlineDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return -1;
    }
    
    if( offset >= location->length )
    {
        return 0;
    }
    
    return cappedCopy( location->data + offset, buffer, bufferLength );
}


int Fieldml_SetParameterFileData( FmlHandle handle, FmlObjectHandle objectHandle, const char * filename, DataFileType type, int offset )
{
    FileDataLocation *location = getFileDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return handle->getLastError();
    }

    location->filename = filename;
    location->fileType = type;
    location->offset = offset;
    
    return handle->getLastError();
}


const char * Fieldml_GetParameterDataFilename( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FileDataLocation *location = getFileDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return NULL;
    }

    return cstrCopy( location->filename );
}


int Fieldml_CopyParameterDataFilename( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetParameterDataFilename( handle, objectHandle ), buffer, bufferLength );
}


int Fieldml_GetParameterDataOffset( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FileDataLocation *location = getFileDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return -1;
    }

    return location->offset;
}


DataFileType Fieldml_GetParameterDataFileType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FileDataLocation *location = getFileDataLocation( handle, objectHandle );
    if( location == NULL )
    {
        return TYPE_UNKNOWN;
    }

    return location->fileType;
}


int Fieldml_AddDenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle setHandle )
{
    if( !checkIsEvaluatorType( handle, indexHandle, false, true ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return handle->getLastError();
    }

    semidense->denseIndexes.push_back( indexHandle );
    semidense->denseSets.push_back( setHandle );
    
    return handle->getLastError();
}


int Fieldml_AddSparseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    if( !checkIsEvaluatorType( handle, indexHandle, false, true ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
        
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return handle->getLastError();
    }

    semidense->sparseIndexes.push_back( indexHandle );
    
    return handle->getLastError();
}


int Fieldml_GetSemidenseIndexCount( FmlHandle handle, FmlObjectHandle objectHandle, int isSparse )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
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
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
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
    
    handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    
    return FML_INVALID_HANDLE;
}


int Fieldml_SetSwizzle( FmlHandle handle, FmlObjectHandle objectHandle, const int *buffer, int count )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return handle->getLastError();
    }
    
    FmlObjectHandle evaluatorHandle = Fieldml_GetSemidenseIndexEvaluator( handle, objectHandle, 1, 0 );
    FmlObjectHandle ensembleHandle = Fieldml_GetValueType( handle, evaluatorHandle );
    int ensembleCount = Fieldml_GetEnsembleTypeElementCount( handle, ensembleHandle );
    
    if( ensembleCount != count )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }
    
    if( semidense->swizzle != NULL )
    {
        delete[] semidense->swizzle;
    }
    
    int *ints = new int[count];
    memcpy( ints, buffer, sizeof( int ) * count );
    
    semidense->swizzleCount = count;
    semidense->swizzle = ints;
    
    return handle->getLastError();
}


int Fieldml_GetSwizzleCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return -1;
    }
    
    return semidense->swizzleCount;
}


const int * Fieldml_GetSwizzleData( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return NULL;
    }
    
    return semidense->swizzle;
}


int Fieldml_CopySwizzleData( FmlHandle handle, FmlObjectHandle objectHandle, int *buffer, int bufferLength )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return -1;
    }

    int length = semidense->swizzleCount;
    
    if( length > bufferLength )
    {
        length = bufferLength;
    }
    
    memcpy( buffer, semidense->swizzle, length * sizeof( int ) );
    
    return length;
}


FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    if( !checkIsValueType( handle, valueType, true, true, false ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    PiecewiseEvaluator *piecewiseEvaluator = new PiecewiseEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( piecewiseEvaluator );
}


FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    if( !checkIsValueType( handle, valueType, true, false, false ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    AggregateEvaluator *aggregateEvaluator = new AggregateEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( aggregateEvaluator );
}


int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( handle, evaluator, true, false ) )
        {
            return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( handle, objectHandle, evaluator ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    FieldmlObject *object = handle->getObject( objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return handle->getLastError();
    }

    if( ( object->type == FHT_PIECEWISE_EVALUATOR ) || ( object->type == FHT_AGGREGATE_EVALUATOR ) )
    {
        map->setDefault( evaluator );
        return handle->getLastError();
    }
    else
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }
}


int Fieldml_GetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
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
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
}


int Fieldml_SetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int element, FmlObjectHandle evaluator )
{
    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( handle, evaluator, true, false ) )
        {
            return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( handle, objectHandle, evaluator ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map != NULL )
    {
        map->set( element, evaluator );
    }

    return handle->getLastError();
}


int Fieldml_GetEvaluatorCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->size();
}


int Fieldml_GetEvaluatorElement( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetElementEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->get( elementNumber, allowDefault == 1 );
}


FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlHandle handle, const char * name, FmlObjectHandle remoteEvaluator )
{
    FmlObjectHandle valueType = Fieldml_GetValueType( handle, remoteEvaluator );

    ReferenceEvaluator *referenceEvaluator = new ReferenceEvaluator( name, LOCAL_LOCATION_HANDLE, remoteEvaluator, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( referenceEvaluator );
}


FmlObjectHandle Fieldml_GetReferenceRemoteEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return referenceEvaluator->remoteEvaluator;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetVariableCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    vector<FmlObjectHandle> *variables = getVariableList( handle, objectHandle );
    if( variables == NULL )
    {
        return -1;
    }
    
    return variables->size();
}


FmlObjectHandle Fieldml_GetVariable( FmlHandle handle, FmlObjectHandle objectHandle, int variableIndex )
{
    vector<FmlObjectHandle> *variables = getVariableList( handle, objectHandle );
    if( variables == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( variableIndex < 1 ) || ( variableIndex > variables->size() ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return variables->at( variableIndex - 1 );
}


int Fieldml_AddVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    vector<FmlObjectHandle> *variables = getVariableList( handle, objectHandle );
    if( variables == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( std::find( variables->begin(), variables->end(), evaluatorHandle ) == variables->end() )
    {
        variables->push_back( evaluatorHandle );
    }

    return handle->getLastError();
}


int Fieldml_GetBindCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( handle, objectHandle );
    if( map == NULL )
    {
        return -1;
    }
    
    return map->size();
}


FmlObjectHandle Fieldml_GetBindVariable( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetBindEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetBindByVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( handle, objectHandle );
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
    if( !checkIsEvaluatorTypeCompatible( handle, variableHandle, sourceHandle ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( handle, objectHandle );
    if( map == NULL )
    {
        return handle->getLastError();
    }
    
    map->set( variableHandle, sourceHandle );
    return handle->getLastError();
}



int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return -1;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
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
        
        handle->setRegionError( FML_ERR_UNSUPPORTED );
        return -1;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}


int Fieldml_SetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    if( !checkIsEvaluatorType( handle, evaluatorHandle, false, true ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }

    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            piecewise->indexEvaluator = evaluatorHandle;
            return handle->getLastError();
        }
        else
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            aggregate->indexEvaluator = evaluatorHandle;
            return handle->getLastError();
        }
        else
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
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
                return handle->getLastError();
            }

            index -= count;
            count = semidense->denseIndexes.size();

            if( index <= count )
            {
                semidense->denseIndexes[index - 1] = evaluatorHandle;
                return handle->getLastError();
            }
            
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            handle->setRegionError( FML_ERR_UNSUPPORTED );
        }
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return handle->getLastError();
}


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            return piecewise->indexEvaluator;
        }
        
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            return aggregate->indexEvaluator;
        }
        
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
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
            
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            handle->setRegionError( FML_ERR_UNSUPPORTED );
        }
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );

    return FML_INVALID_HANDLE;
}


int Fieldml_GetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
    
    int count;
    
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
    if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
        count = semidense->sparseIndexes.size();
        
        if( index <= count )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }

        index -= count;
        count = semidense->denseIndexes.size();

        if( index <= count )
        {
            return semidense->denseSets[index - 1];
        }
        
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }

    return FML_INVALID_HANDLE;
}




int Fieldml_SetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle setHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }
    
    if( index <= 0 )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return handle->getLastError();
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return handle->getLastError();
    }
    
    FieldmlHandleType type = Fieldml_GetObjectType( handle, setHandle );
    if( type != FHT_ELEMENT_SET )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
        return handle->getLastError();
    }
    
    int count;
    
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
    if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
        count = semidense->sparseIndexes.size();
        
        if( index <= count )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return handle->getLastError();
        }

        index -= count;
        count = semidense->denseIndexes.size();

        if( index <= count )
        {
            semidense->denseSets[index - 1] = setHandle;
            return FML_ERR_NO_ERROR;
        }
        
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }

    return handle->getLastError();
}


FmlObjectHandle Fieldml_CreateContinuousType( FmlHandle handle, const char * name, FmlObjectHandle componentDescriptionHandle )
{
    if( ( componentDescriptionHandle != FML_INVALID_HANDLE ) )
    {
        if( !checkIsValueType( handle, componentDescriptionHandle, false, true, false ) )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }
    
    if( componentDescriptionHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_IsEnsembleComponentType( handle, componentDescriptionHandle ) != 1 )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }

    ContinuousType *continuousType = new ContinuousType( name, LOCAL_LOCATION_HANDLE, componentDescriptionHandle );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( continuousType );
}


FmlObjectHandle Fieldml_CreateEnsembleType( FmlHandle handle, const char * name, const int isComponentType )
{
    EnsembleType *ensembleType = new EnsembleType( name, LOCAL_LOCATION_HANDLE, isComponentType == 1 );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlHandle handle, const char * name, FmlObjectHandle xiEnsembleDescription )
{
    FmlObjectHandle xiHandle, elementHandle;
    string xiName, elementsName;

    if( ( xiEnsembleDescription == FML_INVALID_HANDLE ) ||
        ( Fieldml_GetObjectType( handle, xiEnsembleDescription ) != FHT_ENSEMBLE_TYPE ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    xiName = name;
    xiName += ".xi";
    if( Fieldml_GetObjectByName( handle, xiName.c_str() ) != FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    elementsName = name;
    elementsName += ".elements";
    if( Fieldml_GetObjectByName( handle, elementsName.c_str() ) != FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    
    ContinuousType *xiObject = new ContinuousType( xiName.c_str(), VIRTUAL_LOCATION_HANDLE, xiEnsembleDescription );
    xiHandle = handle->addObject( xiObject );
    
    EnsembleType *elementObject = new EnsembleType( elementsName.c_str(), VIRTUAL_LOCATION_HANDLE, false );
    elementHandle = handle->addObject( elementObject );
    
    MeshType *meshType = new MeshType( name, LOCAL_LOCATION_HANDLE, xiHandle, elementHandle );

    handle->setRegionError( FML_ERR_NO_ERROR );

    return handle->addObject( meshType );
}


int Fieldml_SetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle, const char * shape )
{
    SimpleMap<int, string> *map = getShapeMap( handle, objectHandle ); 
    if( map == NULL )
    {
        return handle->getLastError();
    }
    
    map->setDefault( shape );
    return handle->getLastError();
}


const char * Fieldml_GetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SimpleMap<int, string> *map = getShapeMap( handle, objectHandle ); 
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
    SimpleMap<int, string> *map = getShapeMap( handle, objectHandle ); 
    if( map == NULL )
    {
        return handle->getLastError();
    }

    map->set( elementNumber, shape );
    return handle->getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );

    if( semidense == NULL )
    {
        return NULL;
    }

    FieldmlObject *object = handle->getObject( objectHandle );
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;

    ParameterReader *reader = NULL;
    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) ||
        ( semidense->dataLocation->locationType == LOCATION_INLINE ) )
    {
        reader = ParameterReader::create( handle, parameterEvaluator );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }
    
    return reader;
}


int Fieldml_ReadIndexSet( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer )
{
    int err;
    
    if( reader == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    err = reader->readNextIndexSet( indexBuffer );
    
    return handle->setRegionError( err );
}


int Fieldml_ReadIntValues( FmlHandle handle, FmlReaderHandle reader, int *valueBuffer, int bufferSize )
{
    if( reader == NULL )
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readIntValues( valueBuffer, bufferSize );
}


int Fieldml_ReadDoubleValues( FmlHandle handle, FmlReaderHandle reader, double *valueBuffer, int bufferSize )
{
    if( reader == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readDoubleValues( valueBuffer, bufferSize );
}


int Fieldml_CloseReader( FmlHandle handle, FmlReaderHandle reader )
{
    if( reader == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    delete reader;
    
    return handle->setRegionError( FML_ERR_NO_ERROR );
}


FmlWriterHandle Fieldml_OpenWriter( FmlHandle handle, FmlObjectHandle objectHandle, int append )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );

    if( semidense == NULL )
    {
        return NULL;
    }

    FieldmlObject *object = handle->getObject( objectHandle );
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;

    ParameterWriter *writer = NULL;
    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) ||
        ( semidense->dataLocation->locationType == LOCATION_INLINE ) )
    {
        writer = ParameterWriter::create( handle, parameterEvaluator, ( append == 1 ));
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }
    
    return writer;
}


int Fieldml_WriteIndexSet( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer )
{
    int err;
    
    if( writer == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    err = writer->writeNextIndexSet( indexBuffer );
    
    return handle->setRegionError( err );
}


int Fieldml_WriteIntValues( FmlHandle handle, FmlWriterHandle writer, int *valueBuffer, int valueCount )
{
    if( writer == NULL )
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeIntValues( valueBuffer, valueCount );
}


int Fieldml_WriteDoubleValues( FmlHandle handle, FmlWriterHandle writer, double *valueBuffer, int valueCount )
{
    if( writer == NULL )
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeDoubleValues( valueBuffer, valueCount );
}


int Fieldml_CloseWriter( FmlHandle handle, FmlWriterHandle writer )
{
    if( writer == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    delete writer;
    
    return handle->setRegionError( FML_ERR_NO_ERROR );
}


FmlObjectHandle Fieldml_CreateElementSet( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    ElementSet *elementSet = new ElementSet( name, LOCAL_LOCATION_HANDLE, valueType );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( elementSet );
}


int Fieldml_AddElementEntries( FmlHandle handle, FmlObjectHandle setHandle, const int * elements, const int elementCount )
{
    FieldmlObject *object = handle->getObject( setHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }

    if( object->type == FHT_ELEMENT_SET )
    {
        ElementSet *elementSet = (ElementSet*)object;
        
        for( int i = 0; i < elementCount; i++ )
        {
            if( elements[i] < 1 )
            {
                return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            }
            if( elementSet->presentElements.size() <= elements[i] )
            {
                int count = elements[i] - elementSet->presentElements.size();
                elementSet->presentElements.insert( elementSet->presentElements.end(), count + 1, false );
            }

            elementSet->presentElements[ elements[i] ] = true;
            if( elements[i] > elementSet->maxElement )
            {
                elementSet->maxElement = elements[i];
            }
        }
        
        return handle->getLastError();
    }

    return handle->setRegionError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_AddElementRange( FmlHandle handle, FmlObjectHandle setHandle, const int minElement, const int maxElement )
{
    FieldmlObject *object = handle->getObject( setHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }

    if( ( minElement < 1 ) || ( minElement > maxElement ) )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    if( maxElement < 1 )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }
    
    if( object->type == FHT_ELEMENT_SET )
    {
        ElementSet *elementSet = (ElementSet*)object;

        if( elementSet->presentElements.size() <= maxElement )
        {
            int count = maxElement - elementSet->presentElements.size();
            elementSet->presentElements.insert( elementSet->presentElements.end(), count + 1, false );
        }

        for( int i = minElement; i <= maxElement; i++ )
        {
            elementSet->presentElements[ i ] = true;
        }
        if( maxElement > elementSet->maxElement )
        {
            elementSet->maxElement = maxElement;
        }
        
        return handle->getLastError();
    }

    return handle->setRegionError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_GetElementCount( FmlHandle handle, FmlObjectHandle setHandle )
{
    FieldmlObject *object = handle->getObject( setHandle );

    if( object == NULL )
    {
        return -1;
    }

    if( object->type == FHT_ELEMENT_SET )
    {
        ElementSet *elementSet = (ElementSet*)object;
        int count = 0;
        for( int i = 0; i <= elementSet->maxElement; i++ )
        {
            if( elementSet->presentElements[i] )
            {
                count++;
            }
        }
        
        return count;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}


int Fieldml_GetElementEntry( FmlHandle handle, FmlObjectHandle setHandle, const int index )
{
    FieldmlObject *object = handle->getObject( setHandle );

    if( object == NULL )
    {
        return -1;
    }

    if( object->type == FHT_ELEMENT_SET )
    {
        ElementSet *elementSet = (ElementSet*)object;

        if( ( index < 1 ) || ( index > elementSet->presentElements.size() ) )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
            return -1;
        }

        if( index == elementSet->lastSetCount )
        {
            return elementSet->lastIndex;
        }
        
        if( index < elementSet->lastSetCount )
        {
            elementSet->lastSetCount = 0;
            elementSet->lastIndex = 0;
        }
        
        int setCount = index - elementSet->lastSetCount;
        int testIndex = elementSet->lastIndex;
        
        //Look for the setCount'th set bit starting at the one after testIndex.
        while( ( setCount > 0 ) && ( testIndex < elementSet->presentElements.size() ) )
        {
            testIndex++;
            if( elementSet->presentElements[ testIndex ] )
            {
                setCount--;
            }
        }
        
        if( setCount > 0 )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
            return -1;
        }
        
        elementSet->lastSetCount = index;
        elementSet->lastIndex = testIndex;
        
        return testIndex;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}


int Fieldml_GetElementEntries( FmlHandle handle, FmlObjectHandle setHandle, const int firstIndex, int * elements, const int count )
{
    FieldmlObject *object = handle->getObject( setHandle );

    if( object == NULL )
    {
        return -1;
    }

    if( object->type == FHT_ELEMENT_SET )
    {
        if( count < 1 )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_5 );  
            return -1;
        }

        int actualCount = 0;
        for( int i = 0; i < count; i++ )
        {
            elements[i] = Fieldml_GetElementEntry( handle, setHandle, firstIndex + i );
            if( elements[i] == -1 )
            {
                break;
            }
            actualCount++;
        }
        
        if( actualCount == 0 )
        {
            return -1;
        }

        handle->setRegionError( FML_ERR_NO_ERROR );
        return actualCount;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}
