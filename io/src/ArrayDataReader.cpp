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

#include "ArrayDataReader.h"
#include "Hdf5ArrayDataReader.h"
#include "TextArrayDataReader.h"

using namespace std;

ArrayDataReader * ArrayDataReader::create( FieldmlIoContext *context, const string root, FmlObjectHandle source )
{
    ArrayDataReader *reader = NULL;

    FmlObjectHandle resource = Fieldml_GetDataSourceResource( context->getSession(), source );
    string format;
    
    if( !StringUtil::safeString( Fieldml_GetDataResourceFormat( context->getSession(), resource ), format ) )
    {
        context->setError( FML_IOERR_CORE_ERROR );
    }
    else if( format == StringUtil::HDF5_NAME )
    {
#ifdef FIELDML_HDF5_ARRAY
        reader = Hdf5ArrayDataReader::create( context, root, source );
#endif //FIELDML_HDF5_ARRAY
    }
    else if( format == StringUtil::PHDF5_NAME )
    {
#ifdef FIELDML_PHDF5_ARRAY
        reader = Hdf5ArrayDataReader::create( context, root, source );
#endif //FIELDML_PHDF5_ARRAY
    }
    else if( format == StringUtil::PLAIN_TEXT_NAME )
    {
        reader = TextArrayDataReader::create( context, root, source );
    }
    else
    {
        context->setError( FML_IOERR_UNSUPPORTED );
    }
    
    return reader;
}


ArrayDataReader::ArrayDataReader( FieldmlIoContext *_context ) :
    context( _context )
{
}


ArrayDataReader::~ArrayDataReader()
{
    delete context;
}
