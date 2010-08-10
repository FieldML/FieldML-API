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

class DomainBounds
{
public:
    const DomainBoundsType boundsType;
    
protected:   
    DomainBounds( DomainBoundsType _boundsType );
};


class UnknownBounds :
    public DomainBounds
{
public:
    UnknownBounds();
};


class ContiguousBounds :
    public DomainBounds
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


class EnsembleDomain :
    public FieldmlObject
{
public:
    const bool isComponentEnsemble;

    DomainBounds *bounds;
    
    EnsembleDomain( const std::string _name, int _regionHandle, bool _isComponentEnsemble );
};


class ContinuousDomain :
    public FieldmlObject
{
public:
    const FmlObjectHandle componentDomain;
    
    ContinuousDomain( const std::string _name, int _regionHandle, FmlObjectHandle _componentDomain );
};


class MeshDomain :
    public FieldmlObject
{
public:
    const FmlObjectHandle xiDomain;
    const FmlObjectHandle elementDomain;
    
    SimpleMap<int, std::string> shapes;
    SimpleMap<int, int> connectivity;
    
    MeshDomain( const std::string _name, int _region, FmlObjectHandle _xiDomain, FmlObjectHandle _elementDomain );
};


class Evaluator :
    public FieldmlObject
{
public:
    const FmlObjectHandle valueDomain;
    
protected:
    Evaluator( const std::string _name, int _region, FieldmlHandleType _type, FmlObjectHandle _valueDomain );
};


class ContinuousReference :
    public Evaluator
{
public:
    const FmlObjectHandle remoteEvaluator;

    SimpleMap<FmlObjectHandle, FmlObjectHandle> aliases;

    ContinuousReference( const std::string _name, int _region, FmlObjectHandle _evaluator, FmlObjectHandle _valueDomain );
};


class ContinuousPiecewise :
    public Evaluator
{
public:
    FmlObjectHandle indexDomain;
    
    SimpleMap<FmlObjectHandle, FmlObjectHandle> aliases;
    SimpleMap<int, FmlObjectHandle> evaluators;
    
    ContinuousPiecewise( const std::string name, int region, FmlObjectHandle indexDomain, FmlObjectHandle valueDomain );
};


class ContinuousAggregate :
    public Evaluator
{
public:
    SimpleMap<FmlObjectHandle, FmlObjectHandle> aliases;
    SimpleMap<int, FmlObjectHandle> evaluators;
    
    ContinuousAggregate( const std::string _name, int _region, FmlObjectHandle _valueDomain );
};


class Variable :
    public Evaluator
{
public:
    Variable( const std::string name, int region, FmlObjectHandle _valueDomain, bool isEnsemble );
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
    
    const int *swizzle;
    int swizzleCount;

    DataLocation *dataLocation;
    
    SemidenseDataDescription();
};


class Parameters :
    public Evaluator
{
public:
    DataDescription *dataDescription;
    
    Parameters( const std::string _name, int _region, FmlObjectHandle _valueDomain, bool isEnsemble );
};

int Fieldml_GetRegion( FmlHandle handle, FmlObjectHandle objectHandle );

const char *Fieldml_GetName( FmlHandle handle );

int Fieldml_SetRoot( FmlHandle, const char *root );


#endif //H_FIELDML_STRUCTS
