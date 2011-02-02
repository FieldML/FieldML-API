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

#include "fieldml_structs.h"

extern const int INVALID_REGION_HANDLE;
extern const int FILE_REGION_HANDLE;
extern const int LIBRARY_REGION_HANDLE;
extern const int VIRTUAL_REGION_HANDLE;

class FieldmlRegion
{
    int lastError;
    
    int debug;
    
    const std::string name;
    
    std::string root;
    
    std::vector<std::string> errors;
    
    std::vector<FieldmlObject*> objects;
    
public:
    FieldmlRegion( const std::string location, const std::string name );
    virtual ~FieldmlRegion();
    
    FmlObjectHandle addObject( FieldmlObject *object );

    FieldmlObject *getObject( const FmlObjectHandle handle );
    
    const int getObjectByIndex( const int index );
    
    const int getTotal( FieldmlHandleType type );

    const int getTotal();

    const int getNthHandle( const FieldmlHandleType type, const int index );
    
    const int getNamedHandle( const std::string name );

    int setRegionErrorAndLocation( const char *file, const int line, const int error );

    void addError( const std::string string );

    void setDebug( const int debugValue );
    
    const int getErrorCount();
    
    const std::string getError( const int index );

    int setRoot( const std::string newRoot );

    const std::string getRoot();

    const int getLastError();
    
    const std::string getName();

    void finalize();

    void logError( const std::string error, const std::string name1 = "", const std::string name2 = "" );
};


#define setRegionError( error ) setRegionErrorAndLocation( __FILE__, __LINE__, error )

#endif //H_FIELDML_STRUCTS
