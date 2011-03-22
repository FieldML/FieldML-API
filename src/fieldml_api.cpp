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


static SimpleMap<int, FmlObjectHandle> *getEvaluatorMap( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = region->getObject( objectHandle );

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
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getBindMap( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = region->getObject( objectHandle );

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
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static std::vector<FmlObjectHandle> *getVariableList( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = region->getObject( objectHandle );

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

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SimpleMap<int, string> *getShapeMap( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return &meshType->shapes;
    }

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SemidenseDataDescription *getSemidenseDataDescription( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = region->getObject( objectHandle );

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

    region->setRegionError( FML_ERR_INVALID_OBJECT );
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


static InlineDataLocation *getInlineDataLocation( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return NULL;
    }

    if( semidense->dataLocation->locationType == LOCATION_INLINE )
    {
        return (InlineDataLocation*)semidense->dataLocation;
    }
    else
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static FileDataLocation *getFileDataLocation( FieldmlRegion *region, FmlObjectHandle objectHandle )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return NULL;
    }

    if( semidense->dataLocation->locationType == LOCATION_FILE )
    {
        return (FileDataLocation*)semidense->dataLocation;
    }
    else
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static bool checkIsValueType( FieldmlRegion *region, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble, bool allowMesh )
{
    FieldmlObject *object = region->getObject( objectHandle );
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


static bool checkIsEvaluatorType( FieldmlRegion *region, FmlObjectHandle objectHandle, bool allowContinuous, bool allowEnsemble )
{
    FieldmlObject *object = region->getObject( objectHandle );
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
    
    return checkIsValueType( region, Fieldml_GetValueType( region->getRegionHandle(), objectHandle ), allowContinuous, allowEnsemble, false );
}


static bool checkIsTypeCompatible( FieldmlRegion *region, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsValueType( region, objectHandle1, true, true, false ) )
    {
        return false;
    }
    if( !checkIsValueType( region, objectHandle2, true, true, false ) )
    {
        return false;
    }

    FieldmlObject *object1 = region->getObject( objectHandle1 );
    FieldmlObject *object2 = region->getObject( objectHandle2 );
    
    if( ( object1->type == FHT_ENSEMBLE_TYPE ) && ( object2->type == FHT_ENSEMBLE_TYPE ) )
    {
        return objectHandle1 == objectHandle2;
    }
    else if( ( object1->type == FHT_CONTINUOUS_TYPE ) && ( object2->type == FHT_CONTINUOUS_TYPE ) )
    {
        FmlObjectHandle component1 = Fieldml_GetTypeComponentEnsemble( region->getRegionHandle(), objectHandle1 );
        FmlObjectHandle component2 = Fieldml_GetTypeComponentEnsemble( region->getRegionHandle(), objectHandle2 );
        
        if( ( component1 == FML_INVALID_HANDLE ) && ( component2 == FML_INVALID_HANDLE ) )
        {
            return true;
        }
        else if( ( component1 == FML_INVALID_HANDLE ) || ( component2 == FML_INVALID_HANDLE ) )
        {
            return false;
        }
        
        return Fieldml_GetTypeComponentCount( region->getRegionHandle(), objectHandle1 ) == Fieldml_GetTypeComponentCount( region->getRegionHandle(), objectHandle2 );
    }
    else
    {
        return false;
    }
}


static bool checkIsEvaluatorTypeCompatible( FieldmlRegion *region, FmlObjectHandle objectHandle1, FmlObjectHandle objectHandle2 )
{
    if( !checkIsEvaluatorType( region, objectHandle1, true, true ) )
    {
        return false;
    }
    if( !checkIsEvaluatorType( region, objectHandle2, true, true ) )
    {
        return false;
    }
    
    FmlObjectHandle typeHandle1 = Fieldml_GetValueType( region->getRegionHandle(), objectHandle1 );
    FmlObjectHandle typeHandle2 = Fieldml_GetValueType( region->getRegionHandle(), objectHandle2 );
    
    return checkIsTypeCompatible( region, typeHandle1, typeHandle2 );
}


//========================================================================
//
// API
//
//========================================================================

FmlHandle Fieldml_CreateFromFile( const char *filename )
{
    FieldmlRegion *region = parseFieldmlFile( filename, LOCAL_LOCATION_HANDLE, NULL );
    
    region->finalize();
    
    return region->getRegionHandle();
}


FmlHandle Fieldml_Create( const char *location, const char *name, const char *libraryLocation )
{
    if( libraryLocation == NULL )
    {
        libraryLocation = "";
    }
    
    FieldmlRegion *region = new FieldmlRegion( location, name, libraryLocation );
    return region->getRegionHandle();
}


int Fieldml_SetDebug( FmlHandle handle, const int debug )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    region->setDebug( debug );
    
    return region->setRegionError( FML_ERR_NO_ERROR );
}


int Fieldml_GetLastError( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    return region->getLastError();
}


int Fieldml_WriteFile( FmlHandle handle, const char *filename )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    region->setRoot( getDirectory( filename ) );

    return writeFieldmlFile( handle, filename );
}


void Fieldml_Destroy( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return;
    }
        
    delete region;
}


const char * Fieldml_GetName( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( region->getName() );
}


int Fieldml_CopyName( FmlHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetName( handle ), buffer, bufferLength );
}


const char * Fieldml_GetLibraryName( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( region->getLibraryName() );
}


int Fieldml_CopyLibraryName( FmlHandle handle, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetLibraryName( handle ), buffer, bufferLength );
}


int Fieldml_GetErrorCount( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->getErrorCount();
}


const char * Fieldml_GetError( FmlHandle handle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return cstrCopy( region->getError( index - 1 ) );
}


int Fieldml_CopyError( FmlHandle handle, int index, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetError( handle, index ), buffer, bufferLength );
}


int Fieldml_GetTotalObjectCount( FmlHandle handle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->getTotal();
}


FmlObjectHandle Fieldml_GetObjectByIndex( FmlHandle handle, const int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->getObjectByIndex( index );
}


int Fieldml_GetObjectCount( FmlHandle handle, FieldmlHandleType type )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );
    if( type == FHT_UNKNOWN )
    {
        return -1;
    }

    return region->getTotal( type );
}


FmlObjectHandle Fieldml_GetObject( FmlHandle handle, FieldmlHandleType type, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    region->setRegionError( FML_ERR_NO_ERROR );

    FmlObjectHandle object = region->getNthHandle( type, index );
    
    if( object == FML_INVALID_HANDLE )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
    }
    
    return object;
}


FmlObjectHandle Fieldml_GetObjectByName( FmlHandle handle, const char * name )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    return region->getNamedHandle( name );
}


FieldmlHandleType Fieldml_GetObjectType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FHT_UNKNOWN;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FHT_UNKNOWN;
    }
    
    return object->type;
}


FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

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

    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetTypeComponentCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    FmlObjectHandle componentTypeHandle = Fieldml_GetTypeComponentEnsemble( handle, objectHandle );
    
    if( componentTypeHandle == FML_INVALID_HANDLE )
    {
        if( region->getLastError() == FML_ERR_NO_ERROR )
        {
            return 1;
        }
        return -1;
    }
    
    return Fieldml_GetElementCount( handle, componentTypeHandle );
}


int Fieldml_GetElementCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->members.getCount();
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
        

    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_IsEnsembleComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    
    if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensembleType = (EnsembleType*)object;
        return ensembleType->isComponentEnsemble;
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


FmlObjectHandle Fieldml_GetMeshElementType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->elementType;
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


const char * Fieldml_GetMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }
        
    SimpleMap<int, string> *map = getShapeMap( region, objectHandle ); 
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return meshType->xiType;
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_GetMeshXiComponentType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_TYPE ) 
    {
        MeshType *meshType = (MeshType *)object;
        return Fieldml_GetTypeComponentEnsemble( handle, meshType->xiType );
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_IsObjectLocal( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    
    if( object->locationHandle == LOCAL_LOCATION_HANDLE )
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
        
    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }

    object->intValue = value;
    return region->getLastError();
}


int Fieldml_GetObjectInt( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return 0;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return 0;
    }

    return object->intValue;
}


FmlObjectHandle Fieldml_GetValueType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    region->setRegionError( FML_ERR_NO_ERROR );
    
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

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateAbstractEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkIsValueType( region, valueType, true, true, true ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    AbstractEvaluator *abstractEvaluator = new AbstractEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( abstractEvaluator );
}


FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkIsValueType( region, valueType, true, true, false ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ExternalEvaluator *externalEvaluator = new ExternalEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( externalEvaluator );
}


FmlObjectHandle Fieldml_CreateParametersEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkIsValueType( region, valueType, true, true, false ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    ParameterEvaluator *parameterEvaluator = new ParameterEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( parameterEvaluator );
}


int Fieldml_SetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        if( parameterEvaluator->dataDescription->descriptionType != DESCRIPTION_UNKNOWN )
        {
            return region->setRegionError( FML_ERR_ACCESS_VIOLATION );
        }

        if( description == DESCRIPTION_SEMIDENSE )
        {
            delete parameterEvaluator->dataDescription;
            parameterEvaluator->dataDescription = new SemidenseDataDescription();
            return region->getLastError();
        }
        else
        {
            return region->setRegionError( FML_ERR_UNSUPPORTED );  
        }
    }

    return region->setRegionError( FML_ERR_INVALID_OBJECT );
}

DataDescriptionType Fieldml_GetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return DESCRIPTION_UNKNOWN;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return DESCRIPTION_UNKNOWN;
    }

    if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
        return parameterEvaluator->dataDescription->descriptionType;
    }

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return DESCRIPTION_UNKNOWN;
}


DataLocationType Fieldml_GetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return LOCATION_UNKNOWN;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    
    if( semidense == NULL )
    {
        return LOCATION_UNKNOWN;
    }

    return semidense->dataLocation->locationType;
}


int Fieldml_SetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle, DataLocationType location )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return region->getLastError();
    }

    if( semidense->dataLocation->locationType != LOCATION_UNKNOWN )
    {
        return region->setRegionError( FML_ERR_ACCESS_VIOLATION );
    }

    if( location == LOCATION_INLINE )
    {
        delete semidense->dataLocation;
        semidense->dataLocation = new InlineDataLocation();
        return region->setRegionError( FML_ERR_NO_ERROR );
    }
    else if( location == LOCATION_FILE )
    {
        delete semidense->dataLocation;
        semidense->dataLocation = new FileDataLocation();
        return region->setRegionError( FML_ERR_NO_ERROR );
    }
    else
    {
        return region->setRegionError( FML_ERR_UNSUPPORTED );
    }
}


int Fieldml_AddParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle, const char *data, const int length )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    InlineDataLocation *location = getInlineDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return region->getLastError();
    }

    char *newData = new char[location->length + length + 1];
    memcpy( newData, location->data, location->length );
    memcpy( newData + location->length, data, length );

    delete[] location->data;
    location->data = newData;
    
    location->length += length;
    newData[location->length] = 0;
    
    return region->getLastError();
}


int Fieldml_GetParameterInlineDataLength( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    InlineDataLocation *location = getInlineDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return -1;
    }
    
    return location->length;
}


const char * Fieldml_GetParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }

    InlineDataLocation *location = getInlineDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return NULL;
    }
    
    return strdupS( location->data );
}


int Fieldml_CopyInlineParameterData( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    InlineDataLocation *location = getInlineDataLocation( region, objectHandle );
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FileDataLocation *location = getFileDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return region->getLastError();
    }

    location->filename = filename;
    location->fileType = type;
    location->offset = offset;
    
    return region->getLastError();
}


const char * Fieldml_GetParameterDataFilename( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }

    FileDataLocation *location = getFileDataLocation( region, objectHandle );
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    FileDataLocation *location = getFileDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return -1;
    }

    return location->offset;
}


DataFileType Fieldml_GetParameterDataFileType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return TYPE_UNKNOWN;
    }

    FileDataLocation *location = getFileDataLocation( region, objectHandle );
    if( location == NULL )
    {
        return TYPE_UNKNOWN;
    }

    return location->fileType;
}


int Fieldml_AddDenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle setHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorType( region, indexHandle, false, true ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return region->getLastError();
    }

    semidense->denseIndexes.push_back( indexHandle );
    semidense->denseSets.push_back( setHandle );
    
    return region->getLastError();
}


int Fieldml_AddSparseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorType( region, indexHandle, false, true ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
        
    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return region->getLastError();
    }

    semidense->sparseIndexes.push_back( indexHandle );
    
    return region->getLastError();
}


int Fieldml_GetSemidenseIndexCount( FmlHandle handle, FmlObjectHandle objectHandle, int isSparse )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
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
    
    region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    
    return FML_INVALID_HANDLE;
}


int Fieldml_SetSwizzle( FmlHandle handle, FmlObjectHandle objectHandle, const int *buffer, int count )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return region->getLastError();
    }
    
    FmlObjectHandle evaluatorHandle = Fieldml_GetSemidenseIndexEvaluator( handle, objectHandle, 1, 0 );
    FmlObjectHandle ensembleHandle = Fieldml_GetValueType( handle, evaluatorHandle );
    int ensembleCount = Fieldml_GetElementCount( handle, ensembleHandle );
    
    if( ensembleCount != count )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }
    
    if( semidense->swizzle != NULL )
    {
        delete[] semidense->swizzle;
    }
    
    int *ints = new int[count];
    memcpy( ints, buffer, sizeof( int ) * count );
    
    semidense->swizzleCount = count;
    semidense->swizzle = ints;
    
    return region->getLastError();
}


int Fieldml_GetSwizzleCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return -1;
    }
    
    return semidense->swizzleCount;
}


const int * Fieldml_GetSwizzleData( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
    if( semidense == NULL )
    {
        return NULL;
    }
    
    return semidense->swizzle;
}


int Fieldml_CopySwizzleData( FmlHandle handle, FmlObjectHandle objectHandle, int *buffer, int bufferLength )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkIsValueType( region, valueType, true, true, false ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    PiecewiseEvaluator *piecewiseEvaluator = new PiecewiseEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( piecewiseEvaluator );
}


FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( !checkIsValueType( region, valueType, true, false, false ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
        
    AggregateEvaluator *aggregateEvaluator = new AggregateEvaluator( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( aggregateEvaluator );
}


int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( region, evaluator, true, false ) )
        {
            return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( region, objectHandle, evaluator ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    FieldmlObject *object = region->getObject( objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map == NULL )
    {
        return region->getLastError();
    }

    if( ( object->type == FHT_PIECEWISE_EVALUATOR ) || ( object->type == FHT_AGGREGATE_EVALUATOR ) )
    {
        map->setDefault( evaluator );
        return region->getLastError();
    }
    else
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }
}


int Fieldml_GetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
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
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return FML_INVALID_HANDLE;
    }
}


int Fieldml_SetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int element, FmlObjectHandle evaluator )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( Fieldml_GetObjectType( handle, objectHandle ) == FHT_AGGREGATE_EVALUATOR )
    {
        if( !checkIsEvaluatorType( region, evaluator, true, false ) )
        {
            return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( !checkIsEvaluatorTypeCompatible( region, objectHandle, evaluator ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map != NULL )
    {
        map->set( element, evaluator );
    }

    return region->getLastError();
}


int Fieldml_GetEvaluatorCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->size();
}


int Fieldml_GetEvaluatorElement( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map == NULL )
    {
        return -1;
    }

    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetElementEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( region, objectHandle ); 
 
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->get( elementNumber, allowDefault == 1 );
}


FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlHandle handle, const char * name, FmlObjectHandle remoteEvaluator )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle valueType = Fieldml_GetValueType( handle, remoteEvaluator );

    ReferenceEvaluator *referenceEvaluator = new ReferenceEvaluator( name, LOCAL_LOCATION_HANDLE, remoteEvaluator, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( referenceEvaluator );
}


FmlObjectHandle Fieldml_GetReferenceRemoteEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *referenceEvaluator = (ReferenceEvaluator *)object;
        return referenceEvaluator->remoteEvaluator;
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetVariableCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    vector<FmlObjectHandle> *variables = getVariableList( region, objectHandle );
    if( variables == NULL )
    {
        return -1;
    }
    
    return variables->size();
}


FmlObjectHandle Fieldml_GetVariable( FmlHandle handle, FmlObjectHandle objectHandle, int variableIndex )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    vector<FmlObjectHandle> *variables = getVariableList( region, objectHandle );
    if( variables == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( ( variableIndex < 1 ) || ( variableIndex > variables->size() ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    return variables->at( variableIndex - 1 );
}


int Fieldml_AddVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    vector<FmlObjectHandle> *variables = getVariableList( region, objectHandle );
    if( variables == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    if( std::find( variables->begin(), variables->end(), evaluatorHandle ) == variables->end() )
    {
        variables->push_back( evaluatorHandle );
    }

    return region->getLastError();
}


int Fieldml_GetBindCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( region, objectHandle );
    if( map == NULL )
    {
        return -1;
    }
    
    return map->size();
}


FmlObjectHandle Fieldml_GetBindVariable( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( region, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetBindEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( region, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetBindByVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( region, objectHandle );
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorTypeCompatible( region, variableHandle, sourceHandle ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getBindMap( region, objectHandle );
    if( map == NULL )
    {
        return region->getLastError();
    }
    
    map->set( variableHandle, sourceHandle );
    return region->getLastError();
}



int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    FieldmlObject *object = region->getObject( objectHandle );

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
        
        region->setRegionError( FML_ERR_UNSUPPORTED );
        return -1;
    }
    
    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}


int Fieldml_SetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    if( !checkIsEvaluatorType( region, evaluatorHandle, false, true ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            piecewise->indexEvaluator = evaluatorHandle;
            return region->getLastError();
        }
        else
        {
            return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            aggregate->indexEvaluator = evaluatorHandle;
            return region->getLastError();
        }
        else
        {
            return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
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
                return region->getLastError();
            }

            index -= count;
            count = semidense->denseIndexes.size();

            if( index <= count )
            {
                semidense->denseIndexes[index - 1] = evaluatorHandle;
                return region->getLastError();
            }
            
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            region->setRegionError( FML_ERR_UNSUPPORTED );
        }
    }
    else
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return region->getLastError();
}


FmlObjectHandle Fieldml_GetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *piecewise = (PiecewiseEvaluator*)object;

        if( index == 1 )
        {
            return piecewise->indexEvaluator;
        }
        
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *aggregate = (AggregateEvaluator*)object;

        if( index == 1 )
        {
            return aggregate->indexEvaluator;
        }
        
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
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
            
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        }
        else
        {
            region->setRegionError( FML_ERR_UNSUPPORTED );
        }
    }
    else
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return FML_INVALID_HANDLE;
}


int Fieldml_GetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    if( index <= 0 )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
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
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }

        index -= count;
        count = semidense->denseIndexes.size();

        if( index <= count )
        {
            return semidense->denseSets[index - 1];
        }
        
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else
    {
        region->setRegionError( FML_ERR_UNSUPPORTED );
    }

    return FML_INVALID_HANDLE;
}




int Fieldml_SetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle setHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }
    
    if( index <= 0 )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return region->getLastError();
    }
    
    if( object->type != FHT_PARAMETER_EVALUATOR )
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return region->getLastError();
    }
    
    FieldmlHandleType type = Fieldml_GetObjectType( handle, setHandle );
    if( type != FHT_ELEMENT_SEQUENCE )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
        return region->getLastError();
    }
    
    int count;
    
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;
    if( parameterEvaluator->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameterEvaluator->dataDescription;
        count = semidense->sparseIndexes.size();
        
        if( index <= count )
        {
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return region->getLastError();
        }

        index -= count;
        count = semidense->denseIndexes.size();

        if( index <= count )
        {
            semidense->denseSets[index - 1] = setHandle;
            return FML_ERR_NO_ERROR;
        }
        
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    else
    {
        region->setRegionError( FML_ERR_UNSUPPORTED );
    }

    return region->getLastError();
}


FmlObjectHandle Fieldml_CreateContinuousType( FmlHandle handle, const char * name, FmlObjectHandle componentDescriptionHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( ( componentDescriptionHandle != FML_INVALID_HANDLE ) )
    {
        if( !checkIsValueType( region, componentDescriptionHandle, false, true, false ) )
        {
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }
    
    if( componentDescriptionHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_IsEnsembleComponentType( handle, componentDescriptionHandle ) != 1 )
        {
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }

    ContinuousType *continuousType = new ContinuousType( name, LOCAL_LOCATION_HANDLE, componentDescriptionHandle );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( continuousType );
}


FmlObjectHandle Fieldml_CreateEnsembleType( FmlHandle handle, const char * name, const int isComponentType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    EnsembleType *ensembleType = new EnsembleType( name, LOCAL_LOCATION_HANDLE, isComponentType == 1 );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( ensembleType );
}


FmlObjectHandle Fieldml_CreateMeshType( FmlHandle handle, const char * name, FmlObjectHandle xiEnsembleDescription )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FmlObjectHandle xiHandle, elementHandle;
    string xiName, elementsName;

    if( ( xiEnsembleDescription == FML_INVALID_HANDLE ) ||
        ( Fieldml_GetObjectType( handle, xiEnsembleDescription ) != FHT_ENSEMBLE_TYPE ) )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    xiName = name;
    xiName += ".xi";
    if( Fieldml_GetObjectByName( handle, xiName.c_str() ) != FML_INVALID_HANDLE )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    elementsName = name;
    elementsName += ".elements";
    if( Fieldml_GetObjectByName( handle, elementsName.c_str() ) != FML_INVALID_HANDLE )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    
    ContinuousType *xiObject = new ContinuousType( xiName.c_str(), VIRTUAL_LOCATION_HANDLE, xiEnsembleDescription );
    xiHandle = region->addObject( xiObject );
    
    EnsembleType *elementObject = new EnsembleType( elementsName.c_str(), VIRTUAL_LOCATION_HANDLE, false );
    elementHandle = region->addObject( elementObject );
    
    MeshType *meshType = new MeshType( name, LOCAL_LOCATION_HANDLE, xiHandle, elementHandle );

    region->setRegionError( FML_ERR_NO_ERROR );

    return region->addObject( meshType );
}


int Fieldml_SetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle, const char * shape )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SimpleMap<int, string> *map = getShapeMap( region, objectHandle ); 
    if( map == NULL )
    {
        return region->getLastError();
    }
    
    map->setDefault( shape );
    return region->getLastError();
}


const char * Fieldml_GetMeshDefaultShape( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }

    SimpleMap<int, string> *map = getShapeMap( region, objectHandle ); 
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
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    SimpleMap<int, string> *map = getShapeMap( region, objectHandle ); 
    if( map == NULL )
    {
        return region->getLastError();
    }

    map->set( elementNumber, shape );
    return region->getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );

    if( semidense == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    FieldmlObject *object = region->getObject( objectHandle );
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;

    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) ||
        ( semidense->dataLocation->locationType == LOCATION_INLINE ) )
    {
        return ParameterReader::create( region, parameterEvaluator );
    }
    else
    {
        return region->setRegionError( FML_ERR_UNSUPPORTED );
    }
}


int Fieldml_ReadIndexSet( FmlHandle handle, FmlReaderHandle readerHandle, int *indexBuffer )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }
    
    ParameterReader *reader = ParameterReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    int err = reader->readNextIndexSet( indexBuffer );
    
    return region->setRegionError( err );
}


int Fieldml_ReadIntValues( FmlHandle handle, FmlReaderHandle readerHandle, int *valueBuffer, int bufferSize )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterReader *reader = ParameterReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readIntValues( valueBuffer, bufferSize );
}


int Fieldml_ReadDoubleValues( FmlHandle handle, FmlReaderHandle readerHandle, double *valueBuffer, int bufferSize )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterReader *reader = ParameterReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    return reader->readDoubleValues( valueBuffer, bufferSize );
}


int Fieldml_CloseReader( FmlHandle handle, FmlReaderHandle readerHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterReader *reader = ParameterReader::handleToReader( readerHandle );
    if( reader == NULL )
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    delete reader;
    
    return region->setRegionError( FML_ERR_NO_ERROR );
}


FmlWriterHandle Fieldml_OpenWriter( FmlHandle handle, FmlObjectHandle objectHandle, int append )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return NULL;
    }

    SemidenseDataDescription *semidense = getSemidenseDataDescription( region, objectHandle );

    if( semidense == NULL )
    {
        return NULL;
    }

    FieldmlObject *object = region->getObject( objectHandle );
    ParameterEvaluator *parameterEvaluator = (ParameterEvaluator *)object;

    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) ||
        ( semidense->dataLocation->locationType == LOCATION_INLINE ) )
    {
        return ParameterWriter::create( region, parameterEvaluator, ( append == 1 ));
    }
    else
    {
        return region->setRegionError( FML_ERR_UNSUPPORTED );
    }
}


int Fieldml_WriteIndexSet( FmlHandle handle, FmlWriterHandle writerHandle, int *indexBuffer )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterWriter *writer = ParameterWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    int err = writer->writeNextIndexSet( indexBuffer );
    
    return region->setRegionError( err );
}


int Fieldml_WriteIntValues( FmlHandle handle, FmlWriterHandle writerHandle, int *valueBuffer, int valueCount )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterWriter *writer = ParameterWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeIntValues( valueBuffer, valueCount );
}


int Fieldml_WriteDoubleValues( FmlHandle handle, FmlWriterHandle writerHandle, double *valueBuffer, int valueCount )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterWriter *writer = ParameterWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        region->setRegionError( FML_ERR_INVALID_OBJECT );
        return -1;
    }

    return writer->writeDoubleValues( valueBuffer, valueCount );
}


int Fieldml_CloseWriter( FmlHandle handle, FmlWriterHandle writerHandle )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    ParameterWriter *writer = ParameterWriter::handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return region->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    delete writer;
    
    return region->setRegionError( FML_ERR_NO_ERROR );
}


FmlObjectHandle Fieldml_CreateEnsembleElementSequence( FmlHandle handle, const char * name, FmlObjectHandle valueType )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    ElementSequence *elementSequence = new ElementSequence( name, LOCAL_LOCATION_HANDLE, valueType );
    
    region->setRegionError( FML_ERR_NO_ERROR );
    return region->addObject( elementSequence );
}


int Fieldml_AddEnsembleElements( FmlHandle handle, FmlObjectHandle setHandle, const int * elements, const int elementCount )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = region->getObject( setHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }

    if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *elementSequence = (ElementSequence*)object;
        
        for( int i = 0; i < elementCount; i++ )
        {
            elementSequence->members.setBit( elements[i], true );
        }
        
        return region->getLastError();
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        for( int i = 0; i < elementCount; i++ )
        {
            ensemble->members.setBit( elements[i], true );
        }
        
        return region->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_AddEnsembleElements( handle, meshType->elementType, elements, elementCount );
    }

    return region->setRegionError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_AddEnsembleElementRange( FmlHandle handle, FmlObjectHandle setHandle, const int minElement, const int maxElement, const int stride )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return FML_ERR_UNKNOWN_HANDLE;
    }

    FieldmlObject *object = region->getObject( setHandle );

    if( object == NULL )
    {
        return region->getLastError();
    }

    if( ( minElement < 1 ) || ( minElement > maxElement ) )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }

    if( maxElement < 1 )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
    }
    
    if( stride < 1 )
    {
        return region->setRegionError( FML_ERR_INVALID_PARAMETER_5 );
    }
    
    if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *elementSequence = (ElementSequence*)object;

        for( int i = minElement; i <= maxElement; i += stride )
        {
            elementSequence->members.setBit( i, true );
        }
        
        return region->getLastError();
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        for( int i = minElement; i <= maxElement; i += stride )
        {
            ensemble->members.setBit( i, true );
        }
        
        return region->getLastError();
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_AddEnsembleElementRange( handle, meshType->elementType, minElement, maxElement, stride );
    }

    return region->setRegionError( FML_ERR_INVALID_OBJECT );
}


int Fieldml_GetElementEntry( FmlHandle handle, FmlObjectHandle setHandle, const int index )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    FieldmlObject *object = region->getObject( setHandle );

    if( object == NULL )
    {
        return -1;
    }

    if( object->type == FHT_ELEMENT_SEQUENCE )
    {
        ElementSequence *elementSequence = (ElementSequence*)object;

        if( ( index < 1 ) || ( index > elementSequence->members.getCount() ) )
        {
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
            return -1;
        }
        
        return elementSequence->members.getTrueBit( index );
    }
    else if( object->type == FHT_ENSEMBLE_TYPE )
    {
        EnsembleType *ensemble = (EnsembleType*)object;

        if( ( index < 1 ) || ( index > ensemble->members.getCount() ) )
        {
            region->setRegionError( FML_ERR_INVALID_PARAMETER_3 );  
            return -1;
        }
        
        return ensemble->members.getTrueBit( index );
    }
    else if( object->type == FHT_MESH_TYPE )
    {
        MeshType *meshType = (MeshType*)object;
        return Fieldml_GetElementEntry( handle, meshType->elementType, index );
    }

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}


int Fieldml_GetElementEntries( FmlHandle handle, FmlObjectHandle setHandle, const int firstIndex, int * elements, const int count )
{
    FieldmlRegion *region = FieldmlRegion::handleToRegion( handle );
    if( region == NULL )
    {
        return -1;
    }

    FieldmlObject *object = region->getObject( setHandle );

    if( object == NULL )
    {
        return -1;
    }

    if( count < 1 )
    {
        region->setRegionError( FML_ERR_INVALID_PARAMETER_5 );  
        return -1;
    }

    if( ( object->type == FHT_ELEMENT_SEQUENCE ) || ( object->type == FHT_ENSEMBLE_TYPE ) || ( object->type == FHT_MESH_TYPE ) )
    {
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

        region->setRegionError( FML_ERR_NO_ERROR );
        return actualCount;
    }

    region->setRegionError( FML_ERR_INVALID_OBJECT );
    return -1;
}
