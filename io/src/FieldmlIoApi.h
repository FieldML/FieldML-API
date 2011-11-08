/*
 * \file
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

#ifndef H_FIELDML_IO_API
#define H_FIELDML_IO_API

/**
 * \file
 * API notes:
 * 
 * If a function returns a FmlReaderHandle or FmlWriterHandle
 * it will return FML_INVALID_HANDLE on error.
 */


/*

 Typedefs

*/
#include "fieldml_api.h"

#include <stdint.h>

typedef int32_t FmlReaderHandle;                ///< A handle to a data reader.

typedef int32_t FmlWriterHandle;                ///< A handle to a data writer.

typedef int32_t FmlIoErrorNumber;               ///< A FieldML IO library error code.


/*

 Constants

*/
#define FML_IOERR_NO_ERROR              0       ///< No error.
#define FML_IOERR_CORE_ERROR            1200    ///< A call to the core API failed.
#define FML_IOERR_READ_ERROR            1201    ///< A read error was encountered during IO.
#define FML_IOERR_WRITE_ERROR           1202    ///< A write error was encountered during IO.
#define FML_IOERR_UNEXPECTED_EOF        1203    ///< An EOF was encountered when more data was expected.
#define FML_IOERR_NO_DATA               1204    ///< There is no data remaining.
#define FML_IOERR_UNSUPPORTED           1205    ///< Operation not supported for the given IO object.
#define FML_IOERR_RESOURCE_CLOSED       1206    ///< Operation attempted on a closed IO resource.
#define FML_IOERR_CLOSE_FAILED          1207    ///< Attempt to close an IO resource failed.
#define FML_IOERR_NONLOCAL_OBJECT       1208    ///< Attempt to perform an IO action on a nonlocal object.
#define FML_IOERR_UNKNOWN_OBJECT        1209    ///< Attempt to perform an IO action on an unknown object.
#define FML_IOERR_INVALID_LOCATION      1210    ///< Data source location was invalid.

#define FML_IOERR_INVALID_PARAMETER     1300    ///< Invalid parameter.

/*

 Types

*/


/*

 API

*/

#ifdef __cplusplus
extern "C" {

/**
 * Creates a new reader for the given data source's raw data. Fieldml_CloseReader() should be called
 * when the caller no longer needs to use it.
 * 
 * \see Fieldml_ReadIntSlab
 * \see Fieldml_ReadDoubleSlab
 * \see Fieldml_CloseReader
 */
FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Reads data from the multi-dimensional array specified by the given offsets and sizes into the given buffer. The first
 * size/offset is applied to the outermost index, and so on.
 */
FmlIoErrorNumber Fieldml_ReadIntSlab( FmlReaderHandle readerHandle, int *offsets, int *sizes, int *valueBuffer );


/**
 * Reads data from the multi-dimensional array specified by the given offsets and sizes into the given buffer. The first
 * size/offset is applied to the outermost index, and so on.
 */
FmlIoErrorNumber Fieldml_ReadDoubleSlab( FmlReaderHandle readerHandle, int *offsets, int *sizes, double *valueBuffer );


/**
 * Reads data from the multi-dimensional array specified by the given offsets and sizes into the given buffer. The first
 * size/offset is applied to the outermost index, and so on.
 */
FmlIoErrorNumber Fieldml_ReadBooleanSlab( FmlReaderHandle readerHandle, int *offsets, int *sizes, bool *valueBuffer );


/**
 * Closes the given data reader. The reader's handle should not be used after this call.
 * 
 * \see Fieldml_OpenReader
 */
FmlIoErrorNumber Fieldml_CloseReader( FmlReaderHandle readerHandle );


/**
 * Creates a new writer for the given data source's raw data. No post-processing will be done on the
 * provided values. It is up to the application to ensure that the data source's description is consistent with the data
 * actually being written, and with any other data sources that use the same data resource.
 * Fieldml_CloseWriter() should be called when the caller no longer needs to use the writer. 
 * 
 * \see Fieldml_CloseWriter
 */
FmlWriterHandle Fieldml_OpenArrayWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle typeHandle, FmlBoolean append, int *sizes, int rank );

/**
 * Write out some integer values to the given data writer. The data will be interpreted as an n-dimensional array of
 * the given size, and written out at the given offset. The first
 * size/offset is applied to the outermost index, and so on.
 * 
 * \note For text-based arrays, there are limitations on the permissible offsets and sizes. 
 * 
 * \see Fieldml_OpenArrayWriter
 */
FmlIoErrorNumber Fieldml_WriteIntSlab( FmlWriterHandle writerHandle, int *offsets, int *sizes, int *valueBuffer );

/**
 * Write out some double-precision values to the given data writer. The data will be interpreted as an n-dimensional array of
 * the given size, and written out at the given offset. The first
 * size/offset is applied to the outermost index, and so on.
 * 
 * \note For text-based arrays, there are limitations on the permissible offsets and sizes. 
 * 
 * \see Fieldml_OpenArrayWriter
 */
FmlIoErrorNumber Fieldml_WriteDoubleSlab( FmlWriterHandle writerHandle, int *offsets, int *sizes, double *valueBuffer );


/**
 * Write out some boolean values to the given data writer. The data will be interpreted as an n-dimensional array of
 * the given size, and written out at the given offset. The first
 * size/offset is applied to the outermost index, and so on.
 * 
 * \note For text-based arrays, there are limitations on the permissible offsets and sizes. 
 * 
 * \see Fieldml_OpenArrayWriter
 */
FmlIoErrorNumber Fieldml_WriteBooleanSlab( FmlWriterHandle writerHandle, int *offsets, int *sizes, bool *valueBuffer );


/**
 * Closes the given data writer. The writer's handle cannot be used after this call.
 * 
 * \see Fieldml_OpenArrayWriter
 */
FmlIoErrorNumber Fieldml_CloseWriter( FmlWriterHandle writerHandle );

}

#endif // __cplusplus

#endif // H_FIELDML_IO_API
