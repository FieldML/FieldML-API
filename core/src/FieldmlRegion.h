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

#ifndef H_FIELDML_REGION
#define H_FIELDML_REGION

#include <vector>

#include "ObjectStore.h"
#include "ImportInfo.h"
#include "fieldml_structs.h"

class FieldmlRegion
{
private:
    const std::string href;
    
    std::string name;
    
    std::string root;
    
    std::vector<FmlObjectHandle> localObjects;
    
    std::vector<ImportInfo*> imports;
    
    ObjectStore * const store;
    
    ImportInfo *getImportInfo( int importSourceIndex );
    
public:
    FieldmlRegion( const std::string href, const std::string name, const std::string root, ObjectStore *_store );

    virtual ~FieldmlRegion();
    
    void addLocalObject( FmlObjectHandle handle );

    const bool hasLocalObject( FmlObjectHandle handle, bool allowVirtual, bool allowImport );

    const FmlObjectHandle getNamedObject( const std::string name );
    
    const std::string getObjectName( FmlObjectHandle handle );
    
    void setName( const std::string newName );

    void setRoot( const std::string newRoot );

    const std::string getRoot();
    
    const std::string getHref();

    const std::string getName();
    
    const std::string getLibraryName();

    void finalize();

    void addImportSource( int importSourceIndex, std::string href, std::string name );

    void addImport( int importSourceIndex, std::string localName, std::string remoteName, FmlObjectHandle object );
    
    int getImportSourceCount();
    
    int getImportCount( int importSourceIndex );
    
    const std::string getImportSourceHref( int importSourceIndex );
    
    const std::string getImportSourceRegionName( int importSourceIndex );
    
    const std::string getImportLocalName( int importSourceIndex, int importIndex );
    
    const std::string getImportRemoteName( int importSourceIndex, int importIndex );
    
    FmlObjectHandle getImportObject( int importSourceIndex, int importIndex );
};

#endif //H_FIELDML_REGION
