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
#include <string.h>

#include "string_const.h"

using namespace std;

const char WIN_PATH_SEP = '\\';
const char NIX_PATH_SEP = '/';

#ifdef WIN32
#define DEFAULT_SEP WIN_PATH_SEP
#else
#define DEFAULT_SEP NIX_PATH_SEP
#endif

const char * const FML_VERSION_STRING                  = "0.3.5_alpha";

const xmlChar * const FIELDML_TAG                      = (const xmlChar* const)"Fieldml";

const xmlChar * const REGION_TAG                       = (const xmlChar* const)"Region";

const xmlChar * const IMPORT_TAG                       = (const xmlChar* const)"Import";
const xmlChar * const IMPORT_TYPE_TAG                  = (const xmlChar* const)"ImportType";
const xmlChar * const IMPORT_EVALUATOR_TAG             = (const xmlChar* const)"ImportEvaluator";

const xmlChar * const ENSEMBLE_TYPE_TAG                = (const xmlChar* const)"EnsembleType";

const xmlChar * const CONTINUOUS_TYPE_TAG              = (const xmlChar* const)"ContinuousType";
const xmlChar * const COMPONENTS_TAG                   = (const xmlChar* const)"Components";

const xmlChar * const MESH_TYPE_TAG                    = (const xmlChar* const)"MeshType";
const xmlChar * const MESH_SHAPES_TAG                  = (const xmlChar* const)"Shapes";
const xmlChar * const MESH_SHAPE_TAG                   = (const xmlChar* const)"Shape";

const xmlChar * const ELEMENT_SEQUENCE_TAG             = (const xmlChar* const)"ElementSequence";
const xmlChar * const ELEMENTS_TAG                     = (const xmlChar* const)"Elements";
const xmlChar * const XI_TAG                           = (const xmlChar* const)"Xi";
const xmlChar * const MEMBERS_TAG                      = (const xmlChar* const)"Members";

const xmlChar * const MEMBER_RANGE_TAG                 = (const xmlChar* const)"MemberRange";
const xmlChar * const MEMBER_LIST_DATA_TAG             = (const xmlChar* const)"MemberListData";
const xmlChar * const MEMBER_RANGE_DATA_TAG            = (const xmlChar* const)"MemberRangeData";
const xmlChar * const MEMBER_STRIDE_RANGE_DATA_TAG     = (const xmlChar* const)"MemberStrideRangeData";

const xmlChar * const REFERENCE_EVALUATOR_TAG          = (const xmlChar* const)"ReferenceEvaluator";
const xmlChar * const BINDINGS_TAG                     = (const xmlChar* const)"Bindings";
const xmlChar * const BIND_TAG                         = (const xmlChar* const)"Bind";
const xmlChar * const BIND_INDEX_TAG                   = (const xmlChar* const)"BindIndex";

const xmlChar * const VARIABLES_TAG                    = (const xmlChar* const)"Variables";
const xmlChar * const VARIABLE_TAG                     = (const xmlChar* const)"Variable";

const xmlChar * const ABSTRACT_EVALUATOR_TAG           = (const xmlChar* const)"AbstractEvaluator";

const xmlChar * const EXTERNAL_EVALUATOR_TAG           = (const xmlChar* const)"ExternalEvaluator";

const xmlChar * const PARAMETER_EVALUATOR_TAG          = (const xmlChar* const)"ParametersEvaluator";
const xmlChar * const SEMI_DENSE_DATA_TAG              = (const xmlChar* const)"SemidenseData";
const xmlChar * const SPARSE_INDEXES_TAG               = (const xmlChar* const)"SparseIndexes";
const xmlChar * const DENSE_INDEXES_TAG                = (const xmlChar* const)"DenseIndexes";
const xmlChar * const INDEX_EVALUATORS_TAG             = (const xmlChar* const)"IndexEvaluators";
const xmlChar * const INDEX_EVALUATOR_TAG              = (const xmlChar* const)"IndexEvaluator";

const xmlChar * const PIECEWISE_EVALUATOR_TAG          = (const xmlChar* const)"PiecewiseEvaluator";
const xmlChar * const ELEMENT_EVALUATORS_TAG           = (const xmlChar* const)"ElementEvaluators";
const xmlChar * const ELEMENT_EVALUATOR_TAG            = (const xmlChar* const)"ElementEvaluator";

const xmlChar * const AGGREGATE_EVALUATOR_TAG          = (const xmlChar* const)"AggregateEvaluator";
const xmlChar * const COMPONENT_EVALUATORS_TAG         = (const xmlChar* const)"ComponentEvaluators";
const xmlChar * const COMPONENT_EVALUATOR_TAG          = (const xmlChar* const)"ComponentEvaluator";

const xmlChar * const DATA_OBJECT_TAG                  = (const xmlChar* const)"DataObject";
const xmlChar * const SOURCE_TAG                       = (const xmlChar* const)"Source";
const xmlChar * const TEXT_FILE_SOURCE_TAG             = (const xmlChar* const)"TextFileSource";
const xmlChar * const INLINE_SOURCE_TAG                = (const xmlChar* const)"InlineSource";
const xmlChar * const ENTRIES_TAG                      = (const xmlChar* const)"Entries";



const xmlChar * const VERSION_ATTRIB                   = (const xmlChar* const)"version";

const xmlChar * const NAME_ATTRIB                      = (const xmlChar* const)"name";

const xmlChar * const LOCATION_ATTRIB                  = (const xmlChar* const)"location";
const xmlChar * const REGION_ATTRIB                    = (const xmlChar* const)"region";
const xmlChar * const LOCAL_NAME_ATTRIB                = (const xmlChar* const)"localName";
const xmlChar * const REMOTE_NAME_ATTRIB               = (const xmlChar* const)"remoteName";

const xmlChar * const VARIABLE_ATTRIB                  = (const xmlChar* const)"variable";
const xmlChar * const INDEX_NUMBER_ATTRIB              = (const xmlChar* const)"indexNumber";
const xmlChar * const SOURCE_ATTRIB                    = (const xmlChar* const)"source";

const xmlChar * const COMPONENT_ENSEMBLE_ATTRIB        = (const xmlChar* const)"componentEnsemble";

const xmlChar * const VALUE_TYPE_ATTRIB                = (const xmlChar* const)"valueType";

const xmlChar * const KEY_ATTRIB                       = (const xmlChar* const)"key";
const xmlChar * const VALUE_ATTRIB                     = (const xmlChar* const)"value";
const xmlChar * const DEFAULT_ATTRIB                   = (const xmlChar* const)"default";

const xmlChar * const INDEX_VALUE_ATTRIB               = (const xmlChar* const)"indexValue";
const xmlChar * const COMPONENT_ATTRIB                 = (const xmlChar* const)"component";
const xmlChar * const EVALUATOR_ATTRIB                 = (const xmlChar* const)"evaluator";
const xmlChar * const ELEMENT_SEQUENCE_ATTRIB          = (const xmlChar* const)"elementSequence";
const xmlChar * const ORDER_ATTRIB                     = (const xmlChar* const)"order";

const xmlChar * const MIN_ATTRIB                       = (const xmlChar* const)"min";
const xmlChar * const MAX_ATTRIB                       = (const xmlChar* const)"max";
const xmlChar * const STRIDE_ATTRIB                    = (const xmlChar* const)"stride";

const xmlChar * const INDEX_EVALUATOR_ATTRIB           = (const xmlChar* const)"indexEvaluator";

const xmlChar * const FILE_ATTRIB                      = (const xmlChar* const)"file";
const xmlChar * const TYPE_ATTRIB                      = (const xmlChar* const)"type";
const xmlChar * const OFFSET_ATTRIB                    = (const xmlChar* const)"offset";

const xmlChar * const DATA_ATTRIB                      = (const xmlChar* const)"data";

const xmlChar * const FILENAME_ATTRIB                  = (const xmlChar* const)"filename";
const xmlChar * const FIRST_LINE_ATTRIB                = (const xmlChar* const)"firstLine";
const xmlChar * const COUNT_ATTRIB                     = (const xmlChar* const)"count";
const xmlChar * const LENGTH_ATTRIB                    = (const xmlChar* const)"length";
const xmlChar * const HEAD_ATTRIB                      = (const xmlChar* const)"head";
const xmlChar * const TAIL_ATTRIB                      = (const xmlChar* const)"tail";

const char * const STRING_TYPE_TEXT                 = "text";
const char * const STRING_TYPE_LINES                = "lines";

const char * const STRING_TRUE                      = "true";

// strndup not available on all platforms.
const char *strdupN( const char *str, unsigned int len )
{
    char *dup;
    
    if( str == NULL )
    {
        return NULL;
    }

    const int srclen = strlen( str );
    if( srclen < len )
    {
        len = srclen;
    }

    //We use malloc here, because we're potentially passing the result out into a C-only universe.
    dup = (char*)malloc(len + 1);
    memcpy( dup, str, len );
    dup[len] = 0;

    return dup;
}


const char *strdupS( const char *str )
{
    if( str == NULL )
    {
        return NULL;
    }
    
    return strdupN( str, strlen( str ) );
}


const string getDirectory( const string filename )
{
    int index;
    
    index = filename.rfind( NIX_PATH_SEP );
#ifdef WIN32
    if( ( filename.rfind( WIN_PATH_SEP ) > index ) || ( index == string::npos ) )
    {
        index = filename.rfind( WIN_PATH_SEP );
    }
#endif
    
    if( index == string::npos )
    {
        return string();
    }
    else
    {
        return filename.substr( 0, index );
    }
}


const string makeFilename( const string dir, const string file )
{
    if( file.length() == 0 )
    {
        return file;
    }

    if( dir.length() > 0 )
    {
        return dir + DEFAULT_SEP + file;
    }
    
    return file;
}
