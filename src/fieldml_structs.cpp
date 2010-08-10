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

#include "string_const.h"
#include "fieldml_structs.h"
#include "fieldml_sax.h"

using namespace std;

//========================================================================
//
// Bounds
//
//========================================================================

DomainBounds::DomainBounds( DomainBoundsType _boundsType ) :
    boundsType( _boundsType )
{
}

UnknownBounds::UnknownBounds() :
    DomainBounds( BOUNDS_UNKNOWN )
{
}


ContiguousBounds::ContiguousBounds( const int _count ) :
    DomainBounds( BOUNDS_DISCRETE_CONTIGUOUS ),
    count( _count )
{
}

//========================================================================
//
// FieldmlObject
//
//========================================================================

FieldmlObject::FieldmlObject( const string _name, int _regionHandle, FieldmlHandleType _type ) :
    name( _name ),
    type( _type ),
    markup("")
{
    regionHandle = _regionHandle;
    intValue = 0;
}

EnsembleDomain::EnsembleDomain( const string _name, int _region, bool _isComponentEnsemble ) :
    FieldmlObject( _name, _region, FHT_ENSEMBLE_DOMAIN ),
    isComponentEnsemble( _isComponentEnsemble ),
    bounds( new UnknownBounds() )
{
}


ContinuousDomain::ContinuousDomain( const string _name, int _region, FmlObjectHandle _componentDomain ) :
    FieldmlObject( _name, _region, FHT_CONTINUOUS_DOMAIN ),
    componentDomain( _componentDomain )
{
}


MeshDomain::MeshDomain( const string _name, int _region, FmlObjectHandle _xiDomain, FmlObjectHandle _elementDomain ) :
    FieldmlObject( _name, _region, FHT_MESH_DOMAIN ),
    xiDomain( _xiDomain ),
    elementDomain( _elementDomain ),
    shapes(""),
    connectivity( FML_INVALID_HANDLE )
{
}


Evaluator::Evaluator( const string _name, int _region, FieldmlHandleType _type, FmlObjectHandle _valueDomain ) :
    FieldmlObject( _name, _region, _type ),
    valueDomain( _valueDomain )
{
}


ContinuousReference::ContinuousReference( const string _name, int _region, FmlObjectHandle _evaluator, FmlObjectHandle _valueDomain ) :
    Evaluator( _name, _region, FHT_CONTINUOUS_REFERENCE, _valueDomain ),
    remoteEvaluator( _evaluator ),
    aliases( FML_INVALID_HANDLE )
{
}


Variable::Variable( const string _name, int _region, FmlObjectHandle _valueDomain, bool isEnsemble ) :
    Evaluator( _name, _region, isEnsemble ? FHT_ENSEMBLE_VARIABLE : FHT_CONTINUOUS_VARIABLE, _valueDomain )
{
}


Parameters::Parameters( const string _name, int _region, FmlObjectHandle _valueDomain, bool isEnsemble ) :
    Evaluator( _name, _region, isEnsemble ? FHT_ENSEMBLE_PARAMETERS : FHT_CONTINUOUS_PARAMETERS, _valueDomain ),
    dataDescription( new UnknownDataDescription() )
{
}


ContinuousPiecewise::ContinuousPiecewise( const string _name, int _region, FmlObjectHandle _indexDomain, FmlObjectHandle _valueDomain ) :
    Evaluator( _name, _region, FHT_CONTINUOUS_PIECEWISE, _valueDomain ),
    aliases( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE )
{
    indexDomain = _indexDomain;
}


ContinuousAggregate::ContinuousAggregate( const string _name, int _region, FmlObjectHandle _valueDomain ) :
    Evaluator( _name, _region, FHT_CONTINUOUS_AGGREGATE, _valueDomain ),
    aliases( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE )
{
}


DataLocation::DataLocation( DataLocationType _locationType ) :
    locationType( _locationType )
{
}

    
UnknownDataLocation::UnknownDataLocation() :
    DataLocation( LOCATION_UNKNOWN )
{
}


FileDataLocation::FileDataLocation() :
    DataLocation( LOCATION_FILE )
{
    filename = "";
    offset = 0;
    fileType = TYPE_UNKNOWN;
}


InlineDataLocation::InlineDataLocation() :
    DataLocation( LOCATION_INLINE )
{
    data = NULL;
    length = 0;
}


DataDescription::DataDescription( DataDescriptionType _descriptionType ) :
    descriptionType( _descriptionType )
{
}


UnknownDataDescription::UnknownDataDescription() :
    DataDescription( DESCRIPTION_UNKNOWN )
{
}


SemidenseDataDescription::SemidenseDataDescription() :
    DataDescription( DESCRIPTION_SEMIDENSE ),
    dataLocation( new UnknownDataLocation() )
{
    swizzle = NULL;
    swizzleCount = 0;
}

//========================================================================
//
// Utility
//
//========================================================================


int Fieldml_GetRegion( FmlHandle handle, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = handle->getObject( objectHandle );
    
    if( object == NULL )
    {
        handle->setRegionError( FML_ERR_UNKNOWN_OBJECT );
        return INVALID_REGION_HANDLE;
    }
    
    return object->regionHandle;
}
