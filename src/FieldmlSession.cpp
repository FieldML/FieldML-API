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
#include "String_InternalLibrary.h"

using namespace std;

static vector<FieldmlSession *> sessions;

FieldmlSession *FieldmlSession::handleToSession( FmlSessionHandle handle )
{
    if( ( handle < 0 ) || ( handle >= sessions.size() ) )
    {
        return NULL;
    }
    
    return sessions.at( handle );
}


FmlSessionHandle FieldmlSession::addSession( FieldmlSession *session )
{
    sessions.push_back( session );
    return sessions.size() - 1;
}


FieldmlSession::FieldmlSession() :
    objects( new ObjectStore() )
{
    handle = addSession( this );
    
    region = NULL;
}


FieldmlSession::~FieldmlSession()
{
    for_each( regions.begin(), regions.end(), delete_object() );
    
    delete objects;
    
    sessions[handle] = NULL;
}


FieldmlRegion *FieldmlSession::getRegion( string href, string name )
{
    for( vector<FieldmlRegion*>::iterator i = regions.begin(); i != regions.end(); i++ )
    {
        FieldmlRegion *r = *i;
        if( ( r->getHref() == href ) && ( r->getName() == name ) )
        {
            return r;
        }
    }
    
    return NULL;
}


FieldmlRegion *FieldmlSession::addNewRegion( string href, string name )
{
    FieldmlRegion *region = new FieldmlRegion( href, name, "", objects );
    regions.push_back( region );
    
    return region;
}


int FieldmlSession::getRegionIndex( string href, string name )
{
    for( int i = 0; i < regions.size(); i++ )
    {
        FieldmlRegion *r = regions[i];
        if( ( r->getHref() == href ) && ( r->getName() == name ) )
        {
            return i;
        }
    }
    
    return -1;
}


FieldmlRegion *FieldmlSession::getRegion( int index )
{
    if( ( index < 0 ) || ( index >= regions.size() ) )
    {
        return NULL;
    }
    
    return regions[index];
}


FieldmlRegion *FieldmlSession::addResourceRegion( string href, string name )
{
    if( href.length() == 0 )
    {
        return NULL;
    }

    FieldmlRegion *resourceRegion = new FieldmlRegion( href, name, "", objects );
    FieldmlRegion *currentRegion = region;
    region = resourceRegion;
    
    int result = 0;
    if( href == FML_INTERNAL_LIBRARY_NAME )
    {
        result = parseFieldmlString( FML_STRING_INTERNAL_LIBRARY, "Internal library", this );
    }
    else
    {
        string libraryFile = makeFilename( region->getRoot(), href );
        result = parseFieldmlFile( libraryFile.c_str(), this );
    }
    
    region = currentRegion;

    if( ( result != 0 ) || ( getErrorCount() != 0 ) )
    {
        delete resourceRegion;
        resourceRegion = NULL;
    }
    else
    {
        regions.push_back( resourceRegion );
    }
    
    return resourceRegion;
}


FmlErrorNumber FieldmlSession::setErrorAndLocation( const char *file, const int line, const FmlErrorNumber error )
{
    lastError = error;

    if( ( error != FML_ERR_NO_ERROR ) && ( error != FML_ERR_IO_NO_DATA ) )
    {
        if( debug )
        {
            printf("FIELDML %s (%s): Error %d at %s:%d\n", FML_VERSION_STRING, __DATE__, error, file, line );
        }
    }
    
    return error;
}


void FieldmlSession::addError( const string string )
{
    errors.push_back( string );
}


const FmlErrorNumber FieldmlSession::getLastError()
{
    return lastError;
}


const int FieldmlSession::getErrorCount()
{
    return errors.size();
}


const string FieldmlSession::getError( const int index )
{
    if( ( index < 0 ) || ( index >= errors.size() ) )
    {
        return NULL;
    }
    
    return errors[index];
}


void FieldmlSession::setDebug( const int debugValue )
{
    debug = debugValue;
}


FmlSessionHandle FieldmlSession::getHandle()
{
    return handle;
}


void FieldmlSession::logError( const char *error, const char *name1, const char *name2 )
{
    string errorString = error;

    if( name1 != NULL )
    {
        errorString = errorString + ": " + name1;
    }
    if( name2 != NULL )
    {
        errorString = errorString + ":: " + name2;
    }
    
    fprintf( stderr, "%s\n", errorString.c_str() );
    
    addError( errorString );
}


void FieldmlSession::logError( const char *error, FmlObjectHandle objectHandle )
{
    FieldmlObject *object = getObject( objectHandle );
    if( object == NULL )
    {
        logError( error, "UNKNOWN OBJECT" );
    }
    else
    {
        logError( error, object->name.c_str() );
    }
}


FieldmlObject *FieldmlSession::getObject( const FmlObjectHandle handle )
{
    return objects->getObject( handle );
}


DataReader *FieldmlSession::handleToReader( FmlReaderHandle handle )
{
    if( ( handle < 0 ) || ( handle >= readers.size() ) )
    {
        return NULL;
    }
    
    return readers.at( handle );
}


FmlReaderHandle FieldmlSession::addReader( DataReader *reader )
{
    readers.push_back( reader );
    return readers.size() - 1;
}


void FieldmlSession::removeReader( FmlReaderHandle handle )
{
    readers[handle] = NULL;
}


DataWriter *FieldmlSession::handleToWriter( FmlWriterHandle handle )
{
    if( ( handle < 0 ) || ( handle >= writers.size() ) )
    {
        return NULL;
    }
    
    return writers.at( handle );
}


FmlWriterHandle FieldmlSession::addWriter( DataWriter *writer )
{
    writers.push_back( writer );
    return writers.size() - 1;
}


void FieldmlSession::removeWriter( FmlWriterHandle handle )
{
    writers[handle] = NULL;
}
