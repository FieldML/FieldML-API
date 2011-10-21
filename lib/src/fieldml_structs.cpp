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

using namespace std;

//========================================================================
//
// FieldmlObject
//
//========================================================================

FieldmlObject::FieldmlObject( const string _name, FieldmlHandleType _type, bool _isVirtual ) :
    name( _name ),
    type( _type ),
    isVirtual( _isVirtual )
{
    intValue = 0;
}


FieldmlObject::~FieldmlObject()
{
}


ElementSequence::ElementSequence( const string _name, FmlObjectHandle _elementType ) :
    FieldmlObject( _name, FHT_UNKNOWN, false ),
    elementType( _elementType )
{
}


EnsembleType::EnsembleType( const string _name, bool _isComponentEnsemble, bool _isVirtual ) :
    FieldmlObject( _name, FHT_ENSEMBLE_TYPE, _isVirtual ),
    isComponentEnsemble( _isComponentEnsemble )
{
    type = MEMBER_UNKNOWN;
    count = 0;
    min = 0;
    max = 0;
    stride = 1;
}


BooleanType::BooleanType( const string _name, bool _isVirtual ) :
    FieldmlObject( _name, FHT_BOOLEAN_TYPE, _isVirtual )
{
}


ContinuousType::ContinuousType( const string _name, bool _isVirtual ) :
    FieldmlObject( _name, FHT_CONTINUOUS_TYPE, _isVirtual )
{
    componentType = FML_INVALID_HANDLE;
}


MeshType::MeshType( const string _name, bool _isVirtual ) :
    FieldmlObject( _name, FHT_MESH_TYPE, _isVirtual )
{
    shapes = FML_INVALID_HANDLE;
    chartType = FML_INVALID_HANDLE;
    elementsType = FML_INVALID_HANDLE;
}


Evaluator::Evaluator( const string _name, FieldmlHandleType _type, FmlObjectHandle _valueType, bool _isVirtual ) :
    FieldmlObject( _name, _type, _isVirtual ),
    valueType( _valueType )
{
}


ConstantEvaluator::ConstantEvaluator( const string _name, const string _valueString, FmlObjectHandle _valueType ) :
    Evaluator( _name, FHT_CONSTANT_EVALUATOR, _valueType, false ),
    valueString( _valueString )
{
}


void ConstantEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
}


ReferenceEvaluator::ReferenceEvaluator( const string _name, FmlObjectHandle _evaluator, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_REFERENCE_EVALUATOR, _valueType, _isVirtual ),
    sourceEvaluator( _evaluator ),
    binds( FML_INVALID_HANDLE )
{
}


void ReferenceEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    delegates.insert( sourceEvaluator );
    
    const set<FmlObjectHandle> &values = binds.getValues();
    delegates.insert( values.begin(), values.end() );
}


ArgumentEvaluator::ArgumentEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_ARGUMENT_EVALUATOR, _valueType, _isVirtual )
{
}


void ArgumentEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
}


ExternalEvaluator::ExternalEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_EXTERNAL_EVALUATOR, _valueType, _isVirtual )
{
}


void ExternalEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
}


ParameterEvaluator::ParameterEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_PARAMETER_EVALUATOR, _valueType, _isVirtual )
{
    dataDescription = new UnknownDataDescription();
}


void ParameterEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    dataDescription->addDelegates( delegates );
}


ParameterEvaluator::~ParameterEvaluator()
{
    delete dataDescription;
}


PiecewiseEvaluator::PiecewiseEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_PIECEWISE_EVALUATOR, _valueType, _isVirtual ),
    binds( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE ),
    indexEvaluator( FML_INVALID_HANDLE )
{
}


void PiecewiseEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    const set<FmlObjectHandle> &evaluatorValues = evaluators.getValues();
    delegates.insert( evaluatorValues.begin(), evaluatorValues.end() );

    delegates.insert( indexEvaluator );
    
    const set<FmlObjectHandle> &bindValues = binds.getValues();
    delegates.insert( bindValues.begin(), bindValues.end() );
}


AggregateEvaluator::AggregateEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_AGGREGATE_EVALUATOR, _valueType, _isVirtual ),
    binds( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE ),
    indexEvaluator( FML_INVALID_HANDLE )
{
}


void AggregateEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    const set<FmlObjectHandle> &evaluatorValues = evaluators.getValues();
    delegates.insert( evaluatorValues.begin(), evaluatorValues.end() );

    delegates.insert( indexEvaluator );
    
    const set<FmlObjectHandle> &bindValues = binds.getValues();
    delegates.insert( bindValues.begin(), bindValues.end() );
}


DataResource::DataResource( const string _name, DataResourceType _resourceType, const string _format, const string _description ) : 
    FieldmlObject( _name, FHT_DATA_RESOURCE, false ),
    type( _resourceType ),
    format( _format ),
    description( _description )
{
}


DataResource::~DataResource()
{
}

    
BaseDataDescription::BaseDataDescription( DataDescriptionType _descriptionType ) :
    descriptionType( _descriptionType )
{
}


BaseDataDescription::~BaseDataDescription()
{
}


UnknownDataDescription::UnknownDataDescription() :
    BaseDataDescription( DESCRIPTION_UNKNOWN )
{
}


void UnknownDataDescription::addDelegates( set<FmlObjectHandle> &delegates )
{
}


FmlErrorNumber UnknownDataDescription::addIndexEvaluator( bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    return FML_ERR_INVALID_OBJECT;
}


FmlErrorNumber UnknownDataDescription::setIndexEvaluator( int index, bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    return FML_ERR_INVALID_OBJECT;
}


FmlErrorNumber UnknownDataDescription::getIndexEvaluator( int index, bool isSparse, FmlObjectHandle &evaluator )
{
    return FML_INVALID_HANDLE;
}


FmlErrorNumber UnknownDataDescription::setIndexEvaluator( int index, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    return false;
}


FmlErrorNumber UnknownDataDescription::getIndexEvaluator( int index, FmlObjectHandle &evaluator )
{
    return FML_INVALID_HANDLE;
}


FmlErrorNumber UnknownDataDescription::getIndexOrder( int index, FmlObjectHandle &order )
{
    return FML_INVALID_HANDLE;
}


int UnknownDataDescription::getIndexCount( bool isSparse )
{
    return -1;
}


UnknownDataDescription::~UnknownDataDescription()
{
}


DenseArrayDataDescription::DenseArrayDataDescription() :
    BaseDataDescription( DESCRIPTION_DENSE_ARRAY )
{
}


void DenseArrayDataDescription::addDelegates( set<FmlObjectHandle> &delegates )
{
    delegates.insert( denseIndexes.begin(), denseIndexes.end() );
}


FmlErrorNumber DenseArrayDataDescription::addIndexEvaluator( bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    if( isSparse )
    {
        return FML_ERR_INVALID_OBJECT;
    }
    
    denseIndexes.push_back( evaluator );
    denseOrders.push_back( orderEvaluator );
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber DenseArrayDataDescription::setIndexEvaluator( int index, bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    if( isSparse )
    {
        return FML_ERR_INVALID_OBJECT;
    }

    if( ( index < 0 ) || ( index >= denseIndexes.size() ) )
    {
        return FML_ERR_INVALID_INDEX;
    }
    
    denseIndexes[index] = evaluator;
    denseOrders[index] = orderEvaluator;
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber DenseArrayDataDescription::getIndexEvaluator( int index, bool isSparse, FmlObjectHandle &evaluator )
{
    if( isSparse )
    {
        evaluator = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_OBJECT;
    }

    if( ( index < 0 ) || ( index >= denseIndexes.size() ) )
    {
        evaluator = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_INDEX;
    }
    
    evaluator = denseIndexes[index];
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber DenseArrayDataDescription::setIndexEvaluator( int index, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    return setIndexEvaluator( index, false, evaluator, orderEvaluator );
}


FmlErrorNumber DenseArrayDataDescription::getIndexEvaluator( int index, FmlObjectHandle &evaluator )
{
    return getIndexEvaluator( index, false, evaluator );
}


FmlErrorNumber DenseArrayDataDescription::getIndexOrder( int index, FmlObjectHandle &order )
{
    if( ( index < 0 ) || ( index >= denseOrders.size() ) )
    {
        order = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_INDEX;
    }
    
    order = denseOrders[index];
    return FML_ERR_NO_ERROR;
}


int DenseArrayDataDescription::getIndexCount( bool isSparse )
{
    if( isSparse )
    {
        return -1;
    }
    else
    {
        return denseIndexes.size();
    }
}


DenseArrayDataDescription::~DenseArrayDataDescription()
{
}


DokArrayDataDescription::DokArrayDataDescription() :
    BaseDataDescription( DESCRIPTION_DOK_ARRAY )
{
}


void DokArrayDataDescription::addDelegates( set<FmlObjectHandle> &delegates )
{
    delegates.insert( denseIndexes.begin(), denseIndexes.end() );
    delegates.insert( sparseIndexes.begin(), sparseIndexes.end() );
}


FmlErrorNumber DokArrayDataDescription::addIndexEvaluator( bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    if( isSparse )
    {
        sparseIndexes.push_back( evaluator );
    }
    else
    {
        denseIndexes.push_back( evaluator );
        denseOrders.push_back( orderEvaluator );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber DokArrayDataDescription::setIndexEvaluator( int index, bool isSparse, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    if( index < 0 )
    {
        return FML_ERR_INVALID_INDEX;
    }
    
    if( isSparse )
    {
        if( index >= sparseIndexes.size() )
        {
            return FML_ERR_INVALID_INDEX;
        }
        
        sparseIndexes[index] = evaluator;
        
        return FML_ERR_NO_ERROR;
    }
    else
    {
        if( index >= denseIndexes.size() )
        {
            return FML_ERR_INVALID_INDEX;
        }
        
        denseIndexes[index] = evaluator;
        denseOrders[index] = orderEvaluator;
        
        return FML_ERR_NO_ERROR;
    }
}


FmlErrorNumber DokArrayDataDescription::getIndexEvaluator( int index, bool isSparse, FmlObjectHandle &evaluator )
{
    if( index < 0 )
    {
        evaluator = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_INDEX;
    }
    
    if( isSparse )
    {
        if( index >= sparseIndexes.size() )
        {
            evaluator = FML_INVALID_HANDLE;
            return FML_ERR_INVALID_INDEX;
        }
        
        evaluator = sparseIndexes[index];
        return FML_ERR_NO_ERROR;
    }
    else
    {
        if( index >= denseIndexes.size() )
        {
            evaluator = FML_INVALID_HANDLE;
            return FML_ERR_INVALID_INDEX;
        }
        
        evaluator = denseIndexes[index];
        return FML_ERR_NO_ERROR;
    }
}


FmlErrorNumber DokArrayDataDescription::getIndexOrder( int index, FmlObjectHandle &order )
{
    if( index < 0 )
    {
        order = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_INDEX;
    }
    
    if( index >= denseIndexes.size() )
    {
        order = FML_INVALID_HANDLE;
        return FML_ERR_INVALID_INDEX;
    }
    
    order = denseOrders[index];
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber DokArrayDataDescription::setIndexEvaluator( int index, FmlObjectHandle evaluator, FmlObjectHandle orderEvaluator )
{
    if( index >= sparseIndexes.size() )
    {
        index -= sparseIndexes.size();
        return setIndexEvaluator( index, false, evaluator );
    }
    else
    {
        return setIndexEvaluator( index, true, evaluator );
    }
}


FmlErrorNumber DokArrayDataDescription::getIndexEvaluator( int index, FmlObjectHandle &evaluator )
{
    if( index >= sparseIndexes.size() )
    {
        index -= sparseIndexes.size();
        return getIndexEvaluator( index, false, evaluator );
    }
    else
    {
        return getIndexEvaluator( index, true, evaluator );
    }
}


int DokArrayDataDescription::getIndexCount( bool isSparse )
{
    if( isSparse )
    {
        return sparseIndexes.size();
    }
    else
    {
        return denseIndexes.size();
    }
}


DokArrayDataDescription::~DokArrayDataDescription()
{
}


DataSource::DataSource( const std::string _name, DataResource *_resource, DataSourceType _type ) :
    FieldmlObject( _name, FHT_DATA_SOURCE, false ),
    resource( _resource ),
    type( _type )
{
}


ArrayDataSource:: ArrayDataSource( const string _name, DataResource *_resource, const string _location, int _rank ) :
    DataSource( _name, _resource, DATA_SOURCE_ARRAY ),
    rank( _rank ),
    location( _location )
{
    rawSizes.assign( rank, 0 );
    sizes.assign( rank, 0 );
    offsets.assign( rank, 0 );
}


ArrayDataSource::~ ArrayDataSource()
{
}
