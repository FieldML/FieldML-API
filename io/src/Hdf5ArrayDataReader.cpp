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
#include "FieldmlIoApi.h"

#include "Hdf5ArrayDataReader.h"

using namespace std;

#if defined FIELDML_HDF5_ARRAY || FIELDML_PHDF5_ARRAY

Hdf5ArrayDataReader *Hdf5ArrayDataReader::create( FieldmlIoContext *context, const string root, FmlObjectHandle source )
{
    Hdf5ArrayDataReader *reader = NULL;

    FmlObjectHandle resource = Fieldml_GetDataSourceResource( context->getSession(), source );
    string format;

    if( !StringUtil::safeString( Fieldml_GetDataResourceFormat( context->getSession(), resource ), format ) )
    {
        context->setError( FML_IOERR_CORE_ERROR );
    }
    else if( format == StringUtil::HDF5_NAME )
    {
#ifdef FIELDML_HDF5_ARRAY
        Hdf5ArrayDataReader *hdf5reader = new Hdf5ArrayDataReader( context, root, source, H5P_DEFAULT );
        if( !hdf5reader->ok )
        {
            delete hdf5reader;
        }
        else
        {
            reader = hdf5reader;
        }
#endif //FIELDML_HDF5_ARRAY
    }
    else if( format == StringUtil::PHDF5_NAME )
    {
#ifdef FIELDML_PHDF5_ARRAY
        hid_t accessProperties = H5Pcreate( H5P_FILE_ACCESS );
        if( H5Pset_fapl_mpio( accessProperties, MPI_COMM_WORLD, MPI_INFO_NULL ) >= 0 )
        {
            Hdf5ArrayDataReader *hdf5reader = new Hdf5ArrayDataReader( _session, root, source, accessProperties );
            if( !hdf5reader->ok )
            {
                delete hdf5reader;
            }
            else
            {
                reader = hdf5reader;
            }
        }
        H5Pclose( accessProperties );
#endif //FIELDML_PHDF5_ARRAY
    }
    
    return reader;
}


#if defined FIELDML_HDF5_ARRAY || FIELDML_PHDF5_ARRAY
Hdf5ArrayDataReader::Hdf5ArrayDataReader( FieldmlIoContext *_context, const string root, FmlObjectHandle source, hid_t accessProperties ) :
    ArrayDataReader( _context ),
    closed( false )
{
    hStrides = NULL;
    hSizes = NULL;
    hOffsets = NULL;
    
    ok = false;
    closed = true;

    while( true )
    {
        FmlObjectHandle resource = Fieldml_GetDataSourceResource( context->getSession(), source );

        string description;
        if( !StringUtil::safeString( Fieldml_GetDataResourceHref( context->getSession(), resource ), description ) )
        {
            break;
        }
            
        string location;
        if( !StringUtil::safeString( Fieldml_GetArrayDataSourceLocation( context->getSession(), source ), location ) )
        {
            break;
        }

        const string filename = StringUtil::makeFilename( root, description );

        file = H5Fopen( filename.c_str(), H5F_ACC_RDONLY, accessProperties );
        if( file < 0 )
        {
            break;
        }
        
        dataset = H5Dopen( file, location.c_str(), H5P_DEFAULT );
        if( dataset < 0 )
        {
            break;
        }
        
        dataspace = H5Dget_space( dataset );
        if( dataspace < 0 )
        {
            break;
        }
        
        rank = H5Sget_simple_extent_ndims( dataspace );
        if( rank <= 0 )
        {
            break;
        }
        
        datatype = H5Dget_type( dataset );
        if( datatype < 0 )
        {
            break;
        }
        
        hOffsets = new hsize_t[rank];
        hSizes = new hsize_t[rank];
        hStrides = new hsize_t[rank];
        for( int i = 0; i < rank; i++ )
        {
            hStrides[i] = 1;
        }
    
        hid_t nativeType = H5Tget_native_type( datatype, H5T_DIR_DESCEND );
        if( H5Tequal( nativeType, H5T_NATIVE_INT ) || H5Tequal( nativeType, H5T_NATIVE_SHORT ) )
        {
            datatype = H5T_NATIVE_INT;
        }
        else if( H5Tequal( nativeType, H5T_NATIVE_DOUBLE ) || H5Tequal( nativeType, H5T_NATIVE_FLOAT ) )
        {
            datatype = H5T_NATIVE_DOUBLE;
        }
        else if( H5Tequal( nativeType, H5T_NATIVE_INT8 ) )
        {
            datatype = H5T_NATIVE_INT8;
        }
        
        ok = true;
        closed = false;
        break;
    }
}


FmlIoErrorNumber Hdf5ArrayDataReader::readSlab( const int *offsets, const int *sizes, hid_t requiredDatatype, void *valueBuffer )
{
    if( datatype != requiredDatatype )
    {
        return context->setError( FML_IOERR_UNSUPPORTED );
    }

    for( int i = 0; i < rank; i++ )
    {
        hOffsets[i] = offsets[i];
        hSizes[i] = sizes[i];
    }
    
    hid_t bufferSpace = H5Screate_simple( rank, hSizes, NULL );
    
    herr_t status;
    status = H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, hOffsets, NULL, hSizes, NULL );

    hid_t transferProperties = H5P_DEFAULT;

    //if( collective )
    //{
    //    transferProperties = H5Pcreate (H5P_DATASET_XFER);
    //    H5Pset_dxpl_mpio(transferProperties, H5FD_MPIO_COLLECTIVE);
    //}
        
    status = H5Dread( dataset, requiredDatatype, bufferSpace, dataspace, transferProperties, valueBuffer );
    
    //H5Pclose( transferProperties );
    
    H5Sclose( bufferSpace );
    
    if( status >= 0 )
    {
        return FML_IOERR_NO_ERROR;
    }
    
    return context->setError( FML_IOERR_READ_ERROR );
}


FmlIoErrorNumber Hdf5ArrayDataReader::readIntSlab( const int *offsets, const int *sizes, int *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }
    
    return readSlab( offsets, sizes, H5T_NATIVE_INT, valueBuffer );
}


FmlIoErrorNumber Hdf5ArrayDataReader::readDoubleSlab( const int *offsets, const int *sizes, double *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }
    
    return readSlab( offsets, sizes, H5T_NATIVE_DOUBLE, valueBuffer );
}


FmlIoErrorNumber Hdf5ArrayDataReader::readBooleanSlab( const int *offsets, const int *sizes, FmlBoolean *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }
    
    return readSlab( offsets, sizes, H5T_NATIVE_INT8, valueBuffer );
}


FmlIoErrorNumber Hdf5ArrayDataReader::close()
{
    if( closed )
    {
        return FML_IOERR_NO_ERROR;
    }
    
    H5Sclose( dataspace );
    H5Dclose( dataset );
    H5Fclose( file );
    
    closed = true;
    return FML_IOERR_NO_ERROR;
}


Hdf5ArrayDataReader::~Hdf5ArrayDataReader()
{
    if( !closed )
    {
        close();
    }
    
    delete[] hStrides;
    delete[] hSizes;
    delete[] hOffsets;
}

#endif //defined FIELDML_HDF5_ARRAY || FIELDML_PHDF5_ARRAY

#endif //FIELDML_HDF5_ARRAY
