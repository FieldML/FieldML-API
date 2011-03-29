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

#include "FieldmlErrorHandler.h"
#include "ParameterReader.h"

#include "fieldml_api.h"
#include "string_const.h"

using namespace std;

static vector<ParameterReader *> readers;

ParameterReader *ParameterReader::handleToReader( FmlReaderHandle handle )
{
    if( ( handle < 0 ) || ( handle >= readers.size() ) )
    {
        return NULL;
    }
    
    return readers.at( handle );
}


FmlReaderHandle ParameterReader::addReader( ParameterReader *reader )
{
    readers.push_back( reader );
    return readers.size() - 1;
}


class SemidenseParameterReader :
    public ParameterReader
{
private:
    FieldmlErrorHandler *eHandler;
    int currentBlockCount;
    const int sliceCount;
    const int* const swizzle;
    const int indexCount;
    const int blockCount;
    
    bool gotFirstIndexSet;
    int *intSliceBuffer;
    double *doubleSliceBuffer;
    
    int readIntSlice( int *valueBuffer, const int offset );
    int readDoubleSlice( double *valueBuffer, const int offset );

protected:
    

public:
    SemidenseParameterReader( FieldmlErrorHandler *eHandler, FmlInputStream streamHandle, DataFileType _dataType,
        int offset, int indexCount, int blockCount, int sliceCount, const int *swizzle );

    int readNextIndexSet( int *indexValues );
    int readIntValues( int *values, int count );
    int readDoubleValues( double *value, int count );
    
    virtual ~SemidenseParameterReader();
};

ParameterReader::ParameterReader( FmlInputStream _stream, DataFileType _dataType ) :
    stream( _stream ),
    dataType( _dataType )
{
    handle = addReader( this );
}
    
    
SemidenseParameterReader::SemidenseParameterReader( FieldmlErrorHandler *_eHandler, FmlInputStream streamHandle, DataFileType dataType,
    int offset, int _indexCount, int _blockCount, int _sliceCount, const int *_swizzle ) :
    ParameterReader( streamHandle, dataType ),
    eHandler( _eHandler ),
    indexCount( _indexCount ),
    blockCount( _blockCount ),
    sliceCount( _sliceCount ),
    swizzle( _swizzle )
{
    gotFirstIndexSet = false;
    intSliceBuffer = NULL;
    doubleSliceBuffer = NULL;
    currentBlockCount = 0;
    
    if( dataType == TYPE_LINES )
    {
        for( int i = 0; i < offset; i++ )
        {
            stream->skipLine();
        }
    }
}
    
FmlReaderHandle ParameterReader::create( FmlHandle sessionHandle, FieldmlErrorHandler *eHandler, const char *root, ParameterEvaluator *parameters )
{
    if( parameters->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameters->dataDescription;
        
        int indexCount = semidense->sparseIndexes.size();
        int sliceCount;
        int indexType;
        int blockCount = 1;
        int ensembleCount;
        
        for (vector<FmlObjectHandle>::iterator i = semidense->denseIndexes.begin(); i != semidense->denseIndexes.end(); i++ )
        {
            indexType = Fieldml_GetValueType( sessionHandle, *i );
            ensembleCount = Fieldml_GetElementCount( sessionHandle, indexType );
            if( ensembleCount < 1 )
            {
                eHandler->setError( FML_ERR_MISCONFIGURED_OBJECT );
                return FML_INVALID_HANDLE;
            }
            blockCount *= ensembleCount; 
        }
        
        if( semidense->denseIndexes.size() == 0 )
        {
            sliceCount = 1;
        }
        else
        {
            int innermostType = Fieldml_GetValueType( sessionHandle, semidense->denseIndexes[0] );
            if( Fieldml_IsEnsembleComponentType( sessionHandle, innermostType ) != 1 )
            {
                sliceCount = 1;
            }
            else
            {
                sliceCount = Fieldml_GetElementCount( sessionHandle, innermostType );
                if( sliceCount < 1 )
                {
                    eHandler->setError( FML_ERR_MISCONFIGURED_OBJECT );
                    return FML_INVALID_HANDLE;
                }
            }
        }

        if( ( semidense->swizzleCount > 0 ) && ( semidense->swizzleCount != sliceCount ) )
        {
            eHandler->setError( FML_ERR_MISCONFIGURED_OBJECT );
            return FML_INVALID_HANDLE;
        }
        
        FmlInputStream streamHandle = NULL;
        DataFileType dataType = TYPE_UNKNOWN;
        int offset = 0;
        if( semidense->dataLocation->locationType == LOCATION_FILE )
        {
            FileDataLocation *fileDataLocation = (FileDataLocation*)semidense->dataLocation;
            const string filename = makeFilename( root, fileDataLocation->filename );
            streamHandle = FieldmlInputStream::create( filename ); 
            dataType = fileDataLocation->fileType;
            offset = fileDataLocation->offset;
        }
        else if( semidense->dataLocation->locationType == LOCATION_INLINE )
        {
            InlineDataLocation *inlineDataLocation = (InlineDataLocation*)semidense->dataLocation;
            streamHandle = FieldmlInputStream::create( inlineDataLocation->data );
            dataType = TYPE_LINES; //TODO Support other types of inline data.
            offset = 0; //TODO Support non-zero inline offsets (even if they don't make a lot of sense).
        }
        if( streamHandle == NULL )
        {
            eHandler->setError( FML_ERR_FILE_READ );
            return FML_INVALID_HANDLE;
        }
        
        const int *swizzle = NULL;
        if( semidense->swizzleCount > 0 )
        {
            swizzle = semidense->swizzle;
        }
        
        eHandler->setError( FML_ERR_NO_ERROR );

        ParameterReader *reader = new SemidenseParameterReader( eHandler, streamHandle, dataType, offset, indexCount, blockCount, sliceCount, swizzle );
        return reader->handle;
    }
    else
    {
        eHandler->setError( FML_ERR_UNSUPPORTED );
        return FML_INVALID_HANDLE;
    }
}


int SemidenseParameterReader::readNextIndexSet( int *indexValues )
{
    if( indexCount == 0 )
    {
        if( gotFirstIndexSet )
        {
            return FML_ERR_IO_NO_DATA;
        }
    }

    gotFirstIndexSet = true;
    for( int i = 0; i < indexCount; i++ )
    {
        indexValues[i] = stream->readInt();
    }
    
    currentBlockCount = 0;
  
    if( stream->eof() )
    {
        return FML_ERR_IO_UNEXPECTED_EOF;
    }
    
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterReader::readIntSlice( int *valueBuffer, const int offset )
{
    if( intSliceBuffer == NULL )
    {
        intSliceBuffer = new int[sliceCount];
    }

    for( int i = 0; i < sliceCount; i++ )
    {
        intSliceBuffer[i] = stream->readInt();
    }
    
    for( int i = 0; i < sliceCount; i++ )
    {
        if( swizzle != NULL )
        {
            valueBuffer[offset + i] = intSliceBuffer[swizzle[i] - 1];
        }
        else
        {
            valueBuffer[offset + i] = intSliceBuffer[i];
        }
    }
    
    if( stream->eof() )
    {
        return FML_ERR_IO_UNEXPECTED_EOF;
    }

    if( dataType == TYPE_LINES )
    {
        stream->skipLine();
    }
    
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterReader::readIntValues( int *valueBuffer, int count )
{
    if( count < sliceCount )
    {
        //The parameter number is for the corresponding API call, not this particular method.
        eHandler->setError( FML_ERR_INVALID_PARAMETER_4 );
        return -1;
    }
    
    if( currentBlockCount + count >= blockCount )
    {
        count = blockCount - currentBlockCount;
    }
    
    int readCount = 0;
    while( readCount + sliceCount <= count )
    {
        int err = readIntSlice( valueBuffer, readCount );
        
        if( err != FML_ERR_NO_ERROR )
        {
            eHandler->setError( err );
            return -1;
        }
        readCount += sliceCount;
        currentBlockCount += sliceCount;
    }
    
    return readCount;
}


int SemidenseParameterReader::readDoubleSlice( double *valueBuffer, const int offset )
{
    if( doubleSliceBuffer == NULL )
    {
        doubleSliceBuffer = new double[sliceCount];
    }

    int i = 0;
    for( i = 0; i < sliceCount; i++ )
    {
        doubleSliceBuffer[i] = stream->readDouble();
    }
    
    if( ( i != sliceCount ) && stream->eof() )
    {
        return FML_ERR_IO_UNEXPECTED_EOF;
    }

    for( int i = 0; i < sliceCount; i++ )
    {
        if( swizzle != NULL )
        {
            valueBuffer[offset + i] = doubleSliceBuffer[swizzle[i] - 1];
        }
        else
        {
            valueBuffer[offset + i] = doubleSliceBuffer[i];
        }
    }
    
    if( dataType == TYPE_LINES )
    {
        stream->skipLine();
    }
    
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterReader::readDoubleValues( double *valueBuffer, int count )
{
    if( count < sliceCount )
    {
        //The parameter number is for the corresponding API call, not this particular method.
        eHandler->setError( FML_ERR_INVALID_PARAMETER_4 );
        return -1;
    }
    
    if( currentBlockCount + count >= blockCount )
    {
        count = blockCount - currentBlockCount;
    }
    
    int readCount = 0;
    while( readCount + sliceCount <= count )
    {
        int err = readDoubleSlice( valueBuffer, readCount );
        
        if( err != FML_ERR_NO_ERROR )
        {
            eHandler->setError( err );
            return -1;
        }
        readCount += sliceCount;
        currentBlockCount += sliceCount;
    }
    
    return readCount;
}


SemidenseParameterReader::~SemidenseParameterReader()
{
    delete doubleSliceBuffer;
    delete intSliceBuffer;
}


ParameterReader::~ParameterReader()
{
    delete stream;
    
    readers[handle] = NULL;
}
