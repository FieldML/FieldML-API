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

#ifndef H_TEXT_ARRAY_DATA_READER
#define H_TEXT_ARRAY_DATA_READER

#include "FieldmlIoContext.h"
#include "ArrayDataReader.h"
#include "InputStream.h"

class BufferReader;

class TextArrayDataReader :
    public ArrayDataReader
{
private:
    bool closed;
    
    FieldmlInputStream * const stream;
    
    const FmlObjectHandle source;

    int sourceRank;
    
    int *sourceSizes;
    
    int *sourceRawSizes;
    
    int *sourceOffsets;
    
    std::string sourceLocation;
    
    int nextOutermostOffset;
    
    //The seek position of the start of the array data. This is a minor optimization to save us from having to line-skip for each read.
    long startPos;

    TextArrayDataReader( FieldmlIoContext *_context, FieldmlInputStream *_stream, FmlObjectHandle source, int _sourceRank );
    
    bool checkDimensions( const int *offsets, const int *sizes );
    
    bool applyOffsets( const int *offsets, const int *sizes, int depth, bool isHead );
    
    FmlIoErrorNumber readPreSlab( const int *offsets, const int *sizes );
    
    FmlIoErrorNumber readSlice( const int *offsets, const int *sizes, int depth, BufferReader &reader );
    
    FmlIoErrorNumber readSlab( const int *offsets, const int *sizes, BufferReader &reader );
    
    FmlIoErrorNumber skipPreamble();

public:
    virtual FmlIoErrorNumber readIntSlab( const int *offsets, const int *sizes, int *valueBuffer );
    
    virtual FmlIoErrorNumber readDoubleSlab( const int *offsets, const int *sizes, double *valueBuffer );
    
    virtual FmlIoErrorNumber readBooleanSlab( const int *offsets, const int *sizes, FmlBoolean *valueBuffer );
    
    virtual FmlIoErrorNumber close();
    
    virtual ~TextArrayDataReader();
    
    static TextArrayDataReader *create( FieldmlIoContext *_context, const std::string root, FmlObjectHandle source );
};


#endif //H_TEXT_ARRAY_DATA_READER
