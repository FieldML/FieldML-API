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
#include "DataReader.h"
#include "StreamDataReader.h"
#include "ArrayDataReader.h"

using namespace std;


DataReader *DataReader::createTextReader( FieldmlErrorHandler *eHandler, const char *root, TextDataSource *dataSource )
{
    FieldmlInputStream *stream = NULL;
    
    if( dataSource->resource->type == DATA_RESOURCE_TEXT_FILE )
    {
        TextFileDataResource *fileDataResource = (TextFileDataResource*)dataSource->resource;

        //Resolve this href properly, perhaps at parse-time?
        const string filename = makeFilename( root, fileDataResource->href );
        stream = FieldmlInputStream::createTextFileStream( filename );

        if( stream != NULL )
        {
            for( int i = 0; i < dataSource->firstLine - 1; i++ )
            {
                stream->skipLine();
            }
        }
    }
    else if( dataSource->resource->type == DATA_RESOURCE_TEXT_INLINE )
    {
        TextInlineDataResource *inlineDataResource = (TextInlineDataResource*)dataSource->resource;
        stream = FieldmlInputStream::createStringStream( inlineDataResource->inlineString );
    }
    
    if( stream == NULL )
    {
        return NULL;
    }
    
    return new StreamDataReader( stream, eHandler, dataSource->count, dataSource->length, dataSource->head, dataSource->tail );
}


DataReader *DataReader::createArrayReader( FieldmlErrorHandler *eHandler, const char *root, ArrayDataSource *source )
{
    if( source->resource->type != DATA_RESOURCE_ARRAY )
    {
        return NULL;
    }
    
    ArrayDataResource *resource = (ArrayDataResource*)source->resource;
    
    return ArrayDataReader::create( eHandler, root, resource, source );
}


DataReader *DataReader::create( FieldmlErrorHandler *eHandler, const char *root, DataSource *dataSource )
{
    if( ( dataSource == NULL ) || ( root == NULL ) || ( eHandler == NULL ) )
    {
        return NULL;
    }

    if( dataSource->type == DATA_SOURCE_TEXT )
    {
        TextDataSource *textSource = (TextDataSource*)dataSource;
        return createTextReader( eHandler, root, textSource );
    }
    else if( dataSource->type == DATA_SOURCE_ARRAY )
    {
        ArrayDataSource *arraySource = (ArrayDataSource*)dataSource;
        return createArrayReader( eHandler, root, arraySource );
    }
    
    return NULL;
}


DataReader::~DataReader()
{
}
