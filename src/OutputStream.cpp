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

#include <stdio.h>

#include <string>

#include "fieldml_api.h"
#include "OutputStream.h"

static const int BUFFER_SIZE = 1024;

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

FieldmlOutputStream *FieldmlOutputStream::create( const string filename, bool append )
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


int FileOutputStream::writeDouble( double value )
{
    fprintf( file, "%.8g ", value );
    
    return FML_ERR_NO_ERROR;
}


int FileOutputStream::writeInt( int value )
{
    fprintf( file, "%d ", value );
    
    return FML_ERR_NO_ERROR;
}


int FileOutputStream::writeNewline()
{
    fprintf( file, "\n" );
    
    return FML_ERR_NO_ERROR;
}


FileOutputStream::~FileOutputStream()
{
    if( file != NULL )
    {
        fclose( file );
    }
}
