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

#include <cstdio>
#include <cstdlib>
#include <cstring> 

#include "fieldml_api.h"
#include "InputStream.h"

using namespace std;

class FileInputStream :
    public FieldmlInputStream
{
private:
    FILE *file;
    
protected:
    int loadBuffer();
    
public:
    virtual long tell();
    virtual bool seek( long pos );
    
    FileInputStream( FILE *_file );
    virtual ~FileInputStream();
};


class StringInputStream :
    public FieldmlInputStream
{
private:
    const char *string;
    int stringPos;
    int stringMaxLen;

protected:
    int loadBuffer();
    
public:
    virtual long tell();
    virtual bool seek( long pos );
    
    StringInputStream( const char *string );
    virtual ~StringInputStream();
};

static const int BUFFER_SIZE = 1024;

//Using a #define because the relevant buffer is allocated on stack.
#define NBUFFER_SIZE 64


FieldmlInputStream::FieldmlInputStream()
{
    buffer = (char*)calloc( 1, BUFFER_SIZE );
    bufferCount = 0;
    bufferPos = 0;
    isEof = false;
}

int FieldmlInputStream::readInt()
{
    int value = 0;
    int invert = 0;
    int gotDigit = 0;
    int d;
    
    while( 1 )
    {
        if( bufferPos >= bufferCount )
        {
            if( !loadBuffer() )
            {
                if( gotDigit )
                {
                    return value;
                }
                return 0;
            }
        }
        
        while( bufferPos < bufferCount )
        {
            d = buffer[bufferPos++];
            if( ( d >= '0' ) && ( d <= '9' ) )
            {
                gotDigit = 1;
                
                value *= 10;
                value += ( d - '0' );
            }
            else if( ( d == '-' ) && ( !gotDigit ) )
            {
                invert = 1 - invert;
            }
            else if( gotDigit )
            {
                bufferPos--;
                if( invert )
                {
                    value = -value;
                }
                return value;
            }
        }
    }
}


double FieldmlInputStream::readDouble()
{
    int d;
    int count = 0;
    int skipping = 1;
    char nBuffer[NBUFFER_SIZE];
    
    while( 1 )
    {
        if( bufferPos >= bufferCount )
        {
            if( !loadBuffer() )
            {
                if( count > 0 )
                {
                    nBuffer[count] = 0;
                    return strtod( nBuffer, NULL );
                }
                return 0;
            }
        }
        
        while( bufferPos < bufferCount )
        {
            d = buffer[bufferPos++];
            if( ( ( d >= '0' ) && ( d <= '9' ) ) || ( d == 'e' ) || ( d == 'E' ) || ( d == '-' ) || ( d == '+') || ( d == '.' ) )
            {
                skipping = 0;
                
                if( count < NBUFFER_SIZE - 1 )
                {
                    //Yes, this will truncate ridiculously long numbers that can't fit into a double anyway.
                    nBuffer[count++] = d;
                }
            }
            else if( skipping )
            {
                continue;
            }
            else
            {
                bufferPos--;
                nBuffer[count] = 0;
                return strtod( nBuffer, NULL );
            }
        }
    }
}


int FieldmlInputStream::skipLine()
{
    while( true )
    {
        if( bufferPos >= bufferCount )
        {
            if( !loadBuffer() )
            {
                return FML_ERR_IO_READ_ERR;
            }
        }
        
        while( bufferPos < bufferCount )
        {
            if( buffer[bufferPos++] == '\n' )
            {
                return FML_ERR_NO_ERROR;
            }
        }
    }
}


FieldmlInputStream *FieldmlInputStream::createTextFileStream( const string filename )
{
    FILE *file;
    
    file = fopen( filename.c_str(), "r" );
    
    if( file == NULL )
    {
        return NULL;
    }
    
    return new FileInputStream( file );
}


FieldmlInputStream *FieldmlInputStream::createStringStream( const char *sourceString )
{
    return new StringInputStream( sourceString );
}


bool FieldmlInputStream::eof()
{
    return isEof;
}


FieldmlInputStream::~FieldmlInputStream()
{
    free( buffer );
}


FileInputStream::FileInputStream( FILE *_file ) :
    file( _file )
{
}


FileInputStream::~FileInputStream()
{
    if( file != NULL )
    {
        fclose( file );
    }
}


int FileInputStream::loadBuffer()
{
    bufferPos = 0;

    bufferCount = fread( buffer, 1, BUFFER_SIZE, file );
    if( bufferCount <= 0 )
    {
        isEof = true;
        return 0;
    }
    
    return 1;
}


long FileInputStream::tell()
{
    return ftell( file );
}


bool FileInputStream::seek( long pos )
{
    if( fseek( file, pos, SEEK_SET ) == 0 )
    {
        bufferPos = bufferCount;
        return true;
    }
    
    return false;
}


StringInputStream::StringInputStream( const char * const _string ) :
    string( _string )
{
    stringPos = 0;
    stringMaxLen = strlen( _string );
}


int StringInputStream::loadBuffer()
{
    int len;
    
    bufferPos = 0;

    len = BUFFER_SIZE;
    if( len + stringPos > stringMaxLen )
    {
        len = stringMaxLen - stringPos;
    }
    memcpy( buffer, string, len );
    stringPos += len;
    bufferCount = len;

    if( bufferCount <= 0 )
    {
        isEof = true;
        return 0;
    }
    
    return 1;
}


long StringInputStream::tell()
{
    return stringPos;
}


bool StringInputStream::seek( long pos )
{
    if( ( pos < 0 ) || ( pos >= stringMaxLen ) )
    {
        return false;
    }
    
    stringPos = pos;
    bufferPos = bufferCount;
    return true;
}


StringInputStream::~StringInputStream()
{
}
