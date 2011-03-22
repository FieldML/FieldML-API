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
#ifndef H_FIELDML_API
#define H_FIELDML_API

/**
     API notes:
     
     If a function returns a FmlHandle or FmlObjectHandle, it will return
     FML_INVALID_HANDLE on error.
     
     All FieldML objects are referred to only by their integer handle.
     
     All handles are internally type-checked. If an inappropriate handle is
     passed to a function, the function will return -1, NULL or
     FML_INVALID_HANDLE as applicable, and the lastError value set.
     
     For each string-getter, there is a corresponding string-copier. The string-copier
     copies the relevant string into a provided buffer and returns the number of characters
     copied. In cases where the getter returns NULL, the corresponding copier returns 0.
     
     All object names must be unique.
 */

/*

 Constants

*/
#define FML_INVALID_HANDLE -1

#define FML_MAJOR_VERSION               0
#define FML_MINOR_VERSION               3
#define FML_DOT_VERSION                 1

#define FML_ERR_NO_ERROR                0
#define FML_ERR_UNKNOWN_HANDLE          1000
#define FML_ERR_UNKNOWN_OBJECT          1001
#define FML_ERR_INVALID_OBJECT          1002
#define FML_ERR_INCOMPLETE_OBJECT       1003
#define FML_ERR_MISCONFIGURED_OBJECT    1004
#define FML_ERR_ACCESS_VIOLATION        1005
#define FML_ERR_FILE_READ               1006
#define FML_ERR_FILE_WRITE              1007

//Used for giving the user precise feedback on bad parameters passed to the API
//Only used for parameters other than the FieldML handle and object handle parameters.
#define FML_ERR_INVALID_PARAMETER_1     1101
#define FML_ERR_INVALID_PARAMETER_2     1102
#define FML_ERR_INVALID_PARAMETER_3     1103
#define FML_ERR_INVALID_PARAMETER_4     1104
#define FML_ERR_INVALID_PARAMETER_5     1105
#define FML_ERR_INVALID_PARAMETER_6     1106
#define FML_ERR_INVALID_PARAMETER_7     1107
#define FML_ERR_INVALID_PARAMETER_8     1108

#define FML_ERR_IO_READ_ERR             1201
#define FML_ERR_IO_UNEXPECTED_EOF       1202
#define FML_ERR_IO_NO_DATA              1203
#define FML_ERR_IO_UNEXPECTED_DATA      1204

#define FML_ERR_UNSUPPORTED             2000  //Used for operations that are valid, but not yet implemented.

/*

 Types

*/

enum DataFileType
{
    TYPE_UNKNOWN,
    TYPE_TEXT,                  // Text file with CSV/space delimited numbers. Offset is numbers.
    TYPE_LINES,                 // Formatted text file. Offset is lines. CSV/space delimited numbers expected at offset.
};


enum DataDescriptionType
{
    DESCRIPTION_UNKNOWN,
    DESCRIPTION_SEMIDENSE,
};


enum DataLocationType
{
    LOCATION_UNKNOWN,
    LOCATION_INLINE,
    LOCATION_FILE,
};


enum FieldmlHandleType
{
    FHT_UNKNOWN,
    
    FHT_ENSEMBLE_TYPE,
    FHT_CONTINUOUS_TYPE,
    FHT_MESH_TYPE,
    FHT_ABSTRACT_EVALUATOR,
    FHT_EXTERNAL_EVALUATOR,
    FHT_REFERENCE_EVALUATOR,
    FHT_PARAMETER_EVALUATOR,
    FHT_PIECEWISE_EVALUATOR,
    FHT_AGGREGATE_EVALUATOR,
    FHT_ELEMENT_SEQUENCE,
    
    //These are stand-in types used to allow forward-declaration during parsing.
    FHT_UNKNOWN_TYPE,
    FHT_UNKNOWN_EVALUATOR,
    FHT_UNKNOWN_ELEMENT_SEQUENCE
};


typedef long FmlReaderHandle;

typedef long FmlWriterHandle;

typedef int FmlObjectHandle;

typedef long FmlHandle;

/*

 API

*/

#ifdef __cplusplus
extern "C" {

/**
 *      Parses the given XML file, and returns a handle to the parsed data. This
 *      handle is then used for all subsequent API calls.
 *    
 *      NOTE: At the moment, the debug flag can only be set after a parse.
 *      Errors during the parse can be obtains via the error log, but at the
 *      moment they're all just strings. 
 */
FmlHandle Fieldml_CreateFromFile( const char *filename );

/**
 *      Creates an empty FieldML handle. If libraryLocation is "library_0.3.xml", the internal 
 *      library will be used, otherwise libraryLocation will be interpreted as the relative
 *      location of a FieldML file, and parsed as part of the FieldML handle's creation.
 *      
 *      Data files will be created at the given location.
 */
FmlHandle Fieldml_Create( const char *location, const char *name, const char *libraryLocation );

/**
 *      Sets/clears the debug flag. If non-zero, errors are logged to stdout.
 */
int Fieldml_SetDebug( FmlHandle handle, int debug );

/**
 *      Returns the error code generated by the last API call. Note that on success,
 *      the lastError value is set to FML_ERR_NO_ERROR.
 */
int Fieldml_GetLastError( FmlHandle handle );

/**
 *      Writes the contents of the given FieldML handle to the given filename as
 *      an XML file.
 */
int Fieldml_WriteFile( FmlHandle handle, const char *filename );


/**
 *      Frees all resources associated with the given handle. The handle should
 *      not be used after this call.
 */
void Fieldml_Destroy( FmlHandle handle );


/**
 *      Returns the name of the region.
 */
const char * Fieldml_GetName( FmlHandle handle );
int Fieldml_CopyName( FmlHandle handle, char *buffer, int bufferLength );


/**
 *      Returns the name of the region's library.
 *      
 *      Currently, each region may only use a single library, which is specified
 *      either as the string "library_0.3.xml" or as a filename relative to the
 *      enclosing file's location.
 */
const char * Fieldml_GetLibraryName( FmlHandle handle );
int Fieldml_CopyLibraryName( FmlHandle handle, char *buffer, int bufferLength );

/**
 *      Returns the number of parsing errors encountered by the given handle
 *      during XML parsing.
 */
int Fieldml_GetErrorCount( FmlHandle handle );


/**
 *      Returns the nth parsing error string for the given handle.
 */
const char * Fieldml_GetError( FmlHandle handle, int errorIndex );
int Fieldml_CopyError( FmlHandle handle, int errorIndex, char *buffer, int bufferLength );


/**
 *      Returns the total number of objects, or zero if there are none.
 */
int Fieldml_GetTotalObjectCount( FmlHandle handle );


/**
 *      Returns a handle to the nth object.
 */
FmlObjectHandle Fieldml_GetObjectByIndex( FmlHandle handle, const int objectIndex );


/**
 *      Returns the number of objects of the given type, or zero if there are none.
 */
int Fieldml_GetObjectCount( FmlHandle handle, FieldmlHandleType type );


/**
 *      Returns a handle to the nth object of the given type.
 */
FmlObjectHandle Fieldml_GetObject( FmlHandle handle, FieldmlHandleType type, int objectIndex );


/**
 *      Returns the type of the given object.
 */
FieldmlHandleType Fieldml_GetObjectType( FmlHandle handle, FmlObjectHandle object );


/**
 *      Returns a handle to the object with the given name, or FML_INVALID_HANDLE if
 *      there is no such object.
 */
FmlObjectHandle Fieldml_GetObjectByName( FmlHandle handle, const char * name );


/**
 *      Returns 1 if the given object is local, 0 if not, or -1 on error.
 *      
 *      Note that imported objects are not local, and only local objects
 *      get serialized by Fieldml_WriteFile.
 */
int Fieldml_IsObjectLocal( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *   Returns the name of the given object, or NULL if there is no such object.
 */
const char * Fieldml_GetObjectName( FmlHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyObjectName( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength );


/**
 * Associate a client-defined integer with the given object. This value is initialized to 0 when
 * the object is created, but is otherwise ignored by the API.
 */
int Fieldml_SetObjectInt( FmlHandle handle, FmlObjectHandle object, int value );


/**
 * Returns the client-defined integer value associated with the given object.
 */
int Fieldml_GetObjectInt( FmlHandle handle, FmlObjectHandle object );


/**
 *      Returns the handle of the given type's component ensemble.
 */
FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Helper function that returns the element count of the component type of the
 * given type.
 */
int Fieldml_GetTypeComponentCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Creates an ensemble type with the given name.
 */
FmlObjectHandle Fieldml_CreateEnsembleType( FmlHandle handle, const char * name, const int isComponentEnsemble );


/**
 *      Creates a continuous type with the given name and component ensemble.
 */
FmlObjectHandle Fieldml_CreateContinuousType( FmlHandle handle, const char * name, FmlObjectHandle componentHandle );

/**
 *      Creates a mesh type with the given name, and with the given dimensionality.
 *      Each mesh has its own unique element and xi type, which can be accessed by the relevant functions.
 *      Because the xi and element types have a name based on the mesh name, care must be taken to ensure
 *      that neither the mesh's name, nor it's element or xi type names are already in use. If they are,
 *      this function will return an error.
 */
FmlObjectHandle Fieldml_CreateMeshType( FmlHandle handle, const char * name, FmlObjectHandle xiComponentHandle );


/**
 *      Returns the handle of the given mesh type's xi type. This is a unique, n-dimensional
 *      continuous type with the component type specified when the mesh was created. If the
 *      mesh's name is "*", the xi type's name is "*.xi"
 */
FmlObjectHandle Fieldml_GetMeshXiType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns the component handle of the given mesh type's xi type.
 */
FmlObjectHandle Fieldml_GetMeshXiComponentType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *     Returns the handle of the given mesh type's element type. This is a unique ensemble
 *     type whose elements are specified by the user. If the mesh's name is "*", the element type's
 *     name is "*.element"
 */
FmlObjectHandle Fieldml_GetMeshElementType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns a string describing the shape of the element in the given mesh.
 *   
 *      NOTE: At the moment, shapes are only described via strings. This may change in the future.
 */
const char * Fieldml_GetMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault );
int Fieldml_CopyMeshElementShape( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault, char *buffer, int bufferLength );


/**
 *      Sets the default shape for the mesh. This should be set unless all elements have been explicitly
 *      assigned a shape.
 */
int Fieldml_SetMeshDefaultShape( FmlHandle handle, FmlObjectHandle mesh, const char * shape );


/**
 *      Gets the default shape for the mesh.
 */
const char *Fieldml_GetMeshDefaultShape( FmlHandle handle, FmlObjectHandle mesh );
int Fieldml_CopyMeshDefaultShape( FmlHandle handle, FmlObjectHandle mesh, char * buffer, int bufferLength );


/**
 *      Sets the shape of the given element.
 */
int Fieldml_SetMeshElementShape( FmlHandle handle, FmlObjectHandle mesh, int elementNumber, const char * shape );


/**
 * Returns 1 if the ensemble type is a component ensemble, 0 if not, -1 on error.
 */
int Fieldml_IsEnsembleComponentType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
     Returns the value type of the given evaluator.
 */
FmlObjectHandle Fieldml_GetValueType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates a abstract evaluator. An abstract evaluator is a placeholder for a concrete evaluator, and
 * can be bound just as concrete ones can. Only abstract evaluators can be the target of a bind.
 */
FmlObjectHandle Fieldml_CreateAbstractEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Creates an external evaluator. An external evaluator's behaviour is defined by the author of the FieldML
 * document in which it appears, rather than in terms of other FieldML objects. Currently, the only option
 * is 'definition by specification'.
 */
FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Creates a new parameter set. A parameter set contains a store of literal values, indexed by a set
 * of ensemble-valued evaluators. The format and location of the store is intended to be very flexible,
 * and include the ability to describe 3rd-party formats such as HDF5, and allow for locations that
 * refer to inline data, as well as files on the local filesystem, or over the network.
 */
FmlObjectHandle Fieldml_CreateParametersEvaluator( FmlHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Returns the location of the raw data used by the parameter set.
 */
DataLocationType Fieldml_GetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Set the location of the raw data used by the parameter set. At the moment, only LOCATION_FILE and
 * LOCATION_INLINE is supported.
 */
int Fieldml_SetParameterDataLocation( FmlHandle handle, FmlObjectHandle objectHandle, DataLocationType location );


/**
 * Appends some data to the parameter set's inline data. The parameter set's data location must have previously
 * been set to LOCATION_INLINE.
 */
int Fieldml_AddParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle, const char *data, const int length );


/**
 * Returns the number of characters in the parameter set's inline data.
 */
int Fieldml_GetParameterInlineDataLength( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns a pointer to the parameter set's inline data.
 */
const char *Fieldml_GetParameterInlineData( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies a section of the parameter set's inline data into the given buffer, starting from the given offset, and ending
 * either when the buffer is full, or the end of the inline data is reached.
 */
int Fieldml_CopyInlineParameterData( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset );

/**
 * Sets the information for the parameter set's file data. The parameter set's data location must have previously
 * been set to LOCATION_FILE. Currently, the offset value only works for TYPE_LINES files when reading. It is up
 * to the caller to correctly set the offset when writing files. TYPE_TEXT is only partially supported.
 * TYPE_BINARY is not yet supported.
 */
int Fieldml_SetParameterFileData( FmlHandle handle, FmlObjectHandle objectHandle, const char * filename, DataFileType type, int offset );


/**
 * Returns the filename of the parameter set's file data. The parameter set's data location must have previously
 * been set to LOCATION_FILE.
 */
const char *Fieldml_GetParameterDataFilename( FmlHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyParameterDataFilename( FmlHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength );


/**
 * Returns the offset for the parameter set's file data. The parameter set's data location must have previously
 * been set to LOCATION_FILE.
 */
int Fieldml_GetParameterDataOffset( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the file type for the parameter set's file data. The parameter set's data location must have previously
 * been set to LOCATION_FILE.
 */
DataFileType Fieldml_GetParameterDataFileType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the description of the parameter set's raw data. At the moment, only DESCRIPTION_SEMIDENSE is supported.
 */
int Fieldml_SetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description );


/**
 *  Returns the data description type of the given parameter evaluator.
 */
DataDescriptionType Fieldml_GetParameterDataDescription( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Adds a dense index evaluator to the given parameter set's semidense data description.
 */
int Fieldml_AddDenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle setHandle );

/**
 * Adds a sparse index evaluator to the given parameter set's semidense data description.
 */
int Fieldml_AddSparseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle );

/**
 *   Returns the number of sparse or dense index evaluators of the semidense data store
 *   associated with the given parameter evaluator.
 */
int Fieldml_GetSemidenseIndexCount( FmlHandle handle, FmlObjectHandle objectHandle, int isSparse );


/**
 *   Returns the handle of the nth sparse or dense index evaluator of the semidense data
 *   store associated with the given parameter evaluator.
 */
FmlObjectHandle Fieldml_GetSemidenseIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int indexIndex, int isSparse );


/**
 * Sets the swizzle indexes for the given parameter set. The swizzle is only applied to the innermost dense index.
 * 
 * NOTE: Swizzles are an experiment feature, and are likely to be removed.
 */
int Fieldml_SetSwizzle( FmlHandle handle, FmlObjectHandle objectHandle, const int *buffer, int count );


/**
 * Gets the number of swizzle indexes for the given parameter set, or zero if no swizzle is defined.
 * 
 * NOTE: Swizzles are an experiment feature, and are likely to be removed.
 */
int Fieldml_GetSwizzleCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the swizzle indexes, if present.
 * 
 * NOTE: Swizzles are an experiment feature, and are likely to be removed.
 */
const int *Fieldml_GetSwizzleData( FmlHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopySwizzleData( FmlHandle handle, FmlObjectHandle objectHandle, int *buffer, int bufferLength );


/**
 * Creates a new piecewise evaluator. Evaluators used by the piecewise evaluator
 * must all have the same value type as the piecewise itself. Piecewise evaluators need not be defined for all
 * possible index values.
 */
FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Creates a new aggregate evaluator.
 * The aggregate evaluator's index evaluators must all be scalar continuous.
 * The value is obtained by aggregating the scalar evaluators over the
 * value domain's component ensemble. There must be an entry (or a default) for each index value.
 */
FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Set the index evaluator for the given piecewise or aggregate evaluator.
 * 
 * The aggregate's index evaluator must have the same ensemble type as the component ensemble of its value type.
 */
int Fieldml_SetIndexEvaluator( FmlHandle handle, FmlObjectHandle valueType, int index, FmlObjectHandle indexHandle );

/*
 * Experimental.
 */
/*
int Fieldml_GetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle parametersHandle, int index );
*/

/*
 * Experimental.
 */
/*
int Fieldml_SetSemidenseIndexSet( FmlHandle handle, FmlObjectHandle parametersHandle, int index, FmlObjectHandle setHandle );
*/

/**
 * Sets the default evaluator for the given piecewise or aggregate evaluator.
 */
int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator );


/**
 * Returns the default evaluator for the given piecewise or aggregate evaluator.
 */
int Fieldml_GetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the evaluator on the given index for the given aggregate or piecewise evaluator.
 * 
 * Setting the evaluator handle to FML_INVALID_HANDLE removes the index-evaluator association.
 */
int Fieldml_SetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int element, FmlObjectHandle evaluator );


/**
 *  Returns the number of element->evaluator delegations for the given
 *  piecewise or aggregate evaluator.
 */
int Fieldml_GetEvaluatorCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the element number for the nth element->evaluator delegation in
 *  the given piecewise/aggregate evaluator.
 */
int Fieldml_GetEvaluatorElement( FmlHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 *  Returns the evaluator handle for the nth element->evaluator delegation in
 *  the given piecewise/aggregate evaluator.
 */
FmlObjectHandle Fieldml_GetEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 * Returns the evaluator for the given element number in the given piecewise or aggregate
 * evaluator, or FML_INVALID_HANDLE if there is none defined.
 */
FmlObjectHandle Fieldml_GetElementEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int elementNumber, int allowDefault );


/**
    Returns the number of indexes used by the given evaluator.
    
    NOTE: Only defined for piecewise, aggregate and parameter evaluators.
    
    NOTE: For piecewise or aggreate evaluators, this is currently always one. For parameter evaluators,
    it depends on the data store.
 */
int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the evaluator of the nth index used by the given evaluator.
 *  
 *  NOTE: Only defined for piecewise, aggregate and parameter evaluators.
 */
FmlObjectHandle Fieldml_GetIndexEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int indexIndex );


/**
 * Creates a reference evaluator. Reference evaluators delegate their evaluation to another evaluator, and may bind
 * domains and evaluators before doing so.
 */
FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlHandle handle, const char * name, FmlObjectHandle sourceEvaluator );


/**
 * Gets the source evaluator which the given evaluator references.
 */
FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the number of abstract evaluators used by the given evaluator. 
 */
int Fieldml_GetVariableCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the nth abstract evaluator used by the given evaluator. 
 */
FmlObjectHandle Fieldml_GetVariable( FmlHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 *  Returns the nth abstract evaluator used by the given evaluator. 
 */
int Fieldml_AddVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle );


/**
 * Adds an bind to the given evaluator. Only abstract evaluators can be bound. 
 */
int Fieldml_SetBind( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle unboundEvaluator, FmlObjectHandle evaluator );


/**
 *  Returns the number of bind used by the given evaluator. 
 */
int Fieldml_GetBindCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the evaulator used by the nth bind of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetBindVariable( FmlHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 *  Returns the source evaluator used by the nth bind of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetBindEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 * Returns the abstract evaluator to which to given evaluator is bound to in the given evaluator.
 */
FmlObjectHandle Fieldml_GetBindByVariable( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle );


/**
 * Experimental.
 */
/*
FmlObjectHandle Fieldml_CreateEnsembleElementSequence( FmlHandle handle, const char * name, FmlObjectHandle valueType );
*/

/**
 * Add the given elements to the given ensemble.
 */
int Fieldml_AddEnsembleElements( FmlHandle handle, FmlObjectHandle setHandle, const int * elements, const int elementCount );


/**
 * Add the given element range to the given ensemble.
 */
int Fieldml_AddEnsembleElementRange( FmlHandle handle, FmlObjectHandle objectHandle, const int minElement, const int maxElement, const int stride );


/**
 * Get the number of elements in the given ensemble
 */
int Fieldml_GetElementCount( FmlHandle handle, FmlObjectHandle setHandle );


/**
 * Get the nth element from the given ensemble, or -1 on error.
 */
int Fieldml_GetElementEntry( FmlHandle handle, FmlObjectHandle setHandle, const int index );


/**
 * Get n element from the given offset. Returns the number of elements actually retrieved, as this may be less
 * than the number requested. 
 */
int Fieldml_GetElementEntries( FmlHandle handle, FmlObjectHandle setHandle, const int firstIndex, int * elements, const int count );





/**
 * Creates a new reader for the given parameter evaluator's raw data.
 */
FmlReaderHandle Fieldml_OpenReader( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Read in the indexes corresponding to the next block of dense data. If there are no sparse indexes
 * associated with this parameter set, this method can only be called once, and will leave the index
 * buffer untouched. Only valid for a parameters object using DESCRIPTION_SEMIDENSE data.
 * 
 * Returns an error code, or FML_ERR_NO_ERROR on success.
 */
int Fieldml_ReadIndexSet( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer );


/**
 * Reads in some values from the current block of dense data. If the first dense index's value type is a component
 * ensemble, the buffer must be big enough to hold a complete set of values indexable by that component ensemble.
 * 
 * Returns the number of values read, or -1 on error.
 */
int Fieldml_ReadIntValues( FmlHandle handle, FmlReaderHandle reader, int *valueBuffer, int bufferSize );


/**
 * Reads in some values from the current block of dense data. If the first dense index's value type is a component
 * ensemble, the buffer must be big enough to hold a complete set of values indexable by that component ensemble.
 * 
 * Returns the number of values read, or -1 on error.
 */
int Fieldml_ReadDoubleValues( FmlHandle handle, FmlReaderHandle reader, double *valueBuffer, int bufferSize );


/**
 * Closes the given raw data reader.
 */
int Fieldml_CloseReader( FmlHandle handle, FmlReaderHandle reader );


/**
 * Creates a new writer for the given parameter evaluator's raw data.
 */
FmlWriterHandle Fieldml_OpenWriter( FmlHandle handle, FmlObjectHandle objectHandle, int append );


/**
 * Write out the indexes corresponding to the next block of dense data. If there are no sparse indexes
 * associated with this parameter set, this method can only be called once, and will do nothing.
 * Only valid for a parameters object using DESCRIPTION_SEMIDENSE data.
 * 
 * Returns an error code, or FML_ERR_NO_ERROR on success.
 */
int Fieldml_WriteIndexSet( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer );


/**
 * Write out some values for the current block of dense data. If the first dense index's value type is a component
 * ensemble, the buffer must be big enough to hold a complete set of values indexable by that component ensemble.
 * 
 * Returns the number of values written, or -1 on error.
 */
int Fieldml_WriteIntValues( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer, int bufferSize );


/**
 * Write out some values from the current block of dense data. If the first dense index's value type is a component
 * ensemble, the buffer must be big enough to hold a complete set of values indexable by that component ensemble.
 * 
 * Returns the number of values written, or -1 on error.
 */
int Fieldml_WriteDoubleValues( FmlHandle handle, FmlWriterHandle writer, double *indexBuffer, int bufferSize );


/**
 * Closes the given raw data writer.
 */
int Fieldml_CloseWriter( FmlHandle handle, FmlWriterHandle writer );

}
#endif // __cplusplus

#endif // H_FIELDML_API
