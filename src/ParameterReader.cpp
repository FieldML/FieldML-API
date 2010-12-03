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
#include "ParameterReader.h"

#include "fieldml_api.h"
#include "string_const.h"

using namespace std;

class SemidenseParameterReader :
    public ParameterReader
{
private:
    const int valueCount;
    const int* const swizzle;
    const int indexCount;

protected:
    

public:
    SemidenseParameterReader( FmlInputStream streamHandle, DataFileType _dataType,
        int offset, int indexCount, int valueCount, const int *swizzle );

    bool readIntSlice( int *indexBuffer, int *valueBuffer );
    bool readDoubleSlice( int *indexBuffer, double *valueBuffer );
};

ParameterReader::ParameterReader( FmlInputStream _stream, DataFileType _dataType ) :
    stream( _stream ),
    dataType( _dataType )
{
}
    
    
SemidenseParameterReader::SemidenseParameterReader( FmlInputStream streamHandle, DataFileType dataType, int offset,
    int _indexCount, int _valueCount, const int *_swizzle ) :
    ParameterReader( streamHandle, dataType ),
    indexCount( _indexCount ),
    valueCount( _valueCount ),
    swizzle( _swizzle )
{
    if( dataType == TYPE_LINES )
    {
        for( int i = 0; i < offset; i++ )
        {
            stream->skipLine();
        }
    }
}
    
ParameterReader *ParameterReader::create( FmlHandle handle, ParameterEvaluator *parameters )
{
    if( parameters->dataDescription->descriptionType == DESCRIPTION_SEMIDENSE )
    {
        SemidenseDataDescription *semidense = (SemidenseDataDescription *)parameters->dataDescription;
        
        int indexCount = semidense->sparseIndexes.size();
        int valueCount;
        
        if( semidense->denseIndexes.size() == 0 )
        {
            valueCount = 1;
        }
        else if( Fieldml_IsEnsembleComponentType( handle, semidense->denseIndexes[0] ) != 1 )
        {
            valueCount = 1;
        }
        else
        {
            valueCount = Fieldml_GetEnsembleTypeElementCount( handle, semidense->denseIndexes[0] );
            if( valueCount < 1 )
            {
                handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
                return NULL;
            }
        }

        if( ( semidense->swizzleCount > 0 ) && ( semidense->swizzleCount != valueCount ) )
        {
            handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
            return NULL;
        }
        
        FmlInputStream streamHandle;
        DataFileType dataType = TYPE_UNKNOWN;
        int offset = 0;
        if( semidense->dataLocation->locationType == LOCATION_FILE )
        {
            FileDataLocation *fileDataLocation = (FileDataLocation*)semidense->dataLocation;
            const string filename = makeFilename( handle->getRoot(), fileDataLocation->filename );
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
            handle->setRegionError( FML_ERR_FILE_READ );
            return NULL;
        }
        
        const int *swizzle = NULL;
        if( semidense->swizzleCount > 0 )
        {
            swizzle = semidense->swizzle;
        }
        
        handle->setRegionError( FML_ERR_NO_ERROR );
        return new SemidenseParameterReader( streamHandle, dataType, offset, indexCount, valueCount, swizzle );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
        return NULL;
    }
}


bool SemidenseParameterReader::readIntSlice( int *indexBuffer, int *valueBuffer )
{
    int i;
    int *buffer = new int[valueCount];
    
    for( i = 0; i < indexCount; i++ )
    {
        indexBuffer[i] = stream->readInt();
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        buffer[i] = stream->readInt();
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        if( swizzle != NULL )
        {
            valueBuffer[i] = buffer[swizzle[i] - 1];
        }
        else
        {
            valueBuffer[i] = buffer[i];
        }
    }
    
    delete[] buffer;
    
    if( dataType == TYPE_LINES )
    {
        stream->skipLine();
    }
    
    return stream->eof();
}


bool SemidenseParameterReader::readDoubleSlice( int *indexBuffer, double *valueBuffer )
{
    int i;
    double *buffer = new double[valueCount];
    
    for( i = 0; i < indexCount; i++ )
    {
        indexBuffer[i] = stream->readInt();
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        buffer[i] = stream->readDouble();
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        if( swizzle != NULL )
        {
            valueBuffer[i] = buffer[swizzle[i] - 1];
        }
        else
        {
            valueBuffer[i] = buffer[i];
        }
    }
    
    delete[] buffer;

    if( dataType == TYPE_LINES )
    {
        stream->skipLine();
    }

    return stream->eof();
}


ParameterReader::~ParameterReader()
{
    delete stream;
}
