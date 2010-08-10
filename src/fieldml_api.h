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
#define FML_MINOR_VERSION               2
#define FML_DOT_VERSION                 0

#define FML_ERR_NO_ERROR                0
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

#define FML_ERR_UNSUPPORTED             2000  //Used for operations that are valid, but not yet implemented.

/*

 Types

*/

enum DomainBoundsType
{
    BOUNDS_UNKNOWN,              // EnsembleDomain bounds not yet known.
    BOUNDS_DISCRETE_CONTIGUOUS,  // Contiguous bounds (i.e. 1 ... N)
    BOUNDS_DISCRETE_ARBITRARY,   // Arbitrary bounds (not yet supported)
};


enum DataFileType
{
    TYPE_UNKNOWN,
    TYPE_TEXT,                  // Text file with CSV/space delimited numbers. Offset is numbers.
    TYPE_LINES,                 // Formatted text file. Offset is lines. CSV/space delimited numbers expected at offset.
};


typedef enum DataDescriptionType
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
    
    FHT_ENSEMBLE_DOMAIN,
    FHT_CONTINUOUS_DOMAIN,
    FHT_MESH_DOMAIN,
    FHT_CONTINUOUS_REFERENCE,
    FHT_ENSEMBLE_PARAMETERS,
    FHT_CONTINUOUS_PARAMETERS,
    FHT_CONTINUOUS_PIECEWISE,
    FHT_CONTINUOUS_AGGREGATE,
    FHT_CONTINUOUS_VARIABLE,
    FHT_ENSEMBLE_VARIABLE,
    FHT_REMOTE_ENSEMBLE_DOMAIN,
    FHT_REMOTE_CONTINUOUS_DOMAIN,
    FHT_REMOTE_ENSEMBLE_EVALUATOR,
    FHT_REMOTE_CONTINUOUS_EVALUATOR,
    
    //These are stand-in types used to allow forward-declaration during parsing.
    FHT_UNKNOWN_ENSEMBLE_DOMAIN,
    FHT_UNKNOWN_CONTINUOUS_DOMAIN,
    FHT_UNKNOWN_ENSEMBLE_EVALUATOR,
    FHT_UNKNOWN_CONTINUOUS_EVALUATOR,
    FHT_UNKNOWN_ENSEMBLE_SOURCE,
    FHT_UNKNOWN_CONTINUOUS_SOURCE,
};


typedef class ParameterReader *FmlReaderHandle;

typedef class ParameterWriter *FmlWriterHandle;

typedef int FmlObjectHandle;

typedef class FieldmlRegion *FmlHandle;

/*

 API

*/

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
 *      Creates an empty FieldML handle. The built-in library is still implicitly
 *      included. Data files will be created at the given location.
 */
FmlHandle Fieldml_Create( const char *location, const char *name );

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
 *      be discarded after this call.
 */
void Fieldml_Destroy( FmlHandle handle );


/**
 *      Returns the number of parsing errors encountered by the given handle during parsing.
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
FmlObjectHandle Fieldml_GetNamedObject( FmlHandle handle, const char * name );


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
 *      Returns the number of markup entries (attribute/value pairs) for the given object.
 */
int Fieldml_GetMarkupCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns the attribute string of the nth markup entry for the given object.
 */
const char * Fieldml_GetMarkupAttribute( FmlHandle handle, FmlObjectHandle objectHandle, int markupIndex );
int Fieldml_CopyMarkupAttribute( FmlHandle handle, FmlObjectHandle objectHandle, int markupIndex, char *buffer, int bufferLength );


/**
 *      Returns the value string of the nth markup entry for the given object.
 */
const char * Fieldml_GetMarkupValue( FmlHandle handle, FmlObjectHandle objectHandle, int markupIndex );
int Fieldml_CopyMarkupValue( FmlHandle handle, FmlObjectHandle objectHandle, int markupIndex, char *buffer, int bufferLength );


/**
 *      Returns the value string of the markup entry with the given attribute.
 */
const char * Fieldml_GetMarkupAttributeValue( FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute );
int Fieldml_CopyMarkupAttributeValue( FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute, char *buffer, int bufferLength );


/**
 *      Sets the value string of the markup entry with the given attribute.
 */
int Fieldml_SetMarkup(  FmlHandle handle, FmlObjectHandle objectHandle, const char * attribute, const char * value );


/**
 *      Validates the given object. Returns FML_ERR_NO_ERROR if the object is valid,
 *      or an appropriate error code if not. Note that FieldML objects are currently allowed
 *      to remain invalid up to and including at serialization time.
 */
int Fieldml_ValidateObject( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns the handle of the given domain's component ensemble.
 */
FmlObjectHandle Fieldml_GetDomainComponentEnsemble( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Helper function that returns the element count of the component domain of the
 * given domain.
 */
int Fieldml_GetDomainComponentCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Creates an ensemble domain with the given name and component domain.
 */
FmlObjectHandle Fieldml_CreateEnsembleDomain( FmlHandle handle, const char * name, FmlObjectHandle componentHandle );


/**
 *  Creates an component ensemble domain with the given name.
 *      
 *  NOTE: A component ensemble domain is identical to an ensemble domain except that only component
 *  ensembles can be used when creating multi-component domains.     
 */
FmlObjectHandle Fieldml_CreateComponentEnsembleDomain( FmlHandle handle, const char * name );


/**
 *      Creates a continuous domain with the given name and component domain.
 */
FmlObjectHandle Fieldml_CreateContinuousDomain( FmlHandle handle, const char * name, FmlObjectHandle componentHandle );


/**
 *      Creates a mesh domain with the given name, and with a xi-space specified by the given ensemble.
 *      Each mesh has its own unique element and xi domain, which can be accessed by the relevant functions.
 *      Because the xi and element domains have a name based on the mesh name, care must be taken to ensure
 *      that neither the mesh's name, nor it's element or xi domain names are already in use. 
 */
FmlObjectHandle Fieldml_CreateMeshDomain( FmlHandle handle, const char * name, FmlObjectHandle xiEnsemble );


/**
 *      Returns the handle of the given mesh domain's xi domain. This is a unique, n-dimensional
 *      continuous domain with the component domain specified when the mesh was created. If the
 *      mesh's name is "*", the xi domain's name is "*.xi"
 */
FmlObjectHandle Fieldml_GetMeshXiDomain( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *     Returns the handle of the given mesh domain's element domain. This is a unique ensemble
 *     domain whose bounds is specified by the user. If the mesh's name is "*", the element domain's
 *     name is "*.element"
 */
FmlObjectHandle Fieldml_GetMeshElementDomain( FmlHandle handle, FmlObjectHandle objectHandle );


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
 *      Returns the number of connectivities specified for the given mesh domain. Each
 *      connectivity links an ensemble evaluator with a corresponding ensemble point layout ensemble.
 *      
 *      NOTE: The point locations associated with each point layout ensemble are not yet specified.
 */
int Fieldml_GetMeshConnectivityCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *      Returns the domain of the nth connectivity for the given mesh. 
 */
FmlObjectHandle Fieldml_GetMeshConnectivityDomain( FmlHandle handle, FmlObjectHandle objectHandle, int connectivityIndex );


/*
     Returns the source evaluator of the nth connectivity for the given mesh. 
 */
FmlObjectHandle Fieldml_GetMeshConnectivitySource( FmlHandle handle, FmlObjectHandle objectHandle, int connectivityIndex );


int Fieldml_SetMeshConnectivity( FmlHandle handle, FmlObjectHandle mesh, FmlObjectHandle evaluator, FmlObjectHandle pointDomain );

/**
    Returns the bounds-type of the given domain.
    
    NOTE: Currently, only ensemble domains have explicit bounds.
    NOTE: Currently, only discrete contiguous bounds are supported.
 */
DomainBoundsType Fieldml_GetDomainBoundsType( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the number of elements in the given ensemble domain. 
 */
int Fieldml_GetEnsembleDomainElementCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns 1 if the ensemble domain is a component domain, 0 if not, -1 on error.
 */
int Fieldml_IsEnsembleComponentDomain( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the names of the elements in the given ensemble domain. For now, the only supported element names are
 * positive integers.
 * 
 * NOTE: If the bounds are contiguous, the names will simply be 1, 2, 3... n. 
 */
//NYI int Fieldml_GetEnsembleDomainElementNames( FmlHandle handle, FmlObjectHandle objectHandle, const int *array, int arrayLength );


/**
 * Returns the number of elements in the given domains contiguous bounds.
 * Returns -1 if the domain does not have contiguous bounds.
 * 
 * For convenience, this function can be called on a mesh domain, in which case the
 * bounds of the mesh's element domain is returned.
 */
int Fieldml_GetContiguousBoundsCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the given ensemble domain to have contiguous bounds, with elements numbered from
 * 1 to count.
 * 
 * For convenience, this function can be called on a mesh domain, in which case the
 * bounds of the mesh's element domain is set.
 */
int Fieldml_SetContiguousBoundsCount( FmlHandle handle, FmlObjectHandle objectHandle, int count );


/**
     Returns the value domain of the given evaluator.
 */
FmlObjectHandle Fieldml_GetValueDomain( FmlHandle handle, FmlObjectHandle objectHandle );





/**
 * Creates an ensemble-valued variable. A variable is a placeholder for an evaluator. Variables can be
 * aliased by evaluators, or even other variables.
 */
FmlObjectHandle Fieldml_CreateEnsembleVariable( FmlHandle handle, const char *name, FmlObjectHandle valueDomain );


/**
 * Creates an continuous-valued variable. A variable is a placeholder for an evaluator. Variables can be
 * aliased by evaluators, or even other variables.
 */
FmlObjectHandle Fieldml_CreateContinuousVariable( FmlHandle handle, const char *name, FmlObjectHandle valueDomain );


/**
 * Creates an ensemble-valued parameter set. A parameter set contains a store of literal values, indexed by a set
 * of ensembles. The format and location of the store is intended to be very flexible, and include the ability
 * to describe 3rd-party formats such as HDF5, and allow for locations that refer to inline data, as well as
 * files on the local filesystem, or over the network.
 */
FmlObjectHandle Fieldml_CreateEnsembleParameters( FmlHandle handle, const char *name, FmlObjectHandle valueDomain );


/**
 * Creates an continuous-valued parameter set. A parameter set contains a store of literal values, indexed by a set
 * of ensembles. The format and location of the store is intended to be very flexible, and include the ability
 * to describe 3rd-party formats such as HDF5, and allow for locations that refer to inline data, as well as
 * files on the local filesystem, or over the network.
 */
FmlObjectHandle Fieldml_CreateContinuousParameters( FmlHandle handle, const char *name, FmlObjectHandle valueDomain );


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
 * Adds an index to the given parameter set's semidense data description.
 */
int Fieldml_AddSemidenseIndex( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle indexHandle, int isSparse );

/**
 *   Returns the number of sparse or dense indexes of the semidense data store
 *   associated with the given parameter evaluator.
 */
int Fieldml_GetSemidenseIndexCount( FmlHandle handle, FmlObjectHandle objectHandle, int isSparse );


/**
 *   Returns the handle of the nth sparse or dense index of the semidense data
 *   store associated with the given parameter evaluator.
 */
FmlObjectHandle Fieldml_GetSemidenseIndex( FmlHandle handle, FmlObjectHandle objectHandle, int indexIndex, int isSparse );


/**
 * Sets the swizzle indexes for the given parameter set. The swizzle is only applied to the innermost dense index.
 */
int Fieldml_SetSwizzle( FmlHandle handle, FmlObjectHandle objectHandle, const int *buffer, int count );


/**
 * Gets the number of swizzle indexes for the given parameter set, or zero if no swizzle is defined.
 */
int Fieldml_GetSwizzleCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Returns the swizzle indexes, if present.
 */
const int *Fieldml_GetSwizzleData( FmlHandle handle, FmlObjectHandle objectHandle );
int Fieldml_CopySwizzleData( FmlHandle handle, FmlObjectHandle objectHandle, int *buffer, int bufferLength );


/**
 * Creates a new continuous piecewise evaluator, using the given index ensemble. Evaluators must all have the same
 * value domain as the piecewise itself. Piecewise evaluators need not be defined for all possible index values.
 * 
 * NOTE: Ensemble piecewise evaluators are not yet supported.
 */
FmlObjectHandle Fieldml_CreateContinuousPiecewise( FmlHandle handle, const char * name, FmlObjectHandle indexHandle, FmlObjectHandle valueDomain );


/**
 * Creates a new continuous aggregate evaluator. The aggregate's index is the value domain's component ensemble.
 * Evaluators must all be scalar continuous. The value is obtained by aggregating the scalar evaluators over the
 * value domain's component ensemble. There must be an entry for each index.
 * 
 * NOTE: Ensemble aggregate evaluators are not yet supported.
 */
FmlObjectHandle Fieldml_CreateContinuousAggregate( FmlHandle handle, const char * name, FmlObjectHandle valueDomain );


/**
 * Sets the default evaluator for the given piecewise evaluator.
 */
int Fieldml_SetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle evaluator );


/**
 * Returns the default evaluator for the given piecewise evaluator.
 */
int Fieldml_GetDefaultEvaluator( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Sets the evaluator on the given index for the given aggreate or piecewise evaluator.
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
    
    NOTE: Only defined for piecewise and parameter evaluators.
    
    NOTE: For piecewise evalutors, this is always one. For parameter evaluators,
    it depends on the data store.
 */
int Fieldml_GetIndexCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the domain of the nth index used by the given evaluator.
 *  
 *  NOTE: Only defined for piecewise and parameter evaluators.
 */
FmlObjectHandle Fieldml_GetIndexDomain( FmlHandle handle, FmlObjectHandle objectHandle, int indexIndex );


/**
 * Creates a continuous reference evaluator. Reference evaluators delegate their evaluation to another evaluator, and may alias
 * domains and evaluators before doing so.
 * 
 * NOTE: Ensemble references are not yet supported.
 */
FmlObjectHandle Fieldml_CreateContinuousReference( FmlHandle handle, const char * name, FmlObjectHandle remoteEvaluator, FmlObjectHandle valueDomain );


/**
 * Gets the remote evaluator which the given evaluator references.
 */
FmlObjectHandle Fieldml_GetReferenceRemoteEvaluator( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Adds an alias to the given reference evaluator. Domains in the remote evaluator may be aliased by domains, evaluators
 * or variables. Variables in the remote evaluator may be aliases by evaluators or variables. 
 */
int Fieldml_SetAlias( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle remoteDomain, FmlObjectHandle localSource );


/**
 *  Returns the number of aliases used by the given evaluator. 
 */
int Fieldml_GetAliasCount( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 *  Returns the local domain/evaulator used by the nth alias of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetAliasLocal( FmlHandle handle, FmlObjectHandle objectHandle, int aliasIndex );


/**
 *  Returns the remote object used by the nth alias of the given evaluator. 
 */
FmlObjectHandle Fieldml_GetAliasRemote( FmlHandle handle, FmlObjectHandle objectHandle, int aliasIndex );


/**
 * Returns the alias of the given remote object.
 */
FmlObjectHandle Fieldml_GetAliasByRemote( FmlHandle handle, FmlObjectHandle objectHandle, FmlObjectHandle remoteHandle );


/**
 * Creates a new reader for the given parameter evaluator's raw data.
 */
FmlReaderHandle Fieldml_OpenReader( FmlHandle handle, FmlObjectHandle objectHandle );


/**
 * Reads in a slice consisting of n index values, followed by m data values. n is the number of sparse indexes,
 * m is the product of the cardinalities of the dense indexes. Only valid for an ensemble parameters object using
 * DESCRIPTION_SEMIDENSE data.
 */
int Fieldml_ReadIntSlice( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer, int *valueBuffer );


/**
 * Reads in a slice consisting of n index values, followed by m data values. n is the number of sparse indexes,
 * m is the product of the cardinalities of the dense indexes. Only valid for an continuous parameters object using
 * DESCRIPTION_SEMIDENSE data.
 */
int Fieldml_ReadDoubleSlice( FmlHandle handle, FmlReaderHandle reader, int *indexBuffer, double *valueBuffer );


/**
 * Closes the given raw data reader.
 */
int Fieldml_CloseReader( FmlHandle handle, FmlReaderHandle reader );


/**
 * Creates a new writer for the given parameter evaluator's raw data.
 */
FmlWriterHandle Fieldml_OpenWriter( FmlHandle handle, FmlObjectHandle objectHandle, int append );


/**
 * Writes out a slice consisting of n index values, followed by m data values. n is the number of sparse indexes,
 * m is the product of the cardinalities of the dense indexes. Only valid for an ensemble parameters object using
 * DESCRIPTION_SEMIDENSE data.
 */
int Fieldml_WriteIntSlice( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer, int *valueBuffer );


/**
 * Writes out a slice consisting of n index values, followed by m data values. n is the number of sparse indexes,
 * m is the product of the cardinalities of the dense indexes. Only valid for an continuous parameters object using
 * DESCRIPTION_SEMIDENSE data.
 */
int Fieldml_WriteDoubleSlice( FmlHandle handle, FmlWriterHandle writer, int *indexBuffer, double *valueBuffer );


/**
 * Closes the given raw data writer.
 */
int Fieldml_CloseWriter( FmlHandle handle, FmlWriterHandle writer );

#endif // H_FIELDML_API
