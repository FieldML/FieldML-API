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
#include <deque>
#include <set>
#include <utility>

#include "FieldmlErrorHandler.h"
#include "FieldmlRegion.h"

class FieldmlSession :
    public FieldmlErrorHandler
{
private:
    FmlErrorNumber lastError;
    
    std::string lastDescription;
    
    std::deque<std::pair<const std::string, const int>> contextStack;
    
    int debug;
    
    std::vector<std::string> errors;
    
    std::vector<FieldmlRegion*> regions;
    
    std::vector<std::string> importHrefStack;
    
    FmlSessionHandle handle;
    
    bool getDelegateEvaluators(  const std::set<FmlObjectHandle> &evaluators, std::vector<FmlObjectHandle> &stack, std::set<FmlObjectHandle> &set );
    
    bool getDelegateEvaluators( FmlObjectHandle handle, std::vector<FmlObjectHandle> &stack, std::set<FmlObjectHandle> &set );

    void mergeArguments( const SimpleMap<FmlObjectHandle, FmlObjectHandle> &binds, std::set<FmlObjectHandle> &delegateUnbound, std::set<FmlObjectHandle> &delegateUsed, std::set<FmlObjectHandle> &unbound, std::set<FmlObjectHandle> &used );
    
    void getArguments( const std::set<FmlObjectHandle> &handles, std::set<FmlObjectHandle> &unbound, std::set<FmlObjectHandle> &used );

    static FmlSessionHandle addSession( FieldmlSession *session );

    void addError( const std::string string );
    
    virtual ~FieldmlSession();

public:
    FieldmlSession();
    
    void pushErrorContext( const char *file, const int line, const char *function );

    void popErrorContext();
    
    FmlErrorNumber setError( const FmlErrorNumber error, const std::string errorDescription );

    FmlErrorNumber setError( const FmlErrorNumber error, const FmlObjectHandle handle, const std::string description );

    void logError( const std::string string );

    void setDebug( const int debugValue );
    
    const int getErrorCount();
    
    const std::string getError( const int index );
    
    void clearErrors();

    const FmlErrorNumber getLastError();

    void logError( const char *error, const char *name1 = NULL, const char *name2 = NULL );
    
    void logError( const char *error, const FmlObjectHandle object );
    
    FmlSessionHandle getSessionHandle();
    
    FieldmlObject *getObject( const FmlObjectHandle handle );
    
    FieldmlRegion *addResourceRegion( std::string location, std::string name );
    
    FieldmlRegion *addNewRegion( std::string location, std::string name );
    
    FieldmlRegion *getRegion( std::string location, std::string name );
    
    int getRegionIndex( std::string location, std::string name );
    
    FieldmlRegion *getRegion( int index );
    
    FieldmlRegion *region;

    ObjectStore objects;

    bool getDelegateEvaluators( FmlObjectHandle handle, std::set<FmlObjectHandle> &set );
    
    void getArguments( FmlObjectHandle handle, std::set<FmlObjectHandle> &unbound, std::set<FmlObjectHandle> &used, bool addSelf );

    static FieldmlSession *handleToSession( FmlSessionHandle handle );
    
    static void removeSession( FmlSessionHandle handle );
};

#endif //H_FIELDML_SESSION
