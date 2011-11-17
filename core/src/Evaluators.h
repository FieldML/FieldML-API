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

#ifndef H_EVALUATORS
#define H_EVALUATORS

#include <vector>
#include <set>
#include <string>

#include "fieldml_api.h"
#include "fieldml_structs.h"
#include "SimpleMap.h"
#include "FieldmlSession.h"

class Evaluator :
    public FieldmlObject
{
public:
    const FmlObjectHandle valueType;

    Evaluator( const std::string _name, FieldmlHandleType _type, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates ) = 0;

    static Evaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};
    
    
class ConstantEvaluator :
    public Evaluator
{
public:
    const std::string valueString;
    
    ConstantEvaluator( const std::string _name, const std::string _literal, FmlObjectHandle _valueType );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static ConstantEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class ReferenceEvaluator :
    public Evaluator
{
public:
    const FmlObjectHandle sourceEvaluator;

    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;

    ReferenceEvaluator( const std::string _name, FmlObjectHandle _evaluator, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static ReferenceEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class PiecewiseEvaluator :
    public Evaluator
{
public:
    FmlObjectHandle indexEvaluator;
    
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> evaluators;
    
    PiecewiseEvaluator( const std::string name, FmlObjectHandle valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static PiecewiseEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class AggregateEvaluator :
    public Evaluator
{
public:
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> evaluators;
    
    FmlObjectHandle indexEvaluator;
    
    AggregateEvaluator( const std::string _name, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static AggregateEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class ArgumentEvaluator :
    public Evaluator
{
public:
    std::set<FmlObjectHandle> arguments;
    
    ArgumentEvaluator( const std::string name, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static ArgumentEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class ExternalEvaluator :
    public Evaluator
{
public:
    std::set<FmlObjectHandle> arguments;
    
    ExternalEvaluator( const std::string name, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    static ExternalEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


class ParameterEvaluator :
    public Evaluator
{
public:
    BaseDataDescription *dataDescription;
    
    ParameterEvaluator( const std::string _name, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual void addDelegates( std::set<FmlObjectHandle> &delegates );
    
    virtual ~ParameterEvaluator();
    
    static ParameterEvaluator *checkedCast( FieldmlSession *session, FmlObjectHandle objectHandle );
};


#endif //H_EVALUATORS
