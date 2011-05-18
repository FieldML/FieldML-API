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
<Fieldml version=\"0.4_RC2\" xsi:noNamespaceSchemaLocation=\"Fieldml.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"> \
 <Region name=\"library\"> \
  <ContinuousType name=\"real.1d\"/> \
  <ArgumentEvaluator name=\"real.1d.argument\" valueType=\"real.1d\"/> \
 \
  <ContinuousType name=\"real.2d\"> \
    <Components name=\"real.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"real.2d.component.argument\" valueType=\"real.2d.component\"/> \
  <ArgumentEvaluator name=\"real.2d.argument\" valueType=\"real.2d\"/> \
 \
  <ContinuousType name=\"real.3d\"> \
    <Components name=\"real.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"real.3d.component.argument\" valueType=\"real.3d.component\"/> \
  <ArgumentEvaluator name=\"real.3d.argument\" valueType=\"real.3d\"/> \
 \
  <ContinuousType name=\"chart.1d\"> \
    <Components name=\"chart.1d.component\" count=\"1\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"chart.1d.component.argument\" valueType=\"chart.1d.component\"/> \
  <ArgumentEvaluator name=\"chart.1d.argument\" valueType=\"chart.1d\"/> \
 \
  <ContinuousType name=\"chart.2d\"> \
    <Components name=\"chart.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"chart.2d.component.argument\" valueType=\"chart.2d.component\"/> \
  <ArgumentEvaluator name=\"chart.2d.argument\" valueType=\"chart.2d\"/> \
 \
  <ContinuousType name=\"chart.3d\"> \
    <Components name=\"chart.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"chart.3d.component.argument\" valueType=\"chart.3d.component\"/> \
  <ArgumentEvaluator name=\"chart.3d.argument\" valueType=\"chart.3d\"/> \
 \
  <ContinuousType name=\"coordinates.rc.1d\"/> \
  <ArgumentEvaluator name=\"coordinates.rc.1d.argument\" valueType=\"coordinates.rc.1d\"/> \
 \
  <ContinuousType name=\"coordinates.rc.2d\"> \
    <Components name=\"coordinates.rc.2d.component\" count=\"2\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"coordinates.rc.2d.argument\" valueType=\"coordinates.rc.2d\"/> \
  <ArgumentEvaluator name=\"coordinates.rc.2d.component.argument\" valueType=\"coordinates.rc.2d.component\"/> \
 \
  <ContinuousType name=\"coordinates.rc.3d\"> \
    <Components name=\"coordinates.rc.3d.component\" count=\"3\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"coordinates.rc.3d.argument\" valueType=\"coordinates.rc.3d\"/> \
  <ArgumentEvaluator name=\"coordinates.rc.3d.component.argument\" valueType=\"coordinates.rc.3d.component\"/> \
 \
  <EnsembleType name=\"localNodes.1d.line2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"2\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.1d.line2.argument\" valueType=\"localNodes.1d.line2\"/> \
 \
  <ContinuousType name=\"parameters.1d.unit.linearLagrange\"> \
    <Components name=\"parameters.1d.unit.linearLagrange.component\" count=\"2\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.1d.unit.linearLagrange.component.argument\" valueType=\"parameters.1d.unit.linearLagrange.component\"/> \
  <ArgumentEvaluator name=\"parameters.1d.unit.linearLagrange.argument\" valueType=\"parameters.1d.unit.linearLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.linearLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.1d.argument\"/> \
      <Argument name=\"parameters.1d.unit.linearLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.1d.line3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"3\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.1d.line3.argument\" valueType=\"localNodes.1d.line3\"/> \
 \
  <ContinuousType name=\"parameters.1d.unit.quadraticLagrange\"> \
    <Components name=\"parameters.1d.unit.quadraticLagrange.component\" count=\"3\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.1d.unit.quadraticLagrange.component.argument\" valueType=\"parameters.1d.unit.quadraticLagrange.component\"/> \
  <ArgumentEvaluator name=\"parameters.1d.unit.quadraticLagrange.argument\" valueType=\"parameters.1d.unit.quadraticLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.quadraticLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.1d.argument\"/> \
      <Argument name=\"parameters.1d.unit.quadraticLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.2d.square2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"4\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.2d.square2x2.argument\" valueType=\"localNodes.2d.square2x2\"/> \
 \
  <ContinuousType name=\"parameters.2d.unit.bilinearLagrange\"> \
    <Components name=\"parameters.2d.unit.bilinearLagrange.component\" count=\"4\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bilinearLagrange.argument\" valueType=\"parameters.2d.unit.bilinearLagrange\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bilinearLagrange.component.argument\" valueType=\"parameters.2d.unit.bilinearLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bilinearLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.bilinearLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.2d.square3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"9\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.2d.square3x3.argument\" valueType=\"localNodes.2d.square3x3\"/> \
 \
  <ContinuousType name=\"parameters.2d.unit.biquadraticLagrange\"> \
    <Components name=\"parameters.2d.unit.biquadraticLagrange.component\" count=\"9\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.2d.unit.biquadraticLagrange.argument\" valueType=\"parameters.2d.unit.biquadraticLagrange\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.biquadraticLagrange.component.argument\" valueType=\"parameters.2d.unit.biquadraticLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.biquadraticLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.biquadraticLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.3d.cube2x2x2\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"8\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.3d.cube2x2x2.argument\" valueType=\"localNodes.3d.cube2x2x2\"/> \
 \
  <ContinuousType name=\"parameters.3d.unit.trilinearLagrange\"> \
    <Components name=\"parameters.3d.unit.trilinearLagrange.component\" count=\"8\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.3d.unit.trilinearLagrange.argument\" valueType=\"parameters.3d.unit.trilinearLagrange\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.trilinearLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.trilinearLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <EnsembleType name=\"localNodes.3d.cube3x3x3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"27\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.3d.cube3x3x3.argument\" valueType=\"localNodes.3d.cube3x3x3\"/> \
 \
  <ContinuousType name=\"parameters.3d.unit.triquadraticLagrange\"> \
    <Components name=\"parameters.3d.unit.triquadraticLagrange.component\" count=\"27\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.3d.unit.triquadraticLagrange.argument\" valueType=\"parameters.3d.unit.triquadraticLagrange\"/> \
  <ArgumentEvaluator name=\"parameters.3d.unit.triquadraticLagrange.component.argument\" valueType=\"parameters.3d.unit.triquadraticLagrange.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.triquadraticLagrange\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.triquadraticLagrange.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"parameters.1d.unit.cubicHermite\"> \
    <Components name=\"parameters.1d.unit.cubicHermite.component\" count=\"4\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.1d.unit.cubicHermite.argument\" valueType=\"parameters.1d.unit.cubicHermite\"/> \
  <ArgumentEvaluator name=\"parameters.1d.unit.cubicHermite.component.argument\" valueType=\"parameters.1d.unit.cubicHermite.component\"/> \
  <ArgumentEvaluator name=\"parameters.1d.unit.cubicHermiteScaling.argument\" valueType=\"parameters.1d.unit.cubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.cubicHermite\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.1d.argument\"/> \
      <Argument name=\"parameters.1d.unit.cubicHermite.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.1d.unit.cubicHermiteScaled\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.1d.argument\"/> \
      <Argument name=\"parameters.1d.unit.cubicHermite.argument\"/> \
      <Argument name=\"parameters.1d.unit.cubicHermiteScaling.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"parameters.2d.unit.bicubicHermite\"> \
    <Components name=\"parameters.2d.unit.bicubicHermite.component\" count=\"16\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bicubicHermite.argument\" valueType=\"parameters.2d.unit.bicubicHermite\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bicubicHermite.component.argument\" valueType=\"parameters.2d.unit.bicubicHermite.component\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bicubicHermiteScaling.argument\" valueType=\"parameters.2d.unit.bicubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bicubicHermite\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.bicubicHermite.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bicubicHermiteScaled\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.bicubicHermite.argument\"/> \
      <Argument name=\"parameters.2d.unit.bicubicHermiteScaling.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ContinuousType name=\"parameters.3d.unit.tricubicHermite\"> \
    <Components name=\"parameters.3d.unit.tricubicHermite.component\" count=\"64\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.3d.unit.tricubicHermite.argument\" valueType=\"parameters.3d.unit.tricubicHermite\"/> \
  <ArgumentEvaluator name=\"parameters.3d.unit.tricubicHermite.component.argument\" valueType=\"parameters.3d.unit.tricubicHermite.component\"/> \
  <ArgumentEvaluator name=\"parameters.3d.unit.tricubicHermiteScaling.argument\" valueType=\"parameters.3d.unit.tricubicHermite\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.tricubicHermite\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.tricubicHermite.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.tricubicHermiteScaled\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.tricubicHermite.argument\"/> \
      <Argument name=\"parameters.3d.unit.tricubicHermiteScaling.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
 \
<!-- SIMPLEX OBJECTS--> \
 \
  <!-- Bilinear Simplex --> \
  <EnsembleType name=\"localNodes.2d.triangle3\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"3\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.2d.triangle3.argument\" valueType=\"localNodes.2d.triangle3\"/> \
 \
  <ContinuousType name=\"parameters.2d.unit.bilinearSimplex\"> \
    <Components name=\"parameters.2d.unit.bilinearSimplex.component\" count=\"3\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bilinearSimplex.argument\" valueType=\"parameters.2d.unit.bilinearSimplex\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.bilinearSimplex.component.argument\" valueType=\"parameters.2d.unit.bilinearSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.bilinearSimplex\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.bilinearSimplex.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <!-- Biquadratic Simplex --> \
  <EnsembleType name=\"localNodes.2d.triangle6\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"6\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.2d.triangle6.argument\" valueType=\"localNodes.2d.triangle6\"/> \
 \
  <ContinuousType name=\"parameters.2d.unit.biquadraticSimplex\"> \
    <Components name=\"parameters.2d.unit.biquadraticSimplex.component\" count=\"6\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.2d.unit.biquadraticSimplex.argument\" valueType=\"parameters.2d.unit.biquadraticSimplex\"/> \
  <ArgumentEvaluator name=\"parameters.2d.unit.biquadraticSimplex.component.argument\" valueType=\"parameters.2d.unit.biquadraticSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.2d.unit.biquadraticSimplex\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.2d.argument\"/> \
      <Argument name=\"parameters.2d.unit.biquadraticSimplex.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <!-- Trilinear Simplex --> \
  <EnsembleType name=\"localNodes.3d.tetrahedron4\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"4\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.3d.tetrahedron4.argument\" valueType=\"localNodes.3d.tetrahedron4\"/> \
 \
  <ContinuousType name=\"parameters.3d.unit.trilinearSimplex\"> \
    <Components name=\"parameters.3d.unit.trilinearSimplex.component\" count=\"4\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.3d.unit.trilinearSimplex.argument\" valueType=\"parameters.3d.unit.trilinearSimplex\"/> \
  <ArgumentEvaluator name=\"parameters.3d.unit.trilinearSimplex.component.argument\" valueType=\"parameters.3d.unit.trilinearSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.trilinearSimplex\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.trilinearSimplex.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
  <!-- Triquadratic Simplex --> \
  <EnsembleType name=\"localNodes.3d.tetrahedron10\"> \
   <Members> \
    <MemberRange min=\"1\" max=\"10\"/> \
   </Members> \
  </EnsembleType> \
  <ArgumentEvaluator name=\"localNodes.3d.tetrahedron10.argument\" valueType=\"localNodes.3d.tetrahedron10\"/> \
 \
  <ContinuousType name=\"parameters.3d.unit.triquadraticSimplex\"> \
    <Components name=\"parameters.3d.unit.triquadraticSimplex.component\" count=\"10\"/> \
  </ContinuousType> \
  <ArgumentEvaluator name=\"parameters.3d.unit.triquadraticSimplex.argument\" valueType=\"parameters.3d.unit.triquadraticSimplex\"/> \
  <ArgumentEvaluator name=\"parameters.3d.unit.triquadraticSimplex.component.argument\" valueType=\"parameters.3d.unit.triquadraticSimplex.component\"/> \
 \
  <ExternalEvaluator name=\"interpolator.3d.unit.triquadraticSimplex\" valueType=\"real.1d\"> \
    <Arguments> \
      <Argument name=\"chart.3d.argument\"/> \
      <Argument name=\"parameters.3d.unit.triquadraticSimplex.argument\"/> \
    </Arguments> \
  </ExternalEvaluator> \
 \
 </Region> \
</Fieldml> \
";
