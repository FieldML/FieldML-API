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
#include "Hdf5ArrayDataWriter.h"
#include "TextArrayDataWriter.h"

using namespace std;

ArrayDataWriter *ArrayDataWriter::create( FieldmlErrorHandler *eHandler, const char *root, BinaryArrayDataSource *source, bool isDouble, bool append, int *sizes, int rank )
{
    ArrayDataWriter *writer = NULL;
    
    if( source->getResource()->format == HDF5_NAME )
    {
#ifdef FIELDML_HDF5_ARRAY
        writer = Hdf5ArrayDataWriter::create( eHandler, root, source, isDouble, append, sizes, rank );
#endif //FIELDML_HDF5_ARRAY
    }
    else if( source->getResource()->format == PHDF5_NAME )
    {
#ifdef FIELDML_PHDF5_ARRAY
        writer = Hdf5ArrayDataWriter::create( eHandler, root, source, isDouble, append, sizes, rank );
#endif //FIELDML_PHDF5_ARRAY
    }
    
    return writer;
}


ArrayDataWriter *ArrayDataWriter::create( FieldmlErrorHandler *eHandler, const char *root, TextArrayDataSource *source, bool isDouble, bool append, int *sizes, int rank )
{
    return TextArrayDataWriter::create( eHandler, root, source, isDouble, append, sizes, rank );
}


ArrayDataWriter::ArrayDataWriter( FieldmlErrorHandler *_eHandler ) :
    eHandler( _eHandler )
{
}


ArrayDataWriter::~ArrayDataWriter()
{
}
