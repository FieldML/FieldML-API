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

#include <sstream>

#include "StringUtil.h"
#include "FieldmlIoApi.h"

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

    
TextArrayDataReader *TextArrayDataReader::create( FieldmlIoContext *context, const string root, FmlObjectHandle source )
{
    FieldmlInputStream *stream = NULL;
    
    FmlObjectHandle resource = Fieldml_GetDataSourceResource( context->getSession(), source );
    string format;
    if( !StringUtil::safeString( Fieldml_GetDataResourceFormat( context->getSession(), resource ), format ) )
    {
        context->setError( FML_IOERR_CORE_ERROR );
        return NULL;
    }

    DataResourceType type = Fieldml_GetDataResourceType( context->getSession(), resource );
    
    int rank = Fieldml_GetArrayDataSourceRank( context->getSession(), source );
    if( rank <= 0 )
    {
        context->setError( FML_IOERR_CORE_ERROR );
        return NULL;
    }

    if( format != StringUtil::PLAIN_TEXT_NAME )
    {
        context->setError( FML_IOERR_UNSUPPORTED );
        return NULL;
    }

    if( type == DATA_RESOURCE_HREF )
    {
        string href;
        if( !StringUtil::safeString( Fieldml_GetDataResourceHref( context->getSession(), resource ), href ) )
        {
            context->setError( FML_IOERR_CORE_ERROR );
            return NULL;
        }
        stream = FieldmlInputStream::createTextFileStream( StringUtil::makeFilename( root, href ) );
    }
    else if( type == DATA_RESOURCE_INLINE )
    {
        string data;
        if( !StringUtil::safeString( Fieldml_GetInlineData( context->getSession(), resource ), data ) )
        {
            return NULL;
        }
        stream = FieldmlInputStream::createStringStream( data );
    }
    
    if( stream == NULL )
    {
        return NULL;
    }
    
    return new TextArrayDataReader( context, stream, source, rank );
}


TextArrayDataReader::TextArrayDataReader( FieldmlIoContext *_context, FieldmlInputStream *_stream, FmlObjectHandle _source, int rank ) :
    ArrayDataReader( _context ),
    stream( _stream ),
    source( _source ),
    sourceRank( rank ),
    sourceSizes( NULL ),
    sourceOffsets( NULL ),
    sourceRawSizes( NULL ),
    closed( false )
{
    startPos = -1;
    
    nextOutermostOffset = -1;
    
    sourceSizes = new int[sourceRank];
    sourceRawSizes = new int[sourceRank];
    sourceOffsets = new int[sourceRank];
    
    Fieldml_GetArrayDataSourceSizes( context->getSession(), source, sourceSizes );
    Fieldml_GetArrayDataSourceRawSizes( context->getSession(), source, sourceRawSizes );
    Fieldml_GetArrayDataSourceOffsets( context->getSession(), source, sourceOffsets );
    
    StringUtil::safeString( Fieldml_GetArrayDataSourceLocation( context->getSession(), source ), sourceLocation );
}


bool TextArrayDataReader::checkDimensions( int *offsets, int *sizes )
{
    for( int i = 0; i < sourceRank; i++ )
    {
        if( offsets[i] < 0 )
        {
            return false;
        }
        if( sizes[i] <= 0 )
        {
            return false;
        }
        
        int rawSize = sourceSizes[i];
        if( rawSize == 0 )
        {
            //NOTE: Intentional. If the array-source size has not been set, use the underlying size.
            rawSize = sourceRawSizes[i] - sourceOffsets[i];
        }
        if( offsets[i] + sizes[i] > rawSize )
        {
            return false;
        }
    }
    
    return true;
}


FmlIoErrorNumber TextArrayDataReader::skipPreamble()
{
    std::istringstream sstr( sourceLocation );
    int lineNumber;

    if( ! ( sstr >> lineNumber ) )
    {
        return FML_IOERR_INVALID_LOCATION;
    }

    for( int i = 1; i < lineNumber; i++ )
    {
        stream->skipLine();
    }
    
    if( stream->eof() )
    {
        return context->setError( FML_IOERR_UNEXPECTED_EOF );
    }

    startPos = stream->tell();
    
    return FML_IOERR_NO_ERROR;
}


bool TextArrayDataReader::applyOffsets( int *offsets, int *sizes, int depth, bool isHead )
{
    long count = 1;
    
    for( int i = depth+1; i < sourceRank; i++ )
    {
        //NOTE This could overflow in the event that someone puts that much data into a text file. Probability: Lilliputian.
        count *= sourceRawSizes[i];
    }
    
    int sliceCount;
    if( isHead )
    {
        sliceCount = sourceOffsets[depth] + offsets[depth];
        if( ( depth == 0 ) && ( nextOutermostOffset >= 0 ) && ( sliceCount >= nextOutermostOffset ) )
        {
            sliceCount -= nextOutermostOffset;
        }
    }
    else
    {
        sliceCount = sourceRawSizes[depth] - ( sourceOffsets[depth] + offsets[depth] + sizes[depth] );
    }
    
    if( sliceCount == 0 )
    {
        return true;
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


FmlIoErrorNumber TextArrayDataReader::readPreSlab( int *offsets, int *sizes )
{
    if( !checkDimensions( offsets, sizes ) )
    {
        return context->setError( FML_IOERR_INVALID_PARAMETER );
    }
    
    if( ( nextOutermostOffset >= 0 ) && ( sourceOffsets[0] + offsets[0] >= nextOutermostOffset ) )
    {
        return FML_IOERR_NO_ERROR;
    }
    
    if( startPos == -1 )
    {
        int err = skipPreamble();
        if( err != FML_IOERR_NO_ERROR )
        {
            return err;
        }
    }
    else
    {
        stream->seek( startPos );
    }
    
    return FML_IOERR_NO_ERROR;
}


FmlIoErrorNumber TextArrayDataReader::readSlice( int *offsets, int *sizes, int depth, BufferReader &reader )
{
    if( !applyOffsets( offsets, sizes, depth, true ) )
    {
        return context->setError( FML_IOERR_UNEXPECTED_EOF );
    }
    
    if( depth == sourceRank - 1 )
    {
        reader.read( sizes[depth] );
        if( stream->eof() )
        {
            return context->setError( FML_IOERR_UNEXPECTED_EOF );
        }
    }
    else
    {
        int err;
        for( int i = 0; i < sizes[depth]; i++ )
        {
            err = readSlice( offsets, sizes, depth + 1, reader );
            if( err != FML_IOERR_NO_ERROR )
            {
                return err;
            }
        }
    }
    
    if( depth == 0 )
    {
        nextOutermostOffset = sourceOffsets[0] + offsets[0] + sizes[0];
    }
    else if( !applyOffsets( offsets, sizes, depth, false ) )
    {
        return context->setError( FML_IOERR_UNEXPECTED_EOF );
    }
    
    
    return FML_IOERR_NO_ERROR;
}


FmlIoErrorNumber TextArrayDataReader::readSlab( int *offsets, int *sizes, BufferReader &reader )
{
    int err = readPreSlab( offsets, sizes );
    if( err != FML_IOERR_NO_ERROR )
    {
        return err;
    }
    
    return readSlice( offsets, sizes, 0, reader );
}


FmlIoErrorNumber TextArrayDataReader::readIntSlab( int *offsets, int *sizes, int *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }
    
    IntBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


FmlIoErrorNumber TextArrayDataReader::readDoubleSlab( int *offsets, int *sizes, double *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }

    DoubleBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


FmlIoErrorNumber TextArrayDataReader::readBooleanSlab( int *offsets, int *sizes, bool *valueBuffer )
{
    if( closed )
    {
        return FML_IOERR_RESOURCE_CLOSED;
    }

    BooleanBufferReader reader( stream, valueBuffer );
    
    return readSlab( offsets, sizes, reader );
}


FmlIoErrorNumber TextArrayDataReader::close()
{
    if( closed )
    {
        return FML_IOERR_NO_ERROR;
    }
    
    closed = true;

    return FML_IOERR_NO_ERROR;
}


TextArrayDataReader::~TextArrayDataReader()
{
    delete stream;
    
    delete sourceRawSizes;
    delete sourceSizes;
    delete sourceOffsets;
}
