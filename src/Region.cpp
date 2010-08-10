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

#include "string_const.h"
#include "fieldml_structs.h"
#include "fieldml_sax.h"

using namespace std;

//========================================================================
//
// Consts
//
//========================================================================

const int INVALID_REGION_HANDLE = -2;
const int VIRTUAL_REGION_HANDLE = -1; //For derived objects, e.g. mesh domain xi and element domains.
const int LIBRARY_REGION_HANDLE = 0;
const int FILE_REGION_HANDLE = 1;

const string collapse2d[4] = {
    "_xi1C_xi20",
    "_xi1C_xi21",
    "_xi10_xi2C",
    "_xi11_xi2C"
};

const string collapse3d_wedge[12] = {
    "_xi1C_xi20",
    "_xi1C_xi21",
    "_xi1C_xi30",
    "_xi1C_xi31",
    "_xi10_xi2C",
    "_xi11_xi2C",
    "_xi2C_xi30",
    "_xi2C_xi31",
    "_xi10_xi3C",
    "_xi11_xi3C",
    "_xi20_xi3C",
    "_xi21_xi3C"
};

const string collapse3d_pyramid[6] = {
    "_xi10_xi2C_xi3C",
    "_xi11_xi2C_xi3C",
    "_xi1C_xi20_xi3C",
    "_xi1C_xi21_xi3C",
    "_xi1C_xi2C_xi30",
    "_xi1C_xi2C_xi31",
};



static void addMarkup( FieldmlRegion *region, FmlObjectHandle handle, const string attribute, const string value )
{
    FieldmlObject *object = region->getObject( handle );

    object->markup.set( attribute, value );
}


static void setRegionHandle( FieldmlRegion *region, FmlObjectHandle handle, int regionHandle )
{
    FieldmlObject *object = region->getObject( handle );
    object->regionHandle = regionHandle;
}


static FmlObjectHandle addEnsembleDomain( FieldmlRegion *region, int regionHandle, const string name, int count, int isComponentDomain )
{
    int handle;

    if( isComponentDomain )
    {
        handle = Fieldml_CreateComponentEnsembleDomain( region, name.c_str() );
    }
    else
    {
        handle = Fieldml_CreateEnsembleDomain( region, name.c_str(), FML_INVALID_HANDLE );
    }
    Fieldml_SetContiguousBoundsCount( region, handle, count );
    setRegionHandle( region, handle, regionHandle );
    
    return handle;
}


static FmlObjectHandle addContinuousDomain( FieldmlRegion *region, int regionHandle, const string name, FmlObjectHandle componentHandle )
{
    int handle;

    handle = Fieldml_CreateContinuousDomain( region, name.c_str(), componentHandle );
    setRegionHandle( region, handle, regionHandle );
    
    return handle;
}


static FmlObjectHandle addEvaluator( FieldmlRegion *region, int regionHandle, const string name, FmlObjectHandle domainHandle )
{
    FieldmlObject *object;
    int handle;
    int type;
    
    type = Fieldml_GetObjectType( region, domainHandle );
    
    if( type == FHT_ENSEMBLE_DOMAIN )
    {
        object = new FieldmlObject( name, regionHandle, FHT_REMOTE_ENSEMBLE_EVALUATOR );
    }
    else if( type == FHT_CONTINUOUS_DOMAIN )
    {
        object = new FieldmlObject( name, regionHandle, FHT_REMOTE_CONTINUOUS_EVALUATOR );
    }

    handle = region->addObject( object );
    setRegionHandle( region, handle, regionHandle );
    
    return handle;
}




static void addLibraryDomains( FieldmlRegion *region )
{
    FmlObjectHandle handle;
    int i;

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.generic.1d", 1, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.real.1d", handle );

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.generic.2d", 2, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.real.2d", handle );

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.generic.3d", 3, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.real.3d", handle );
    
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.xi.1d", 1, 1 );
    handle = addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.xi.1d", handle );
    addMarkup( region, handle, "xi", "true" );

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.xi.2d", 2, 1 );
    handle = addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.xi.2d", handle );
    addMarkup( region, handle, "xi", "true" );

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.xi.3d", 3, 1 );
    handle = addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.xi.3d", handle );
    addMarkup( region, handle, "xi", "true" );

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.line.2", 2, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.linear_lagrange", handle ); 
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.line.3", 3, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.quadratic_lagrange", handle ); 

    // 2x2 quad
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.square.2x2", 4, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.bilinear_lagrange", handle );

    for( i = 0; i < 4; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.square.2x2" + collapse2d[i], 3, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.bilinear_lagrange" + collapse2d[i], handle );
    }
    
    // 3x3 quad
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.square.3x3", 9, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.biquadratic_lagrange", handle ); 

    for( i = 0; i < 4; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.square.3x3" + collapse2d[i], 7, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.biquadratic_lagrange" + collapse2d[i], handle );
    }

    // 2x2x2 cube
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.2x2x2", 8, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.trilinear_lagrange", handle );

    for( i = 0; i < 12; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.2x2x2" + collapse3d_wedge[i], 6, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.trilinear_lagrange" + collapse3d_wedge[i], handle );
    }

    for( i = 0; i < 6; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.2x2x2" + collapse3d_pyramid[i], 5, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.trilinear_lagrange" + collapse3d_pyramid[i], handle );
    }

    // 3x3x3 cube
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.3x3x3", 27, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.triquadratic_lagrange", handle ); 

    for( i = 0; i < 12; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.3x3x3" + collapse3d_wedge[i], 21, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.triquadratic_lagrange" + collapse3d_wedge[i], handle );
    }

    for( i = 0; i < 6; i++ )
    {
        handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.local_nodes.cube.3x3x3" + collapse3d_pyramid[i], 19, 1 );
        addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.parameters.triquadratic_lagrange" + collapse3d_pyramid[i], handle );
    }

    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.rc.1d", 1, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.coordinates.rc.1d", handle );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.velocity.rc.1d", handle );
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.rc.2d", 2, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.coordinates.rc.2d", handle );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.velocity.rc.2d", handle );
    handle = addEnsembleDomain( region, LIBRARY_REGION_HANDLE, "library.ensemble.rc.3d", 3, 1 );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.coordinates.rc.3d", handle );
    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.velocity.rc.3d", handle );

    addContinuousDomain( region, LIBRARY_REGION_HANDLE, "library.pressure", FML_INVALID_HANDLE );
}


static void addLibraryEvaluators( FieldmlRegion *region )
{
    FmlObjectHandle domainHandle;
    int i;
    
    domainHandle = Fieldml_GetNamedObject( region, "library.real.1d" );
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.linear_lagrange", domainHandle );
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.bilinear_lagrange", domainHandle );
    for( i = 0; i < 4; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.bilinear_lagrange" + collapse2d[i], domainHandle );
    }
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.trilinear_lagrange", domainHandle );
    for( i = 0; i < 12; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.trilinear_lagrange" + collapse3d_wedge[i], domainHandle );
    }
    for( i = 0; i < 6; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.trilinear_lagrange" + collapse3d_pyramid[i], domainHandle );
    }

    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.quadratic_lagrange", domainHandle );
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.biquadratic_lagrange", domainHandle );
    for( i = 0; i < 4; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.biquadratic_lagrange" + collapse2d[i], domainHandle );
    }
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.triquadratic_lagrange", domainHandle );
    for( i = 0; i < 12; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.triquadratic_lagrange" + collapse3d_wedge[i], domainHandle );
    }
    for( i = 0; i < 6; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.triquadratic_lagrange" + collapse3d_pyramid[i], domainHandle );
    }

    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.cubic_lagrange", domainHandle );
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.bicubic_lagrange", domainHandle );
    for( i = 0; i < 4; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.bicubic_lagrange" + collapse2d[i], domainHandle );
    }
    
    addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.tricubic_lagrange", domainHandle );
    for( i = 0; i < 12; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.tricubic_lagrange" + collapse3d_wedge[i], domainHandle );
    }
    for( i = 0; i < 6; i++ )
    {
        addEvaluator( region, LIBRARY_REGION_HANDLE, "library.fem.tricubic_lagrange" + collapse3d_pyramid[i], domainHandle );
    }
}


FieldmlRegion::FieldmlRegion( const string _location, const string _name ) :
    name( _name )
{
    root = _location;
    
    addLibraryDomains( this );
    
    addLibraryEvaluators( this );
}


FieldmlRegion::~FieldmlRegion()
{
    std::for_each( objects.begin(), objects.end(), delete_object() );
}


void FieldmlRegion::finalize()
{
    FieldmlObject *object, *newObject;
    int i, count;
    
    count = objects.size();
    
    for( i = 0; i < count; i++ )
    {
        object = objects[i];
        
        if( ( object->type == FHT_UNKNOWN_CONTINUOUS_DOMAIN ) || ( object->type == FHT_UNKNOWN_CONTINUOUS_SOURCE ) )
        {
            newObject = new FieldmlObject( object->name, object->regionHandle, FHT_REMOTE_CONTINUOUS_DOMAIN );
            objects[i] = newObject;
            delete object;
        }
        else if( ( object->type == FHT_UNKNOWN_ENSEMBLE_DOMAIN ) || ( object->type == FHT_UNKNOWN_ENSEMBLE_SOURCE ) )
        {
            newObject = new FieldmlObject( object->name, object->regionHandle, FHT_REMOTE_ENSEMBLE_DOMAIN );
            objects[i] = newObject;
            delete object;
        }
        else if( object->type == FHT_UNKNOWN_CONTINUOUS_EVALUATOR )
        {
            newObject = new FieldmlObject( object->name, object->regionHandle, FHT_REMOTE_CONTINUOUS_EVALUATOR );
            objects[i] = newObject;
            delete object;
        }
        else if( object->type == FHT_UNKNOWN_ENSEMBLE_EVALUATOR )
        {
            newObject = new FieldmlObject( object->name, object->regionHandle, FHT_REMOTE_ENSEMBLE_EVALUATOR );
            objects[i] = newObject;
            delete object;
        }
    }
}


FieldmlObject *FieldmlRegion::getObject( const FmlObjectHandle handle )
{
    if( ( handle < 0 ) || ( handle >= objects.size() ) )
    {
        setRegionError( FML_ERR_UNKNOWN_OBJECT );
        return NULL;
    }
    
    setRegionError( FML_ERR_NO_ERROR );
    return objects[handle];
}


int FieldmlRegion::setRegionErrorAndLocation( const char *file, const int line, const int error )
{
    lastError = error;

    if( error != FML_ERR_NO_ERROR )
    {
        if( debug )
        {
            printf("FIELDML %s (%s): Error %d at %s:%d\n", FML_VERSION_STRING, __DATE__, error, file, line );
        }
    }
    
    return error;
}


int FieldmlRegion::setRoot( const string newRoot )
{
    root = newRoot;

    return setRegionError( FML_ERR_NO_ERROR );  
}


const string FieldmlRegion::getRoot()
{
    return root;
}


const string FieldmlRegion::getName()
{
    return name;
}


FmlObjectHandle FieldmlRegion::addObject( FieldmlObject *object )
{
    int doSwitch;
    FieldmlObject *oldObject;
    FmlObjectHandle handle = Fieldml_GetNamedObject( this, object->name.c_str() );
    
    if( handle == FML_INVALID_HANDLE )
    {
        objects.push_back( object );
        return objects.size() - 1;
    }

    doSwitch = 0;
    
    oldObject = getObject( handle );
    
    if( ( oldObject->regionHandle != VIRTUAL_REGION_HANDLE ) ||
        ( object->regionHandle == VIRTUAL_REGION_HANDLE ) )
    {
        // Do nothing. Virtual objects should never replace non-virtual ones.
    }
    if( oldObject->type == FHT_UNKNOWN_ENSEMBLE_DOMAIN )
    {
        if( object->type == FHT_ENSEMBLE_DOMAIN )
        {
            doSwitch = 1;
        }
    }
    else if( oldObject->type == FHT_UNKNOWN_CONTINUOUS_DOMAIN )
    {
        if( object->type == FHT_CONTINUOUS_DOMAIN )
        {
            doSwitch = 1;
        }
    }
    else if( oldObject->type == FHT_UNKNOWN_ENSEMBLE_SOURCE )
    {
        if( ( object->type == FHT_ENSEMBLE_DOMAIN ) ||
            ( object->type == FHT_ENSEMBLE_PARAMETERS ) ||
            ( object->type == FHT_ENSEMBLE_VARIABLE ) )
        {
            doSwitch = 1;
        }
    }
    else if( oldObject->type == FHT_UNKNOWN_CONTINUOUS_SOURCE )
    {
        if( ( object->type == FHT_CONTINUOUS_DOMAIN ) ||
            ( object->type == FHT_CONTINUOUS_PIECEWISE ) ||
            ( object->type == FHT_CONTINUOUS_REFERENCE ) ||
            ( object->type == FHT_CONTINUOUS_AGGREGATE ) ||
            ( object->type == FHT_CONTINUOUS_PARAMETERS ) ||
            ( object->type == FHT_CONTINUOUS_VARIABLE ) )
        {
            doSwitch = 1;
        }
    }
    else if( oldObject->type == FHT_UNKNOWN_CONTINUOUS_EVALUATOR )
    {
        if( ( object->type == FHT_CONTINUOUS_PIECEWISE ) ||
            ( object->type == FHT_CONTINUOUS_REFERENCE ) ||
            ( object->type == FHT_CONTINUOUS_AGGREGATE ) ||
            ( object->type == FHT_CONTINUOUS_PARAMETERS ) ||
            ( object->type == FHT_CONTINUOUS_VARIABLE ) )
        {
            doSwitch = 1;
        }
    }
    else if( oldObject->type == FHT_UNKNOWN_ENSEMBLE_EVALUATOR )
    {
        if( ( object->type == FHT_ENSEMBLE_PARAMETERS ) ||
            ( object->type == FHT_ENSEMBLE_VARIABLE ) )
        {
            doSwitch = 1;
        }
    }
    
    if( doSwitch )
    {
        objects[handle] = object;
        delete oldObject;
        
        return handle;
    }
    
    logError( "Handle collision. Cannot replace", object->name, oldObject->name );
    fprintf( stderr, "Handle collision. Cannot replace %s:%d with %s:%d\n", object->name, object->type, oldObject->name, oldObject->type );
    delete object;
    
    return FML_INVALID_HANDLE;
}


void FieldmlRegion::addError( const string string )
{
    errors.push_back( string );
}


const int FieldmlRegion::getLastError()
{
    return lastError;
}


const int FieldmlRegion::getTotal( FieldmlHandleType type )
{
    int count, i, total;
    FieldmlObject *object;

    total = 0;
    count = objects.size();
    for( i = 0; i < count; i++ )
    {
        object = objects[i];
        if( object->type == type )
        {
            total++;
        }
    }

    return total;
}


const int FieldmlRegion::getTotal()
{
    return objects.size();
}


const int FieldmlRegion::getNthHandle( const FieldmlHandleType type, const int index )
{
    int count, i;
    FieldmlObject *object;

    if( index <= 0 )
    {
        return FML_INVALID_HANDLE;
    }

    count = objects.size();
    int counter = index;
    for( i = 0; i < count; i++ )
    {
        object = objects[i];
        if( object->type != type )
        {
            continue;
        }
        
        counter--;
        if( counter == 0 )
        {
            return i;
        }
    }

    return FML_INVALID_HANDLE;
}


const int FieldmlRegion::getNamedHandle( const string name )
{
    int count, i;
    FieldmlObject *object;

    setRegionError( FML_ERR_NO_ERROR );  

    count = objects.size();
    for( i = 0; i < count; i++ )
    {
        object = objects[i];
        if( object->name == name )
        {
            return i;
        }
    }

    return FML_INVALID_HANDLE;
}


const int FieldmlRegion::getObjectByIndex( const int index )
{
    return index - 1;
}


const int FieldmlRegion::getErrorCount()
{
    return errors.size();
}


const string FieldmlRegion::getError( const int index )
{
    if( ( index < 0 ) || ( index >= errors.size() ) )
    {
        return NULL;
    }
    
    return errors[index];
}


void FieldmlRegion::setDebug( const int debugValue )
{
    debug = debugValue;
}


void FieldmlRegion::logError( const string error, const string name1, const string name2 )
{
    string errorString = error;

    if( name1.length() > 0 )
    {
        errorString = errorString + ": " + name1;
    }
    if( name2.length() > 0 )
    {
        errorString = error + ": " + name2;
    }
    
    fprintf( stderr, "%s\n", errorString.c_str() );
    
    addError( errorString );
}
