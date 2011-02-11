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

const char * const FML_VERSION_STRING                  = "0.3_alpha";

const xmlChar * const FIELDML_TAG                      = (const xmlChar* const)"fieldml";

const xmlChar * const REGION_TAG                       = (const xmlChar* const)"Region";

const xmlChar * const ENSEMBLE_TYPE_TAG                = (const xmlChar* const)"EnsembleType";
const xmlChar * const BOUNDS_TAG                       = (const xmlChar* const)"bounds";
const xmlChar * const CONTIGUOUS_ENSEMBLE_BOUNDS_TAG   = (const xmlChar* const)"ContiguousEnsembleBounds";
const xmlChar * const ARBITRARY_ENSEMBLE_BOUNDS_TAG    = (const xmlChar* const)"ArbitraryEnsembleBounds";

const xmlChar * const CONTINUOUS_TYPE_TAG              = (const xmlChar* const)"ContinuousType";

const xmlChar * const MESH_TYPE_TAG                    = (const xmlChar* const)"MeshType";
const xmlChar * const MESH_XI_TAG                      = (const xmlChar* const)"xi";
const xmlChar * const MESH_SHAPES_TAG                  = (const xmlChar* const)"shapes";
const xmlChar * const MESH_SHAPE_TAG                   = (const xmlChar* const)"shape";

const xmlChar * const ELEMENT_SET_TAG                  = (const xmlChar* const)"ElementSet";
const xmlChar * const ELEMENTS_TAG                     = (const xmlChar* const)"elements";

const xmlChar * const REFERENCE_EVALUATOR_TAG          = (const xmlChar* const)"ReferenceEvaluator";
const xmlChar * const BINDS_TAG                        = (const xmlChar* const)"binds";
const xmlChar * const BIND_TAG                         = (const xmlChar* const)"bind";
const xmlChar * const BIND_INDEX_TAG                   = (const xmlChar* const)"bind_index";

const xmlChar * const VARIABLES_TAG                    = (const xmlChar* const)"variables";
const xmlChar * const VARIABLE_TAG                     = (const xmlChar* const)"variable";

const xmlChar * const ABSTRACT_EVALUATOR_TAG           = (const xmlChar* const)"AbstractEvaluator";

const xmlChar * const PARAMETER_EVALUATOR_TAG          = (const xmlChar* const)"ParametersEvaluator";
const xmlChar * const SEMI_DENSE_DATA_TAG              = (const xmlChar* const)"SemidenseData";
const xmlChar * const SPARSE_INDEXES_TAG               = (const xmlChar* const)"sparseIndexes";
const xmlChar * const DENSE_INDEXES_TAG                = (const xmlChar* const)"denseIndexes";
const xmlChar * const INDEX_TAG                        = (const xmlChar* const)"index";

const xmlChar * const PIECEWISE_EVALUATOR_TAG          = (const xmlChar* const)"PiecewiseEvaluator";
const xmlChar * const ELEMENT_EVALUATORS_TAG           = (const xmlChar* const)"elementEvaluators";
const xmlChar * const ELEMENT_TAG                      = (const xmlChar* const)"element";

const xmlChar * const AGGREGATE_EVALUATOR_TAG          = (const xmlChar* const)"AggregateEvaluator";
const xmlChar * const COMPONENT_EVALUATORS_TAG         = (const xmlChar* const)"componentEvaluators";
const xmlChar * const COMPONENT_TAG                    = (const xmlChar* const)"component";

const xmlChar * const ENTRY_TAG                        = (const xmlChar* const)"entry";

const xmlChar * const DATA_LOCATION_TAG                = (const xmlChar* const)"dataLocation";

const xmlChar * const INLINE_DATA_TAG                  = (const xmlChar* const)"inlineData";
const xmlChar * const FILE_DATA_TAG                    = (const xmlChar* const)"fileData";
const xmlChar * const SWIZZLE_TAG                      = (const xmlChar* const)"swizzle";

const xmlChar * const NAME_TAG                         = (const xmlChar* const)"name";



const xmlChar * const VERSION_ATTRIB                   = (const xmlChar* const)"version";

const xmlChar * const NAME_ATTRIB                      = (const xmlChar* const)"name";

const xmlChar * const VARIABLE_ATTRIB                  = (const xmlChar* const)"variable";
const xmlChar * const INDEX_NUMBER_ATTRIB              = (const xmlChar* const)"index_number";
const xmlChar * const SOURCE_ATTRIB                    = (const xmlChar* const)"source";

const xmlChar * const COMPONENT_ENSEMBLE_ATTRIB        = (const xmlChar* const)"componentEnsemble";

const xmlChar * const IS_COMPONENT_ENSEMBLE_ATTRIB     = (const xmlChar* const)"isComponentEnsemble";

const xmlChar * const VALUE_TYPE_ATTRIB                = (const xmlChar* const)"valueType";

const xmlChar * const KEY_ATTRIB                       = (const xmlChar* const)"key";
const xmlChar * const VALUE_ATTRIB                     = (const xmlChar* const)"value";
const xmlChar * const DEFAULT_ATTRIB                   = (const xmlChar* const)"default";

const xmlChar * const VALUE_COUNT_ATTRIB                = (const xmlChar* const)"valueCount";

const xmlChar * const XI_COMPONENT_ATTRIB               = (const xmlChar* const)"xi_component";

const xmlChar * const NUMBER_ATTRIB                     = (const xmlChar* const)"number";
const xmlChar * const EVALUATOR_ATTRIB                  = (const xmlChar* const)"evaluator";
const xmlChar * const ELEMENT_SET_ATTRIB                = (const xmlChar* const)"element_set";

const xmlChar * const INDEX_EVALUATOR_ATTRIB            = (const xmlChar* const)"indexEvaluator";

const xmlChar * const FILE_ATTRIB                       = (const xmlChar* const)"file";
const xmlChar * const TYPE_ATTRIB                       = (const xmlChar* const)"type";
const xmlChar * const OFFSET_ATTRIB                     = (const xmlChar* const)"offset";

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
