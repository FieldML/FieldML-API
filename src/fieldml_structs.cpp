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


ContinuousType::ContinuousType( const string _name, bool _isVirtual ) :
    FieldmlObject( _name, FHT_CONTINUOUS_TYPE, _isVirtual )
{
    componentType = FML_INVALID_HANDLE;
}


MeshType::MeshType( const string _name, bool _isVirtual ) :
    FieldmlObject( _name, FHT_MESH_TYPE, _isVirtual ),
    shapes("")
{
    chartType = FML_INVALID_HANDLE;
    elementsType = FML_INVALID_HANDLE;
}


Evaluator::Evaluator( const string _name, FieldmlHandleType _type, FmlObjectHandle _valueType, bool _isVirtual ) :
    FieldmlObject( _name, _type, _isVirtual ),
    valueType( _valueType )
{
}


ReferenceEvaluator::ReferenceEvaluator( const string _name, FmlObjectHandle _evaluator, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_REFERENCE_EVALUATOR, _valueType, _isVirtual ),
    sourceEvaluator( _evaluator ),
    binds( FML_INVALID_HANDLE )
{
}


ArgumentEvaluator::ArgumentEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_ARGUMENT_EVALUATOR, _valueType, _isVirtual )
{
}


ExternalEvaluator::ExternalEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_EXTERNAL_EVALUATOR, _valueType, _isVirtual )
{
}


ParameterEvaluator::ParameterEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_PARAMETER_EVALUATOR, _valueType, _isVirtual ),
    dataDescription( new UnknownDataDescription() )
{
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


AggregateEvaluator::AggregateEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_AGGREGATE_EVALUATOR, _valueType, _isVirtual ),
    binds( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE ),
    indexEvaluator( FML_INVALID_HANDLE )
{
}


DataResource::DataResource( const string _name, DataResourceType _resourceType ) :
    FieldmlObject( _name, FHT_DATA_RESOURCE, false ),
    type( _resourceType )
{
}


DataResource::~DataResource()
{
}

    
TextDataResource::TextDataResource( const string _name, DataResourceType _type ) :
    DataResource( _name, _type )
{
}


TextDataResource::~TextDataResource()
{
}


TextFileDataResource::TextFileDataResource( const string _name, const string _href ) :
    TextDataResource( _name, DATA_RESOURCE_TEXT_FILE ),
    href( _href )
{
}


TextFileDataResource::~TextFileDataResource()
{
}


TextInlineDataResource::TextInlineDataResource( const string _name ) : 
    TextDataResource( _name, DATA_RESOURCE_TEXT_INLINE )
{
    inlineString = new char[1];
    ((char*)inlineString)[0] = 0; //Dirty hack.
    length = 0;
}

TextInlineDataResource::~TextInlineDataResource()
{
    delete[] inlineString;
}


ArrayDataResource::ArrayDataResource( const string _name, const string _format, const string _href ) : 
    DataResource( _name, DATA_RESOURCE_ARRAY ),
    format( _format ),
    href( _href )
{
}

ArrayDataResource::~ArrayDataResource()
{
}


DataDescription::DataDescription( DataDescriptionType _descriptionType ) :
    descriptionType( _descriptionType )
{
}


DataDescription::~DataDescription()
{
}


UnknownDataDescription::UnknownDataDescription() :
    DataDescription( DESCRIPTION_UNKNOWN )
{
}


SemidenseDataDescription::SemidenseDataDescription() :
    DataDescription( DESCRIPTION_SEMIDENSE )
{
}


SemidenseDataDescription::~SemidenseDataDescription()
{
}


DenseArrayDataDescription::DenseArrayDataDescription() :
    DataDescription( DESCRIPTION_DENSE_ARRAY )
{
}


DenseArrayDataDescription::~DenseArrayDataDescription()
{
}


DOKArrayDataDescription::DOKArrayDataDescription() :
    DataDescription( DESCRIPTION_DOK_ARRAY )
{
}


DOKArrayDataDescription::~DOKArrayDataDescription()
{
}


TextDataSource::TextDataSource( const string _name, TextDataResource *_resource, int _firstLine, int _count, int _length, int _head, int _tail ) :
    DataSource<TextDataResource>( _name, _resource, DATA_SOURCE_TEXT ),
    firstLine( _firstLine ),
    count( _count ),
    length( _length ),
    head( _head ),
    tail( _tail )
{
}


TextDataSource::~TextDataSource()
{
}


ArrayDataSource::ArrayDataSource( const string _name, ArrayDataResource *_resource, const string _sourceName ) :
    DataSource<ArrayDataResource>( _name, _resource, DATA_SOURCE_ARRAY ),
    sourceName( _sourceName )
{
}


ArrayDataSource::~ArrayDataSource()
{
}
