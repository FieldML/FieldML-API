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
#include "Util.h"
#include "fieldml_structs.h"
#include "FieldmlDOM.h"
#include "FieldmlSession.h"
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
    lastError = FML_ERR_NO_ERROR;
    
    region = NULL;
}


FieldmlSession::~FieldmlSession()
{
    for_each( regions.begin(), regions.end(), FmlUtil::delete_object() );
    
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
    
    //NOTE: This will be insufficient when the region name starts being used.
    if( FmlUtil::contains( importHrefStack, href ) )
    {
        addError( "Recursive import involving " + href );
        return NULL;
    }
    
    importHrefStack.push_back( href );

    FieldmlRegion *resourceRegion = new FieldmlRegion( href, name, "", objects );
    FieldmlRegion *currentRegion = region;
    region = resourceRegion;
    
    int result = 0;
    if( href == FML_INTERNAL_LIBRARY_NAME )
    {
        result = FieldmlDOM::parseFieldmlString( FML_STRING_INTERNAL_LIBRARY, "Internal library", FML_INTERNAL_LIBRARY_NAME, this, getSessionHandle() );
    }
    else
    {
        string filename = makeFilename( region->getRoot(), href );
        result = FieldmlDOM::parseFieldmlFile( filename.c_str(), this, getSessionHandle() );
    }
    
    importHrefStack.pop_back();
    
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


void FieldmlSession::setErrorContext( const char *file, const int line )
{
    contextFile = file;
    contextLine = line;
}


FmlErrorNumber FieldmlSession::setError( const FmlErrorNumber error )
{
    lastError = error;

    if( ( error != FML_ERR_NO_ERROR ) && ( error != FML_ERR_IO_NO_DATA ) )
    {
        if( debug )
        {
            fprintf( stderr, "FIELDML %s (%s): Error %d at %s:%d\n", FML_VERSION_STRING, __DATE__, error, contextFile, contextLine );
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


void FieldmlSession::clearErrors()
{
    errors.clear();
}


void FieldmlSession::setDebug( const int debugValue )
{
    debug = debugValue;
}


FmlSessionHandle FieldmlSession::getSessionHandle()
{
    return handle;
}


void FieldmlSession::logError( const string error )
{
    addError( error );
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


ArrayDataReader *FieldmlSession::handleToReader( FmlReaderHandle handle )
{
    if( ( handle < 0 ) || ( handle >= readers.size() ) )
    {
        return NULL;
    }
    
    return readers.at( handle );
}


FmlReaderHandle FieldmlSession::addReader( ArrayDataReader *reader )
{
    readers.push_back( reader );
    return readers.size() - 1;
}


void FieldmlSession::removeReader( FmlReaderHandle handle )
{
    readers[handle] = NULL;
}


ArrayDataWriter *FieldmlSession::handleToWriter( FmlWriterHandle handle )
{
    if( ( handle < 0 ) || ( handle >= writers.size() ) )
    {
        return NULL;
    }
    
    return writers.at( handle );
}


FmlWriterHandle FieldmlSession::addWriter( ArrayDataWriter *writer )
{
    writers.push_back( writer );
    return writers.size() - 1;
}


void FieldmlSession::removeWriter( FmlWriterHandle handle )
{
    writers[handle] = NULL;
}


bool FieldmlSession::getDelegateEvaluators( const set<FmlObjectHandle> &evaluators, vector<FmlObjectHandle> &stack, set<FmlObjectHandle> &delegates )
{
    for( set<FmlObjectHandle>::const_iterator i = evaluators.begin(); i != evaluators.end(); i++ )
    {
        if( !getDelegateEvaluators( *i, stack, delegates ) )
        {
            return false;
        }
        delegates.insert( *i );
    }
    return true;
}

    
bool FieldmlSession::getDelegateEvaluators( FmlObjectHandle handle, vector<FmlObjectHandle> &stack, set<FmlObjectHandle> &delegates )
{
    FieldmlObject *object = getObject( handle );
    set<FmlObjectHandle> evaluators;
    
    if( handle == FML_INVALID_HANDLE )
    {
        //Convenience so that callers don't have to check
        return true;
    }
    
    if( FmlUtil::contains( stack, handle ) )
    {
        //Recursive dependency!
        return false;
    }
    
    stack.push_back( handle );
    if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *evaluator = (ReferenceEvaluator*)object;
        evaluator->addDelegates( evaluators );
        if( !getDelegateEvaluators( evaluators, stack, delegates ) )
        {
            return false;
        }
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *evaluator = (AggregateEvaluator*)object;
        evaluator->addDelegates( evaluators );
        if( !getDelegateEvaluators( evaluators, stack, delegates ) )
        {
            return false;
        }
    }
    else if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *evaluator = (PiecewiseEvaluator*)object;
        evaluator->addDelegates( evaluators );
        if( !getDelegateEvaluators( evaluators, stack, delegates ) )
        {
            return false;
        }
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *evaluator = (ParameterEvaluator*)object;
        evaluator->addDelegates( evaluators );
        if( !getDelegateEvaluators( evaluators, stack, delegates ) )
        {
            return false;
        }
    }
    
    return true;
}


bool FieldmlSession::getDelegateEvaluators( FmlObjectHandle handle, set<FmlObjectHandle> &delegates )
{
    vector<FmlObjectHandle> stack;
    
    return getDelegateEvaluators( handle, stack, delegates );
}


void FieldmlSession::mergeArguments( const SimpleMap<FmlObjectHandle, FmlObjectHandle> &binds, set<FmlObjectHandle> &delegateUnbound, set<FmlObjectHandle> &delegateUsed, set<FmlObjectHandle> &unbound, set<FmlObjectHandle> &used )
{
    set<FmlObjectHandle> tmpUnbound;
    for( SimpleMap<FmlObjectHandle, FmlObjectHandle>::ConstIterator i = binds.begin(); i != binds.end(); i++ )
    {
        if( FmlUtil::contains( delegateUsed, i->first ) )
        {
            getArguments( i->second, tmpUnbound, delegateUsed, true );

            ArgumentEvaluator *arg = (ArgumentEvaluator*)getObject( i->first );
            for( set<FmlObjectHandle>::const_iterator i = arg->arguments.begin(); i != arg->arguments.end(); i++ )
            {
                tmpUnbound.erase( *i );
            }
            for( set<FmlObjectHandle>::const_iterator i = tmpUnbound.begin(); i != tmpUnbound.end(); i++ )
            {
                delegateUnbound.insert( *i );
            }
        }
    }

    for( SimpleMap<FmlObjectHandle, FmlObjectHandle>::ConstIterator i = binds.begin(); i != binds.end(); i++ )
    {
        delegateUnbound.erase( i->first );
    }

    used.insert( delegateUsed.begin(), delegateUsed.end() );
    unbound.insert( delegateUnbound.begin(), delegateUnbound.end() );

}


void FieldmlSession::getArguments( const set<FmlObjectHandle> &handles, set<FmlObjectHandle> &unbound, set<FmlObjectHandle> &used )
{
    for( set<FmlObjectHandle>::const_iterator i = handles.begin(); i != handles.end(); i++ )
    {
        getArguments( *i, unbound, used, true );
    }
}


void FieldmlSession::getArguments( FmlObjectHandle handle, set<FmlObjectHandle> &unbound, set<FmlObjectHandle> &used, bool addSelf )
{
    FieldmlObject *object = getObject( handle );
    set<FmlObjectHandle> tmpUnbound, tmpUsed;
    
    if( handle == FML_INVALID_HANDLE )
    {
        //Convenience so that callers don't have to check
        return;
    }
    
    if( object->type == FHT_ARGUMENT_EVALUATOR )
    {
        if( addSelf )
        {
            used.insert( handle );
            unbound.insert( handle );
        }
        ArgumentEvaluator *evaluator = (ArgumentEvaluator*)object;
        used.insert( evaluator->arguments.begin(), evaluator->arguments.end() );
        unbound.insert( evaluator->arguments.begin(), evaluator->arguments.end() );
    }
    else if( object->type == FHT_EXTERNAL_EVALUATOR )
    {
        ExternalEvaluator *evaluator = (ExternalEvaluator*)object;
        used.insert( evaluator->arguments.begin(), evaluator->arguments.end() );
        unbound.insert( evaluator->arguments.begin(), evaluator->arguments.end() );
    }
    else if( object->type == FHT_REFERENCE_EVALUATOR )
    {
        ReferenceEvaluator *evaluator = (ReferenceEvaluator*)object;
        getArguments( evaluator->sourceEvaluator, tmpUnbound, tmpUsed, true );
        mergeArguments( evaluator->binds, tmpUnbound, tmpUsed, unbound, used );
    }
    else if( object->type == FHT_AGGREGATE_EVALUATOR )
    {
        AggregateEvaluator *evaluator = (AggregateEvaluator*)object;
        getArguments( evaluator->evaluators.getValues(), tmpUnbound, tmpUsed );
        getArguments( evaluator->indexEvaluator, tmpUnbound, tmpUsed, true );
        mergeArguments( evaluator->binds, tmpUnbound, tmpUsed, unbound, used );
        unbound.erase( evaluator->indexEvaluator );
        used.insert( evaluator->indexEvaluator );
    }
    else if( object->type == FHT_PIECEWISE_EVALUATOR )
    {
        PiecewiseEvaluator *evaluator = (PiecewiseEvaluator*)object;
        getArguments( evaluator->evaluators.getValues(), tmpUnbound, tmpUsed );
        getArguments( evaluator->indexEvaluator, tmpUnbound, tmpUsed, true );
        mergeArguments( evaluator->binds, tmpUnbound, tmpUsed, unbound, used );
    }
    else if( object->type == FHT_PARAMETER_EVALUATOR )
    {
        ParameterEvaluator *evaluator = (ParameterEvaluator*)object;
        set<FmlObjectHandle> indexEvaluators;
        evaluator->addDelegates( indexEvaluators );
        
        getArguments( indexEvaluators, unbound, used );
    }
}
