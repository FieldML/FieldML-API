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
 * Version 1.1 (the \"License\"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an \"AS IS\"
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
 * either the GNU General Public License Version 2 or later (the \"GPL\"), or
 * the GNU Lesser General Public License Version 2.1 or later (the \"LGPL\"),
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

#include "String_InternalXSD.h"

const char * const FML_STRING_FIELDML_XSD_LOCATION = "http://www.fieldml.org/resources/xml/0.4/FieldML_0.4.xsd";

const char * const FML_STRING_FIELDML_XSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?> \
<xs:schema id=\"fieldml\" \
    xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" \
    xmlns:xlink=\"http://www.w3.org/1999/xlink\" \
    > \
 \
    <xs:import namespace=\"http://www.w3.org/1999/xlink\" \
            schemaLocation=\"http://www.cellml.org/tools/cellml_1_1_schema/common/xlink-href.xsd\" /> \
 \
    <xs:complexType name=\"FieldmlRdfTargetType\"> \
        <xs:attribute name=\"id\" type=\"xs:string\" form=\"qualified\" use=\"optional\"/> \
    </xs:complexType> \
 \
    <xs:complexType name=\"FieldmlObject_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlRdfTargetType\"> \
                <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArrayDataSize_Type\"> \
      <xs:simpleContent> \
        <xs:extension base=\"xs:string\" /> \
      </xs:simpleContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArrayDataOffset_Type\"> \
      <xs:simpleContent> \
        <xs:extension base=\"xs:string\" /> \
      </xs:simpleContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"RawArraySize_Type\"> \
      <xs:simpleContent> \
        <xs:extension base=\"xs:string\" /> \
      </xs:simpleContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArrayDataSource_Type\"> \
      <xs:choice minOccurs=\"1\" maxOccurs=\"unbounded\"> \
          <xs:element name=\"ArrayDataSize\" type=\"ArrayDataSize_Type\" minOccurs=\"0\" maxOccurs=\"1\"/> \
          <xs:element name=\"ArrayDataOffset\" type=\"ArrayDataOffset_Type\" minOccurs=\"0\" maxOccurs=\"1\"/> \
          <xs:element name=\"RawArraySize\" type=\"RawArraySize_Type\" minOccurs=\"0\" maxOccurs=\"1\"/> \
      </xs:choice> \
      <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
      <xs:attribute name=\"location\" type=\"xs:string\" use=\"required\" /> \
      <xs:attribute name=\"rank\" type=\"xs:positiveInteger\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DataResourceHref_Type\"> \
      <xs:attribute ref=\"xlink:href\" use=\"required\" /> \
      <xs:attribute name=\"format\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DataResourceString_Type\"> \
      <xs:simpleContent> \
        <xs:extension base=\"xs:string\" /> \
      </xs:simpleContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DataResourceDescription_Type\"> \
        <xs:choice minOccurs=\"1\" maxOccurs=\"1\"> \
            <xs:element name=\"DataResourceHref\" type=\"DataResourceHref_Type\" /> \
            <xs:element name=\"DataResourceString\" type=\"DataResourceString_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DataResource_Type\"> \
      <xs:choice minOccurs=\"1\" maxOccurs=\"unbounded\"> \
          <xs:element name=\"DataResourceDescription\" type=\"DataResourceDescription_Type\" minOccurs=\"1\" maxOccurs=\"1\"/> \
          <xs:element name=\"ArrayDataSource\" type=\"ArrayDataSource_Type\" /> \
      </xs:choice> \
      <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ImportTypeEntry_Type\"> \
        <xs:attribute name=\"localName\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"remoteName\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ImportEvaluatorEntry_Type\"> \
        <xs:attribute name=\"localName\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"remoteName\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"Import_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"ImportType\" type=\"ImportTypeEntry_Type\" /> \
            <xs:element name=\"ImportEvaluator\" type=\"ImportEvaluatorEntry_Type\" /> \
        </xs:choice> \
        <xs:attribute ref=\"xlink:href\" use=\"required\" /> \
        <xs:attribute name=\"region\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"NumberedIndexEvaluator_Type\"> \
        <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"indexNumber\" type=\"xs:positiveInteger\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"NumberedIndexEvaluatorList_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"IndexEvaluator\" type=\"NumberedIndexEvaluator_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"BindMapEntry_Type\"> \
        <xs:attribute name=\"argument\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"source\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"BindMapIndexEntry_Type\"> \
        <xs:attribute name=\"argument\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"indexNumber\" type=\"xs:positiveInteger\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"BindMapWithIndexes_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"BindIndex\" type=\"BindMapIndexEntry_Type\" /> \
            <xs:element name=\"Bind\" type=\"BindMapEntry_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"BindMap_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"Bind\" type=\"BindMapEntry_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ComponentEvaluatorsEntry_Type\"> \
        <xs:attribute name=\"component\" type=\"xs:positiveInteger\" use=\"required\" /> \
        <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ComponentEvaluators_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"ComponentEvaluator\" type=\"ComponentEvaluatorsEntry_Type\" /> \
        </xs:choice> \
        <xs:attribute name=\"default\" type=\"xs:string\" use=\"optional\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ShapeMapEntry_Type\"> \
        <xs:attribute name=\"element\" type=\"xs:nonNegativeInteger\" use=\"required\" /> \
        <xs:attribute name=\"shape\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ShapeMap_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"Shape\" type=\"ShapeMapEntry_Type\" /> \
        </xs:choice> \
        <xs:attribute name=\"default\" type=\"xs:string\" use=\"optional\"/> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ElementMapEntry_Type\"> \
        <xs:attribute name=\"indexValue\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ElementMap_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"ElementEvaluator\" type=\"ElementMapEntry_Type\" /> \
        </xs:choice> \
        <xs:attribute name=\"default\" type=\"xs:string\" use=\"optional\"/> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArgumentListEntry_Type\"> \
        <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArgumentList_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"Argument\" type=\"ArgumentListEntry_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"EvaluatorListEntry_Type\"> \
        <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
"
"\
    </xs:complexType> \
 \
    <xs:complexType name=\"IndexEvaluatorList_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"IndexEvaluator\" type=\"EvaluatorListEntry_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"OrderedEvaluatorListEntry_Type\"> \
        <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"order\" type=\"xs:string\" use=\"optional\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"OrderedIndexEvaluatorList_Type\"> \
        <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
            <xs:element name=\"IndexEvaluator\" type=\"OrderedEvaluatorListEntry_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"MemberRange_Type\"> \
        <xs:attribute name=\"min\" type=\"xs:nonNegativeInteger\" use=\"required\" /> \
        <xs:attribute name=\"max\" type=\"xs:nonNegativeInteger\" use=\"required\" /> \
        <xs:attribute name=\"stride\" type=\"xs:positiveInteger\" use=\"optional\"/> \
    </xs:complexType> \
 \
    <xs:complexType name=\"MemberData_Type\"> \
        <xs:attribute name=\"count\" type=\"xs:positiveInteger\" use=\"required\" /> \
        <xs:attribute name=\"data\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"EnsembleMembers_Type\"> \
        <xs:choice minOccurs=\"1\" maxOccurs=\"1\"> \
            <xs:element name=\"MemberListData\" type=\"MemberData_Type\" /> \
            <xs:element name=\"MemberRangeData\" type=\"MemberData_Type\" /> \
            <xs:element name=\"MemberStrideRangeData\" type=\"MemberData_Type\" /> \
            <xs:element name=\"MemberRange\" type=\"MemberRange_Type\" /> \
        </xs:choice> \
    </xs:complexType> \
 \
    <xs:complexType name=\"EnsembleType_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlObject_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Members\" type=\"EnsembleMembers_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                </xs:sequence> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ContinuousComponent_Type\"> \
        <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"count\" type=\"xs:positiveInteger\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ContinuousType_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlObject_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Components\" type=\"ContinuousComponent_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                </xs:sequence> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"MeshType_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlObject_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Elements\" type=\"EnsembleType_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                    <xs:element name=\"Chart\" type=\"ContinuousType_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                    <xs:element name=\"Shapes\" type=\"ShapeMap_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                </xs:sequence> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"FieldmlEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlObject_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Arguments\" type=\"ArgumentList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                </xs:sequence> \
                <xs:attribute name=\"valueType\" type=\"xs:string\" use=\"required\" /> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ArgumentEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlEvaluator_Type\" /> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ExternalEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlEvaluator_Type\" /> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ReferenceEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlObject_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Arguments\" type=\"ArgumentList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                    <xs:element name=\"Bindings\" type=\"BindMap_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                </xs:sequence> \
                <xs:attribute name=\"valueType\" type=\"xs:string\" use=\"optional\" /> \
                <xs:attribute name=\"evaluator\" type=\"xs:string\" use=\"required\" /> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"PiecewiseEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlEvaluator_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Bindings\" type=\"BindMap_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                    <xs:element name=\"IndexEvaluators\" type=\"NumberedIndexEvaluatorList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                    <xs:element name=\"ElementEvaluators\" type=\"ElementMap_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                </xs:sequence> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"SemidenseData_Type\"> \
        <xs:sequence> \
            <xs:element name=\"DenseIndexes\" type=\"OrderedIndexEvaluatorList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
            <xs:element name=\"SparseIndexes\" type=\"IndexEvaluatorList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
        </xs:sequence> \
        <xs:attribute name=\"data\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DenseArrayData_Type\"> \
        <xs:sequence> \
            <xs:element name=\"DenseIndexes\" type=\"OrderedIndexEvaluatorList_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
        </xs:sequence> \
        <xs:attribute name=\"data\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"DOKArrayData_Type\"> \
        <xs:sequence> \
            <xs:element name=\"SparseIndexes\" type=\"OrderedIndexEvaluatorList_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
            <xs:element name=\"DenseIndexes\" type=\"IndexEvaluatorList_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
        </xs:sequence> \
        <xs:attribute name=\"keyData\" type=\"xs:string\" use=\"required\" /> \
        <xs:attribute name=\"valueData\" type=\"xs:string\" use=\"required\" /> \
    </xs:complexType> \
 \
    <xs:complexType name=\"ParameterEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlEvaluator_Type\"> \
                <xs:choice> \
                    <xs:element name=\"SemidenseData\" type=\"SemidenseData_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                    <xs:element name=\"DenseArrayData\" type=\"DenseArrayData_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                    <xs:element name=\"DOKArrayData\" type=\"DOKArrayData_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                </xs:choice> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"AggregateEvaluator_Type\"> \
        <xs:complexContent> \
            <xs:extension base=\"FieldmlEvaluator_Type\"> \
                <xs:sequence> \
                    <xs:element name=\"Bindings\" type=\"BindMapWithIndexes_Type\" minOccurs=\"0\" maxOccurs=\"1\" /> \
                    <xs:element name=\"ComponentEvaluators\" type=\"ComponentEvaluators_Type\" minOccurs=\"1\" maxOccurs=\"1\" /> \
                </xs:sequence> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"Region_Type\"> \
        <xs:complexContent > \
            <xs:extension base=\"FieldmlRdfTargetType\"> \
                <xs:choice minOccurs=\"0\" maxOccurs=\"unbounded\"> \
                    <xs:element name=\"Import\" type=\"Import_Type\" /> \
                    <xs:element name=\"DataResource\" type=\"DataResource_Type\" /> \
                    <xs:element name=\"EnsembleType\" type=\"EnsembleType_Type\" /> \
                    <xs:element name=\"ContinuousType\" type=\"ContinuousType_Type\" /> \
                    <xs:element name=\"MeshType\" type=\"MeshType_Type\" /> \
                    <xs:element name=\"ArgumentEvaluator\" type=\"ArgumentEvaluator_Type\" /> \
                    <xs:element name=\"ExternalEvaluator\" type=\"ExternalEvaluator_Type\" /> \
                    <xs:element name=\"ReferenceEvaluator\" type=\"ReferenceEvaluator_Type\" /> \
                    <xs:element name=\"PiecewiseEvaluator\" type=\"PiecewiseEvaluator_Type\" /> \
                    <xs:element name=\"ParameterEvaluator\" type=\"ParameterEvaluator_Type\" /> \
                    <xs:element name=\"AggregateEvaluator\" type=\"AggregateEvaluator_Type\" /> \
                </xs:choice> \
                <xs:attribute name=\"name\" type=\"xs:string\" use=\"required\" /> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:complexType name=\"Fieldml_Type\"> \
        <xs:complexContent > \
            <xs:extension base=\"FieldmlRdfTargetType\"> \
                <xs:choice minOccurs=\"1\" maxOccurs=\"1\"> \
                    <xs:element name=\"Region\" type=\"Region_Type\" /> \
                </xs:choice> \
                <xs:attribute name=\"version\" type=\"xs:string\" /> \
            </xs:extension> \
        </xs:complexContent> \
    </xs:complexType> \
 \
    <xs:element name=\"Fieldml\" type=\"Fieldml_Type\"/> \
 \
</xs:schema> \
";
