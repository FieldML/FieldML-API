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
#include "string_const.h"
#include "fieldml_structs.h"
#include "FieldmlRegion.h"

using namespace std;

//========================================================================
//
// Consts
//
//========================================================================

FieldmlRegion::FieldmlRegion( const string _href, const string _name, const string _root, ObjectStore &_store ) :
    href( _href ),
    name( _name ),
    store( _store )
{
    root = _root;
}


FieldmlRegion::~FieldmlRegion()
{
    for_each( imports.begin(), imports.end(), FmlUtil::delete_object() );
}


ImportInfo *FieldmlRegion::getImportInfo( int importSourceIndex )
{
    if( ( importSourceIndex < 0 ) || ( importSourceIndex >= imports.size() ) )
    {
        return NULL;
    }
    
    return imports[importSourceIndex];
}


void FieldmlRegion::finalize()
{
}


void FieldmlRegion::setName( const string newName )
{
    name = newName;
}


void FieldmlRegion::setRoot( const string newRoot )
{
    root = newRoot;
}


const string FieldmlRegion::getRoot()
{
    return root;
}


const string FieldmlRegion::getHref()
{
    return href;
}


const string FieldmlRegion::getName()
{
    return name;
}


void FieldmlRegion::addLocalObject( FmlObjectHandle handle )
{
    localObjects.push_back( handle );
}


const bool FieldmlRegion::hasLocalObject( FmlObjectHandle handle, bool allowVirtual, bool allowImport )
{
    if( !allowVirtual )
    {
        FieldmlObject *object = store.getObject( handle );
        if( ( object != NULL ) && ( object->isVirtual ) )
        {
            return false;
        }
    }
    
    if( FmlUtil::contains( localObjects, handle ) )
    {
        return true;
    }
    
    if( allowImport )
    {
        for( vector<ImportInfo*>::iterator i = imports.begin(); i != imports.end(); i++ )
        {
            ImportInfo *info = *i;
            
            if( info == NULL )
            {
                continue;
            }
            
            if( info->hasObject( handle ) )
            {
                return true;
            }
        }
    }
    
    return false;
}


const FmlObjectHandle FieldmlRegion::getNamedObject( const string name )
{
    for( vector<FmlObjectHandle>::iterator i = localObjects.begin(); i != localObjects.end(); i++ )
    {
        FieldmlObject *object = store.getObject( *i );
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
        
        FmlObjectHandle object = info->getObject( name );
        if( object != FML_INVALID_HANDLE )
        {
            return object;
        }
    }
    
    return FML_INVALID_HANDLE;
}


const string FieldmlRegion::getObjectName( FmlObjectHandle handle )
{
    if( FmlUtil::contains( localObjects, handle ) )
    {
        FieldmlObject *object = store.getObject( handle );
        return object->name;
    }

    
    for( vector<ImportInfo*>::iterator i = imports.begin(); i != imports.end(); i++ )
    {
        ImportInfo *info = *i;
        
        if( info == NULL )
        {
            continue;
        }
        
        string name = info->getLocalName( handle );
        if( name != "" )
        {
            return name;
        }
    }
    
    return "";
}


void FieldmlRegion::addImportSource( int importSourceIndex, string href, string name )
{
    while( imports.size() <= importSourceIndex )
    {
        imports.push_back( NULL );
    }
    
    if( imports[importSourceIndex] == NULL )
    {
        imports[importSourceIndex] = new ImportInfo( href, name );
    }
}


void FieldmlRegion::addImport( int importSourceIndex, string localName, string remoteName, FmlObjectHandle handle )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        //TODO Set an error code
        return;
    }
    
    import->addImport( localName, remoteName, handle );
}


int FieldmlRegion::getImportSourceCount()
{
    return imports.size();
}


int FieldmlRegion::getImportCount( int importSourceIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return 0;
    }
    
    return import->getImportCount();
}


const string FieldmlRegion::getImportSourceHref( int importSourceIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return "";
    }

    return import->href;
}


const string FieldmlRegion::getImportSourceRegionName( int importSourceIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return "";
    }

    return import->name;
}


const string FieldmlRegion::getImportLocalName( int importSourceIndex, int importIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return "";
    }
    
    return import->getLocalNameByIndex( importIndex );
}


const string FieldmlRegion::getImportRemoteName( int importSourceIndex, int importIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return "";
    }
    
    return import->getRemoteNameByIndex( importIndex );
}


FmlObjectHandle FieldmlRegion::getImportObject( int importSourceIndex, int importIndex )
{
    ImportInfo *import = getImportInfo( importSourceIndex );
    if( import == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return import->getObjectByIndex( importIndex );
}
