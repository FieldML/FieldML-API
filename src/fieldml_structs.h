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
#include <string>

#include "fieldml_api.h"
#include "SimpleMap.h"

extern const int FILE_REGION_HANDLE;

extern const int LIBRARY_REGION_HANDLE;

extern const int VIRTUAL_REGION_HANDLE;

class TypeBounds
{
public:
    const TypeBoundsType boundsType;
    
protected:   
    TypeBounds( TypeBoundsType _boundsType );
};


class UnknownBounds :
    public TypeBounds
{
public:
    UnknownBounds();
};


class ContiguousBounds :
    public TypeBounds
{
public:
    const int count;
    
    ContiguousBounds( const int _count );
};


class FieldmlObject
{
public:
    const FieldmlHandleType type;
    const std::string name;
    int regionHandle; // One day this will be meaningful. For now, 0 = library, 1 = not library.

    SimpleMap<std::string, std::string> markup;
    int intValue;
    
    FieldmlObject( const std::string _name, int _regionHandle, FieldmlHandleType _type );
};


class ElementSet :
    public FieldmlObject
{
public:
    const FmlObjectHandle valueType;

    //NOTE Can't use std::bitset, as the size cannot be set at compile-time.
    //TODO Encapsulate all the relevant logic into a custom bitset class that wraps vector<bool>
    std::vector<int> presentElements;
    int maxElement;
    int lastSetCount;
    int lastIndex;
    
    ElementSet( const std::string _name, int _regionHandle, FmlObjectHandle _valueType );
};


class EnsembleType :
    public FieldmlObject
{
public:
    const bool isComponentEnsemble;

    TypeBounds *bounds;
    
    EnsembleType( const std::string _name, int _regionHandle, bool _isComponentEnsemble );
};


class ContinuousType :
    public FieldmlObject
{
public:
    const FmlObjectHandle componentType;
    
    ContinuousType( const std::string _name, int _regionHandle, FmlObjectHandle _componentType );
};


class MeshType :
    public FieldmlObject
{
public:
    const FmlObjectHandle xiType;
    const FmlObjectHandle elementType;
    
    SimpleMap<int, std::string> shapes;
    SimpleMap<FmlObjectHandle, FmlObjectHandle> connectivity;
    
    MeshType( const std::string _name, int _region, FmlObjectHandle _xiType, FmlObjectHandle _elementType );
};


class Evaluator :
    public FieldmlObject
{
public:
    const FmlObjectHandle valueType;

    std::vector<FmlObjectHandle> variables;

    Evaluator( const std::string _name, int _region, FieldmlHandleType _type, FmlObjectHandle _valueType );
};


class ReferenceEvaluator :
    public Evaluator
{
public:
    const FmlObjectHandle remoteEvaluator;

    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;

    ReferenceEvaluator( const std::string _name, int _region, FmlObjectHandle _evaluator, FmlObjectHandle _valueType );
};


class PiecewiseEvaluator :
    public Evaluator
{
public:
    FmlObjectHandle indexEvaluator;
    
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<int, FmlObjectHandle> evaluators;
    
    PiecewiseEvaluator( const std::string name, int region, FmlObjectHandle valueType );
};


class AggregateEvaluator :
    public Evaluator
{
public:
    SimpleMap<FmlObjectHandle, FmlObjectHandle> binds;
    SimpleMap<int, FmlObjectHandle> evaluators;
    
    FmlObjectHandle indexEvaluator;
    
    AggregateEvaluator( const std::string _name, int _region, FmlObjectHandle _valueType );
};


class AbstractEvaluator :
    public Evaluator
{
public:
    AbstractEvaluator( const std::string name, int region, FmlObjectHandle _valueType );
};


class DataLocation
{
public:
    const DataLocationType locationType;

protected:
    DataLocation( DataLocationType _locationType );
};


class UnknownDataLocation :
    public DataLocation
{
public:
    UnknownDataLocation();
};


class InlineDataLocation :
     public DataLocation
{
public:
    const char *data;
    int length;
    
    InlineDataLocation();
};


class FileDataLocation :
    public DataLocation
{
public:
    std::string filename;
    int offset;
    DataFileType fileType;

    FileDataLocation();
};


class DataDescription
{
public:
    const DataDescriptionType descriptionType;

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
    std::vector<FmlObjectHandle> sparseIndexes;
    std::vector<FmlObjectHandle> denseIndexes;
    std::vector<FmlObjectHandle> denseSets;
    
    const int *swizzle;
    int swizzleCount;

    DataLocation *dataLocation;
    
    SemidenseDataDescription();
};


class ParameterEvaluator :
    public Evaluator
{
public:
    DataDescription *dataDescription;
    
    ParameterEvaluator( const std::string _name, int _region, FmlObjectHandle _valueType );
};

int Fieldml_GetRegion( FmlHandle handle, FmlObjectHandle objectHandle );

const char *Fieldml_GetName( FmlHandle handle );

int Fieldml_SetRoot( FmlHandle, const char *root );


#endif //H_FIELDML_STRUCTS
