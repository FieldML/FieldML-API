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
#include "ArrayDataWriter.h"

using namespace std;

#ifdef FIELDML_HDF5_ARRAY
#include <hdf5.h>

class Hdf5DataWriter :
    public ArrayDataWriter
{
private:
    hid_t file;
    hid_t dataset;
    hid_t dataspace;
    
    //Note: In the future, support will be added for non-scalar types that correspond to structured FieldML types.
    //The datatype will need to be checked against the FieldML type to ensure commensurability.
    hid_t datatype;
    int rank;
    hsize_t *hStrides;
    hsize_t *hSizes;
    hsize_t *hOffsets;
    
    bool initializeWithExistingDataset( int *sizes );
    
    bool initializeWithNewDataset( const string sourceName, int *sizes, bool isDouble );

public:
    bool ok;

    Hdf5DataWriter( FieldmlErrorHandler *eHandler, const char *root, ArrayDataSource *source, bool isDouble, bool append, int *sizes, int rank );
    
    virtual int writeIntSlab( int *offsets, int *sizes, int *valueBuffer );
    
    virtual int writeDoubleSlab( int *offsets, int *sizes, double *valueBuffer );
    
    virtual ~Hdf5DataWriter();
};
#endif //FIELDML_HDF5_ARRAY
    

ArrayDataWriter *ArrayDataWriter::create( FieldmlErrorHandler *eHandler, const char *root, ArrayDataSource *source, bool isDouble, bool append, int *sizes, int rank )
{
    ArrayDataWriter *writer = NULL;
    
    if( source->resource->format == HDF5_NAME )
    {
#ifdef FIELDML_HDF5_ARRAY
        Hdf5DataWriter *hdf5writer = new Hdf5DataWriter( eHandler, root, source, isDouble, append, sizes, rank );
        if( !hdf5writer->ok )
        {
            delete hdf5writer;
        }
        else
        {
            writer = hdf5writer;
        }
#endif //FIELDML_HDF5_ARRAY
    }
    
    return writer;
}


ArrayDataWriter::ArrayDataWriter( FieldmlErrorHandler *_eHandler ) :
    eHandler( _eHandler )
{
}


int ArrayDataWriter::writeIntValues( int *valueBuffer, int count )
{
    eHandler->setError( FML_ERR_IO_UNSUPPORTED );
    return -1;
}


int ArrayDataWriter::writeDoubleValues( double *valueBuffer, int count )
{
    eHandler->setError( FML_ERR_IO_UNSUPPORTED );
    return -1;
}


ArrayDataWriter::~ArrayDataWriter()
{
}


#ifdef FIELDML_HDF5_ARRAY
Hdf5DataWriter::Hdf5DataWriter( FieldmlErrorHandler *eHandler, const char *root, ArrayDataSource *source, bool isDouble, bool append, int *sizes, int _rank ) :
    ArrayDataWriter( eHandler )
{
    rank = _rank;
    file = -1;
    dataset = -1;
    dataspace = -1;
    
    hStrides = NULL;
    hSizes = NULL;
    hOffsets = NULL;
    
    ok = false;
    
    const string filename = makeFilename( root, source->resource->href );
    while( true )
    {
        //TODO Add an API-level enum to allow the user to append data, nuke any existing file, or fail if the file already exists. 
        if( !append )
        {
            file = H5Fcreate( filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
        }
        else
        {
            file = H5Fopen( filename.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );
            if( file < 0 )
            {
                file = H5Fcreate( filename.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT );
            }
        }
        
        if( file < 0 )
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
        
        dataset = H5Dopen( file, source->sourceName.c_str(), H5P_DEFAULT );
        if( dataset < 0 )
        {
            if( !initializeWithNewDataset( source->sourceName, sizes, isDouble ) )
            {
                break;
            }
        }
        else
        {
            if( !initializeWithExistingDataset( sizes ) )
            {
                break;
            }
        }

        datatype = H5Dget_type( dataset );
        if( datatype < 0 )
        {
            break;
        }
        //Note: At the moment, only native int and native double can be written, so we get the native type immediately.
        
        hid_t nativeType = H5Tget_native_type( datatype, H5T_DIR_DESCEND );
        if( H5Tequal( nativeType, H5T_NATIVE_INT ) || H5Tequal( nativeType, H5T_NATIVE_SHORT ) )
        {
            datatype = H5T_NATIVE_INT;
        }
        else if( H5Tequal( nativeType, H5T_NATIVE_DOUBLE ) || H5Tequal( nativeType, H5T_NATIVE_FLOAT ) )
        {
            datatype = H5T_NATIVE_DOUBLE;
        }
        
        ok = true;
        break;
    }
}


bool Hdf5DataWriter::initializeWithNewDataset( const string sourceName, int *sizes, bool isDouble )
{
    for( int i = 0; i < rank; i++ )
    {
        hSizes[i] = sizes[i];
    }
    dataspace = H5Screate_simple( rank, hSizes, NULL );
    if( dataspace < 0 )
    {
        return false;
    }
    
    if( isDouble )
    {
        datatype = H5T_NATIVE_DOUBLE;
    }
    else
    {
        datatype = H5T_NATIVE_INT;
    }

    dataset = H5Dcreate( file, sourceName.c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    if( dataset < 0 )
    {
        return false;
    }
    
    return true;
}


bool Hdf5DataWriter::initializeWithExistingDataset( int *sizes )
{
    //The dataset already exists. Make sure its dataspace is compatible with the one requested.
    dataspace = H5Dget_space( dataset );
    if( dataspace < 0 )
    {
        return false;
    }
    
    int existingRank = H5Sget_simple_extent_ndims( dataspace );
    if( ( existingRank < 0 ) || ( existingRank != rank ) ) 
    {
        return false;
    }
    
    existingRank = H5Sget_simple_extent_dims( dataspace, NULL, hSizes );
    if( existingRank != rank )
    {
        return false;
    }
    
    for( int i = 0; i < rank; i++ )
    {
        if( ( hSizes[i] != H5S_UNLIMITED ) && ( hSizes[i] < sizes[i] ) )
        {
            existingRank = -1;
        }
    }
    if( existingRank < 0 )
    {
        return false;
    }

    //The dataset already exists. Make sure its datatype is compatible with the one requested.
    
    return true;
}

int Hdf5DataWriter::writeIntSlab( int *offsets, int *sizes, int *valueBuffer )
{
    if( datatype != H5T_NATIVE_INT )
    {
        eHandler->setError( FML_ERR_IO_UNSUPPORTED );
        return -1;
    }

    for( int i = 0; i < rank; i++ )
    {
        hOffsets[i] = offsets[i];
        hSizes[i] = sizes[i];
    }
    
    hid_t bufferSpace = H5Screate_simple( rank, hSizes, NULL );
    
    herr_t status;
    status = H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, hOffsets, NULL, hSizes, NULL );
    status = H5Dwrite( dataset, H5T_NATIVE_INT, bufferSpace, dataspace, H5P_DEFAULT, valueBuffer );
    
    H5Sclose( bufferSpace );
    
    return 1;
}


int Hdf5DataWriter::writeDoubleSlab( int *offsets, int *sizes, double *valueBuffer )
{
    if( datatype != H5T_NATIVE_DOUBLE )
    {
        eHandler->setError( FML_ERR_IO_UNSUPPORTED );
        return -1;
    }
    
    for( int i = 0; i < rank; i++ )
    {
        hOffsets[i] = offsets[i];
        hSizes[i] = sizes[i];
    }
    
    hid_t bufferSpace = H5Screate_simple( rank, hSizes, NULL );
    
    herr_t status;
    status = H5Sselect_hyperslab( dataspace, H5S_SELECT_SET, hOffsets, NULL, hSizes, NULL );
    status = H5Dwrite( dataset, H5T_NATIVE_DOUBLE, bufferSpace, dataspace, H5P_DEFAULT, valueBuffer );
    
    H5Sclose( bufferSpace );
    
    return 1;
}


Hdf5DataWriter::~Hdf5DataWriter()
{
    H5Sclose( dataspace );
    H5Dclose( dataset );
    H5Fclose( file );
    
    delete[] hStrides;
    delete[] hSizes;
    delete[] hOffsets;
}
#endif //FIELDML_HDF5_ARRAY
