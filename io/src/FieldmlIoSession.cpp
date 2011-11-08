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

#include "StringUtil.h"
#include "FieldmlIoSession.h"

using namespace std;

FieldmlIoSession FieldmlIoSession::singleton;

class FieldmlIoSessionContext :
    public FieldmlIoContext
{
private:
    const FmlSessionHandle session;
    
public:
    FieldmlIoSessionContext( FmlSessionHandle _session ) :
        session( _session )
    {
    }
    
    
    FmlSessionHandle getSession()
    {
        return session;
    }
    
    
    FmlIoErrorNumber setError( FmlIoErrorNumber error )
    {
        return FieldmlIoSession::getSession().setError( error );
    }
    
    
    FmlIoErrorNumber getLastError()
    {
        return FieldmlIoSession::getSession().getLastError();
    }
    
    
    void setErrorContext( const char *file, const int line )
    {
        FieldmlIoSession::getSession().setErrorContext( file, line );
    }
};


FieldmlIoSession &FieldmlIoSession::getSession()
{
    return singleton;
}


FieldmlIoSession::FieldmlIoSession()
{
    debug = 1;
    lastError = FML_IOERR_NO_ERROR;
}


FieldmlIoSession::~FieldmlIoSession()
{
    for( vector<ArrayDataReader*>::iterator i = readers.begin(); i != readers.end(); i++ )
    {
        delete *i;
    }
    for( vector<ArrayDataWriter*>::iterator i = writers.begin(); i != writers.end(); i++ )
    {
        delete *i;
    }
}


void FieldmlIoSession::setErrorContext( const char *file, const int line )
{
    contextFile = file;
    contextLine = line;
}


FmlIoErrorNumber FieldmlIoSession::setError( const FmlIoErrorNumber error )
{
    lastError = error;

    if( error != FML_IOERR_NO_ERROR )
    {
        if( debug )
        {
            if( contextFile == NULL )
            {
                fprintf( stderr, "FIELDML IO %s (%s): Error %d\n", StringUtil::FMLIO_VERSION_STRING.c_str(), __DATE__, error );
            }
            else
            {
                fprintf( stderr, "FIELDML IO %s (%s): Error %d at %s:%d\n", StringUtil::FMLIO_VERSION_STRING.c_str(), __DATE__, error, contextFile, contextLine );
            }
        }
    }
    
    return error;
}


const FmlIoErrorNumber FieldmlIoSession::getLastError()
{
    return lastError;
}


void FieldmlIoSession::setDebug( const int debugValue )
{
    debug = debugValue;
}


FieldmlIoContext *FieldmlIoSession::createContext( FmlSessionHandle session )
{
    return new FieldmlIoSessionContext( session );
}


ArrayDataReader *FieldmlIoSession::handleToReader( FmlReaderHandle handle )
{
    if( ( handle < 0 ) || ( handle >= readers.size() ) )
    {
        return NULL;
    }
    
    return readers.at( handle );
}


FmlReaderHandle FieldmlIoSession::addReader( ArrayDataReader *reader )
{
    readers.push_back( reader );
    return readers.size() - 1;
}


void FieldmlIoSession::removeReader( FmlReaderHandle handle )
{
    readers[handle] = NULL;
}


ArrayDataWriter *FieldmlIoSession::handleToWriter( FmlWriterHandle handle )
{
    if( ( handle < 0 ) || ( handle >= writers.size() ) )
    {
        return NULL;
    }
    
    return writers[handle];
}


FmlWriterHandle FieldmlIoSession::addWriter( ArrayDataWriter *writer )
{
    writers.push_back( writer );
    return writers.size() - 1;
}


void FieldmlIoSession::removeWriter( FmlWriterHandle handle )
{
    writers[handle] = NULL;
}
