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

#include <cstring>

#include "StringUtil.h"
#include "fieldml_api.h"

#include "FieldmlIoApi.h"
#include "FieldmlIoSession.h"

#include "ArrayDataReader.h"
#include "ArrayDataWriter.h"

using namespace std;

//========================================================================
//
// Utility
//
//========================================================================



//========================================================================
//
// API
//
//========================================================================


FmlIoErrorNumber FieldmlIo_GetLastError()
{
    return FieldmlIoSession::getSession().getLastError();
}


FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle )
{
    if( Fieldml_IsObjectLocal( handle, objectHandle, 0 ) != 1 )
    {
        FieldmlIoSession::getSession().setError( FML_IOERR_NONLOCAL_OBJECT );
        return FML_INVALID_HANDLE;
    }

    ArrayDataReader *reader = NULL;
    if( Fieldml_GetDataSourceType( handle, objectHandle ) == DATA_SOURCE_ARRAY )
    {
        string root;
        if( !StringUtil::safeString( Fieldml_GetRegionRoot( handle ), root ) )
        {
            FieldmlIoSession::getSession().setError( FML_IOERR_CORE_ERROR );
        }
        else
        {
            reader = ArrayDataReader::create( FieldmlIoSession::getSession().createContext( handle ), root, objectHandle );
        }
    }
    else
    {
        FieldmlIoSession::getSession().setError( FML_IOERR_UNSUPPORTED );
        return FML_INVALID_HANDLE;
    }
    
    if( reader == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return FieldmlIoSession::getSession().addReader( reader );
}


FmlIoErrorNumber Fieldml_ReadIntSlab( FmlReaderHandle readerHandle, const int *offsets, const int *sizes, int *valueBuffer )
{
    ArrayDataReader *reader = FieldmlIoSession::getSession().handleToReader( readerHandle );
    if( reader == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return reader->readIntSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_ReadDoubleSlab( FmlReaderHandle readerHandle, const int *offsets, const int *sizes, double *valueBuffer )
{
    ArrayDataReader *reader = FieldmlIoSession::getSession().handleToReader( readerHandle );
    if( reader == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return reader->readDoubleSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_ReadBooleanSlab( FmlReaderHandle readerHandle, const int *offsets, const int *sizes, FmlBoolean *valueBuffer )
{
    ArrayDataReader *reader = FieldmlIoSession::getSession().handleToReader( readerHandle );
    if( reader == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return reader->readBooleanSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_CloseReader( FmlReaderHandle readerHandle )
{
    ArrayDataReader *reader = FieldmlIoSession::getSession().handleToReader( readerHandle );
    if( reader == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    FieldmlIoSession::getSession().removeReader( readerHandle );
    
    FmlIoErrorNumber err = reader->close();
    
    delete reader;
    
    return FieldmlIoSession::getSession().setError( err );
}


FmlWriterHandle Fieldml_OpenArrayWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle typeHandle, FmlBoolean append, int *sizes, int rank )
{
    if( Fieldml_IsObjectLocal( handle, objectHandle, 0 ) != 1 )
    {
        FieldmlIoSession::getSession().setError( FML_IOERR_NONLOCAL_OBJECT );
        return FML_INVALID_HANDLE;
    }

    if( Fieldml_IsObjectLocal( handle, typeHandle, 0 ) != 1 )
    {
        FieldmlIoSession::getSession().setError( FML_IOERR_NONLOCAL_OBJECT );
        return FML_INVALID_HANDLE;
    }

    FieldmlHandleType type = Fieldml_GetObjectType( handle, typeHandle );
    if( type == FHT_ENSEMBLE_TYPE )
    {
        //Ensemble types can always be serialized.
    }
    else if( type == FHT_CONTINUOUS_TYPE )
    {
        if( Fieldml_GetTypeComponentEnsemble( handle, typeHandle ) != FML_INVALID_HANDLE )
        {
            //Only scalars are supported for now.
            FieldmlIoSession::getSession().setError( FML_IOERR_INVALID_PARAMETER );
            return FML_INVALID_HANDLE;
        }
    }
    else if( type == FHT_BOOLEAN_TYPE )
    {
        //Boolean values can always scalar, and can always be serialized.
    }
    else
    {
        FieldmlIoSession::getSession().setError( FML_IOERR_INVALID_PARAMETER );
        return FML_INVALID_HANDLE;
    }

    ArrayDataWriter *writer = NULL;

    if( Fieldml_GetDataSourceType( handle, objectHandle ) == DATA_SOURCE_ARRAY )
    {
        FieldmlIoContext *context = FieldmlIoSession::getSession().createContext( handle );
        string root;
        if( !StringUtil::safeString( Fieldml_GetRegionRoot( handle ), root ) )
        {
            FieldmlIoSession::getSession().setError( FML_IOERR_CORE_ERROR );
        }
        else
        {
            writer = ArrayDataWriter::create( context, root, objectHandle, type, ( append == 1 ), sizes, rank );
        }
    }
    

    if( writer == NULL )
    {
        return FML_INVALID_HANDLE;
    }
    
    return FieldmlIoSession::getSession().addWriter( writer );
}


FmlIoErrorNumber Fieldml_WriteIntSlab( FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const int *valueBuffer )
{
    ArrayDataWriter *writer = FieldmlIoSession::getSession().handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return writer->writeIntSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_WriteDoubleSlab( FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const double *valueBuffer )
{
    ArrayDataWriter *writer = FieldmlIoSession::getSession().handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return writer->writeDoubleSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_WriteBooleanSlab( FmlWriterHandle writerHandle, const int *offsets, const int *sizes, const FmlBoolean *valueBuffer )
{
    ArrayDataWriter *writer = FieldmlIoSession::getSession().handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    return writer->writeBooleanSlab( offsets, sizes, valueBuffer );
}


FmlIoErrorNumber Fieldml_CloseWriter( FmlWriterHandle writerHandle )
{
    ArrayDataWriter *writer = FieldmlIoSession::getSession().handleToWriter( writerHandle );
    if( writer == NULL )
    {
        return FieldmlIoSession::getSession().setError( FML_IOERR_UNKNOWN_OBJECT );
    }

    FieldmlIoSession::getSession().removeWriter( writerHandle );
    
    FmlIoErrorNumber err = writer->close();
    
    delete writer;
    
    return FieldmlIoSession::getSession().setError( err );
}
