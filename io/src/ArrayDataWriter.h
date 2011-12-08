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

#ifndef H_ARRAY_DATA_WRITER
#define H_ARRAY_DATA_WRITER

#include <vector>

#include "FieldmlIoContext.h"

class ArrayDataWriter
{
protected:
    FieldmlIoContext *const context;

    ArrayDataWriter( FieldmlIoContext *_context );
public:
    virtual FmlIoErrorNumber writeIntSlab( const int *offsets, const int *sizes, const int *valueBuffer ) = 0;
    
    virtual FmlIoErrorNumber writeDoubleSlab( const int *offsets, const int *sizes, const double *valueBuffer ) = 0;
    
    //TODO Provide options for writing from 32/64 bit packed boolean arrays?
    virtual FmlIoErrorNumber writeBooleanSlab( const int *offsets, const int *sizes, const FmlBoolean *valueBuffer ) = 0;
    
    virtual FmlIoErrorNumber close() = 0;
    
    virtual ~ArrayDataWriter();
    
    static ArrayDataWriter *create( FieldmlIoContext *context, const std::string root, FmlObjectHandle source, FieldmlHandleType handleType, bool append, int *sizes, int rank );
};


#endif //H_ARRAY_DATA_WRITER
