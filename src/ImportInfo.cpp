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
#include "fieldml_api.h"
#include "ImportInfo.h"

using namespace std;

class ObjectImport
{
public:
    const string localName;
    
    const string remoteName;
    
    const FmlObjectHandle handle;
    
    ObjectImport( string _localName, string _remoteName, FmlObjectHandle _handle );
    
    virtual ~ObjectImport();
};


ObjectImport::ObjectImport( string _localName, string _remoteName, FmlObjectHandle _handle ) :
    localName( _localName ),
    remoteName( _remoteName ),
    handle( _handle )
{
}


ObjectImport::~ObjectImport()
{
}


ImportInfo::ImportInfo( string _location, string _name ) :
    location( _location ),
    name( _name )
{
}


ImportInfo::~ImportInfo()
{
    for_each( imports.begin(), imports.end(), delete_object() );
}


FmlObjectHandle ImportInfo::getObject( string localName )
{
    for( vector<ObjectImport*>::iterator i = imports.begin(); i != imports.end(); i++ )
    {
        ObjectImport *import = *i;
        if( import->localName == localName )
        {
            return import->handle;
        }
    }
    
    return FML_INVALID_HANDLE;
}


const string ImportInfo::getLocalName( FmlObjectHandle handle )
{
    for( vector<ObjectImport*>::iterator i = imports.begin(); i != imports.end(); i++ )
    {
        ObjectImport *import = *i;
        if( import->handle == handle )
        {
            return import->localName;
        }
    }
    
    return "";
}


void ImportInfo::addImport( string localName, string remoteName, FmlObjectHandle handle )
{
    if( ( localName == "" ) || ( remoteName == "" ) || ( handle == FML_INVALID_HANDLE ) )
    {
        return;
    }
    
    imports.push_back( new ObjectImport( localName, remoteName, handle ) );
}


int ImportInfo::getImportCount()
{
    return imports.size();
}


const string ImportInfo::getLocalNameByIndex( int index )
{
    if( ( index <= 0 ) || ( index > imports.size() ) )
    {
        return "";
    }
    
    return imports[index-1]->localName;
}


const string ImportInfo::getRemoteNameByIndex( int index )
{
    if( ( index <= 0 ) || ( index > imports.size() ) )
    {
        return "";
    }
    
    return imports[index-1]->remoteName;
}


int ImportInfo::getObjectByIndex( int index )
{
    if( ( index <= 0 ) || ( index > imports.size() ) )
    {
        return FML_INVALID_HANDLE;
    }
    
    return imports[index-1]->handle;
}
