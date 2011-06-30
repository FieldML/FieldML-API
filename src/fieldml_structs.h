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

#ifndef H_FIELDML_STRUCTS
#define H_FIELDML_STRUCTS

#include <vector>
#include <set>
#include <string>

#include "fieldml_api.h"
#include "SimpleMap.h"
#include "SimpleBitset.h"

class FieldmlObject
{
public:
    const FieldmlHandleType type;
    const std::string name;
    const bool isVirtual;

    int intValue;
    
    FieldmlObject( const std::string _name, FieldmlHandleType _type, bool _isVirtual );
    
    virtual ~FieldmlObject();
};


class EnsembleType :
    public FieldmlObject
{
public:
    const bool isComponentEnsemble;

    EnsembleMembersType type;
    
    FmlEnsembleValue min;
    
    FmlEnsembleValue max;
    
    int stride;
    
    int count;
    
    FmlObjectHandle dataSource;
    
    EnsembleType( const std::string _name, bool _isComponentEnsemble, bool _isVirtual );
};


class ElementSequence :
    public FieldmlObject
{
public:
    const FmlObjectHandle elementType;

    SimpleBitset members;
    
    ElementSequence( const std::string _name, FmlObjectHandle _componentType );
};


class ContinuousType :
    public FieldmlObject
{
public:
    FmlObjectHandle componentType;
    
    ContinuousType( const std::string _name, bool _isVirtual );
};


class MeshType :
    public FieldmlObject
{
public:
    FmlObjectHandle chartType;
    FmlObjectHandle elementsType;
    
    SimpleMap<FmlEnsembleValue, std::string> shapes;
    
    MeshType( const std::string _name, bool _isVirtual );
};


class Evaluator :
    public FieldmlObject
{
public:
    const FmlObjectHandle valueType;

    Evaluator( const std::string _name, FieldmlHandleType _type, FmlObjectHandle _valueType, bool _isVirtual );
};


class ReferenceEvaluator :
    public Evaluator
{
public:
    const FmlObjectHandle sourceEvaluator;

    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;

    ReferenceEvaluator( const std::string _name, FmlObjectHandle _evaluator, FmlObjectHandle _valueType, bool _isVirtual );
};


class PiecewiseEvaluator :
    public Evaluator
{
public:
    FmlObjectHandle indexEvaluator;
    
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> evaluators;
    
    PiecewiseEvaluator( const std::string name, FmlObjectHandle valueType, bool _isVirtual );
};


class AggregateEvaluator :
    public Evaluator
{
public:
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<FmlEnsembleValue, FmlObjectHandle> evaluators;
    
    FmlObjectHandle indexEvaluator;
    
    AggregateEvaluator( const std::string _name, FmlObjectHandle _valueType, bool _isVirtual );
};


class ArgumentEvaluator :
    public Evaluator
{
public:
    std::set<FmlObjectHandle> arguments;
    
    ArgumentEvaluator( const std::string name, FmlObjectHandle _valueType, bool _isVirtual );
};


class ExternalEvaluator :
    public Evaluator
{
public:
    std::set<FmlObjectHandle> arguments;
    
    ExternalEvaluator( const std::string name, FmlObjectHandle _valueType, bool _isVirtual );
};


class DataResource :
    public FieldmlObject
{
protected:
    DataResource( const std::string _name, DataResourceType _type );
        
public:
    std::vector<FmlObjectHandle> dataSources;

    const DataResourceType type;
    
    virtual ~DataResource();
};
 

class TextFileDataResource :
    public DataResource
{
public:
    const std::string href;
        
    TextFileDataResource( const std::string _name, const std::string _href );
    
    virtual ~TextFileDataResource(); 
};


class TextInlineDataResource :
    public DataResource
{
public:
    char *inlineString;
    int length;

    TextInlineDataResource( const std::string _name );
    
    virtual ~TextInlineDataResource();
};
    
    
class ArrayDataResource :
    public DataResource
{
public:
    const std::string format;
    const std::string href;

    ArrayDataResource( const std::string _name, const std::string _type, const std::string _href );
    
    virtual ~ArrayDataResource();
};

    
class DataSource :
    public FieldmlObject
{
protected:
    DataSource( const std::string _name, DataResource *_resource, DataSourceType _type );
    
public:
    const DataSourceType type;
    
    const DataResource *resource;

    virtual ~DataSource();
};

    
class TextDataSource :
    public DataSource
{
public:
    const int firstLine;
    
    const int count;
    
    const int length;
    
    const int head;
    
    const int tail;
    
    TextDataSource( const std::string _name, DataResource *_resource, int _firstLine, int _count, int _length, int _head, int _tail );
    
    virtual ~TextDataSource();
};


class ArrayDataSource :
    public DataSource
{
public:
    const std::string sourceName;
    
    ArrayDataSource( const std::string _name, ArrayDataResource *_resource, const std::string _sourceName );
    
    virtual ~ArrayDataSource();
};


class DataDescription
{
public:
    const DataDescriptionType descriptionType;
    
    virtual ~DataDescription();

protected:
    DataDescription( DataDescriptionType _descriptionType );
};


class UnknownDataDescription :
    public DataDescription
{
public:
    UnknownDataDescription();
};


class SemidenseDataDescription :
    public DataDescription
{
public:
    FmlObjectHandle dataSource;
    
    std::vector<FmlObjectHandle> sparseIndexes;
    std::vector<FmlObjectHandle> denseIndexes;
    std::vector<FmlObjectHandle> denseOrders;
    
    SemidenseDataDescription();
    
    virtual ~SemidenseDataDescription();
};
    

class DenseArrayDataDescription :
    public DataDescription
{
public:
    FmlObjectHandle dataSource;
    
    std::vector<FmlObjectHandle> denseIndexes;
    std::vector<FmlObjectHandle> denseOrders;
    
    DenseArrayDataDescription();
    virtual ~DenseArrayDataDescription();
};


class DOKArrayDataDescription :
    public DataDescription
{
public:
    FmlObjectHandle keySource;
    FmlObjectHandle valueSource;
    
    std::vector<FmlObjectHandle> sparseIndexes;
    std::vector<FmlObjectHandle> denseIndexes;
    std::vector<FmlObjectHandle> denseOrders;
    
    DOKArrayDataDescription();
    virtual ~DOKArrayDataDescription();
};


class ParameterEvaluator :
    public Evaluator
{
public:
    DataDescription *dataDescription;
    
    ParameterEvaluator( const std::string _name, FmlObjectHandle _valueType, bool _isVirtual );
    
    virtual ~ParameterEvaluator();
};


#endif //H_FIELDML_STRUCTS
