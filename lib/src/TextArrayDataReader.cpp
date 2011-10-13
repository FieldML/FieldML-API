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
#include "TextArrayDataReader.h"
#include "InputStream.h"

using namespace std;

/**
 * A pseudo-lambda class that removes the need to duplicate the slab and slice reading implementations.
 * No point in making this a template class, as we need to use a different method on stream depending on the type,
 * and there's no superclass functionality, so template specialization is redundant.
 */
class BufferReader
{
protected:
    int bufferPos;
    FieldmlInputStream * const stream;
    
public:
    BufferReader( FieldmlInputStream *_stream ) :
        stream( _stream ), bufferPos( 0 ) {}
    
    virtual ~BufferReader() {}
    
    virtual void read( int count ) = 0;
};


class DoubleBufferReader :
    public BufferReader
{
private:
    double * const buffer;
    
public:
    DoubleBufferReader( FieldmlInputStream *_stream, double *_buffer ) :
        BufferReader( _stream ), buffer( _buffer ) {}
    
    void read( int count )
    {
        for( int i = 0; i < count; i++ )
        {
            buffer[bufferPos++] = stream->readDouble();
        }
    }
};


class IntBufferReader :
    public BufferReader
{
private:
    int * const buffer;
    
public:
    IntBufferReader( FieldmlInputStream *_stream, int *_buffer ) :
        BufferReader( _stream ), buffer( _buffer ) {}
    
    void read( int count )
    {
        for( int i = 0; i < count; i++ )
        {
            buffer[bufferPos++] = stream->readInt();
        }
    }
};


class BooleanBufferReader :
    public BufferReader
{
private:
    bool * const buffer;
    
public:
    BooleanBufferReader( FieldmlInputStream *_stream, bool *_buffer ) :
        BufferReader( _stream ), buffer( _buffer ) {}
    
    void read( int count )
    {
        for( int i = 0; i < count; i++ )
        {
            buffer[bufferPos++] = stream->readBoolean();
        }
    }
};

    
TextArrayDataReader *TextArrayDataReader::create( FieldmlErrorHandler *eHandler, const char *root, ArrayDataSource *source )
{
    FieldmlInputStream *stream = NULL;
    
    if( source->resource->format != PLAIN_TEXT_NAME )
    {
        eHandler->setError( FML_ERR_INVALID_OBJECT );
        return NULL;
    }

    if( source->resource->type == DATA_RESOURCE_HREF )
    {
        stream = FieldmlInputStream::createTextFileStream( makeFilename( root, source->resource->description ) );
    }
    else if( source->resource->type == DATA_RESOURCE_INLINE )
    {
        //TODO This is unsafe, as the user can modify the string while the reader is still active.
        stream = FieldmlInputStream::createStringStream( source->resource->description );
    }
    
    if( stream == NULL )
    {
        return NULL;
    }
    
    return new TextArrayDataReader( stream, source, eHandler );
}


TextArrayDataReader::TextArrayDataReader( FieldmlInputStream *_stream, ArrayDataSource *_source, FieldmlErrorHandler *_eHandler ) :
    ArrayDataReader( _eHandler ),
    stream( _stream ),
    source( _source )
{
    startPos = -1;
}


bool TextArrayDataReader::checkDimensions( int *offsets, int *sizes )
{
    for( int i = 0; i < source->rank; i++ )
    {
        if( offsets[i] < 0 )
        {
            return false;
        }
        if( sizes[i] <= 0 )
        {
            return false;
        }
        
        int rawSize = source->sizes[i];
        if( rawSize == 0 )
        {
            //NOTE: Intentional. If the array-source size has not been set, use the underlying size.
            rawSize = source->rawSizes[i] - source->offsets[i];
        }
        if( offsets[i] + sizes[i] > rawSize )
        {
            return false;
        }
    }
    
    return true;
}


FmlErrorNumber TextArrayDataReader::skipPreamble()
{
    bool ok;
    int lineNumber = getInt( source->location, ok );
    if( !ok )
    {
        return FML_ERR_INVALID_PARAMETERS;
    }
    
    for( int i = 1; i < lineNumber; i++ )
    {
        stream->skipLine();
    }
    
    if( stream->eof() )
    {
        return eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
    }

    startPos = stream->tell();
    
    return FML_ERR_NO_ERROR;
}


bool TextArrayDataReader::applyOffsets( int *offsets, int *sizes, int depth, bool isHead )
{
    long count = 1;
    
    for( int i = depth+1; i < source->rank; i++ )
    {
        //NOTE This could overflow in the event that someone puts that much data into a text file. Probability: Lilliputian.
        count *= source->rawSizes[i];
    }
    
    int sliceCount;
    if( isHead )
    {
        sliceCount = source->offsets[depth] + offsets[depth];
    }
    else
    {
        sliceCount = source->rawSizes[depth] - ( source->offsets[depth] + offsets[depth] + sizes[depth] );
    }
    
    for( int j = 0; j < sliceCount; j++ )
    {
        for( int i = 0; i < count; i++ )
        {
            stream->readDouble(); 
        }
    }
    
    return !stream->eof();
}


FmlErrorNumber TextArrayDataReader::readPreSlab( int *offsets, int *sizes )
{
    if( !checkDimensions( offsets, sizes ) )
    {
        return eHandler->setError( FML_ERR_INVALID_PARAMETERS );
    }
    
    if( startPos == -1 )
    {
        int err = skipPreamble();
        if( err != FML_ERR_NO_ERROR )
        {
            return err;
        }
    }
    else
    {
        stream->seek( startPos );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber TextArrayDataReader::readSlice( int *offsets, int *sizes, int depth, BufferReader &reader )
{
    if( !applyOffsets( offsets, sizes, depth, true ) )
    {
        return eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
    }
    
    if( depth == source->rank - 1 )
    {
        reader.read( sizes[depth] );
        if( stream->eof() )
        {
            return eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
        }
    }
    else
    {
        int err;
        for( int i = 0; i < sizes[depth]; i++ )
        {
            err = readSlice( offsets, sizes, depth + 1, reader );
            if( err != FML_ERR_NO_ERROR )
            {
                return err;
            }
        }
    }
    
    if( ( depth > 0 ) && ( !applyOffsets( offsets, sizes, depth, false ) ) )
    {
        return eHandler->setError( FML_ERR_IO_UNEXPECTED_EOF );
    }
    
    return FML_ERR_NO_ERROR;
}


FmlErrorNumber TextArrayDataReader::readSlab( int *offsets, int *sizes, BufferReader &reader )
{
    int err = readPreSlab( offsets, sizes );
    if( err != FML_ERR_NO_ERROR )
    {
        return err;
    }
    
    return readSlice( offsets, sizes, 0, reader );
}


FmlErrorNumber TextArrayDataReader::readIntSlab( int *offsets, int *sizes, int *valueBuffer )
{
    IntBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


FmlErrorNumber TextArrayDataReader::readDoubleSlab( int *offsets, int *sizes, double *valueBuffer )
{
    DoubleBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


FmlErrorNumber TextArrayDataReader::readBooleanSlab( int *offsets, int *sizes, bool *valueBuffer )
{
    BooleanBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


TextArrayDataReader::~TextArrayDataReader()
{
    delete stream;
}
