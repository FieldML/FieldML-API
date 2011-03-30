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

FieldmlRegion::FieldmlRegion( const string _location, const string _name, const string _root, ObjectStore *_store ) :
    location( _location ),
    name( _name ),
    store( _store )
{
    root = _root;
}


FieldmlRegion::~FieldmlRegion()
{
    for_each( imports.begin(), imports.end(), delete_object() );
}


void FieldmlRegion::finalize()
{
}


void FieldmlRegion::setRoot( const string newRoot )
{
    root = newRoot;
}


const string FieldmlRegion::getRoot()
{
    return root;
}


const string FieldmlRegion::getLocation()
{
    return location;
}


const string FieldmlRegion::getName()
{
    return name;
}


void FieldmlRegion::addLocalObject( int handle )
{
    localObjects.push_back( handle );
}


const int FieldmlRegion::getTotal( FieldmlHandleType type )
{
    int total = 0;

    for( vector<int>::iterator i = localObjects.begin(); i != localObjects.end(); i++ )
    {
        FieldmlObject *object = store->getObject( *i );
        if( object->type == type )
        {
            total++;
        }
    }

    return total;
}


const bool FieldmlRegion::hasLocalObject( int handle )
{
    for( vector<int>::iterator i = localObjects.begin(); i != localObjects.end(); i++ )
    {
        if( *i == handle )
        {
            return true;
        }
    }
    
    return false;
}


const int FieldmlRegion::getTotal()
{
    return localObjects.size();
}


const int FieldmlRegion::getNthHandle( const FieldmlHandleType type, const int index )
{
    if( index <= 0 )
    {
        return FML_INVALID_HANDLE;
    }

    int counter = index;

    for( vector<int>::iterator i = localObjects.begin(); i != localObjects.end(); i++ )
    {
        FieldmlObject *object = store->getObject( *i );
        if( object->type != type )
        {
            continue;
        }
        
        counter--;
        if( counter == 0 )
        {
            return *i;
        }
    }

    return FML_INVALID_HANDLE;
}


const int FieldmlRegion::getNamedHandle( const string name )
{
    for( vector<int>::iterator i = localObjects.begin(); i != localObjects.end(); i++ )
    {
        FieldmlObject *object = store->getObject( *i );
        if( object->name == name )
        {
            return *i;
        }
    }
    
    for( vector<ImportInfo*>::iterator i = imports.begin(); i != imports.end(); i++ )
    {
        ImportInfo *info = *i;
        
        if( info == NULL )
        {
            continue;
        }
        
        FmlObjectHandle object = info->importNames.get( name, true );
        if( object != FML_INVALID_HANDLE )
        {
            return object;
        }
    }
    
    return FML_INVALID_HANDLE;
}


const int FieldmlRegion::getObjectByIndex( const int index )
{
    //TODO Deliberate bomb. Implement this properly.
    int *foo = NULL;
    printf("FIX ME!\n");
    
    return index -1;
}


void FieldmlRegion::addImportSource( int importIndex, string location, string name )
{
    while( imports.size() <= importIndex )
    {
        imports.push_back( NULL );
    }
    
    if( imports[importIndex] == NULL )
    {
        imports[importIndex] = new ImportInfo( location, name );
    }
}


void FieldmlRegion::addImport( int importIndex, string localName, int handle )
{
    if( ( importIndex < 0 ) || ( importIndex >= imports.size() ) )
    {
        return;
    }
    
    ImportInfo *import = imports[importIndex];
    if( import == NULL )
    {
        //TODO Set an error code
        return;
    }
    
    import->importNames.set( localName, handle );
}
