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
#include "fieldml_library_0.3.h"

using namespace std;

//========================================================================
//
// Consts
//
//========================================================================

const int INVALID_LOCATION_HANDLE = -2;
const int LIBRARY_LOCATION_HANDLE = 0;
const int LOCAL_LOCATION_HANDLE = 1;


static vector<FieldmlRegion *> regions;

FieldmlRegion *FieldmlRegion::handleToRegion( FmlHandle handle )
{
    if( ( handle < 0 ) || ( handle >= regions.size() ) )
    {
        return NULL;
    }
    
    return regions.at( handle );
}


long FieldmlRegion::addRegion( FieldmlRegion *region )
{
    regions.push_back( region );
    return regions.size() - 1;
}


static FieldmlObject *resolveSubEvaluator( FieldmlRegion *region, string name, int locationHandle )
{
    int index;
    
    index = name.rfind( '.' );
    
    if( index == string::npos )
    {
        return NULL;
    }
    
    string superName = name.substr( 0, index );
    FmlObjectHandle handle = region->getNamedHandle( superName.c_str() );
    if( handle == FML_INVALID_HANDLE )
    {
        return NULL;
    }
    
    FieldmlHandleType superHandleType = Fieldml_GetObjectType( region->getRegionHandle(), handle ); 
    if( ( superHandleType != FHT_PIECEWISE_EVALUATOR ) &&
        ( superHandleType != FHT_REFERENCE_EVALUATOR ) &&
        ( superHandleType != FHT_AGGREGATE_EVALUATOR ) &&
        ( superHandleType != FHT_PARAMETER_EVALUATOR ) &&
        ( superHandleType != FHT_EXTERNAL_EVALUATOR ) &&
        ( superHandleType != FHT_ABSTRACT_EVALUATOR ) )
    {
        return NULL;
    }
    
    FmlObjectHandle superTypeHandle = Fieldml_GetValueType( region->getRegionHandle(), handle );
    if( superTypeHandle == FML_INVALID_HANDLE )
    {
        return NULL;
    }
    
    FieldmlHandleType superType = Fieldml_GetObjectType( region->getRegionHandle(), superTypeHandle );
    if( superType != FHT_MESH_TYPE )
    {
        //Don't currently support any other kind of structured type
        return NULL;
    }
    
    string subName = name.substr( index + 1 );
    FmlObjectHandle typeHandle = FML_INVALID_HANDLE;
    
    if( subName == "element" )
    {
        typeHandle = Fieldml_GetMeshElementType( region->getRegionHandle(), superTypeHandle );

    }
    else if( subName == "xi" )
    {
        typeHandle = Fieldml_GetMeshXiType( region->getRegionHandle(), superTypeHandle );
    }
    else
    {
        return NULL;
    }

    return new Evaluator( name, LOCAL_LOCATION_HANDLE, superHandleType, typeHandle, true );
}


FieldmlRegion::FieldmlRegion( const string _location, const string _name, const string _library ) :
    name( _name ),
    library( _library )
{
    root = _location;

    handle = addRegion( this );

    if( library.length() != 0 )
    {
        if( library == "library_0.3.xml" )
        {
            parseFieldmlString( FML_LIBRARY_0_3_STRING, "Internal library 0.3", LIBRARY_LOCATION_HANDLE, this );
        }
        else
        {
            string libraryFile = makeFilename( root, library );
            parseFieldmlFile( libraryFile.c_str(), LIBRARY_LOCATION_HANDLE, this );
        }
    }
}


FieldmlRegion::~FieldmlRegion()
{
    std::for_each( objects.begin(), objects.end(), delete_object() );
    
    regions[handle] = NULL;
}


void FieldmlRegion::finalize()
{
    FieldmlObject *object, *newObject;
    int i, count;
    
    count = objects.size();
    
    for( i = 0; i < count; i++ )
    {
        object = objects[i];
        
        if( object->type == FHT_UNKNOWN_TYPE )
        {
            logError( "Unknown type: ", object->name.c_str() );
        }
        else if( object->type == FHT_UNKNOWN_ELEMENT_SEQUENCE )
        {
            logError( "Unknown element set: ", object->name.c_str() );
        }
        else if( object->type == FHT_UNKNOWN_EVALUATOR )
        {
            newObject = resolveSubEvaluator( this, object->name, object->locationHandle );
            if( newObject != NULL )
            {
                objects[i] = newObject;
                delete object;
            }
            else
            {
                logError( "Unknown evaluator: ", object->name.c_str() );
            }
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

    if( ( error != FML_ERR_NO_ERROR ) && ( error != FML_ERR_IO_NO_DATA ) )
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


const string FieldmlRegion::getLibraryName()
{
    return library;
}


FmlObjectHandle FieldmlRegion::addObject( FieldmlObject *object )
{
    int doSwitch;
    FieldmlObject *oldObject;
    FmlObjectHandle handle = getNamedHandle( object->name.c_str() );
    
    if( handle == FML_INVALID_HANDLE )
    {
        objects.push_back( object );
        return objects.size() - 1;
    }
    
    oldObject = getObject( handle );
    
    logError( "Handle collision. Cannot replace", object->name.c_str(), oldObject->name.c_str() );
    fprintf( stderr, "Handle collision. Cannot replace %s:%d with %s:%d\n", object->name.c_str(), object->type, oldObject->name.c_str(), oldObject->type );
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
    int count, i, handle;
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
    
    object = resolveSubEvaluator( this, name, 0 );
    if( object == NULL )
    {
        return FML_INVALID_HANDLE;
    }

    objects.push_back( object );
    handle = objects.size() - 1;
    
    return handle;
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


void FieldmlRegion::logError( const char *error, const char *name1, const char *name2 )
{
    string errorString = error;

    if( name1 != NULL )
    {
        errorString = errorString + ": " + name1;
    }
    if( name2 != NULL )
    {
        errorString = errorString + ":: " + name2;
    }
    
    fprintf( stderr, "%s\n", errorString.c_str() );
    
    addError( errorString );
}


void FieldmlRegion::setLocationHandle( FmlObjectHandle handle, int locationHandle )
{
    FieldmlObject *object = getObject( handle );
    object->locationHandle = locationHandle;
}


FmlHandle FieldmlRegion::getRegionHandle() const
{
    return handle;
}
