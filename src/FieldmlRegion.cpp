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


FieldmlRegion::FieldmlRegion( const string _location, const string _name, const string _library ) :
    name( _name ),
    library( _library )
{
    root = _location;
}


FieldmlRegion::~FieldmlRegion()
{
    std::for_each( objects.begin(), objects.end(), delete_object() );
}


void FieldmlRegion::finalize()
{
}


FieldmlObject *FieldmlRegion::getObject( const FmlObjectHandle handle )
{
    if( ( handle < 0 ) || ( handle >= objects.size() ) )
    {
        return NULL;
    }
    
    return objects[handle];
}


void FieldmlRegion::setRoot( const string newRoot )
{
    root = newRoot;
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
    objects.push_back( object );
    return objects.size() - 1;
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


void FieldmlRegion::setLocationHandle( FmlObjectHandle handle, int locationHandle )
{
    FieldmlObject *object = getObject( handle );
    object->locationHandle = locationHandle;
}
