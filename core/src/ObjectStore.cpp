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

#include "Util.h"

#include "ObjectStore.h"

using namespace std;

ObjectStore::ObjectStore()
{
}

ObjectStore::~ObjectStore()
{
    for_each( objects.begin(), objects.end(), FmlUtil::delete_object() );
}

FieldmlObject *ObjectStore::getObject( FmlObjectHandle handle )
{
    if( ( handle < 0 ) || ( handle >= objects.size() ) )
    {
        return NULL;
    }
    
    return objects[handle];
}


FmlObjectHandle ObjectStore::addObject( FieldmlObject *object )
{
    //TODO Uniqueness check
    objects.push_back( object );
    return objects.size() - 1;
}


int ObjectStore::getCount()
{
    return objects.size();
}


int ObjectStore::getCount( FieldmlHandleType type )
{
    int count = 0;
    
    for( vector<FieldmlObject*>::iterator i = objects.begin(); i != objects.end(); i++ )
    {
        FieldmlObject *object = *i;
        if( object->objectType == type )
        {
            count++;
        }
    }
    
    return count;
}


FmlObjectHandle ObjectStore::getObjectByIndex( int index )
{
    if( ( index <= 0 ) || ( index > objects.size() ) )
    {
        return FML_INVALID_HANDLE;
    }
    
    return index - 1;
}


FmlObjectHandle ObjectStore::getObjectByIndex( int index, FieldmlHandleType type )
{
    if( index <= 0 )
    {
        return FML_INVALID_HANDLE;
    }
    
    int count = 0;
    
    for( int i = 0; i < objects.size(); i++ )
    {
        FieldmlObject *object = objects[i];
        if( object->objectType == type )
        {
            count++;
            if( count == index )
            {
                return i;
            }
        }
    }
    
    return FML_INVALID_HANDLE;
}


FmlObjectHandle ObjectStore::getObjectByName( const string name )
{
    for( int i = 0; i < objects.size(); i++ )
    {
        FieldmlObject *object = objects[i];
        if( object->name == name )
        {
            return i;
        }
    }
    
    return FML_INVALID_HANDLE;
}
