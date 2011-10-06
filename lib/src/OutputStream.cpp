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
#include <sstream>

#include "fieldml_api.h"
#include "OutputStream.h"

//Using a #define because the relevant buffer is allocated on stack.
#define NBUFFER_SIZE 64

using namespace std;

class FileOutputStream :
    public FieldmlOutputStream
{
private:
    FILE *file;
    
public:
    FileOutputStream( FILE *_file );
    int writeInt( int value );
    int writeDouble( double value );
    int writeNewline();
    virtual ~FileOutputStream();
};

        
class StringOutputStream :
    public FieldmlOutputStream
{
private:
    string &destination;
    const bool append;
    stringstream buffer;
    
public:
    StringOutputStream( string &_destination, bool _append );
    int writeInt( int value );
    int writeDouble( double value );
    int writeNewline();
    virtual ~StringOutputStream();
};

    
FieldmlOutputStream::FieldmlOutputStream()
{
}


FieldmlOutputStream::~FieldmlOutputStream()
{
}

    
FileOutputStream::FileOutputStream( FILE *_file ) :
    file( _file )
{
}


FieldmlOutputStream *FieldmlOutputStream::createTextFileStream( const string filename, bool append )
{
    FILE *file;
    
    if( append )
    {
        file = fopen( filename.c_str(), "a" );
    }
    else
    {
        file = fopen( filename.c_str(), "w" );
    }
    
    if( file == NULL )
    {
        return NULL;
    }
    
    return new FileOutputStream( file );
}


FieldmlOutputStream *FieldmlOutputStream::createStringStream( string &destination, bool append )
{
    return new StringOutputStream( destination, append );
}


int FileOutputStream::writeDouble( double value )
{
    int err = fprintf( file, "%.8g ", value );

    if( err < 0 )
    {
        return FML_ERR_IO_WRITE_ERR;
    }
    
    return FML_ERR_NO_ERROR;
}


int FileOutputStream::writeInt( int value )
{
    int err = fprintf( file, "%d ", value );
    
    if( err < 0 )
    {
        return FML_ERR_IO_WRITE_ERR;
    }
    
    return FML_ERR_NO_ERROR;
}


int FileOutputStream::writeNewline()
{
    int err = fprintf( file, "\n" );
    
    if( err < 0 )
    {
        return FML_ERR_IO_WRITE_ERR;
    }
    
    return FML_ERR_NO_ERROR;
}


FileOutputStream::~FileOutputStream()
{
    if( file != NULL )
    {
        fclose( file );
    }
}


StringOutputStream::StringOutputStream( string &_destination, bool _append ) :
    destination( _destination ),
    append( _append )
{
}


int StringOutputStream::writeDouble( double value )
{
    buffer << value << " ";
    
    return FML_ERR_NO_ERROR;
}


int StringOutputStream::writeInt( int value )
{
    buffer << value << " ";
    
    return FML_ERR_NO_ERROR;
}


int StringOutputStream::writeNewline()
{
    buffer << "\n";
    
    return FML_ERR_NO_ERROR;
}


StringOutputStream::~StringOutputStream()
{
    if( append )
    {
        destination += buffer.str();
    }
    else
    {
        destination = buffer.str();
    }
}
