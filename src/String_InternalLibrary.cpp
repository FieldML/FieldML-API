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

#include "String_InternalLibrary.h"

const char * const FML_INTERNAL_LIBRARY_NAME = "library.xml";

const char * const FML_STRING_INTERNAL_LIBRARY = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> \
<Fieldml version=\"0.4_RC1\" xsi:noNamespaceSchemaLocation=\"Fieldml_0.4.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"> \
 <Region name=\"library\"> \
  <ContinuousType name=\"real.1d\"/> \
  <AbstractEvaluator name=\"real.1d.variable\" valueType=\"real.1d\"/> \
 \
  <ContinuousType name=\"real.2d\"> \
    <Components name=\"real.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"real.2d.component.variable\" valueType=\"real.2d.component\"/> \
  <AbstractEvaluator name=\"real.2d.variable\" valueType=\"real.2d\"/> \
 \
  <ContinuousType name=\"real.3d\"> \
    <Components name=\"real.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"real.3d.component.variable\" valueType=\"real.3d.component\"/> \
  <AbstractEvaluator name=\"real.3d.variable\" valueType=\"real.3d\"/> \
 \
  <ContinuousType name=\"chart.1d\"> \
    <Components name=\"chart.1d.component\" count=\"1\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"chart.1d.component.variable\" valueType=\"chart.1d.component\"/> \
  <AbstractEvaluator name=\"chart.1d.variable\" valueType=\"chart.1d\"/> \
 \
  <ContinuousType name=\"chart.2d\"> \
    <Components name=\"chart.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"chart.2d.component.variable\" valueType=\"chart.2d.component\"/> \
  <AbstractEvaluator name=\"chart.2d.variable\" valueType=\"chart.2d\"/> \
 \
  <ContinuousType name=\"chart.3d\"> \
    <Components name=\"chart.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"chart.3d.component.variable\" valueType=\"chart.3d.component\"/> \
  <AbstractEvaluator name=\"chart.3d.variable\" valueType=\"chart.3d\"/> \
 \
  <EnsembleType name=\"localNodes.1d.line2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"2\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.1d.line2.variable\" valueType=\"localNodes.1d.line2\"/> \
 \
  <ContinuousType name=\"parameters.1d.linearLagrange\"> \
    <Components name=\"parameters.1d.linearLagrange.component\" count=\"2\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.1d.linearLagrange.component.variable\" valueType=\"parameters.1d.linearLagrange.component\"/> \
  <AbstractEvaluator name=\"parameters.1d.linearLagrange.variable\" valueType=\"parameters.1d.linearLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.linearLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.1d.variable\"/> \
      <Variable name=\"parameters.1d.linearLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.1d.line3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"3\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.1d.line3.variable\" valueType=\"localNodes.1d.line3\"/> \
 \
  <ContinuousType name=\"parameters.1d.quadraticLagrange\"> \
    <Components name=\"parameters.1d.quadraticLagrange.component\" count=\"3\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.1d.quadraticLagrange.component.variable\" valueType=\"parameters.1d.quadraticLagrange.component\"/> \
  <AbstractEvaluator name=\"parameters.1d.quadraticLagrange.variable\" valueType=\"parameters.1d.quadraticLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.quadraticLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.1d.variable\"/> \
      <Variable name=\"parameters.1d.quadraticLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.2d.square2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"4\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.2d.square2x2.variable\" valueType=\"localNodes.2d.square2x2\"/> \
 \
  <ContinuousType name=\"parameters.2d.bilinearLagrange\"> \
    <Components name=\"parameters.2d.bilinearLagrange.component\" count=\"4\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.2d.bilinearLagrange.variable\" valueType=\"parameters.2d.bilinearLagrange\"/> \
  <AbstractEvaluator name=\"parameters.2d.bilinearLagrange.component.variable\" valueType=\"parameters.2d.bilinearLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bilinearLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.bilinearLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.2d.square3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"9\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.2d.square3x3.variable\" valueType=\"localNodes.2d.square3x3\"/> \
 \
  <ContinuousType name=\"parameters.2d.biquadraticLagrange\"> \
    <Components name=\"parameters.2d.biquadraticLagrange.component\" count=\"9\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.2d.biquadraticLagrange.variable\" valueType=\"parameters.2d.biquadraticLagrange\"/> \
  <AbstractEvaluator name=\"parameters.2d.biquadraticLagrange.component.variable\" valueType=\"parameters.2d.biquadraticLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.biquadraticLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.biquadraticLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.3d.cube2x2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"8\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.3d.cube2x2x2.variable\" valueType=\"localNodes.3d.cube2x2x2\"/> \
 \
  <ContinuousType name=\"parameters.3d.trilinearLagrange\"> \
    <Components name=\"parameters.3d.trilinearLagrange.component\" count=\"8\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.3d.trilinearLagrange.variable\" valueType=\"parameters.3d.trilinearLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.trilinearLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.trilinearLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.3d.cube3x3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"27\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.3d.cube3x3x3.variable\" valueType=\"localNodes.3d.cube3x3x3\"/> \
 \
  <ContinuousType name=\"parameters.3d.triquadraticLagrange\"> \
    <Components name=\"parameters.3d.triquadraticLagrange.component\" count=\"27\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.3d.triquadraticLagrange.variable\" valueType=\"parameters.3d.triquadraticLagrange\"/> \
  <AbstractEvaluator name=\"parameters.3d.triquadraticLagrange.component.variable\" valueType=\"parameters.3d.triquadraticLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.triquadraticLagrange\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.triquadraticLagrange.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"coordinates.rc.1d\"/> \
  <AbstractEvaluator name=\"coordinates.rc.1d.variable\" valueType=\"coordinates.rc.1d\"/> \
 \
  <ContinuousType name=\"coordinates.rc.2d\"> \
    <Components name=\"coordinates.rc.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"coordinates.rc.2d.variable\" valueType=\"coordinates.rc.2d\"/> \
  <AbstractEvaluator name=\"coordinates.rc.2d.component.variable\" valueType=\"coordinates.rc.2d.component\"/> \
 \
  <ContinuousType name=\"coordinates.rc.3d\"> \
    <Components name=\"coordinates.rc.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"coordinates.rc.3d.variable\" valueType=\"coordinates.rc.3d\"/> \
  <AbstractEvaluator name=\"coordinates.rc.3d.component.variable\" valueType=\"coordinates.rc.3d.component\"/> \
 \
  <ContinuousType name=\"parameters.1d.cubicHermite\"> \
    <Components name=\"parameters.1d.cubicHermite.component\" count=\"4\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.1d.cubicHermite.variable\" valueType=\"parameters.1d.cubicHermite\"/> \
  <AbstractEvaluator name=\"parameters.1d.cubicHermite.component.variable\" valueType=\"parameters.1d.cubicHermite.component\"/> \
  <AbstractEvaluator name=\"parameters.1d.cubicHermiteScaling.variable\" valueType=\"parameters.1d.cubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.cubicHermite\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.1d.variable\"/> \
      <Variable name=\"parameters.1d.cubicHermite.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.cubicHermiteScaled\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.1d.variable\"/> \
      <Variable name=\"parameters.1d.cubicHermite.variable\"/> \
      <Variable name=\"parameters.1d.cubicHermiteScaling.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"parameters.2d.bicubicHermite\"> \
    <Components name=\"parameters.2d.bicubicHermite.component\" count=\"16\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.2d.bicubicHermite.variable\" valueType=\"parameters.2d.bicubicHermite\"/> \
  <AbstractEvaluator name=\"parameters.2d.bicubicHermite.component.variable\" valueType=\"parameters.2d.bicubicHermite.component\"/> \
  <AbstractEvaluator name=\"parameters.2d.bicubicHermiteScaling.variable\" valueType=\"parameters.2d.bicubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bicubicHermite\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.bicubicHermite.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bicubicHermiteScaled\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.bicubicHermite.variable\"/> \
      <Variable name=\"parameters.2d.bicubicHermiteScaling.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"parameters.3d.tricubicHermite\"> \
    <Components name=\"parameters.3d.tricubicHermite.component\" count=\"64\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.3d.tricubicHermite.variable\" valueType=\"parameters.3d.tricubicHermite\"/> \
  <AbstractEvaluator name=\"parameters.3d.tricubicHermite.component.variable\" valueType=\"parameters.3d.tricubicHermite.component\"/> \
  <AbstractEvaluator name=\"parameters.3d.tricubicHermiteScaling.variable\" valueType=\"parameters.3d.tricubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.tricubicHermite\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.tricubicHermite.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.tricubicHermiteScaled\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.tricubicHermite.variable\"/> \
      <Variable name=\"parameters.3d.tricubicHermiteScaling.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
 \
<!-- SIMPLEX OBJECTS--> \
 \
  <!-- Bilinear Simplex --> \
  <EnsembleType name=\"localNodes.2d.triangle2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"3\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.2d.triangle2x2.variable\" valueType=\"localNodes.2d.triangle2x2\"/> \
 \
  <ContinuousType name=\"parameters.2d.bilinearSimplex\"> \
    <Components name=\"parameters.2d.bilinearSimplex.component\" count=\"3\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.2d.bilinearSimplex.variable\" valueType=\"parameters.2d.bilinearSimplex\"/> \
  <AbstractEvaluator name=\"parameters.2d.bilinearSimplex.component.variable\" valueType=\"parameters.2d.bilinearSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bilinearSimplex\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.bilinearSimplex.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <!-- Biquadratic Simplex --> \
  <EnsembleType name=\"localNodes.2d.triangle3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"6\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.2d.triangle3x3.variable\" valueType=\"localNodes.2d.triangle3x3\"/> \
 \
  <ContinuousType name=\"parameters.2d.biquadraticSimplex\"> \
    <Components name=\"parameters.2d.biquadraticSimplex.component\" count=\"6\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.2d.biquadraticSimplex.variable\" valueType=\"parameters.2d.biquadraticSimplex\"/> \
  <AbstractEvaluator name=\"parameters.2d.biquadraticSimplex.component.variable\" valueType=\"parameters.2d.biquadraticSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.biquadraticSimplex\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.2d.variable\"/> \
      <Variable name=\"parameters.2d.biquadraticSimplex.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <!-- Trilinear Simplex --> \
  <EnsembleType name=\"localNodes.3d.tetrahedron2x2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"4\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.3d.tetrahedron2x2x2.variable\" valueType=\"localNodes.3d.tetrahedron2x2x2\"/> \
 \
  <ContinuousType name=\"parameters.3d.trilinearSimplex\"> \
    <Components name=\"parameters.3d.trilinearSimplex.component\" count=\"4\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.3d.trilinearSimplex.variable\" valueType=\"parameters.3d.trilinearSimplex\"/> \
  <AbstractEvaluator name=\"parameters.3d.trilinearSimplex.component.variable\" valueType=\"parameters.3d.trilinearSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.trilinearSimplex\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.trilinearSimplex.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
  <!-- Triquadratic Simplex --> \
  <EnsembleType name=\"localNodes.3d.tetrahedron3x3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"10\"/> \
   </Members> \
  </EnsembleType> \
  <AbstractEvaluator name=\"localNodes.3d.tetrahedron3x3x3.variable\" valueType=\"localNodes.3d.tetrahedron3x3x3\"/> \
 \
  <ContinuousType name=\"parameters.3d.triquadraticSimplex\"> \
    <Components name=\"parameters.3d.triquadraticSimplex.component\" count=\"10\"/> \
  </ContinuousType> \
  <AbstractEvaluator name=\"parameters.3d.triquadraticSimplex.variable\" valueType=\"parameters.3d.triquadraticSimplex\"/> \
  <AbstractEvaluator name=\"parameters.3d.triquadraticSimplex.component.variable\" valueType=\"parameters.3d.triquadraticSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.triquadraticSimplex\" valueType=\"real.1d\"> \
    <Variables> \
      <Variable name=\"chart.3d.variable\"/> \
      <Variable name=\"parameters.3d.triquadraticSimplex.variable\"/> \
    </Variables> \
  </ExternalEvaluator> \
 \
 </Region> \
</Fieldml> \
";
