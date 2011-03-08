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

#include "Region.h"
#include "ParameterWriter.h"

#include "fieldml_api.h"
#include "string_const.h"

using namespace std;

class SemidenseParameterWriter :
    public ParameterWriter
{
private:
    FieldmlRegion *region;
    int currentBlockCount;
    const int sliceCount;
    const int* const swizzle;
    const int indexCount;
    const int blockCount;
    
    bool gotFirstIndexSet;
    int *intSliceBuffer;
    double *doubleSliceBuffer;

    int writeIntSlice( int *valueBuffer, const int offset );
    int writeDoubleSlice( double *valueBuffer, const int offset );

protected:
    

public:
    SemidenseParameterWriter( FieldmlRegion *region, FmlOutputStream streamHandle, DataFileType _dataType,
        int indexCount, int blockCount, int sliceCount, const int *swizzle );

    int writeNextIndexSet( int *indexValues );
    int writeIntValues( int *values, int count );
    int writeDoubleValues( double *value, int count ); 
};

ParameterWriter::ParameterWriter( FmlOutputStream _stream, DataFileType _dataType ) :
    stream( _stream ),
    dataType( _dataType )
{
}
    
    
SemidenseParameterWriter::SemidenseParameterWriter( FieldmlRegion *_region, FmlOutputStream streamHandle, DataFileType dataType,
    int _indexCount, int _blockCount, int _sliceCount, const int *_swizzle ) :
    ParameterWriter( streamHandle, dataType ),
    region( _region ),
    indexCount( _indexCount ),
    blockCount( _blockCount ),
    sliceCount( _sliceCount ),
    swizzle( _swizzle )
{
    gotFirstIndexSet = false;
    intSliceBuffer = NULL;
    doubleSliceBuffer = NULL;
    currentBlockCount = 0;
}

ParameterWriter *ParameterWriter::create( FieldmlRegion *region, ParameterEvaluator *parameters, bool append )
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
            indexType = Fieldml_GetValueType( region->getRegionHandle(), *i );
            ensembleCount = Fieldml_GetEnsembleTypeElementCount( region->getRegionHandle(), indexType );
            if( ensembleCount < 1 )
            {
                region->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
                return NULL;
            }
            blockCount *= ensembleCount; 
        }
        
        if( semidense->denseIndexes.size() == 0 )
        {
            sliceCount = 1;
        }
        else
        {
            int innermostType = Fieldml_GetValueType( region->getRegionHandle(), semidense->denseIndexes[0] );
            if( Fieldml_IsEnsembleComponentType( region->getRegionHandle(), innermostType ) != 1 )
            {
                sliceCount = 1;
            }
            else
            {
                sliceCount = Fieldml_GetEnsembleTypeElementCount( region->getRegionHandle(), innermostType );
                if( sliceCount < 1 )
                {
                    region->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
                    return NULL;
                }
            }
        }

        if( ( semidense->swizzleCount > 0 ) && ( semidense->swizzleCount != sliceCount ) )
        {
            region->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
            return NULL;
        }
        
        FmlOutputStream streamHandle;
        DataFileType dataType = TYPE_UNKNOWN;
        if( semidense->dataLocation->locationType == LOCATION_FILE )
        {
            FileDataLocation *fileDataLocation = (FileDataLocation*)semidense->dataLocation;
            const string filename = makeFilename( region->getRoot(), fileDataLocation->filename );
            streamHandle = FieldmlOutputStream::create( filename, append );
            dataType = fileDataLocation->fileType;
        }
        else if( semidense->dataLocation->locationType == LOCATION_INLINE )
        {
            InlineDataLocation *inlineDataLocation = (InlineDataLocation*)semidense->dataLocation;
            streamHandle = FieldmlOutputStream::create( (char**)&inlineDataLocation->data );
            dataType = TYPE_LINES; //TODO Support other types of inline data.
        }
        if( streamHandle == NULL )
        {
            region->setRegionError( FML_ERR_FILE_WRITE );
            return NULL;
        }
        
        //TODO Support de-swizzle
        const int *swizzle = NULL;
        if( semidense->swizzleCount > 0 )
        {
            region->setRegionError( FML_ERR_UNSUPPORTED );
            return NULL;
        }
        
        region->setRegionError( FML_ERR_NO_ERROR );

        return new SemidenseParameterWriter( region, streamHandle, dataType, indexCount, blockCount, sliceCount, swizzle );
    }
    else
    {
        region->setRegionError( FML_ERR_UNSUPPORTED );
        return NULL;
    }
}


int SemidenseParameterWriter::writeNextIndexSet( int *indexValues )
{
    if( indexCount == 0 )
    {
        if( gotFirstIndexSet )
        {
            return FML_ERR_IO_UNEXPECTED_DATA;
        }
    }

    gotFirstIndexSet = true;
    for( int i = 0; i < indexCount; i++ )
    {
        stream->writeInt( indexValues[i] );
    }
    
    if( ( dataType == TYPE_LINES ) && ( sliceCount != blockCount ) )
    {
        stream->writeNewline();
    }
    
    currentBlockCount = 0;
  
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterWriter::writeIntSlice( int *valueBuffer, const int offset )
{
    if( intSliceBuffer == NULL )
    {
        intSliceBuffer = new int[sliceCount];
    }

    //TODO de-swizzle into slice buffer
    for( int i = 0; i < sliceCount; i++ )
    {
        intSliceBuffer[i] = valueBuffer[offset + i];
    }
    
    for( int i = 0; i < sliceCount; i++ )
    {
        stream->writeInt( intSliceBuffer[i] );
    }

    if( dataType == TYPE_LINES )
    {
        stream->writeNewline();
    }
    
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterWriter::writeIntValues( int *valueBuffer, int count )
{
    if( count < sliceCount )
    {
        //The parameter number is for the corresponding API call, not this particular method.
        region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
        return -1;
    }
    
    if( currentBlockCount + count > blockCount )
    {
        return FML_ERR_IO_UNEXPECTED_DATA;
    }
    
    int writeCount = 0;
    while( writeCount < count )
    {
        int err = writeIntSlice( valueBuffer, writeCount );
        
        if( err != FML_ERR_NO_ERROR )
        {
            region->setRegionError( err );
            return -1;
        }
        writeCount += sliceCount;
        currentBlockCount += sliceCount;
    }
    
    return writeCount;
}


int SemidenseParameterWriter::writeDoubleSlice( double *valueBuffer, const int offset )
{
    if( doubleSliceBuffer == NULL )
    {
        doubleSliceBuffer = new double[sliceCount];
    }

    //TODO de-swizzle into slice buffer
    for( int i = 0; i < sliceCount; i++ )
    {
        doubleSliceBuffer[i] = valueBuffer[offset + i];
    }
    
    for( int i = 0; i < sliceCount; i++ )
    {
        stream->writeDouble( doubleSliceBuffer[i] );
    }

    if( dataType == TYPE_LINES )
    {
        stream->writeNewline();
    }
    
    return FML_ERR_NO_ERROR;
}


int SemidenseParameterWriter::writeDoubleValues( double *valueBuffer, int count )
{
    if( count < sliceCount )
    {
        //The parameter number is for the corresponding API call, not this particular method.
        region->setRegionError( FML_ERR_INVALID_PARAMETER_4 );
        return -1;
    }
    
    if( currentBlockCount + count > blockCount )
    {
        return FML_ERR_IO_UNEXPECTED_DATA;
    }
    
    int writeCount = 0;
    while( writeCount < count )
    {
        int err = writeDoubleSlice( valueBuffer, writeCount );
        
        if( err != FML_ERR_NO_ERROR )
        {
            region->setRegionError( err );
            return -1;
        }
        writeCount += sliceCount;
        currentBlockCount += sliceCount;
    }
    
    return writeCount;
}


ParameterWriter::~ParameterWriter()
{
    delete stream;
}
