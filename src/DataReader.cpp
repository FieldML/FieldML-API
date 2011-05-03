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

#include "fieldml_api.h"
#include "string_const.h"

#include "FieldmlErrorHandler.h"
#include "DataReader.h"

using namespace std;

static vector<DataReader *> readers;

DataReader *DataReader::handleToReader( FmlReaderHandle handle )
{
    if( ( handle < 0 ) || ( handle >= readers.size() ) )
    {
        return NULL;
    }
    
    return readers.at( handle );
}


FmlReaderHandle DataReader::addReader( DataReader *reader )
{
    readers.push_back( reader );
    return readers.size() - 1;
}


DataReader::DataReader( FieldmlInputStream *_stream, FieldmlErrorHandler *_eHandler, int _count, int _length, int _head, int _tail ) :
    eHandler( _eHandler ),
    stream( _stream ),
    entryCount( _count ),
    entryLength( _length ),
    head( _head ),
    tail( _tail )
{
    handle = addReader( this );
    dataCounter = 0;
    entryCounter = 0;
    
    skip( head );
}


FmlReaderHandle DataReader::create( FieldmlErrorHandler *eHandler, const char *root, DataObject *dataObject )
{
    FieldmlInputStream *stream = NULL;
    
    if( ( dataObject == NULL ) || ( root == NULL ) || ( eHandler == NULL ) )
    {
        return FML_INVALID_HANDLE;
    }
    else if( dataObject->source->sourceType == SOURCE_TEXT_FILE )
    {
        TextFileDataSource *fileDataSource = (TextFileDataSource*)dataObject->source;

        const string filename = makeFilename( root, fileDataSource->filename );
        stream = FieldmlInputStream::createTextFileStream( filename );

        if( stream != NULL )
        {
            for( int i = 0; i < fileDataSource->lineOffset - 1; i++ )
            {
                stream->skipLine();
            }
        }
    }
    else if( dataObject->source->sourceType == SOURCE_INLINE )
    {
        InlineDataSource *inlineDataSource = (InlineDataSource*)dataObject->source;
        stream = FieldmlInputStream::createStringStream( inlineDataSource->data );
    }
    
    if( stream == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    DataReader *reader = new DataReader( stream, eHandler, dataObject->entryCount, dataObject->entryLength, dataObject->entryHead, dataObject->entryTail );
    return reader->handle;
}


void DataReader::skip( int amount )
{
    for( int i = 0; i < amount; i++ )
    {
        stream->readDouble();
    }
}


int DataReader::readIntValues( int *valueBuffer, int count )
{
    if( entryCounter == entryCount )
    {
        eHandler->setError( FML_ERR_IO_NO_DATA );
        return -1;
    }
    
    if( count + dataCounter > entryLength )
    {
        count = entryLength - dataCounter;
    }
    
    int index = 0;
    while( ( index < count ) && ( !stream->eof() ) )
    {
        valueBuffer[index++] = stream->readInt();
        dataCounter++;
    }
    
    if( stream->eof() )
    {
        eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
        return -1;
    }
    
    if( dataCounter == entryLength )
    {
        entryCounter++;
        dataCounter = 0;
        skip( tail + head );
    }
    
    return count;
}


int DataReader::readDoubleValues( double *valueBuffer, int count )
{
    if( entryCounter == entryCount )
    {
        eHandler->setError( FML_ERR_IO_NO_DATA );
        return -1;
    }
    
    if( count + dataCounter > entryLength )
    {
        count = entryLength - dataCounter;
    }
    
    int index = 0;
    while( index < count )
    {
        valueBuffer[index++] = stream->readDouble();
        dataCounter++;
    }
    
    if( stream->eof() )
    {
        eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
        return -1;
    }
    
    if( dataCounter == entryLength )
    {
        entryCounter++;
        dataCounter = 0;
        skip( tail + head );
    }
    
    return count;
}


DataReader::~DataReader()
{
    delete stream;
    
    readers[handle] = NULL;
}
