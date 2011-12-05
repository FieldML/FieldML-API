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
#include "Evaluators.h"
#include "ErrorContextAutostack.h"

using namespace std;

namespace EvaluatorsUtil
{

template <typename T> T *checkedCast(FieldmlSession *session, const FmlObjectHandle objectHandle, const FieldmlHandleType type )
{
    FieldmlObject *object = session->getObject( objectHandle );
    if( object == NULL )
    {
        return NULL;
    }

    if( object->objectType != type )
    {
        return NULL;
    }
    
    return (T *)object;
}

} //End namespace EvaluatorsUtil

Evaluator::Evaluator( const string _name, FieldmlHandleType _type, FmlObjectHandle _valueType, bool _isVirtual ) :
    FieldmlObject( _name, _type, _isVirtual ),
    valueType( _valueType )
{
}


Evaluator *Evaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = session->getObject( objectHandle );
    if( object == NULL )
    {
        return NULL;
    }

    if( ( object->objectType != FHT_AGGREGATE_EVALUATOR ) &&
        ( object->objectType != FHT_ARGUMENT_EVALUATOR ) &&
        ( object->objectType != FHT_EXTERNAL_EVALUATOR ) &&
        ( object->objectType != FHT_PARAMETER_EVALUATOR ) &&
        ( object->objectType != FHT_PIECEWISE_EVALUATOR ) &&
        ( object->objectType != FHT_REFERENCE_EVALUATOR ) &&
        ( object->objectType != FHT_CONSTANT_EVALUATOR ) )
    {
        return NULL;
    }
    
    return (Evaluator*)object;
}


ConstantEvaluator::ConstantEvaluator( const string _name, const string _valueString, FmlObjectHandle _valueType ) :
    Evaluator( _name, FHT_CONSTANT_EVALUATOR, _valueType, false ),
    valueString( _valueString )
{
}


bool ConstantEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    return false;
}


ConstantEvaluator *ConstantEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<ConstantEvaluator>( session, objectHandle, FHT_CONSTANT_EVALUATOR );
}


ReferenceEvaluator::ReferenceEvaluator( const string _name, FmlObjectHandle _evaluator, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_REFERENCE_EVALUATOR, _valueType, _isVirtual ),
    sourceEvaluator( _evaluator ),
    binds( FML_INVALID_HANDLE )
{
}


bool ReferenceEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    delegates.insert( sourceEvaluator );
    
    const set<FmlObjectHandle> &values = binds.getValues();
    delegates.insert( values.begin(), values.end() );
    
    return true;
}


ReferenceEvaluator *ReferenceEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<ReferenceEvaluator>( session, objectHandle, FHT_REFERENCE_EVALUATOR );
}


ArgumentEvaluator::ArgumentEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_ARGUMENT_EVALUATOR, _valueType, _isVirtual )
{
}


bool ArgumentEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    return false;
}


ArgumentEvaluator *ArgumentEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<ArgumentEvaluator>( session, objectHandle, FHT_ARGUMENT_EVALUATOR );
}


ExternalEvaluator::ExternalEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_EXTERNAL_EVALUATOR, _valueType, _isVirtual )
{
}


bool ExternalEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    return false;
}


ExternalEvaluator *ExternalEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<ExternalEvaluator>( session, objectHandle, FHT_EXTERNAL_EVALUATOR );
}


ParameterEvaluator::ParameterEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_PARAMETER_EVALUATOR, _valueType, _isVirtual )
{
    dataDescription = new UnknownDataDescription();
}


bool ParameterEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    dataDescription->addDelegates( delegates );
    
    return true;
}


ParameterEvaluator::~ParameterEvaluator()
{
    delete dataDescription;
}


ParameterEvaluator *ParameterEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<ParameterEvaluator>( session, objectHandle, FHT_PARAMETER_EVALUATOR );
}


PiecewiseEvaluator::PiecewiseEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_PIECEWISE_EVALUATOR, _valueType, _isVirtual ),
    binds( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE ),
    indexEvaluator( FML_INVALID_HANDLE )
{
}


bool PiecewiseEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    const set<FmlObjectHandle> &evaluatorValues = evaluators.getValues();
    delegates.insert( evaluatorValues.begin(), evaluatorValues.end() );

    delegates.insert( indexEvaluator );
    
    const set<FmlObjectHandle> &bindValues = binds.getValues();
    delegates.insert( bindValues.begin(), bindValues.end() );
    
    return true;
}


PiecewiseEvaluator *PiecewiseEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<PiecewiseEvaluator>( session, objectHandle, FHT_PIECEWISE_EVALUATOR );
}


AggregateEvaluator::AggregateEvaluator( const string _name, FmlObjectHandle _valueType, bool _isVirtual ) :
    Evaluator( _name, FHT_AGGREGATE_EVALUATOR, _valueType, _isVirtual ),
    binds( FML_INVALID_HANDLE ),
    evaluators( FML_INVALID_HANDLE ),
    indexEvaluator( FML_INVALID_HANDLE )
{
}


bool AggregateEvaluator::addDelegates( set<FmlObjectHandle> &delegates )
{
    const set<FmlObjectHandle> &evaluatorValues = evaluators.getValues();
    delegates.insert( evaluatorValues.begin(), evaluatorValues.end() );

    delegates.insert( indexEvaluator );
    
    const set<FmlObjectHandle> &bindValues = binds.getValues();
    delegates.insert( bindValues.begin(), bindValues.end() );
    
    return true;
}


AggregateEvaluator *AggregateEvaluator::checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle )
{
    return EvaluatorsUtil::checkedCast<AggregateEvaluator>( session, objectHandle, FHT_AGGREGATE_EVALUATOR );
}
