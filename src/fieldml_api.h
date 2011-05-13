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
     
     If a function returns a FmlSessionHandle, FmlReaderHandle, FmlWriterHandle or
     FmlObjectHandle, it will return FML_INVALID_HANDLE on error.
     
     All FieldML objects are referred to only by their handle.
     
     All handles are internally type-checked. If an inappropriate handle is
     passed to a function, the function will return -1, NULL or
     FML_INVALID_HANDLE as applicable, and the lastError value set.
     
     For each string-getter, there is a corresponding string-copier. The string-copier
     copies the relevant string into a provided buffer and returns the number of characters
     copied. In cases where the getter returns NULL, the corresponding copier returns 0.
     
     All object names must be unique within their region.
 */


/*

 Typedefs

*/
#include <stdint.h>

typedef int32_t FmlSessionHandle;

typedef int32_t FmlReaderHandle;

typedef int32_t FmlWriterHandle;

typedef int32_t FmlObjectHandle;

typedef int32_t FmlErrorNumber;

//NOTE When used as a return value, FmlBoolean-valued functions are ternary, with -1 indictating error.
typedef int32_t FmlBoolean;

//NOTE It may be necessary to support 64-bit ensemble values in the future.
typedef int32_t FmlEnsembleValue;


/*

 Constants

*/
#define FML_INVALID_HANDLE -1

#define FML_MAJOR_VERSION               0
#define FML_MINOR_VERSION               4
#define FML_DOT_VERSION                 0

#define FML_ERR_NO_ERROR                0
#define FML_ERR_UNKNOWN_HANDLE          1000
#define FML_ERR_UNKNOWN_OBJECT          1001
#define FML_ERR_INVALID_OBJECT          1002
#define FML_ERR_INCOMPLETE_OBJECT       1003
#define FML_ERR_MISCONFIGURED_OBJECT    1004
#define FML_ERR_ACCESS_VIOLATION        1005
#define FML_ERR_FILE_READ               1006
#define FML_ERR_FILE_WRITE              1007
#define FML_ERR_NAME_COLLISION          1008
#define FML_ERR_INVALID_REGION          1009
#define FML_ERR_NONLOCAL_OBJECT         1010

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


enum EnsembleMembersType
{
    MEMBER_RANGE,
    MEMBER_LIST_DATA,
    MEMBER_RANGE_DATA,
    MEMBER_STRIDE_RANGE_DATA,
    MEMBER_UNKNOWN,
};

enum DataDescriptionType
{
    DESCRIPTION_UNKNOWN,
    DESCRIPTION_SEMIDENSE,
};


enum DataResourceType
{
    DATA_RESOURCE_UNKNOWN,
    DATA_RESOURCE_TEXT_INLINE,
    DATA_RESOURCE_TEXT_FILE,
    //DATA_RESOURCE_HDF5_FILE,
};


enum DataSourceType
{
    DATA_SOURCE_UNKNOWN,
    DATA_SOURCE_TEXT,
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
    FHT_DATA_RESOURCE,
    FHT_DATA_SOURCE,
};


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
FmlSessionHandle Fieldml_CreateFromFile( const char *filename );

/**
 *      Creates an empty FieldML handle.
 *      
 *      Data files will be created at the given location.
 */
FmlSessionHandle Fieldml_Create( const char *location, const char *name );

/**
 *      Sets/clears the debug flag. If non-zero, errors are logged to stdout.
 */
FmlErrorNumber Fieldml_SetDebug( FmlSessionHandle handle, int debug );

/**
 *      Returns the error code generated by the last API call. Note that on success,
 *      the lastError value is set to FML_ERR_NO_ERROR.
 */
FmlErrorNumber Fieldml_GetLastError( FmlSessionHandle handle );

/**
 *      Writes the contents of the given FieldML handle to the given filename as
 *      an XML file.
 */
FmlErrorNumber Fieldml_WriteFile( FmlSessionHandle handle, const char *filename );


/**
 *      Frees all resources associated with the given handle. The handle will
 *      become invalid after this call.
 */
void Fieldml_Destroy( FmlSessionHandle handle );


/**
 * Frees any string returned by a char* valued Fieldml_Get function.
 */
FmlErrorNumber Fieldml_FreeString( char *string );


/**
 *      Returns the name of the region.
 */
const char * Fieldml_GetRegionName( FmlSessionHandle handle );
int Fieldml_CopyRegionName( FmlSessionHandle handle, char *buffer, int bufferLength );


/**
 *      Returns the number of parsing errors encountered by the given handle
 *      during XML parsing.
 */
int Fieldml_GetErrorCount( FmlSessionHandle handle );


/**
 *      Returns the nth parsing error string for the given handle.
 */
const char * Fieldml_GetError( FmlSessionHandle handle, int index );
int Fieldml_CopyError( FmlSessionHandle handle, int errorIndex, char *buffer, int bufferLength );


/**
 *      Returns the total number of objects, or zero if there are none.
 */
int Fieldml_GetTotalObjectCount( FmlSessionHandle handle );


/**
 *      Returns a handle to the nth object.
 */
FmlObjectHandle Fieldml_GetObjectByIndex( FmlSessionHandle handle, const int objectIndex );


/**
 *      Returns the number of objects of the given type, or zero if there are none.
 */
int Fieldml_GetObjectCount( FmlSessionHandle handle, FieldmlHandleType type );


/**
 *      Returns a handle to the nth object of the given type.
 */
FmlObjectHandle Fieldml_GetObject( FmlSessionHandle handle, FieldmlHandleType type, int objectIndex );


/**
 *      Returns the type of the given object.
 */
FieldmlHandleType Fieldml_GetObjectType( FmlSessionHandle handle, FmlObjectHandle object );


/**
 *      Returns a handle to the object with the given local name, or FML_INVALID_HANDLE if
 *      there is no such object.
 */
FmlObjectHandle Fieldml_GetObjectByName( FmlSessionHandle handle, const char * name );


/**
 * Returns a handle to the given declared name. This may differ from the object's
 * local name if the object has been imported.
 */
FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlSessionHandle handle, const char * name );

/**
 *      Returns 1 if the given object is local, 0 if not, or -1 on error.
 *      
 *      Note that imported objects are not local, and only local objects
 *      get serialized by Fieldml_WriteFile.
 */
FmlBoolean Fieldml_IsObjectLocal( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *   Returns the name of the given object, or NULL if there is no such object.
 */
const char * Fieldml_GetObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength );


/**
 * Returns the given object's declared name. This is the name the object was given in the
 * region in which is was declared, and may differ from the the object's local name.
 */
const char * Fieldml_GetObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength );


/**
 * Associate a client-defined integer with the given object. This value is initialized to 0 when
 * the object is created, but is otherwise ignored by the API.
 */
FmlErrorNumber Fieldml_SetObjectInt( FmlSessionHandle handle, FmlObjectHandle object, int value );


/**
 * Returns the client-defined integer value associated with the given object.
 */
int Fieldml_GetObjectInt( FmlSessionHandle handle, FmlObjectHandle object );


/**
 *      Returns the handle of the given type's component ensemble.
 */
FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Helper function that returns the element count of the component type of the
 * given type.
 */
int Fieldml_GetTypeComponentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *      Creates an ensemble type with the given name.
 */
FmlObjectHandle Fieldml_CreateEnsembleType( FmlSessionHandle handle, const char * name );


/**
 *      Creates a continuous type with the given name and component ensemble.
 */
FmlObjectHandle Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name );


/**
 * Create the indexing ensemble for the given continuous type. The result is a newly initialized ensemble type,
 * which must be configured in the same manner as a 'top level' ensemble.
 * 
 * NOTE: A continuous type need not have an indexing ensemble, in which case it is equivalent to a scalar value type.
 */
FmlObjectHandle Fieldml_CreateContinuousTypeComponents( FmlSessionHandle handle, FmlObjectHandle typeHandle, const char *name, const int count );

/**
 *      Creates a mesh type with the given name, and with the given dimensionality.
 *      Each mesh has its own unique element and chart type, which can be accessed by the relevant functions.
 *      Because the chart and element types have a name based on the mesh name, care must be taken to ensure
 *      that neither the mesh's name, nor it's element or chart type names are already in use. If they are,
 *      this function will return an error.
 */
FmlObjectHandle Fieldml_CreateMeshType( FmlSessionHandle handle, const char * name );

/**
 * Create the element ensemble for the given mesh type. The result is a newly initialized ensemble type,
 * which must be configured in the same manner as a 'top level' ensemble.
 * 
 * NOTE: A mesh type must have an element ensemble, and will be considered misconfigured if not.
 */
FmlObjectHandle Fieldml_CreateMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char *name );

/**
 * Create the chart type for the given mesh type. The result is a newly initialized continuous type,
 * which must be configured in the same manner as a 'top level' continuous type, including its indexing
 * ensemble.
 * 
 * NOTE: A mesh type must have a chart type, and will be considered misconfigured if not.
 */
FmlObjectHandle Fieldml_CreateMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char *name );


/**
 *      Returns the handle of the given mesh type's chart type. This is a unique, n-dimensional
 *      continuous type with the component type specified when the mesh was created.
 */
FmlObjectHandle Fieldml_GetMeshChartType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns the component handle of the given mesh type's chart type.
 */
FmlObjectHandle Fieldml_GetMeshChartComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *     Returns the handle of the given mesh type's element type. This is a unique ensemble
 *     type whose elements are specified by the user.
 */
FmlObjectHandle Fieldml_GetMeshElementsType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns a string describing the shape of the element in the given mesh.
 *   
 *      NOTE: At the moment, shapes are only described via strings. This may change in the future.
 */
const char * Fieldml_GetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault );
int Fieldml_CopyMeshElementShape( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault, char *buffer, int bufferLength );


/**
 *      Sets the default shape for the mesh. This should be set unless all elements have been explicitly
 *      assigned a shape.
 */
FmlErrorNumber Fieldml_SetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle mesh, const char * shape );


/**
 *      Gets the default shape for the mesh.
 */
const char * Fieldml_GetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle mesh, char * buffer, int bufferLength );


/**
 *      Sets the shape of the given element.
 */
FmlErrorNumber Fieldml_SetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle mesh, FmlEnsembleValue elementNumber, const char * shape );


/**
 * Returns 1 if the ensemble type is a component ensemble, 0 if not, -1 on error.
 */
FmlBoolean Fieldml_IsEnsembleComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
     Returns the value type of the given evaluator.
 */
FmlObjectHandle Fieldml_GetValueType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates a abstract evaluator. An abstract evaluator is a placeholder for a concrete evaluator, and
 * can be bound just as concrete ones can. Only abstract evaluators can be the target of a bind.
 */
FmlObjectHandle Fieldml_CreateAbstractEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Creates an external evaluator. An external evaluator's behaviour is defined by the author of the FieldML
 * document in which it appears, rather than in terms of other FieldML objects. Currently, the only option
 * is 'definition by specification'.
 */
FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Creates a new parameter set. A parameter set contains a store of literal values, indexed by a set
 * of ensemble-valued evaluators. The format and location of the store is intended to be very flexible,
 * and include the ability to describe 3rd-party formats such as HDF5, and allow for locations that
 * refer to inline data, as well as files on the local filesystem, or over the network.
 */
FmlObjectHandle Fieldml_CreateParametersEvaluator( FmlSessionHandle handle, const char *name, FmlObjectHandle valueType );


/**
 * Sets the description of the parameter set's raw data. At the moment, only DESCRIPTION_SEMIDENSE is supported.
 */
FmlErrorNumber Fieldml_SetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description );

/**
 * Returns the data source used by the given object.
 */
FmlObjectHandle Fieldml_GetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle );

/**
 * Set the data source to be used by the given object.
 * 
 * NOTE: Currently, only parameter evaluators use data sources.
 */
FmlErrorNumber Fieldml_SetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource );

/**
 *  Returns the data description type of the given parameter evaluator.
 *  
 *  NOTE: Currently, only DESCRIPTION_SEMIDENSE is valid.
 */
DataDescriptionType Fieldml_GetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Adds a dense index evaluator to the given parameter set's semidense data description, using the order
 * specified by the given data source (if not FML_INVALID_HANDLE).
 * 
 */
FmlErrorNumber Fieldml_AddDenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle );

/**
 * Adds a sparse index evaluator to the given parameter set's semidense data description.
 */
FmlErrorNumber Fieldml_AddSparseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle );

/**
 *   Returns the number of sparse or dense index evaluators of the semidense data store
 *   associated with the given parameter evaluator.
 */
int Fieldml_GetSemidenseIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isSparse );


/**
 *   Returns the handle of the nth sparse or dense index evaluator of the semidense data
 *   store associated with the given parameter evaluator.
 */
FmlObjectHandle Fieldml_GetSemidenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlBoolean isSparse );


/**
 * Returns the data source containing the ordering for the given dense index. For an n-member ensemble, the data source
 * must contain n entries representing valid members of the given ensemble.
 * 
 * NOTE: The ordering may be omitted, in which case the integer ordering will be used.
 */
FmlObjectHandle Fieldml_GetSemidenseIndexOrder( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index );

/**
 * Creates a new piecewise evaluator. Evaluators used by the piecewise evaluator
 * must all have the same value type as the piecewise itself. Piecewise evaluators need not be defined for all
 * possible index values.
 */
FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Creates a new aggregate evaluator.
 * The aggregate evaluator's index evaluators must all be scalar continuous.
 * The value is obtained by aggregating the scalar evaluators over the
 * value domain's component ensemble. There must be an entry (or a default) for each index value.
 */
FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Set the index evaluator for the given piecewise or aggregate evaluator.
 * 
 * The aggregate's index evaluator must have the same ensemble type as the component ensemble of its value type.
 */
FmlErrorNumber Fieldml_SetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle valueType, int index, FmlObjectHandle indexHandle );

/**
 * Sets the default evaluator for the given piecewise or aggregate evaluator.
 */
FmlErrorNumber Fieldml_SetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator );


/**
 * Returns the default evaluator for the given piecewise or aggregate evaluator.
 */
FmlObjectHandle Fieldml_GetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the evaluator on the given index for the given aggregate or piecewise evaluator.
 * 
 * Setting the evaluator handle to FML_INVALID_HANDLE removes the index-evaluator association.
 */
FmlErrorNumber Fieldml_SetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue element, FmlObjectHandle evaluator );


/**
 *  Returns the number of element->evaluator delegations for the given
 *  piecewise or aggregate evaluator.
 */
int Fieldml_GetEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the element number for the nth element->evaluator delegation in
 *  the given piecewise/aggregate evaluator.
 */
FmlEnsembleValue Fieldml_GetEvaluatorElement( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 *  Returns the evaluator handle for the nth element->evaluator delegation in
 *  the given piecewise/aggregate evaluator.
 */
FmlObjectHandle Fieldml_GetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 * Returns the evaluator for the given element number in the given piecewise or aggregate
 * evaluator, or FML_INVALID_HANDLE if there is none defined.
 */
FmlObjectHandle Fieldml_GetElementEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault );


/**
    Returns the number of indexes used by the given evaluator.
    
    NOTE: Only defined for piecewise, aggregate and parameter evaluators.
    
    NOTE: For piecewise or aggreate evaluators, this is currently always one. For parameter evaluators,
    it depends on the data store.
 */
int Fieldml_GetIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the evaluator of the nth index used by the given evaluator.
 *  
 *  NOTE: Only defined for piecewise, aggregate and parameter evaluators.
 */
FmlObjectHandle Fieldml_GetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int indexIndex );


/**
 * Creates a reference evaluator. Reference evaluators delegate their evaluation to another evaluator, and may bind
 * domains and evaluators before doing so.
 */
FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle sourceEvaluator );


/**
 * Gets the source evaluator which the given evaluator references.
 */
FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the number of abstract evaluators used by the given evaluator. 
 */
int Fieldml_GetVariableCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the nth abstract evaluator used by the given evaluator. 
 */
FmlObjectHandle Fieldml_GetVariable( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 *  Add an abstract evaluator to the list of abstract evaluators used by the given evaluator. 
 */
FmlErrorNumber Fieldml_AddVariable( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle );


/**
 * Adds an bind to the given evaluator. Only abstract evaluators can be bound. 
 */
FmlErrorNumber Fieldml_SetBind( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle unboundEvaluator, FmlObjectHandle evaluator );


/**
 *  Returns the number of bind used by the given evaluator. 
 */
int Fieldml_GetBindCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the evaulator used by the nth bind of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetBindVariable( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 *  Returns the source evaluator used by the nth bind of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetBindEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 * Returns the abstract evaluator to which to given evaluator is bound to in the given evaluator.
 */
FmlObjectHandle Fieldml_GetBindByVariable( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle variableHandle );


/**
 * Experimental.
 */
/*
FmlObjectHandle Fieldml_CreateEnsembleElementSequence( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );
*/


/**
 * Returns the EnsembleMembersType describing the means by which the members of the given ensemble are specified.
 */
EnsembleMembersType Fieldml_GetEnsembleMembersType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the data source for the given ensemble's member list. The ensemble type's members type must be one
 * of MEMBER_LIST_DATA, MEMBER_RANGE_DATA or MEMBER_STRIDE_RANGE_DATA.
 *  
 */
FmlErrorNumber Fieldml_SetEnsembleMembersDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, EnsembleMembersType type, int count, FmlObjectHandle dataSource );

/**
 * Sets the given ensemble's member list directly via a minimum, maximum and stride triple (the maximum is inclusive).
 * Also sets the ensemble's members type to MEMBER_RANGE. This is provided as a convenience to define trivial ensembles
 * without having to use a data source.
 */
FmlErrorNumber Fieldml_SetEnsembleElementRange( FmlSessionHandle handle, FmlObjectHandle objectHandle, const FmlEnsembleValue minElement, const FmlEnsembleValue maxElement, const int stride );


/**
 * Returns the number of members in the given ensemble.
 */
int Fieldml_GetElementCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the minimum ensemble member used when directly declaring ensemble members.
 * Only valid if the ensemble's members type is MEMBER_RANGE.
 */
FmlEnsembleValue Fieldml_GetEnsembleMembersMin( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the maximum ensemble member used when directly declaring ensemble members.
 * Only valid if the ensemble's members type is MEMBER_RANGE.
 */
FmlEnsembleValue Fieldml_GetEnsembleMembersMax( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the stride used when directly declaring ensemble members.
 * Only valid if the ensemble's members type is MEMBER_RANGE.
 */
int Fieldml_GetEnsembleMembersStride( FmlSessionHandle handle, FmlObjectHandle objectHandle );

/**
 * Creates a new reader for the given data source's raw data.
 */
FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Reads in some values from the current block of dense data.
 * 
 * Returns the number of values read, or -1 on error.
 */
FmlErrorNumber Fieldml_ReadIntValues( FmlSessionHandle handle, FmlReaderHandle reader, int *valueBuffer, int bufferSize );


/**
 * Reads in some values from the current block of dense data.
 * 
 * Returns the number of values read, or -1 on error.
 */
FmlErrorNumber Fieldml_ReadDoubleValues( FmlSessionHandle handle, FmlReaderHandle reader, double *valueBuffer, int bufferSize );


/**
 * Closes the given data reader.
 */
FmlErrorNumber Fieldml_CloseReader( FmlSessionHandle handle, FmlReaderHandle reader );


/**
 * Creates a new writer for the given data source's raw data.
 * 
 * NOTE: It is up to the application to ensure that the data source's description is consistent with the data
 * actually being written.
 */
FmlWriterHandle Fieldml_OpenWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean append );


/**
 * Write out some values for the current block of dense data.
 * 
 * Returns the number of values written, or -1 on error.
 */
FmlErrorNumber Fieldml_WriteIntValues( FmlSessionHandle handle, FmlWriterHandle writer, int *indexBuffer, int bufferSize );


/**
 * Write out some values from the current block of dense data.
 * 
 * Returns the number of values written, or -1 on error.
 */
FmlErrorNumber Fieldml_WriteDoubleValues( FmlSessionHandle handle, FmlWriterHandle writer, double *indexBuffer, int bufferSize );


/**
 * Closes the given raw data writer.
 */
FmlErrorNumber Fieldml_CloseWriter( FmlSessionHandle handle, FmlWriterHandle writer );


/**
 * Add an import source for the current region. The location will typically be the location of
 * another FieldML resource. The string "library_0.3.xml" is also permitted, and refers to the
 * built-in library.
 * 
 * Returns an import index to use with subsequent import API calls, or -1 on error.
 * 
 * NOTE: Attempting to add the same import source more than once will succeed, but will result
 * in the same index being returned each time.
 */
int Fieldml_AddImportSource( FmlSessionHandle handle, const char *location, const char *regionName );


/**
 * Import a FieldML object from the given import source into the current region. The local name must
 * be unique (i.e. no other local names or imports may have that name). The remote name must be an
 * object that has either been declared in, or imported into the import source region.
 */
FmlObjectHandle Fieldml_AddImport( FmlSessionHandle handle, int importSourceIndex, const char *localName, const char *remoteName );


/**
 * Returns the number of import sources used by the current region.
 */
int Fieldml_GetImportSourceCount( FmlSessionHandle handle );


/**
 * Returns the location of the FieldML resource used by the given indexed import source. This will typically
 * be a FieldML document.
 */
int Fieldml_CopyImportSourceLocation( FmlSessionHandle handle, int importSourceIndex, char *buffer, int bufferLength );


/**
 * Returns the region name used by the given indexed import source.
 */
int Fieldml_CopyImportSourceRegionName( FmlSessionHandle handle, int importSourceIndex, char *buffer, int bufferLength );


/**
 * Returns the number of objects imported into the current region from the given import source.
 */
int Fieldml_GetImportCount( FmlSessionHandle handle, int importSourceIndex );


/**
 * Copies the local name of the given imported object into the given buffer.
 */
int Fieldml_CopyImportLocalName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength );


/**
 * Copies the remote name of the given imported object into the given buffer.
 */
int Fieldml_CopyImportRemoteName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char *buffer, int bufferLength );


/**
 * Returns the handle for given imported object.
 */
FmlObjectHandle Fieldml_GetImportObject( FmlSessionHandle handle, int importSourceIndex, int importIndex );


/**
 * Creates a new text-based file data resource with the given name and href.
 * 
 * NOTE: Currently, the only valid hrefs supported are local filenames.
 */
FmlObjectHandle Fieldml_CreateTextFileDataResource( FmlSessionHandle handle, const char *name, const char *href );


/**
 * Creates a new text-based inline data resource. The resource will initially be an empty string, but its contents
 * can be set either directly via Fieldml_AddInlineData calls, or indirectly via FieldML writer calls.
 */
FmlObjectHandle Fieldml_CreateTextInlineDataResource( FmlSessionHandle handle, const char *name );


/**
 * Returns the type of the given data resource.
 */
DataResourceType Fieldml_GetDataResourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates a new text-based data source. The given data resource must be text-based.
 */
FmlErrorNumber Fieldml_CreateTextDataSource( FmlSessionHandle handle, const char *name, FmlObjectHandle dataResource, int firstLine, int count, int length, int head, int tail );


/**
 * Returns the number of data sources associated with the given data resource.
 */
int Fieldml_GetDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the nth data source associated with the given data resource. 
 */
FmlObjectHandle Fieldml_GetDataSourceByIndex( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index );


/**
 * Returns the data resource used by the given data source.
 */
FmlObjectHandle Fieldml_GetDataSourceResource( FmlSessionHandle handle, FmlObjectHandle objectHandle );

/**
 * Returns the first line for the given text data source.
 */
int Fieldml_GetTextDataSourceFirstLine( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the entry count for the given text data source.
 */
int Fieldml_GetTextDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the entry length for the given text data source.
 */
int Fieldml_GetTextDataSourceLength( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the entry head length for the given text data source.
 */
int Fieldml_GetTextDataSourceHead( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the entry tail length for the given text data source.
 */
int Fieldml_GetTextDataSourceTail( FmlSessionHandle handle, FmlObjectHandle objectHandle );

/**
 * Returns the data source type of the given data source.
 */
DataSourceType Fieldml_GetDataSourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Appends some data to the given data resource's inline data. The data resource's type must be
 * DATA_RESOURCE_TEXT_INLINE.
 */
FmlErrorNumber Fieldml_AddInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char *data, const int length );


/**
 * Returns the number of characters in the data resource's inline data.
 */
int Fieldml_GetInlineDataLength( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns a pointer to the data resource's inline data.
 * 
 * NOTE: This is obviously dangerous and should be replaced with something more robust.
 */
const char * Fieldml_GetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies a section of the data resource's inline data into the given buffer, starting from the given offset, and ending
 * either when the buffer is full, or the end of the inline data is reached.
 */
int Fieldml_CopyInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength, int offset );

/**
 * Returns the href of the data resource's file. The data resource's type must be DATA_RESOURCE_TEXT_FILE.
 */
const char * Fieldml_GetDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopyDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle, char *buffer, int bufferLength );

}
#endif // __cplusplus

#endif // H_FIELDML_API
