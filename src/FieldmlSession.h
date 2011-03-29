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

#ifndef H_FIELDML_SESSION
#define H_FIELDML_SESSION

#include <vector>

#include "FieldmlErrorHandler.h"
#include "FieldmlRegion.h"

class FieldmlSession :
    public FieldmlErrorHandler
{
private:
    int lastError;
    
    int debug;
    
    std::vector<std::string> errors;
    
    int handle;
    
    static FmlHandle addSession( FieldmlSession *session );
    
public:
    FieldmlSession();
    virtual ~FieldmlSession();
    
    int setErrorAndLocation( const char *file, const int line, const int error );

    void addError( const std::string string );

    void setDebug( const int debugValue );
    
    const int getErrorCount();
    
    const std::string getError( const int index );

    const int getLastError();

    void logError( const char *error, const char *name1 = NULL, const char *name2 = NULL );
    
    FmlHandle getHandle();
    
    FieldmlRegion *region;


    static FieldmlSession *handleToSession( FmlHandle handle );
};

#endif //H_FIELDML_SESSION