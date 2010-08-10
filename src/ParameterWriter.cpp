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
    const int valueCount;
    const int* const swizzle;
    const int indexCount;

protected:
    

public:
    SemidenseParameterWriter( FmlOutputStream streamHandle, DataFileType _dataType,
        int indexCount, int valueCount, const int *swizzle );

    int writeIntSlice( int *indexBuffer, int *valueBuffer );
    int writeDoubleSlice( int *indexBuffer, double *valueBuffer );
};

ParameterWriter::ParameterWriter( FmlOutputStream _stream, DataFileType _dataType ) :
    stream( _stream ),
    dataType( _dataType )
{
}
    
    
SemidenseParameterWriter::SemidenseParameterWriter( FmlOutputStream streamHandle, DataFileType dataType,
    int _indexCount, int _valueCount, const int *_swizzle ) :
    ParameterWriter( streamHandle, dataType ),
    indexCount( _indexCount ),
    valueCount( _valueCount ),
    swizzle( _swizzle )
{
}

ParameterWriter *ParameterWriter::create( FmlHandle handle, Parameters *parameters, bool append )
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
        else if( Fieldml_IsEnsembleComponentDomain( handle, semidense->denseIndexes[0] ) != 1 )
        {
            valueCount = 1;
        }
        else
        {
            valueCount = Fieldml_GetEnsembleDomainElementCount( handle, semidense->denseIndexes[0] );
            if( valueCount < 1 )
            {
                handle->setRegionError( FML_ERR_MISCONFIGURED_OBJECT );
                return NULL;
            }
        }

        if( semidense->swizzleCount != 0 )
        {
            handle->setRegionError( FML_ERR_UNSUPPORTED );
            return NULL;
        }
        
        FmlOutputStream streamHandle;
        DataFileType dataType = TYPE_UNKNOWN;
        if( semidense->dataLocation->locationType == LOCATION_FILE )
        {
            FileDataLocation *fileDataLocation = (FileDataLocation*)semidense->dataLocation;
            const string filename = makeFilename( handle->getRoot(), fileDataLocation->filename );
            streamHandle = FieldmlOutputStream::create( filename, append );
            dataType = fileDataLocation->fileType;
        }
        else if( semidense->dataLocation->locationType == LOCATION_INLINE )
        {
            handle->setRegionError( FML_ERR_UNSUPPORTED );
            return NULL;
            //TODO Support inline output
            /*
            InlineDataLocation *inlineDataLocation = (InlineDataLocation*)semidense->dataLocation;
            streamHandle = FmlCreateStringOutputStream( inlineDataLocation->data );
            dataType = TYPE_LINES; //TODO Support other types of inline data.
            */
        }
        if( streamHandle == NULL )
        {
            handle->setRegionError( FML_ERR_FILE_WRITE );
            return NULL;
        }
        
        //TODO Support de-swizzle
        
        handle->setRegionError( FML_ERR_NO_ERROR );
        return new SemidenseParameterWriter( streamHandle, dataType, indexCount, valueCount, NULL );
    }
    else
    {
        handle->setRegionError( FML_ERR_UNSUPPORTED );
        return NULL;
    }
}


int SemidenseParameterWriter::writeIntSlice( int *indexBuffer, int *valueBuffer )
{
    int i, err;
    
    for( i = 0; i < indexCount; i++ )
    {
        err = stream->writeInt( indexBuffer[i] );
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        err = stream->writeInt( valueBuffer[i] );
    }

    if( dataType == TYPE_LINES )
    {
        stream->writeNewline();
    }

    return err;
}


int SemidenseParameterWriter::writeDoubleSlice( int *indexBuffer, double *valueBuffer )
{
    int i, err;
    
    for( i = 0; i < indexCount; i++ )
    {
        err = stream->writeInt( indexBuffer[i] );
    }
    
    for( i = 0; i < valueCount; i++ )
    {
        err = stream->writeDouble( valueBuffer[i] );
    }

    if( dataType == TYPE_LINES )
    {
        stream->writeNewline();
    }

    return err;
}


ParameterWriter::~ParameterWriter()
{
    delete stream;
}
