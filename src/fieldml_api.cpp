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

    if( object->type == FHT_CONTINUOUS_AGGREGATE )
    {
        ContinuousAggregate *aggregate = (ContinuousAggregate *)object;
        return &aggregate->evaluators;
    }
    else if( object->type == FHT_CONTINUOUS_PIECEWISE )
    {
        ContinuousPiecewise *piecewise = (ContinuousPiecewise *)object;
        return &piecewise->evaluators;
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getAliasMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_CONTINUOUS_AGGREGATE )
    {
        ContinuousAggregate *aggregate = (ContinuousAggregate *)object;
        return &aggregate->aliases;
    }
    else if( object->type == FHT_CONTINUOUS_PIECEWISE )
    {
        ContinuousPiecewise *piecewise = (ContinuousPiecewise *)object;
        return &piecewise->aliases;
    }
    else if( object->type == FHT_CONTINUOUS_REFERENCE )
    {
        ContinuousReference *reference = (ContinuousReference *)object;
        return &reference->aliases;
    }
    else
    {
        handle->setRegionError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }
}


static SimpleMap<int, string> *getShapeMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_MESH_DOMAIN )
    {
        MeshDomain *meshDomain = (MeshDomain*)object;
        return &meshDomain->shapes;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return NULL;
}


static SimpleMap<FmlObjectHandle, FmlObjectHandle> *getConnectivityMap( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }

    if( object->type == FHT_MESH_DOMAIN )
    {
        MeshDomain *meshDomain = (MeshDomain*)object;
        return &meshDomain->connectivity;
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

    if( ( object->type == FHT_CONTINUOUS_PARAMETERS ) || ( object->type == FHT_ENSEMBLE_PARAMETERS ) )
    {
        Parameters *parameters = (Parameters *)object;
        
        if( parameters->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            return (SemidenseDataDescription *)parameters->dataDescription;
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

//========================================================================
//
// API
//
//========================================================================

FmlHandle Fieldml_CreateFromFile( const char *filename )
{
    return parseFieldmlFile( filename );
}


FmlHandle Fieldml_Create( const char *location, const char *name )
{
    return new FieldmlRegion( location, name );
}


int Fieldml_SetDebug( FmlHandle handle, const int debug )
{
    handle->setDebug( debug );
    
    return handle->setRegionError( FML_ERR_NO_ERROR );
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


FmlObjectHandle Fieldml_GetNamedObject( FmlHandle handle, const char * name )
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


int Fieldml_SetMarkup( FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute, const char * value )
{
    FieldmlObject *object = handle->getObject( objectHandle );
    
    if( object != NULL )
    {
        object->markup.set( attribute, value );
    }

    return handle->getLastError();
}


int Fieldml_GetMarkupCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return -1;
    }
    
    return object->markup.size();
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


const char * Fieldml_GetMarkupAttribute( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( object->markup.getKey( index -  1 ) );
}


int Fieldml_CopyMarkupAttribute( FmlHandle handle, FmlObjectHandle objectHandle, int index, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMarkupAttribute( handle, objectHandle, index ), buffer, bufferLength );
}


const char * Fieldml_GetMarkupValue( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( object->markup.getValue( index -  1 ) );
}


int Fieldml_CopyMarkupValue( FmlHandle handle, FmlObjectHandle objectHandle, int index, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMarkupValue( handle, objectHandle, index ), buffer, bufferLength );
}


const char * Fieldml_GetMarkupAttributeValue( FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return NULL;
    }
    
    return cstrCopy( object->markup.get( attribute, 0 ) );
}


int Fieldml_CopyMarkupAttributeValue( FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute, char *buffer, int bufferLength )
{
    return cappedCopyAndFree( Fieldml_GetMarkupAttributeValue( handle, objectHandle, attribute ), buffer, bufferLength );
}


FmlObjectHandle Fieldml_GetDomainComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_CONTINUOUS_DOMAIN )
    {
        ContinuousDomain *continuousDomain = (ContinuousDomain*)object;
        return continuousDomain->componentDomain;
    }
    else if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        return FML_INVALID_HANDLE;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


int Fieldml_GetDomainComponentCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FmlObjectHandle componentDomainHandle = Fieldml_GetDomainComponentEnsemble( handle, objectHandle );
    
    if( componentDomainHandle == FML_INVALID_HANDLE )
    {
        if( handle->getLastError() == FML_ERR_NO_ERROR )
        {
            return 1;
        }
        return -1;
    }
    
    return Fieldml_GetEnsembleDomainElementCount( handle, componentDomainHandle );
}


DomainBoundsType Fieldml_GetDomainBoundsType( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return BOUNDS_UNKNOWN;
    }

    if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)object;
        return ensembleDomain->bounds->boundsType;
    }
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return BOUNDS_UNKNOWN;
}


int Fieldml_GetEnsembleDomainElementCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }

    if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)object;
        if( ensembleDomain->bounds->boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            ContiguousBounds *contiguous = (ContiguousBounds *)ensembleDomain->bounds;
            return contiguous->count;
        }
        
        handle->setRegionError( FML_ERR_UNSUPPORTED );  
        return -1;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return -1;
}


int Fieldml_IsEnsembleComponentDomain( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL ) 
    {
        return -1;
    }
    if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)object;
        return ensembleDomain->isComponentEnsemble;
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
    
    if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)object;
        if( ensembleDomain->bounds->boundsType == BOUNDS_DISCRETE_CONTIGUOUS )
        {
            ContiguousBounds *contiguous = (ContiguousBounds *)ensembleDomain->bounds;
            handle->setRegionError( FML_ERR_NO_ERROR );  
            return contiguous->count;
        }
        
        handle->setRegionError( FML_ERR_INVALID_OBJECT );  
        return -1;
    }
    else if( object->type == FHT_MESH_DOMAIN )
    {
        MeshDomain *meshDomain = (MeshDomain*)object;
        FieldmlObject *subObject;
        
        subObject = handle->getObject( meshDomain->elementDomain );
        
        if( ( subObject == NULL ) || ( subObject->type != FHT_ENSEMBLE_DOMAIN ) )
        {
            return handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
        }
        
        return Fieldml_GetContiguousBoundsCount( handle, meshDomain->elementDomain );
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

    if( object->regionHandle != FILE_REGION_HANDLE )
    {
        return handle->setRegionError( FML_ERR_ACCESS_VIOLATION );
    }
    
    if( object->type == FHT_ENSEMBLE_DOMAIN )
    {
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)object;
        delete ensembleDomain->bounds;
        ensembleDomain->bounds = new ContiguousBounds( count );
        
        return handle->getLastError();
    }
    else if( object->type == FHT_MESH_DOMAIN )
    {
        MeshDomain *meshDomain = (MeshDomain*)object;

        FieldmlObject *subObject;
        
        subObject = handle->getObject( meshDomain->elementDomain );
        
        if( ( subObject == NULL ) || ( subObject->type != FHT_ENSEMBLE_DOMAIN ) )
        {
            return handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
        }
        
        EnsembleDomain *ensembleDomain = (EnsembleDomain*)subObject;
        delete ensembleDomain->bounds;
        ensembleDomain->bounds = new ContiguousBounds( count );
        
        return handle->getLastError();
    }

    return handle->setRegionError( FML_ERR_INVALID_OBJECT );
}


FmlObjectHandle Fieldml_GetMeshElementDomain( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_DOMAIN ) 
    {
        MeshDomain *meshDomain = (MeshDomain *)object;
        return meshDomain->elementDomain;
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


int Fieldml_GetMeshConnectivityCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getConnectivityMap( handle, objectHandle );
    if( map == NULL )
    {
        return -1;
    }

    return map->size();
}


FmlObjectHandle Fieldml_GetMeshConnectivityDomain( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getConnectivityMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetMeshConnectivitySource( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getConnectivityMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetMeshXiDomain( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    if( object->type == FHT_MESH_DOMAIN ) 
    {
        MeshDomain *meshDomain = (MeshDomain *)object;
        return meshDomain->xiDomain;
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


FmlObjectHandle Fieldml_GetValueDomain( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    handle->setRegionError( FML_ERR_NO_ERROR );  
    if( ( object->type == FHT_ENSEMBLE_PARAMETERS ) || ( object->type == FHT_CONTINUOUS_PARAMETERS ) ) 
    {
        Parameters *parameters = (Parameters *)object;
        return parameters->valueDomain;
    }
    else if( object->type == FHT_CONTINUOUS_REFERENCE )
    {
        ContinuousReference *continuousReference = (ContinuousReference *)object;
        return continuousReference->valueDomain;
    }
    else if( object->type == FHT_CONTINUOUS_AGGREGATE )
    {
        ContinuousAggregate *aggregate = (ContinuousAggregate *)object;
        return aggregate->valueDomain;
    }
    else if( object->type == FHT_CONTINUOUS_PIECEWISE )
    {
        ContinuousPiecewise *piecewise = (ContinuousPiecewise *)object;
        return piecewise->valueDomain;
    }
    else if( ( object->type == FHT_CONTINUOUS_VARIABLE ) || ( object->type == FHT_ENSEMBLE_VARIABLE ) )
    {
        Variable *variable = (Variable *)object;
        return variable->valueDomain;
    }

    handle->setRegionError( FML_ERR_INVALID_OBJECT );  
    return FML_INVALID_HANDLE;
}


FmlObjectHandle Fieldml_CreateEnsembleVariable( FmlHandle handle, const char *name, FmlObjectHandle valueDomain )
{
    Variable *variable = new Variable( name, FILE_REGION_HANDLE, valueDomain, true );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( variable );
}


FmlObjectHandle Fieldml_CreateContinuousVariable( FmlHandle handle, const char *name, FmlObjectHandle valueDomain )
{
    Variable *variable = new Variable( name, FILE_REGION_HANDLE, valueDomain, false );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( variable );
}


FmlObjectHandle Fieldml_CreateEnsembleParameters( FmlHandle handle, const char *name, FmlObjectHandle valueDomain )
{
    Parameters *parameters = new Parameters( name, FILE_REGION_HANDLE, valueDomain, true );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( parameters );
}


FmlObjectHandle Fieldml_CreateContinuousParameters( FmlHandle handle, const char *name, FmlObjectHandle valueDomain )
{
    Parameters *parameters = new Parameters( name, FILE_REGION_HANDLE, valueDomain, false );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( parameters );
}


int Fieldml_SetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return handle->getLastError();
    }

    if( ( object->type == FHT_ENSEMBLE_PARAMETERS ) || ( object->type == FHT_CONTINUOUS_PARAMETERS ) ) 
    {
        Parameters *parameters = (Parameters *)object;
        if( parameters->dataDescription->descriptionType != DESCRIPTION_UNKNOWN )
        {
            return handle->setRegionError( FML_ERR_ACCESS_VIOLATION );
        }

        if( description == DESCRIPTION_SEMIDENSE )
        {
            delete parameters->dataDescription;
            parameters->dataDescription = new SemidenseDataDescription();
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

    if( ( object->type == FHT_ENSEMBLE_PARAMETERS ) || ( object->type == FHT_CONTINUOUS_PARAMETERS ) ) 
    {
        Parameters *parameters = (Parameters *)object;
        return parameters->dataDescription->descriptionType;
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


int Fieldml_AddSemidenseIndex( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, int isSparse )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return handle->getLastError();
    }

    if( isSparse )
    {
        semidense->sparseIndexes.push_back( indexHandle );
    }
    else
    {
        semidense->denseIndexes.push_back( indexHandle );
    }
    
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


FmlObjectHandle Fieldml_GetSemidenseIndex( FmlHandle handle, FmlObjectHandle objectHandle, int index, int isSparse )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( isSparse )
    {
        return semidense->sparseIndexes[index - 1];
    }
    else
    {
        return semidense->denseIndexes[index - 1];
    }
}


int Fieldml_SetSwizzle( FmlHandle handle, FmlObjectHandle objectHandle, const int *buffer, int count )
{
    SemidenseDataDescription *semidense = getSemidenseDataDescription( handle, objectHandle );
    if( semidense == NULL )
    {
        return handle->getLastError();
    }
    
    FmlObjectHandle ensembleHandle = Fieldml_GetSemidenseIndex( handle, objectHandle, 1, 0 );
    int ensembleCount = Fieldml_GetEnsembleDomainElementCount( handle, ensembleHandle );
    
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


FmlObjectHandle Fieldml_CreateContinuousPiecewise( FmlHandle handle, const char * name, FmlObjectHandle indexHandle, FmlObjectHandle valueDomain )
{
    ContinuousPiecewise *continuousPiecewise = new ContinuousPiecewise( name, FILE_REGION_HANDLE, indexHandle, valueDomain );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( continuousPiecewise );
}


FmlObjectHandle Fieldml_CreateContinuousAggregate( FmlHandle handle, const char * name, FmlObjectHandle valueDomain )
{
    ContinuousAggregate *continuousAggregate = new ContinuousAggregate( name, FILE_REGION_HANDLE, valueDomain );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( continuousAggregate );
}


int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator )
{
    FieldmlObject *object = handle->getObject( objectHandle );
    SimpleMap<int, FmlObjectHandle> *map = getEvaluatorMap( handle, objectHandle ); 
 
    if( map == NULL )
    {
        return handle->getLastError();
    }

    if( object->type == FHT_CONTINUOUS_PIECEWISE )
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

    if( object->type == FHT_CONTINUOUS_PIECEWISE )
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


FmlObjectHandle Fieldml_CreateContinuousReference( FmlHandle handle, const char * name, FmlObjectHandle remoteEvaluator, FmlObjectHandle valueDomain )
{
    ContinuousReference *continuousReference = new ContinuousReference( name, FILE_REGION_HANDLE, remoteEvaluator, valueDomain );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( continuousReference );
}


FmlObjectHandle Fieldml_GetReferenceRemoteEvaluator( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    if( object->type == FHT_CONTINUOUS_REFERENCE )
    {
        ContinuousReference *continuousReference = (ContinuousReference *)object;
        return continuousReference->remoteEvaluator;
    }
    
    
    handle->setRegionError( FML_ERR_INVALID_OBJECT );
    return FML_INVALID_HANDLE;
}


int Fieldml_GetAliasCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getAliasMap( handle, objectHandle );
    if( map == NULL )
    {
        return -1;
    }
    
    return map->size();
}


FmlObjectHandle Fieldml_GetAliasLocal( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getAliasMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getValue( index - 1 );
}


FmlObjectHandle Fieldml_GetAliasRemote( FmlHandle handle, FmlObjectHandle objectHandle, int index )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getAliasMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return map->getKey( index - 1 );
}


FmlObjectHandle Fieldml_GetAliasByRemote( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle remoteHandle )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getAliasMap( handle, objectHandle );
    if( map == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    for( int i = 0; i < map->size(); i++ )
    {
        if( map->getKey( i ) == remoteHandle )
        {
            return map->getValue( i );
        }
    }

    return FML_INVALID_HANDLE;
}


int Fieldml_SetAlias( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle remoteDomain, FmlObjectHandle localSource )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getAliasMap( handle, objectHandle );
    if( map == NULL )
    {
        return handle->getLastError();
    }
    
    map->set( remoteDomain, localSource );
    return handle->getLastError();
}



int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );

    if( object == NULL )
    {
        return -1;
    }
    
    if( object->type == FHT_CONTINUOUS_PIECEWISE )
    {
        return 1;
    }
    else if( ( object->type == FHT_CONTINUOUS_PARAMETERS ) || ( object->type == FHT_ENSEMBLE_PARAMETERS ) )
    {
        int count1, count2;
        
        Parameters *parameters = (Parameters *)object;
        if( parameters->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameters->dataDescription;
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


FmlObjectHandle Fieldml_GetIndexDomain( FmlHandle handle, FmlObjectHandle objectHandle, int index )
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
    
    if( object->type == FHT_CONTINUOUS_PIECEWISE )
    {
        ContinuousPiecewise *piecewise = (ContinuousPiecewise*)object;

        if( index == 1 )
        {
            return piecewise->indexDomain;
        }
        
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    else if( ( object->type == FHT_CONTINUOUS_PARAMETERS ) || ( object->type == FHT_ENSEMBLE_PARAMETERS ) )
    {
        int count;
        
        Parameters *parameters = (Parameters *)object;
        if( parameters->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
        {
            SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameters->dataDescription;
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


FmlObjectHandle Fieldml_CreateContinuousDomain( FmlHandle handle, const char * name, FmlObjectHandle componentHandle )
{
    if( ( componentHandle != FML_INVALID_HANDLE ) &&
        ( Fieldml_GetObjectType( handle, componentHandle ) != FHT_ENSEMBLE_DOMAIN ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }
    
    if( componentHandle != FML_INVALID_HANDLE )
    {
        if( Fieldml_IsEnsembleComponentDomain( handle, componentHandle ) != 1 )
        {
            handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
            return FML_INVALID_HANDLE;
        }
    }

    ContinuousDomain *continuousDomain = new ContinuousDomain( name, FILE_REGION_HANDLE, componentHandle );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( continuousDomain );
}


FmlObjectHandle Fieldml_CreateEnsembleDomain( FmlHandle handle, const char * name, FmlObjectHandle componentHandle )
{
    if( componentHandle != FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
        return FML_INVALID_HANDLE;
    }

    EnsembleDomain *ensembleDomain = new EnsembleDomain( name, FILE_REGION_HANDLE, false );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( ensembleDomain );
}


FmlObjectHandle Fieldml_CreateComponentEnsembleDomain( FmlHandle handle, const char * name )
{
    EnsembleDomain *ensembleDomain = new EnsembleDomain( name, FILE_REGION_HANDLE, true );
    
    handle->setRegionError( FML_ERR_NO_ERROR );
    return handle->addObject( ensembleDomain );
}


FmlObjectHandle Fieldml_CreateMeshDomain( FmlHandle handle, const char * name, FmlObjectHandle xiEnsemble )
{
    FmlObjectHandle xiHandle, elementHandle;
    string subName;

    if( ( xiEnsemble == FML_INVALID_HANDLE ) ||
        ( Fieldml_GetObjectType( handle, xiEnsemble ) != FHT_ENSEMBLE_DOMAIN ) )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
        return FML_INVALID_HANDLE;
    }

    subName = name;
    subName += ".xi";
    if( Fieldml_GetNamedObject( handle, subName.c_str() ) != FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }

    subName = name;
    subName += ".elements";
    if( Fieldml_GetNamedObject( handle, subName.c_str() ) != FML_INVALID_HANDLE )
    {
        handle->setRegionError( FML_ERR_INVALID_PARAMETER_2 );
        return FML_INVALID_HANDLE;
    }
    
    subName = name;
    subName += ".xi";
    ContinuousDomain *xiObject = new ContinuousDomain( subName.c_str(), VIRTUAL_REGION_HANDLE, xiEnsemble );
    xiHandle = handle->addObject( xiObject );
    
    subName = name;
    subName += ".elements";
    EnsembleDomain *elementObject = new EnsembleDomain( subName.c_str(), VIRTUAL_REGION_HANDLE, false );
    elementHandle = handle->addObject( elementObject );
    
    MeshDomain *meshDomain = new MeshDomain( name, FILE_REGION_HANDLE, xiHandle, elementHandle );

    handle->setRegionError( FML_ERR_NO_ERROR );

    return handle->addObject( meshDomain );
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


int Fieldml_SetMeshConnectivity( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator, FmlObjectHandle pointDomain )
{
    SimpleMap<FmlObjectHandle, FmlObjectHandle> *map = getConnectivityMap( handle, objectHandle );
    if( map == NULL )
    {
        return handle->getLastError();
    }

    if( ( pointDomain == FML_INVALID_HANDLE ) || ( evaluator == FML_INVALID_HANDLE ) )
    {
        // This could be use to 'un-set' a connectivity.
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    if( Fieldml_GetObjectType( handle, pointDomain ) != FHT_ENSEMBLE_DOMAIN )
    {
        return handle->setRegionError( FML_ERR_INVALID_PARAMETER_3 );
    }
    
    map->set( evaluator, pointDomain );
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
    Parameters *parameters = (Parameters *)object;

    ParameterReader *reader = NULL;
    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) ||
        ( semidense->dataLocation->locationType == LOCATION_INLINE ) )
    {
        reader = ParameterReader::create( handle, parameters );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }
    
    return reader;
}


int Fieldml_ReadIntSlice( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer, int *valueBuffer )
{
    int err;
    
    if( reader == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    err = reader->readIntSlice( indexBuffer, valueBuffer );
    
    return handle->setRegionError( err );
}


int Fieldml_ReadDoubleSlice( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer, double *valueBuffer )
{
    int err;
    
    if( reader == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    err = reader->readDoubleSlice( indexBuffer, valueBuffer );
    
    return handle->setRegionError( err );
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
    Parameters *parameters = (Parameters *)object;

    ParameterWriter *writer = NULL;
    if( ( semidense->dataLocation->locationType == LOCATION_FILE ) )
//        ( semidense->dataLocation->locationType == LOCATION_INLINE ) ) TODO
    {
        writer = ParameterWriter::create( handle, parameters, ( append == 1 ));
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
    }
    
    return writer;
}


int Fieldml_WriteIntSlice( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer, int *valueBuffer )
{
    int err;
    
    if( writer == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }
    
    err = writer->writeIntSlice( indexBuffer, valueBuffer );
    
    return handle->setRegionError( err );
}


int Fieldml_WriteDoubleSlice( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer, double *valueBuffer )
{
    int err;
    
    if( writer == NULL )
    {
        return handle->setRegionError( FML_ERR_INVALID_OBJECT );
    }

    err = writer->writeDoubleSlice( indexBuffer, valueBuffer );
    
    return handle->setRegionError( err );
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


int Fieldml_GetLastError( FmlHandle handle )
{
    return handle->getLastError();
}
