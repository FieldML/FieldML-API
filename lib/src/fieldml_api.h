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
#ifndef H_FIELDML_API
#define H_FIELDML_API

/**
 * \file
 * API notes:
 * 
 * If a function returns a FmlSessionHandle, FmlReaderHandle, FmlWriterHandle or
 * FmlObjectHandle, it will return FML_INVALID_HANDLE on error.
 * 
 * All FieldML objects are referred to only by their handle.
 * 
 * All handles are internally type-checked. If an inappropriate handle is
 * passed to a function, the function will return -1, NULL or
 * FML_INVALID_HANDLE as applicable, and the lastError value set.
 * 
 * For each string-getter, there is a corresponding string-copier. The string-copier
 * copies the relevant string into a provided buffer and returns the number of characters
 * copied. In cases where the getter returns NULL, the corresponding copier returns 0.
 * 
 * All object names must be unique within their region.
 * 
 * \note String getters are being considered for deprecation.
 */


/*

 Typedefs

*/
#include <stdint.h>

typedef int32_t FmlSessionHandle;               ///< A handle to a FieldML session. Almost all FieldML API calls require a session handle.

typedef int32_t FmlReaderHandle;                ///< A handle to a data reader.

typedef int32_t FmlWriterHandle;                ///< A handle to a data writer.

typedef int32_t FmlObjectHandle;                ///< A handle to a FieldML object.

typedef int32_t FmlErrorNumber;                 ///< A FieldML error code.

typedef int32_t FmlBoolean;                     ///< A pseudo-boolean. 0 = false, 1 = true. FmlBoolean-valued functions also return -1 to indictate an error.

typedef int32_t FmlEnsembleValue;               ///< An integer-valued ensemble member identifier.


/*

 Constants

*/
#define FML_INVALID_HANDLE -1                   ///< The valid used to indicate a missing object or failed object instantiation.

#define FML_MAJOR_VERSION               0
#define FML_MINOR_VERSION               4
#define FML_DOT_VERSION                 0

#define FML_ERR_NO_ERROR                0       ///< The API call was successful.
#define FML_ERR_UNKNOWN_HANDLE          1000    ///< An unknown session handle was used.
#define FML_ERR_UNKNOWN_OBJECT          1001    ///< An unknown object handle was used.
#define FML_ERR_INVALID_OBJECT          1002    ///< An object handle referred to an object of the wrong type.
#define FML_ERR_MISCONFIGURED_OBJECT    1003    ///< An object handle referred to an object with an invalid combination of attributes.
#define FML_ERR_ACCESS_VIOLATION        1004    ///< An attempt was made to modify a read-only object (e.g. imported from another region).
#define FML_ERR_NAME_COLLISION          1005    ///< An attempt was made to create an object with a name that was already in use. 
#define FML_ERR_INVALID_REGION          1006    ///< An attempt was made to access an invalid region. This usually indicates a failure to deserialise a FieldML file.
#define FML_ERR_NONLOCAL_OBJECT         1007    ///< An attempt was made to reference a non-local object (i.e. one that has not been imported).
#define FML_ERR_CYCLIC_DEPENDENCY       1008    ///< An attempt was made to create a cyclic dependency.
#define FML_ERR_INVALID_INDEX           1009    ///< An attempt was made to use an out-of-bounds index.

//Used for giving the user precise feedback on bad parameters passed to the API
//Only used for parameters other than the FieldML handle and object handle parameters.
#define FML_ERR_INVALID_PARAMETERS      1100    ///< A general-purpose error code indicating that some parameters to the API call were invalid.
#define FML_ERR_INVALID_PARAMETER_1     1101    ///< A general-purpose error code indicating that the first parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_2     1102    ///< A general-purpose error code indicating that the second parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_3     1103    ///< A general-purpose error code indicating that the third parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_4     1104    ///< A general-purpose error code indicating that the fourth parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_5     1105    ///< A general-purpose error code indicating that the fifth parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_6     1106    ///< A general-purpose error code indicating that the sixth parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_7     1107    ///< A general-purpose error code indicating that the seventh parameter to the API call was invalid.
#define FML_ERR_INVALID_PARAMETER_8     1108    ///< A general-purpose error code indicating that the eigth parameter to the API call was invalid.

#define FML_ERR_IO_READ_ERR             1201    ///< A read error was encountered during IO.
#define FML_ERR_IO_WRITE_ERR            1202    ///< A write error was encountered during IO.
#define FML_ERR_IO_UNEXPECTED_EOF       1203    ///< An EOF was encountered when more data was expected.
#define FML_ERR_IO_NO_DATA              1204    ///< There is no data remaining.
#define FML_ERR_IO_UNSUPPORTED          1205    ///< Operation not supported for the given IO object.

#define FML_ERR_UNSUPPORTED             2000    ///< Used for operations that are valid, but not yet implemented.

/*

 Types

*/


/**
 * Describes the means by which ensemble type members are described.
 * 
 * \see Fieldml_GetEnsembleMembersType
 * \see Fieldml_SetEnsembleMembersDataSource
 * 
 * \see Fieldml_GetDataSource
 * \see Fieldml_GetEnsembleMembersMin
 * \see Fieldml_GetEnsembleMembersMax
 * \see Fieldml_GetEnsembleMembersStride
 */
enum EnsembleMembersType
{
    MEMBER_UNKNOWN,           ///< Member definition is not yet known.
    MEMBER_RANGE,             ///< Members are defined directly via a min/max/stride triple, without using a data source.
    MEMBER_LIST_DATA,         ///< Members are defined by a data source containing a list of integers.
    MEMBER_RANGE_DATA,        ///< Members are defined by a data source containing a list of min/max integer pairs, with an assumed stride of 1.
    MEMBER_STRIDE_RANGE_DATA, ///< Members are defined by a data source containing a list of min/max/stride integer triples.
};


/**
 * Describes the format of the data associated with a ParameterEvaluator.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_GetParameterDataDescription
 */
enum DataDescriptionType
{
    DESCRIPTION_UNKNOWN,      ///< The data's format is unknown.
    DESCRIPTION_DENSE_ARRAY,  ///< The data is a fully-populated array.
    DESCRIPTION_DOK_ARRAY,    ///< The data is a (potentially) sparse array represented by an array of keys and a separate array of values.
};


/**
 * Describes the type of external data encapsulated by a DataResource object.
 * 
 * \note Currently, only inline and external text is supported, but it is intended
 * that additional formats such as HDF5 be added.
 * 
 * \see Fieldml_GetDataResourceType
 * \see Fieldml_CreateTextFileDataResource
 * \see Fieldml_CreateTextInlineDataResource
 */
enum DataResourceType
{
    DATA_RESOURCE_UNKNOWN,     ///< The data's type is unknown.
    DATA_RESOURCE_HREF,        ///< The data is a resource specified by an href.
    DATA_RESOURCE_INLINE,      ///< The data is inline text.
};


/**
 * Describes the type of data encapsulated by a DataSource object, and therefore what attribute can be
 * used with that DataSource.
 * 
 * \see Fieldml_GetDataSourceType
 * \see Fieldml_CreateTextDataSource
 */
enum DataSourceType
{
    DATA_SOURCE_UNKNOWN,      ///< The data source's type is unknown.
    DATA_SOURCE_ARRAY,        ///< The data source is an array. The array dimensions are given by the associated raw source in the resource data itself.
};


/**
 * Describes the various types of FieldML object.
 * 
 * \see Fieldml_GetObject
 * \see Fieldml_GetObjectCount
 * \see Fieldml_GetObjectType
 * 
 * \see Fieldml_CreateEnsembleType
 * \see Fieldml_CreateContinuousType
 * \see Fieldml_CreateMeshType
 * \see Fieldml_CreateArgumentEvaluator
 * \see Fieldml_CreateExternalEvaluator
 * \see Fieldml_CreateReferenceEvaluator
 * \see Fieldml_CreateParameterEvaluator
 * \see Fieldml_CreatePiecewiseEvaluator
 * \see Fieldml_CreateAggregateEvaluator
 * \see Fieldml_CreateTextFileDataResource
 * \see Fieldml_CreateTextInlineDataResource
 * \see Fieldml_CreateTextDataSource
 */
enum FieldmlHandleType
{
    FHT_UNKNOWN,              ///< The object's type is unknown.
    
    FHT_ENSEMBLE_TYPE,        ///< The object is an EnsembleType.
    FHT_CONTINUOUS_TYPE,      ///< The object is a ContinuousType.
    FHT_MESH_TYPE,            ///< The object is a MeshType.
    FHT_ARGUMENT_EVALUATOR,   ///< The object is an ArgumentEvaluator.
    FHT_EXTERNAL_EVALUATOR,   ///< The object is an ExternalEvaluator.
    FHT_REFERENCE_EVALUATOR,  ///< The object is a ReferenceEvaluator.
    FHT_PARAMETER_EVALUATOR,  ///< The object is a ParameterEvaluator.
    FHT_PIECEWISE_EVALUATOR,  ///< The object is a PiecewiseEvaluator.
    FHT_AGGREGATE_EVALUATOR,  ///< The object is a AggregateEvaluator.
    FHT_DATA_RESOURCE,        ///< The object is a DataResource.
    FHT_DATA_SOURCE,          ///< The object is a DataSource.
};


/*

 API

*/

#ifdef __cplusplus
extern "C" {


/**
 * Parses the given XML file, and returns a handle to the parsed data. This
 * handle is then used for all subsequent API calls. If an error is encountered,
 * a valid session handle will still be returned, but can only be used to obtain
 * detailed error information. Almost all other FieldML API calls will fail.
 * 
 * \note At the moment, the debug flag can only be set after a parse.
 * Errors during the parse can be obtained via the error log, but at the
 * moment they're all just strings.
 * 
 * \note Currently, only local filenames are supported. This will be changed in later versions.
 * 
 * \see Fieldml_Destroy
 */
FmlSessionHandle Fieldml_CreateFromFile( const char * filename );


/**
 * Creates an empty FieldML handle.
 * 
 * Data files will be created at the given location.
 * 
 * \note Currently, only local directory locations are supported. This will be changed in later versions.
 * 
 * \see Fieldml_Destroy
 */
FmlSessionHandle Fieldml_Create( const char * location, const char * name );


/**
 * Sets/clears the debug flag. If non-zero, error encoutered by API calls are logged to stdout.
 * 
 * \note This does not include errors encountered when parsing a FieldML file.
 */
FmlErrorNumber Fieldml_SetDebug( FmlSessionHandle handle, int debug );


/**
 * \return The error code generated by the last API call.
 * 
 * \see FML_ERR_NO_ERROR
 */
FmlErrorNumber Fieldml_GetLastError( FmlSessionHandle handle );


/**
 * Writes the contents of the given FieldML handle to the given filename as
 * an XML file.
 * 
 * \note Currently, only local filenames are supported. This will be changed in later versions.
 * 
 * \note For diagnostic purposes, errors encountered during writing will not cause the
 * file to be deleted.
 */
FmlErrorNumber Fieldml_WriteFile( FmlSessionHandle handle, const char * filename );


/**
 * Frees all resources associated with the given handle. The handle will
 * become invalid after this call.
 * 
 * \see Fieldml_Create
 * \see Fieldml_CreateFromFile
 */
void Fieldml_Destroy( FmlSessionHandle handle );


/**
 * Frees any string returned by a char* valued Fieldml_Get* function.
 * 
 * \see Fieldml_GetRegionName
 */
FmlErrorNumber Fieldml_FreeString( char * string );


/**
 * \return A newly-allocated buffer containing the name of the region, or NULL on error.
 * 
 * \warning All char* valued Fieldm_Get API calls return a newly-allocated buffer
 * containing the string. This must be freed either by the caller (via the C standard library's free() call),
 * or by passing the value to Fieldml_FreeString()
 * 
 * \see Fieldml_FreeString
 */
char * Fieldml_GetRegionName( FmlSessionHandle handle );


/**
 * Copies the name of the region into the provided buffer. If the buffer is too short, as much data as
 * possible will be copied.
 * 
 * \return The length of the resulting string, or -1 on error.
 * 
 * \note The bufferLength includes the terminating null character.
 * 
 * \note It is recommended that FieldML object names be less than 128 characters. Preferably much less.
 */
int Fieldml_CopyRegionName( FmlSessionHandle handle, char * buffer, int bufferLength );


/**
 * \return The number of parsing errors encountered by the given handle during FieldML file parsing.
 */
int Fieldml_GetErrorCount( FmlSessionHandle handle );


/**
 * \returns the nth parsing-error string for the given handle.
 * 
 * \note Currently, these string are freeform, and only intended for diagnostic purposes.
 * 
 * \see Fieldml_GetErrorCount
 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 */
char * Fieldml_GetError( FmlSessionHandle handle, int index );


/**
 * Copies the nth parsing-error string for the given handle.
 * 
 * \see Fieldml_GetErrorCount
 * \see Fieldml_GetError
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyError( FmlSessionHandle handle, int errorIndex, char * buffer, int bufferLength );


/**
 * Clears the given session's parsing errors and error number.
 * 
 * \see Fieldml_GetErrorCount
 * \see Fieldml_GetError
 * \see Fieldml_CopyRegionName
 * \see Fieldml_GetLastError
 */
FmlErrorNumber Fieldml_ClearErrors( FmlSessionHandle handle );


/**
 * \return The total number of FieldML objects, or zero if there are none.
 * 
 * \see Fieldml_GetObjectByIndex
 */
int Fieldml_GetTotalObjectCount( FmlSessionHandle handle );


/**
 * \return A handle to the nth object.
 * 
 * \see Fieldml_GetTotalObjectCount
 */
FmlObjectHandle Fieldml_GetObjectByIndex( FmlSessionHandle handle, const int objectIndex );


/**
 * \return The number of objects of the given type, or zero if there are none.
 * 
 * \see Fieldml_GetObject
 */
int Fieldml_GetObjectCount( FmlSessionHandle handle, FieldmlHandleType type );


/**
 * \return A handle to the nth object of the given type.
 * 
 * \see Fieldml_GetObjectCount
 */
FmlObjectHandle Fieldml_GetObject( FmlSessionHandle handle, FieldmlHandleType objectType, int objectIndex );


/**
 * \return The type of the given object.
 */
FieldmlHandleType Fieldml_GetObjectType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return A handle to the object with the given local name.
 * 
 * \see Fieldml_GetObjectByDeclaredName
 * \see Fieldml_GetObjectName
 */
FmlObjectHandle Fieldml_GetObjectByName( FmlSessionHandle handle, const char * name );


/**
 * \return A handle to the given declared name. This may differ from the object's
 * local name if the object has been imported.
 */
FmlObjectHandle Fieldml_GetObjectByDeclaredName( FmlSessionHandle handle, const char * name );

/**
 * \return 1 if the given object is local, 0 if not, or -1 on error.
 * 
 * \note Imported objects are not considered local by this function.
 */
FmlBoolean Fieldml_IsObjectLocal( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The local name of the given object.
 * 
 * \note FieldML objects will only have a local name if they have either been imported into, or declared in the
 * current region. Therefore an object may exist and have a declared name, even if it has no local name.
 * 
 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 * \see Fieldml_GetObjectDeclaredName
 */
char * Fieldml_GetObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies the given object's local name into the given buffer.
 * 
 * \see Fieldml_CopyRegionName
 * \see Fieldml_GetObjectName
 */
int Fieldml_CopyObjectName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength );


/**
 * \return The given object's declared name. This is the name the object was given in the
 * region in which is was declared, and may differ from the the object's local name.

 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 * \see Fieldml_GetObjectByDeclaredName
 */
char * Fieldml_GetObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies the given object's declared name into the given buffer.
 * 
 * \see Fieldml_CopyRegionName
 * \see Fieldml_GetObjectDeclaredName
 */
int Fieldml_CopyObjectDeclaredName( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength );


/**
 * Associate a client-defined integer with the given object. This value is initialized to 0 when
 * the object is created, but is otherwise ignored by the API.
 * 
 * \see Fieldml_GetObjectInt
 */
FmlErrorNumber Fieldml_SetObjectInt( FmlSessionHandle handle, FmlObjectHandle objectHandle, int value );


/**
 * \return The client-defined integer value associated with the given object.
 * 
 * \see Fieldml_SetObjectInt
 */
int Fieldml_GetObjectInt( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The handle of the given type's component ensemble.
 * 
 * \note Only valid for ContinuousType objects.
 */
FmlObjectHandle Fieldml_GetTypeComponentEnsemble( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The number of components in the given continuous type.
 * 
 * \note Only valid for ContinuousType objects.
 * 
 * \see Fieldml_GetTypeComponentEnsemble
 */
int Fieldml_GetTypeComponentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates an ensemble type with the given name.
 * 
 * \note The ensemble's members must be declared either via Fieldml_SetEnsembleMembersDataSource()
 * or Fieldml_SetEnsembleMembersRange().
 * 
 * \see Fieldml_SetEnsembleMembersDataSource
 * \see Fieldml_SetEnsembleMembersRange
 * \see Fieldml_GetDataSource
 */
FmlObjectHandle Fieldml_CreateEnsembleType( FmlSessionHandle handle, const char * name );


/**
 * Creates a continuous type with the given name. Multi-component continuous types can be formed with a
 * subsequent call to Fieldml_CreateContinuousTypeComponents()
 * 
 * \see Fieldml_CreateContinuousTypeComponents
 */
FmlObjectHandle Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name );


/**
 * Create the indexing ensemble for the given continuous type. The result is a newly initialized ensemble type,
 * which must be configured in the same manner as a 'top level' ensemble.
 * 
 * \note A continuous type need not have an indexing ensemble, in which case it is equivalent to a scalar value type.
 * 
 * \see Fieldml_CreateContinuousType
 * \see Fieldml_CreateEnsembleType
 * \see Fieldml_GetTypeComponentEnsemble
 */
FmlObjectHandle Fieldml_CreateContinuousTypeComponents( FmlSessionHandle handle, FmlObjectHandle typeHandle, const char * name, const int count );

/**
 * Creates a mesh type with the given name.
 * Each mesh has its own unique element and chart type, which can be accessed by the relevant functions.
 * Because the chart and element types have a name based on the mesh name, care must be taken to ensure
 * that neither the mesh's name, nor it's element or chart type names are already in use. If they are,
 * this function will return an error.
 * 
 * The mesh's element and chart type must be defined using Fieldml_CreateMeshElementsType() and
 * Fieldml_CreateMeshChartType() respectively.
 * 
 * \see Fieldml_CreateMeshElementsType
 * \see Fieldml_CreateMeshChartType
 * \see Fieldml_GetMeshChartType
 * \see Fieldml_GetMeshElementsType
 */
FmlObjectHandle Fieldml_CreateMeshType( FmlSessionHandle handle, const char * name );

/**
 * Create the element ensemble for the given mesh type. The result is a newly initialized ensemble type,
 * which must be configured in the same manner as a 'top level' ensemble.
 * 
 * \note A mesh type must have an element ensemble, and will be considered misconfigured if not.
 * 
 * \see Fieldml_CreateEnsembleType
 * \see Fieldml_CreateMeshType
 */
FmlObjectHandle Fieldml_CreateMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * name );

/**
 * Create the chart type for the given mesh type. The result is a newly initialized continuous type,
 * which must be configured in the same manner as a 'top level' continuous type, including its indexing
 * ensemble.
 * 
 * \note A mesh type must have a chart type, and will be considered misconfigured if not.
 * 
 * \see Fieldml_CreateContinuousType
 * \see Fieldml_CreateMeshType
 * \see Fieldml_GetMeshChartType
 */
FmlObjectHandle Fieldml_CreateMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * name );


/**
 * \return The handle of the given mesh type's chart type. This is a unique, n-dimensional
 * continuous type with the component type specified when the mesh was created.
 * 
 * \see Fieldml_CreateMeshChartType
 */
FmlObjectHandle Fieldml_GetMeshChartType( FmlSessionHandle handle, FmlObjectHandle meshHandle );


/**
 * \return The component handle of the given mesh type's chart type.
 * 
 * \see Fieldml_CreateMeshChartType
 */
FmlObjectHandle Fieldml_GetMeshChartComponentType( FmlSessionHandle handle, FmlObjectHandle meshHandle );


/**
 * \return The handle of the given mesh type's element type. This is a unique ensemble
 * type whose elements are specified by the user.
 * 
 * \see Fieldml_CreateMeshElementsType
 */
FmlObjectHandle Fieldml_GetMeshElementsType( FmlSessionHandle handle, FmlObjectHandle meshHandle );


/**
 * \return A string describing the shape of the given element in the given mesh.
 *
 * If allowDefault is 1, the default shape will be returned if there is no shape explicitly associated with the given element.
 * 
 * \note Currently, shapes are only described via strings. This will be changed in later versions.

 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 * 
 * \see Fieldml_SetMeshDefaultShape
 * \see Fieldml_SetMeshElementShape
 */
char * Fieldml_GetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault );


/**
 * Copies the shape of the given element in the given mesh into the given buffer.
 * 
 * \see Fieldml_GetMeshElementShape
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault, char * buffer, int bufferLength );


/**
 * Sets the default shape for the mesh. This should be set unless all elements have been explicitly
 * assigned a shape.
 * 
 * \see Fieldml_GetMeshElementShape
 * \see Fieldml_GetMeshDefaultShape
 */
FmlErrorNumber Fieldml_SetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, const char * shape );


/**
 * Gets the default shape for the mesh.
 *
 * \see Fieldml_SetMeshDefaultShape
 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 */
char * Fieldml_GetMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle );


/**
 * Copies default shape for the mesh into the given buffer.
 * 
 * \see Fieldml_CopyRegionName
 * \see Fieldml_GetMeshDefaultShape
 */
int Fieldml_CopyMeshDefaultShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, char * buffer, int bufferLength );


/**
 * Sets the shape of the given element in the given mesh.
 * 
 * \see Fieldml_GetMeshElementShape
 */
FmlErrorNumber Fieldml_SetMeshElementShape( FmlSessionHandle handle, FmlObjectHandle meshHandle, FmlEnsembleValue elementNumber, const char * shape );


/**
 * \return 1 if the ensemble type is a component ensemble, 0 if not, -1 on error.
 * 
 * \note Ensembles created with the Fieldml_CreateContinuousTypeComponents() are considered to be component ensembles.
 * Ensembles created with Fieldml_CreateEnsembleType() or Fieldml_CreateMeshElementsType() are not.
 * 
 * \see Fieldml_CreateContinuousTypeComponents
 * \see Fieldml_CreateMeshElementsType
 * \see Fieldml_CreateEnsembleType
 */
FmlBoolean Fieldml_IsEnsembleComponentType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The value type of the given evaluator.
 * 
 * \see Fieldml_CreateArgumentEvaluator
 * \see Fieldml_CreateAggregateEvaluator
 * \see Fieldml_CreatePiecewiseEvaluator
 * \see Fieldml_CreateParameterEvaluator
 * \see Fieldml_CreateReferenceEvaluator
 * \see Fieldml_CreateExternalEvaluator
 */
FmlObjectHandle Fieldml_GetValueType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates an argument evaluator. An argument evaluator is a placeholder evaluator, and
 * can be used as the source paramter for a bind just like any other evaluator.
 * However, only argument evaluators can be the target of a bind.
 * 
 * \see Fieldml_SetBind
 */
FmlObjectHandle Fieldml_CreateArgumentEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Creates an external evaluator. An external evaluator's behaviour is defined by the author of the FieldML
 * document in which it appears, rather than in terms of other FieldML objects. Currently, the only option
 * is 'definition by specification'. However, it is anticipated that it will be possible to define an external
 * evaluator's behavour inline using a non-FieldML markup language.
 * 
 * \note Typically, external evaluators will have arguments specified by Fieldml_AddArgument()
 * 
 * \see Fieldml_AddArgument
 */
FmlObjectHandle Fieldml_CreateExternalEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Creates a new parameter evaluator. A parameter evaluator presents a data source as an indexable data store.
 * 
 * \note Currently, the value type must either be a scalar continuous types or an ensemble type.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_SetDataSource
 */
FmlObjectHandle Fieldml_CreateParameterEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Sets the description of the parameter evaluator's raw data.
 * 
 * \see Fieldml_GetParameterDataDescription
 */
FmlErrorNumber Fieldml_SetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle, DataDescriptionType description );

/**
 * \return The data source used by the given object.
 * 
 * \see Fieldml_OpenReader
 * \see Fieldml_OpenWriter
 * \see Fieldml_SetDataSource
 * \see Fieldml_SetEnsembleMembersDataSource
 */
FmlObjectHandle Fieldml_GetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle );

FmlObjectHandle Fieldml_GetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle );

/**
 * Set the data source to be used by the given object. Currently, only parameter evaluators and ensemble types
 * use data sources.
 * 
 * \see Fieldml_GetDataSource
 * \see Fieldml_SetEnsembleMembersDataSource
 */
FmlErrorNumber Fieldml_SetDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource );

FmlErrorNumber Fieldml_SetKeyDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle dataSource );

/**
 * \return The data description type of the given parameter evaluator.
 */
DataDescriptionType Fieldml_GetParameterDataDescription( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Adds a dense index evaluator to the given parameter evaluator's data description.  The given evaluator must be ensemble-valued.
 * If the order is FML_INVALID_HANDLE, the integer ordering will be used when deserializing data. Otherwise, the order must refer
 * to a data source containing an ordering for the index evaluator's type. For an n-member ensemble, this must be a list of n
 * unique members of the ensemble. 
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_GetParameterIndexCount
 * \see Fieldml_GetParameterIndexEvaluator
 * \see Fieldml_GetParameterIndexOrder
 * \see Fieldml_AddSparseIndexEvaluator
 */
FmlErrorNumber Fieldml_AddDenseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, FmlObjectHandle orderHandle );

/**
 * Adds a sparse index evaluator to the given parameter evaluator's data description. The given evaluator must be ensemble-valued.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_GetParameterIndexCount
 * \see Fieldml_GetParameterIndexEvaluator
 * \see Fieldml_AddDenseIndexEvaluator
 */
FmlErrorNumber Fieldml_AddSparseIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle );


/**
 * \return The number of sparse or dense index evaluators of the parameter evaluator
 * associated with the given parameter evaluator.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_GetParameterIndexEvaluator
 * \see Fieldml_AddDenseIndexEvaluator
 * \see Fieldml_AddSparseIndexEvaluator
 */
int Fieldml_GetParameterIndexCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isSparse );


/**
 * \return The handle of the nth sparse or dense index evaluator of the given parameter evaluator.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_GetParameterIndexCount
 * \see Fieldml_AddDenseIndexEvaluator
 * \see Fieldml_AddSparseIndexEvaluator
 */
FmlObjectHandle Fieldml_GetParameterIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlBoolean isSparse );


/**
 * \return The data source containing the ordering for the given dense index, or FML_INVALID_HANDLE if none was
 * specified. For an n-member ensemble, the data source must contain n entries representing valid members of the given ensemble.
 * 
 * \see Fieldml_SetParameterDataDescription
 * \see Fieldml_AddDenseIndexEvaluator
 * 
 */
FmlObjectHandle Fieldml_GetParameterIndexOrder( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index );

/**
 * Creates a new piecewise evaluator. Piecewise evaluators delegate their evaluation based on the value
 * of their index evaluator, analogous to a switch statement in C. Evaluators delegated to by the piecewise evaluator
 * must all have a value type commensurable with the piecewise itself. Piecewise evaluators need not be defined for all
 * possible index values, and a default may be specified. Piecewise evaluators must have an index evaluator,
 * and will be considered misconfigured otherwise.
 * 
 * \see Fieldml_SetIndexEvaluator
 * \see Fieldml_SetDefaultEvaluator
 * \see Fieldml_SetEvaluator
 */
FmlObjectHandle Fieldml_CreatePiecewiseEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Creates a new aggregate evaluator. Aggregate evaluators construct a multi-component continuous value from
 * the values a set of scalar-valued evaluators. For each component index in the output value's type, an
 * evaluator must be specified, or a default evaluator must be available.
 * 
 * The value type of the index evaluator must be an ensemble that is commensurable with the index ensemble of
 * the value type of the aggregate evaluator itself.
 * 
 * \see Fieldml_SetIndexEvaluator
 * \see Fieldml_SetDefaultEvaluator
 * \see Fieldml_SetEvaluator
 */
FmlObjectHandle Fieldml_CreateAggregateEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle valueType );


/**
 * Set the index evaluator for the given piecewise or aggregate evaluator. The index evaluator must be ensemble-valued.
 * 
 * \note Currently, only one index evaluator is permitted. This will be changed in later versions.
 * 
 * \see Fieldml_CreateAggregateEvaluator
 * \see Fieldml_CreatePiecewiseEvaluator
 * \see Fieldml_GetIndexEvaluator
 */
FmlErrorNumber Fieldml_SetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index, FmlObjectHandle evaluatorHandle );

/**
 * Sets the default evaluator for the given piecewise or aggregate evaluator.
 * 
 * \see Fieldml_CreateAggregateEvaluator
 * \see Fieldml_CreatePiecewiseEvaluator
 * \see Fieldml_GetDefaultEvaluator
 */
FmlErrorNumber Fieldml_SetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator );


/**
 * \return The default evaluator for the given piecewise or aggregate evaluator.
 * 
 * \see Fieldml_CreateAggregateEvaluator
 * \see Fieldml_CreatePiecewiseEvaluator
 * \see Fieldml_SetDefaultEvaluator
 */
FmlObjectHandle Fieldml_GetDefaultEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets an explicit index value to evaluator pairing for the given aggregate or piecewise evaluator. An evaluator
 * need not be explicitly associated with each value that the index evaluator may yield. However, in the case
 * of aggregate evaluators, a default must be supplied in such cases.
 * 
 * Setting the evaluator handle to FML_INVALID_HANDLE removes the index-evaluator association.
 * 
 * \see Fieldml_GetEvaluatorCount
 * \see Fieldml_GetEvaluatorElement
 * \see Fieldml_GetElementEvaluator
 * \see Fieldml_GetEvaluator
 */
FmlErrorNumber Fieldml_SetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue element, FmlObjectHandle evaluator );


/**
 * \return The number of explicit index-value to evaluator pairings for the given
 * piecewise or aggregate evaluator.
 * 
 * \see Fieldml_SetEvaluator
 * \see Fieldml_GetEvaluatorElement
 * \see Fieldml_GetElementEvaluator
 */
int Fieldml_GetEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The index value for the nth index-value to evaluator pair in
 * the given piecewise or aggregate evaluator.
 * 
 * \see Fieldml_SetEvaluator
 * \see Fieldml_GetElementEvaluator
 * \see Fieldml_GetEvaluatorCount
 */
FmlEnsembleValue Fieldml_GetEvaluatorElement( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 * \return The evaluator for the nth index-value to evaluator pair in
 * the given piecewise or aggregate evaluator.
 * 
 * \see Fieldml_SetEvaluator
 * \see Fieldml_GetEvaluatorElement
 * \see Fieldml_GetElementEvaluator
 * \see Fieldml_GetEvaluatorCount
 */
FmlObjectHandle Fieldml_GetEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int evaluatorIndex );


/**
 * \return The evaluator for the given index value in the given piecewise or aggregate
 * evaluator, or FML_INVALID_HANDLE if there is none defined.
 * 
 * \see Fieldml_SetEvaluator
 */
FmlObjectHandle Fieldml_GetElementEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlEnsembleValue elementNumber, FmlBoolean allowDefault );


/**
 * \return The number of index evaluators used by the given evaluator.
 * 
 * \note For piecewise or aggreate evaluators, this is currently always one. For parameter evaluators,
 * it depends on the data format.
 * 
 * \see Fieldml_GetIndexEvaluator
 */
int Fieldml_GetIndexEvaluatorCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The evaluator of the nth index used by the given evaluator.
 * 
 * \note Only defined for piecewise, aggregate and parameter evaluators.
 * 
 * \see Fieldml_GetIndexEvaluatorCount
 */
FmlObjectHandle Fieldml_GetIndexEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int indexNumber );


/**
 * Creates a reference evaluator. Reference evaluators delegate their evaluation directly to another evaluator, but can bind
 * argument evaluators before doing so.
 * 
 * \note A reference evaluator's value type is the same as the value type of its source evaluator.
 * 
 * \see Fieldml_GetReferenceSourceEvaluator
 */
FmlObjectHandle Fieldml_CreateReferenceEvaluator( FmlSessionHandle handle, const char * name, FmlObjectHandle sourceEvaluator );


/**
 * Gets the source evaluator which the given reference evaluator delegates its evaluation to.
 * 
 * \see Fieldml_CreateReferenceEvaluator
 */
FmlObjectHandle Fieldml_GetReferenceSourceEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Add an argument evaluator to the list of argument evaluators used by the given evaluator.
 * Arguments can only be directly set for external and argument evaluators, because they represent abstractions.
 * However, the arguments for all types of evaluator is accessible via the relevant functions.
 * 
 * \see Fieldml_GetArgumentCount
 * \see Fieldml_GetArgument
 */
FmlErrorNumber Fieldml_AddArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluatorHandle );


/**
 * \return The number of argument evaluators used by the given evaluator, subject to the given qualifiers.
 * 
 * \see Fieldml_AddArgument
 * \see Fieldml_GetArgument
 */
int Fieldml_GetArgumentCount( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlBoolean isUnbound, FmlBoolean isUsed );


/**
 * \return The nth argument evaluator used by the given evaluator, subject to the given qualifiers. 
 * 
 * \see Fieldml_AddArgument
 * \see Fieldml_GetArgumentCount
 */
FmlObjectHandle Fieldml_GetArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int argumentIndex, FmlBoolean isUnbound, FmlBoolean isUsed );


/**
 * Binds the given argument evaluator to the given source within the scope of the given evaluator.
 * A bound argument can be re-bound, but the overriding bind's scope remains that of the enclosing evaluator.
 * 
 * \see Fieldml_GetBindCount
 * \see Fieldml_GetBindCount
 * \see Fieldml_GetBindArgument
 * \see Fieldml_GetBindEvaluator
 * \see Fieldml_GetBindByArgument
 */
FmlErrorNumber Fieldml_SetBind( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle, FmlObjectHandle sourceHandle );


/**
 * \return The number of binds specified for the given evaluator.
 * 
 * \see Fieldml_SetBind
 */
int Fieldml_GetBindCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The argument evaulator used by the nth bind in the given evaluator.
 * 
 * \see Fieldml_GetBindCount
 * \see Fieldml_SetBind
 */
FmlObjectHandle Fieldml_GetBindArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 * \return The source evaluator used by the nth bind of the given evaluator. 
 * 
 * \see Fieldml_GetBindCount
 * \see Fieldml_SetBind
 */
FmlObjectHandle Fieldml_GetBindEvaluator( FmlSessionHandle handle, FmlObjectHandle objectHandle, int bindIndex );


/**
 * \return The argument evaluator to which to given evaluator is bound to in the given evaluator.
 * 
 * \see Fieldml_SetBind
 */
FmlObjectHandle Fieldml_GetBindByArgument( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle argumentHandle );



/**
 * \return The EnsembleMembersType describing the means by which the members of the given ensemble are specified.
 * 
 * \see Fieldml_SetEnsembleMembersDataSource
 * \see Fieldml_SetEnsembleMembersRange
 */
EnsembleMembersType Fieldml_GetEnsembleMembersType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the members type and data source for the given ensemble's member list. The ensemble type's members type must be one
 * of ::MEMBER_LIST_DATA, ::MEMBER_RANGE_DATA or ::MEMBER_STRIDE_RANGE_DATA.
 * 
 * \see EnsembleMembersType
 * \see Fieldml_GetEnsembleMembersType
 * \see Fieldml_CreateEnsembleType
 */
FmlErrorNumber Fieldml_SetEnsembleMembersDataSource( FmlSessionHandle handle, FmlObjectHandle objectHandle, EnsembleMembersType type, int count, FmlObjectHandle dataSourceHandle );

/**
 * Sets the given ensemble's member list directly via a minimum, maximum and stride triple (the maximum is inclusive).
 * Also sets the ensemble's members type to ::MEMBER_RANGE. This is provided as a convenience to define trivial ensembles
 * without having to use a data source.
 * 
 * \see Fieldml_GetEnsembleMembersType
 * \see Fieldml_CreateEnsembleType
 */
FmlErrorNumber Fieldml_SetEnsembleMembersRange( FmlSessionHandle handle, FmlObjectHandle objectHandle, const FmlEnsembleValue minElement, const FmlEnsembleValue maxElement, const int stride );


/**
 * \return The number of members in the given ensemble.
 * 
 * \see Fieldml_SetEnsembleMembersDataSource
 * \see Fieldml_SetEnsembleMembersRange
 */
int Fieldml_GetMemberCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The minimum ensemble member used when directly declaring ensemble members.
 * 
 * \note Only valid if the ensemble's members type is ::MEMBER_RANGE.
 * 
 * \see Fieldml_SetEnsembleMembersRange
 */
FmlEnsembleValue Fieldml_GetEnsembleMembersMin( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The maximum possible ensemble member used when directly declaring ensemble members.
 * 
 * \note If the corresponding stride is not 1, a member with this value may not actually exist.
 * 
 * \note Only valid if the ensemble's members type is ::MEMBER_RANGE.
 * 
 * \see Fieldml_SetEnsembleMembersRange
 */
FmlEnsembleValue Fieldml_GetEnsembleMembersMax( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The stride used when directly declaring ensemble members.
 * Only valid if the ensemble's members type is ::MEMBER_RANGE.
 * 
 * \see Fieldml_SetEnsembleMembersRange
 */
int Fieldml_GetEnsembleMembersStride( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates a new reader for the given data source's raw data. Fieldml_CloseReader() should be called
 * when the caller no longer needs to use it. Subsequent calls to Fieldml_ReadIntValues() and
 * Fieldml_ReadDoubleValues() will use the attributes of the associated data source to to preprocess
 * incoming data before extracting the values that will be placed into the given buffers.
 * 
 * \note Currently, it is not possible to state the layout of the data to be read. It is up to the
 * caller to ensure that Fieldml_ReadIntValues() and Fieldml_ReadDoubleValues() is called at appropriate
 * times and with appropriate parameters.
 * 
 * \see Fieldml_ReadIntValues
 * \see Fieldml_ReadDoubleValues
 * \see Fieldml_CloseReader
 */
FmlReaderHandle Fieldml_OpenReader( FmlSessionHandle handle, FmlObjectHandle objectHandle );


FmlErrorNumber Fieldml_ReadIntSlab( FmlSessionHandle handle, FmlReaderHandle readerHandle, int *offsets, int *sizes, int *valueBuffer );


FmlErrorNumber Fieldml_ReadDoubleSlab( FmlSessionHandle handle, FmlReaderHandle readerHandle, int *offsets, int *sizes, double *valueBuffer );


/**
 * Closes the given data reader. The reader's handle should not be used after this call.
 * 
 * \see Fieldml_OpenReader
 */
FmlErrorNumber Fieldml_CloseReader( FmlSessionHandle handle, FmlReaderHandle readerHandle );


/**
 * Creates a new writer for the given data source's raw data. No post-processing will be done on the
 * provided values. It is up to the application to ensure that the data source's description is consistent with the data
 * actually being written, and with any other data sources that use the same data resource.
 * Fieldml_CloseWriter() should be called when the caller no longer needs to use the writer. 
 * 
 * \see Fieldml_CloseWriter
 */
FmlWriterHandle Fieldml_OpenWriter( FmlSessionHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle typeHandle, FmlBoolean append, int *sizes, int rank );

/**
 * Write out some integer values to the given data writer.
 * 
 * \return The number of values written, or -1 on error.
 * 
 * \see Fieldml_OpenWriter
 */
FmlErrorNumber Fieldml_WriteIntSlab( FmlSessionHandle handle, FmlWriterHandle writerHandle, int *offsets, int *sizes, int *valueBuffer );

/**
 * Write out some double-precision floating point values to the given data writer.
 * 
 * \return The number of values written, or -1 on error.
 * 
 * \see Fieldml_OpenWriter
 */
FmlErrorNumber Fieldml_WriteDoubleSlab( FmlSessionHandle handle, FmlWriterHandle writerHandle, int *offsets, int *sizes, double *valueBuffer );


/**
 * Closes the given raw data writer. The writer's handle should not be used after this call.
 * 
 * \see Fieldml_OpenWriter
 */
FmlErrorNumber Fieldml_CloseWriter( FmlSessionHandle handle, FmlWriterHandle writerHandle );


/**
 * Add an import source for the current region. The href will typically be the location of
 * another FieldML resource. The API will attempt to parse the given FieldML resource. If the
 * parsing process fails for any reason (e.g. non-existant file, invalid FieldML), -1 will be
 * returned. 
 * 
 * \return An import index to use with subsequent import API calls, or -1 on error.
 * 
 * \note Attempting to add the same import source more than once will succeed, but will result
 * in the same index being returned each time.
 * 
 * \note The string 'http://www.fieldml.org/resources/xml/0.4/fieldml_library.xml' will direct
 * the API to use an internally-cached version of fieldml_library.xml.
 * 
 * \note At the moment, only filenames are supported.
 * 
 * \see Fieldml_AddImport
 * \see Fieldml_CopyImportSourceHref
 * \see Fieldml_CopyImportSourceRegionName
 * \see Fieldml_GetImportCount
 * \see Fieldml_CopyImportLocalName
 * \see Fieldml_CopyImportRemoteName
 * \see Fieldml_GetImportObject
 */
int Fieldml_AddImportSource( FmlSessionHandle handle, const char * href, const char * regionName );


/**
 * Import a FieldML object from the given import source into the current region. The local name must
 * be locally unique (i.e. no other local names or imports may have that name). The remote name must be an
 * object that has either been declared in, or imported into the import source region.
 * 
 * \see Fieldml_AddImportSource
 */
FmlObjectHandle Fieldml_AddImport( FmlSessionHandle handle, int importSourceIndex, const char * localName, const char * remoteName );


/**
 * \return The number of import sources used by the current region.
 * 
 * \see Fieldml_AddImportSource
 */
int Fieldml_GetImportSourceCount( FmlSessionHandle handle );


/**
 * Copies the given import source's href into the given buffer.
 * 
 * \see Fieldml_AddImportSource
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyImportSourceHref( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength );


/**
 * Copies the given import source's region name into the given buffer.
 * 
 * \see Fieldml_AddImportSource
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyImportSourceRegionName( FmlSessionHandle handle, int importSourceIndex, char * buffer, int bufferLength );


/**
 * \return The number of objects imported into the current region from the given import source.
 *
 * \see Fieldml_AddImportSource
 * \see Fieldml_AddImport
 */
int Fieldml_GetImportCount( FmlSessionHandle handle, int importSourceIndex );


/**
 * Copies the local name of the given imported object into the given buffer.
 * 
 * \see Fieldml_AddImport
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyImportLocalName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength );


/**
 * Copies the remote name of the given imported object into the given buffer.
 * 
 * \see Fieldml_AddImport
 * \see Fieldml_CopyRegionName
 */
int Fieldml_CopyImportRemoteName( FmlSessionHandle handle, int importSourceIndex, int importIndex, char * buffer, int bufferLength );


/**
 * \return The handle for given imported object.
 * 
 * \see Fieldml_AddImport
 */
FmlObjectHandle Fieldml_GetImportObject( FmlSessionHandle handle, int importSourceIndex, int importIndex );


FmlObjectHandle Fieldml_CreateHrefDataResource( FmlSessionHandle handle, const char * name, const char * format, const char * href );

/**
 * Creates a new text-based inline data resource. The resource will initially be an empty string, but its contents
 * can be set either directly via Fieldml_AddInlineData calls, or indirectly via FieldML writer calls. The
 * text data will be serialized as character data in the FieldML document. The resulting data resource object
 * can then have one or more data sources associated with it.
 * 
 * \see Fieldml_CreateTextDataSource
 */
FmlObjectHandle Fieldml_CreateInlineDataResource( FmlSessionHandle handle, const char * name );


/**
 * \return The type of the given data resource.
 * 
 * \see Fieldml_CreateTextFileDataResource
 * \see Fieldml_CreateTextInlineDataResource
 */
DataResourceType Fieldml_GetDataResourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Creates a new text-based data source. The given data resource must also be text-based. The remaining parameters
 * are used by data readers to pre-process the raw data. Once opened, the data reader will skip forward until firstLine
 * (if firstLine is 1, no lines will be skipped). From that point, the text data will be treated as a sequence of numbers
 * separated but non-numerical characters (e.g. commas, spaces, linefeeds). Any combination of such characters will
 * be treated as a single delimiter. Decimal points and exponential notation is permitted when reading doubles, but
 * will generate an error when reading integers. Each record will then be read by skipping 'head' numbers,
 * reading 'length' numbers, then skipping 'tail' more numbers. For more information, see the FieldML documentation. 
 * 
 * \see Fieldml_OpenReader
 * \see Fieldml_GetDataSourceResource
 */
FmlObjectHandle Fieldml_CreateArrayDataSource( FmlSessionHandle handle, const char * name, FmlObjectHandle resourceHandle, const char * location, int rank );

/**
 * \return The number of data sources associated with the given data resource.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_GetDataSourceByIndex
 */
int Fieldml_GetDataSourceCount( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return The nth data source associated with the given data resource.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_GetDataSourceCount
 */
FmlObjectHandle Fieldml_GetDataSourceByIndex( FmlSessionHandle handle, FmlObjectHandle objectHandle, int index );


/**
 * \return The data resource used by the given data source.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_CreateTextFileDataResource
 * \see Fieldml_CreateTextInlineDataResource
 */
FmlObjectHandle Fieldml_GetDataSourceResource( FmlSessionHandle handle, FmlObjectHandle objectHandle );

const char * Fieldml_GetArrayDataSourceLocation( FmlSessionHandle handle, FmlObjectHandle objectHandle );

int Fieldml_CopyArrayDataSourceLocation( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength );


/**
 * \return The array rank for the given array data source.
 * 
 * \see Fieldml_CreateTextDataSource
 */
int Fieldml_GetArrayDataSourceRank( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Raw sizes are optional. Values will be set to 0 if not present.
 */
FmlErrorNumber Fieldml_GetArrayDataSourceRawSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes );


FmlErrorNumber Fieldml_SetArrayDataSourceRawSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes );

/**
 * Get the offsets of the array data accessible via the given data source.
 * 
 * \see Fieldml_SetArrayDataSourceOffsets
 */
FmlErrorNumber Fieldml_GetArrayDataSourceOffsets( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *offsets );


/**
 * Sets the offsets of the array data accessible via the given data source. These are offsets into the containing
 * array exposed via the data source's associated resource.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_CreateBinaryArrayDataSource
 */
FmlErrorNumber Fieldml_SetArrayDataSourceOffsets( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *offsets );


/**
 * Get the sizes of the array data accessible via the given data source.
 * 
 * \see Fieldml_SetArrayDataSourceSizes
 */
FmlErrorNumber Fieldml_GetArrayDataSourceSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes );


/**
 * Sets the sizes of the array data accessible via the given data source.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_CreateBinaryArrayDataSource
 */
FmlErrorNumber Fieldml_SetArrayDataSourceSizes( FmlSessionHandle handle, FmlObjectHandle objectHandle, int *sizes );


/**
 * \return The data source type of the given data source.
 * 
 * \see Fieldml_CreateTextDataSource
 * \see Fieldml_CreateBinaryArrayDataSource
 */
DataSourceType Fieldml_GetDataSourceType( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Appends the given string to the given data resource's inline data. The data resource's type must be
 * DataResourceType::DATA_RESOURCE_TEXT_INLINE.
 * 
 * \see Fieldml_CreateTextInlineDataResource
 */
FmlErrorNumber Fieldml_AddInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, const char * data, const int length );


/**
 * \return The number of characters in the data resource's inline data.
 * 
 * \see Fieldml_CreateTextInlineDataResource
 */
int Fieldml_GetInlineDataLength( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * \return A pointer to the data resource's inline data.
 * 
 * \warning This is obviously dangerous and should be replaced with something safer. It is recommended that
 * Fieldml_OpenReader() and associated API be used instead.

 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 * \see Fieldml_CreateTextInlineDataResource
 */
char * Fieldml_GetInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies a section of the data resource's inline data into the given buffer, starting from the given offset, and ending
 * either when the buffer is full, or the end of the inline data is reached.
 * 
 * \see Fieldml_CopyRegionName
 * \see Fieldml_CreateTextInlineDataResource
 */
int Fieldml_CopyInlineData( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength, int offset );

/**
 * \return The href of the data resource's file. The data resource's type must be DataResourceType::DATA_RESOURCE_TEXT_HREF.

 * \see Fieldml_GetRegionName
 * \see Fieldml_FreeString
 * \see Fieldml_CreateTextFileDataResource
 */
char * Fieldml_GetDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies the given data resource's href into the given buffer. The data resource's type must be DataResourceType::DATA_RESOURCE_TEXT_HREF.
 *  
 * \see Fieldml_CopyRegionName
 * \see Fieldml_CreateTextFileDataResource
 */
int Fieldml_CopyDataResourceHref( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength );

/**
 * \return The data format of the given data resource's format into the given buffer. The data resource's type must be DataResourceType::DATA_RESOURCE_ARRAY.
 * 
 * \see Fieldml_CreateArrayDataResource
 */
char * Fieldml_GetDataResourceFormat( FmlSessionHandle handle, FmlObjectHandle objectHandle );


/**
 * Copies the given data resource's format into the given buffer. The data resource's type must be DataResourceType::DATA_RESOURCE_ARRAY.
 * 
 * \see Fieldml_CreateArrayDataResource
 */
int Fieldml_CopyDataResourceFormat( FmlSessionHandle handle, FmlObjectHandle objectHandle, char * buffer, int bufferLength );
}

#endif // __cplusplus

#endif // H_FIELDML_API
