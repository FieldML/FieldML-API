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

#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <mpi.h>
#include "math.h"
#include "FieldmlIoApi.h"
#include "fieldml_api.h"

#define PI 3.14159265

using namespace std;

int mpi_size, mpi_rank;

int store_argc;
char **store_argv;
double **highResCoordinates, **lowResCoordinates;
int size = 41, sizeY = 3, numberOfTimes = 100;
int arraySize = size * size *size;
double maximumTime = 40.0;
int *arraySizeForEachNode, *numberOfRowsForEachNode;
int lowResSize = 11, lowResNumberOfNodes = 1;
int lowResArraySize = lowResSize * lowResSize * lowResSize;
int lowResElementSize = (lowResSize - 1) * (lowResSize - 1) * (lowResSize - 1);

int findUsefulArraySizeForNode(int ratioPerNode, int *offset)
{
	int requiredRows = 1, upperRow = 0, lowerRow = 0;
	if (mpi_rank == 0)
	{
		lowerRow = 0;
		upperRow = numberOfRowsForEachNode[0] * ratioPerNode - 1;
	}
	else if (mpi_rank == lowResNumberOfNodes - 1)
	{
		int temp = -1;
		for (int i = 0; i < mpi_rank; i ++)
		{
			temp = temp + numberOfRowsForEachNode[i];
		}
		lowerRow = temp * ratioPerNode + 1;
		upperRow = size - 1;
	}
	else
	{
		int temp = -1;
		for (int i = 0; i < mpi_rank; i ++)
		{
			temp = temp + numberOfRowsForEachNode[i];
		}
		lowerRow = temp * ratioPerNode + 1;
		upperRow = (temp + numberOfRowsForEachNode[mpi_rank] + 1) * ratioPerNode - 1;
	}
	offset[0] = lowerRow * size *size;
	requiredRows = upperRow - lowerRow + 1;

	return requiredRows;
}

int mappedLowResIdenatifierToHighRes(int lowResIdentifier)
{
	return lowResCoordinates[0][lowResIdentifier] + lowResCoordinates[1][lowResIdentifier] * size + lowResCoordinates[2][lowResIdentifier] * size *size;
}

double **readHDF5Potential(int ratioPerNode, int *highResOffset)
{
	bool testOk = true;

	printf("Testing HDF5 array read\n");

	int offsets[2];
	int sizes[2];
	sizes[1] = 1;
	double **potentialArray = 0;
	if (lowResNumberOfNodes > mpi_rank)
	{
		if (lowResNumberOfNodes > 1)
		{
			int requiredRows = findUsefulArraySizeForNode(ratioPerNode, &offsets[0]);
			sizes[0] = requiredRows * size * size;
			potentialArray = new double*[numberOfTimes];
			for( long int i = 0; i < numberOfTimes ; i++)
			{
				potentialArray[i] = new double[sizes[0]];
			}
		}
		else
		{
			potentialArray = new double*[numberOfTimes];
			for( long int i = 0; i < numberOfTimes ; i++)
			{
				potentialArray[i] = new double[arraySize];
			}
			sizes[0] = arraySize;
			offsets[0] = 0;
		}
	}

	FmlSessionHandle session = Fieldml_Create( "test", "test" );

	FmlObjectHandle resource = Fieldml_CreateHrefDataResource( session, "potential.resource2", "PHDF5", "./output/potential.h5" );
	FmlObjectHandle sourceD = Fieldml_CreateArrayDataSource( session, "potential.source2_double", resource, "potential", 2 );

	FmlObjectHandle reader = Fieldml_OpenReader( session, sourceD );

	if (lowResNumberOfNodes > mpi_rank)
	{
		printf("Reading: mpi_size %d, mpi_rank %d highresOffset %d ratio %d \n", lowResNumberOfNodes, mpi_rank,offsets[0], ratioPerNode);

		for( long int i = 0; i < numberOfTimes ; i++)
		{
			offsets[1] = i;
			Fieldml_ReadDoubleSlab( reader, offsets, sizes, &(potentialArray[i][0]) );
		}
	}
	//for (int i = 0; i< arraySize; i++)
	//	printf("mpi_rank %d potentialArray %d %g\n", mpi_rank, i+offsets[0], potentialArray[0][i]);

	Fieldml_CloseReader( reader );

	Fieldml_Destroy( session );

	*highResOffset = offsets[0];

	if( testOk )
	{
		printf( "TestHdf5Read - ok\n" );
	}
	else
	{
		printf( "TestHdf5Read - failed\n" );
	}

	return potentialArray;
}

int isNodeOnBoundary(int identifier)
{
	for (int i = 0; i < sizeY; i++)
	{
		double coordinates = lowResCoordinates[i][identifier];
		if (coordinates == 0.0 || coordinates == (double)(size - 1))
			return 1;
	}
	return 0;
}

double findAveragePotentialLevel(double **potentialArray, int identifier, int time, double ratioPerNode)
{
	int lowerLimit = -(ratioPerNode - 1);
	int upperLimit = ratioPerNode - 1;
	int numberOfNodesTaken = 0;
	double totalPotential = 0.0;
	int localIdentifier = 0;
	for (int z = lowerLimit; z <= upperLimit; z++)
	{
		localIdentifier = identifier + (z * size * size);
		int current_z_id = localIdentifier;
		for (int y = lowerLimit; y <= upperLimit; y++)
		{
			localIdentifier = current_z_id + (y * size);
			int current_y_id = localIdentifier;
			for (int x = lowerLimit; x <= upperLimit; x++)
			{
				localIdentifier = current_y_id + x;
				totalPotential += potentialArray[time][localIdentifier];
				numberOfNodesTaken++;
			}
		}
	}
	return (totalPotential/(double)numberOfNodesTaken);
}

void findActivationTimes(double **potentialArray, double *activationTimesArray, int localOffset, int highResOffset, double ratioPerNode)
{
	printf("findActivationTimes localOffset %d\n", localOffset);
	for (int i = 0; i < arraySizeForEachNode[mpi_rank]; i++)
	{
		activationTimesArray[i] = 100.0;
		int found = 0;
		int highResIdentifier = mappedLowResIdenatifierToHighRes(localOffset + i);

		for (int j = 0; j < numberOfTimes && !found; j++)
		{
			if (isNodeOnBoundary(localOffset + i))
			{
				if (potentialArray[j][highResIdentifier - highResOffset] > (double)0.8)
				{
					found = 1;
				}
			}
			else
			{
				if (findAveragePotentialLevel(potentialArray, highResIdentifier - highResOffset, j, ratioPerNode) > (double)0.8)
				{
					found = 1;
				}
			}
			if (found)
			{
				//printf("highResIdentifier %d, localIdentifier %d potential %d value %g\n", highResIdentifier, localOffset + i, j, potentialArray[j][highResIdentifier - highResOffset]);
				activationTimesArray[i] = (double)j * maximumTime / (numberOfTimes - 1);
			}
		}
	}
}

int writeLowResActivationTime(FmlSessionHandle session, FmlObjectHandle cType, FmlObjectHandle sourceD)
{
	bool testOk = true;
	int offsets;
	int sizes;
	int ratioPerNode = (size - 1) / (lowResSize - 1);
	int highResOffset = 0;
	double *activationTimes = NULL;
	double **potentialArray = readHDF5Potential(ratioPerNode, &highResOffset);

	sizes = lowResArraySize;

	FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, cType, 0, &sizes, 1 );

	if (lowResNumberOfNodes > mpi_rank)
	{
		sizes = arraySizeForEachNode[mpi_rank];
		offsets = 0;

		for (int i = 0; i < mpi_rank; i ++)
		{
			offsets = offsets + arraySizeForEachNode[i];
		}
		activationTimes = new double[arraySizeForEachNode[mpi_rank]];
		findActivationTimes(potentialArray, &activationTimes[0], offsets, highResOffset, ratioPerNode);

		Fieldml_WriteDoubleSlab( writer, &offsets, &sizes, activationTimes );
	}

	Fieldml_CloseWriter( writer );


	if (potentialArray)
	{
		for ( long int i = 0; i < numberOfTimes; i++ )
		{
			delete[] potentialArray[i];
		}
		delete[] potentialArray;
	}
	if (activationTimes)
		delete[] activationTimes;

	return testOk;
}

int writeLowResCoordinates(FmlSessionHandle session, FmlObjectHandle cType, FmlObjectHandle sourceD)
{
	bool testOk = true;
	int offsets[2];
	int sizes[2];

	sizes[0] = lowResArraySize;
	sizes[1] = sizeY;
	FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, cType, 0, sizes, 2 );

	if (lowResNumberOfNodes > mpi_rank)
	{
		sizes[0] = arraySizeForEachNode[mpi_rank];
		sizes[1] = 1;
		offsets[0] = 0;

		for (int i = 0; i < mpi_rank; i ++)
		{
			offsets[0] = offsets[0] + arraySizeForEachNode[i];
		}
		for( long int i = 0; i < sizeY ; i++)
		{
			offsets[1] = i;
			Fieldml_WriteDoubleSlab( writer, offsets, sizes, &(lowResCoordinates[i][offsets[0]]) );
		}
	}
	Fieldml_CloseWriter( writer );

	return testOk;
}

int findElementsRangeToExport(int numebrOfNodes, int nodeId, int *lowElemID, int *highElemID)
{
	int remainder = lowResElementSize % numebrOfNodes;
	int lowSize =  (int)(lowResElementSize / numebrOfNodes);
	int lowElemLocal = 1;
	int highElemLocal = 0;
	for (int i = 0; i < numebrOfNodes; i++)
	{
		highElemLocal += lowSize;
		if (remainder != 0)
		{
			highElemLocal++;
			remainder--;
		}
		if (nodeId == i)
		{
			*lowElemID = lowElemLocal;
			*highElemID = highElemLocal;
			return *highElemID - *lowElemID + 1;
		}
		lowElemLocal = highElemLocal + 1;
	}
	return 0;
}

int getFirstNodeNumberForElement(int elemNo, int lowResSize)
{
	int numberOfElemsPerRow = lowResSize - 1;
	int elemNoMinusOne = elemNo - 1;
	int numOnZ = (int) ((elemNoMinusOne) / ((numberOfElemsPerRow) * (numberOfElemsPerRow)));
	int remainingElem = (elemNoMinusOne % ((numberOfElemsPerRow) * (numberOfElemsPerRow)));
	int numOnY = (int) (remainingElem / (numberOfElemsPerRow));
	int numOnX = (remainingElem % (numberOfElemsPerRow));
	return 1 + numOnX + numOnY * lowResSize + numOnZ * lowResSize * lowResSize;

}

int **constructLowResConnectivityArray(int nodesPerElement, int *numberOfElements, int *firstElementNo)
{
	int firstElemID = 1, lastElemID = 1, sizeToExport= 1;
	int **connectivityArray = NULL;

	if (lowResNumberOfNodes > mpi_rank)
	{
		sizeToExport = findElementsRangeToExport(lowResNumberOfNodes, mpi_rank, &firstElemID, &lastElemID);
		printf("Node No: %d Element array size %d firstElemID %d lastElemID %d \n",
			mpi_rank, sizeToExport, firstElemID, lastElemID);

		connectivityArray = new int*[nodesPerElement];
		for(int i = 0; i < nodesPerElement ; i++)
		{
			connectivityArray[i] = new int[sizeToExport];
		}
		int j = 0;
		for (int i = firstElemID; i <= lastElemID; i++)
		{
			connectivityArray[0][j] = getFirstNodeNumberForElement(i, lowResSize);
			connectivityArray[1][j] = connectivityArray[0][j] + 1;
			connectivityArray[2][j] = connectivityArray[0][j] + lowResSize;
			connectivityArray[3][j] = connectivityArray[2][j] + 1;
			connectivityArray[4][j] = connectivityArray[0][j] + lowResSize *  lowResSize;
			connectivityArray[5][j] = connectivityArray[4][j] + 1;
			connectivityArray[6][j] = connectivityArray[4][j] + lowResSize;
			connectivityArray[7][j] = connectivityArray[6][j] + 1;
			j++;
		}
		*numberOfElements = sizeToExport;
		*firstElementNo = firstElemID;
	}
	return connectivityArray;
}

int writeLowResConnectivity(FmlSessionHandle session, FmlObjectHandle emsembleType, FmlObjectHandle sourceD)
{
	bool testOk = true;
	int offsets[2];
	int sizes[2];
	int firstElementNo = 1;
	int nodesPerElement = pow(2.0, sizeY);
	sizes[0] = lowResElementSize;
	sizes[1] = nodesPerElement;
	int numberOfElements = 1;
	int **connectivityArray = constructLowResConnectivityArray(nodesPerElement, &numberOfElements, &firstElementNo);


	FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, emsembleType, 0, sizes, 2 );

	if (lowResNumberOfNodes > mpi_rank)
	{
		sizes[0] = numberOfElements;
		sizes[1] = 1;
		offsets[0] = firstElementNo - 1;

		for( int i = 0; i < nodesPerElement ; i++)
		{
			offsets[1] = i;
			Fieldml_WriteIntSlab( writer, offsets, sizes, &(connectivityArray[i][0]) );
		}
	}

	Fieldml_CloseWriter( writer );

	if (connectivityArray)
	{
		for ( int i = 0; i < nodesPerElement; i++ )
		{
			delete[] connectivityArray[i];
		}
		delete[] connectivityArray;
	}

	return testOk;
}

/* done differently for finding the average potential easier */
void findLowResArraySizePerNode(int resSize, int numebrOfNodes)
{
	arraySizeForEachNode = new int[numebrOfNodes];
	numberOfRowsForEachNode = new int[numebrOfNodes];
	int remainder = resSize % numebrOfNodes;
	int lowSize =  (int)(resSize / numebrOfNodes);
	for (int i = 0; i < numebrOfNodes; i++)
	{
		numberOfRowsForEachNode[i] = lowSize;
		if (remainder != 0)
		{
			numberOfRowsForEachNode[i]++;
			remainder--;
		}
		arraySizeForEachNode[i] = numberOfRowsForEachNode[i] * resSize * resSize;
	}
}

int lowResHdf5Write(FmlSessionHandle session, FmlObjectHandle sourceD, FmlObjectHandle potentialSourceD,
	FmlObjectHandle emsembleType, FmlObjectHandle connectivityD)
{
	bool testOk = true;

	printf("low res HDF5 array write\n");

	findLowResArraySizePerNode(lowResSize, lowResNumberOfNodes);

	if (lowResNumberOfNodes > mpi_rank)
	{
		printf("mpi_size %d, mpi_rank %d number of rows %d size %d \n", lowResNumberOfNodes, mpi_rank,
			numberOfRowsForEachNode[mpi_rank], arraySizeForEachNode[mpi_rank]);
	}

	if (arraySizeForEachNode[mpi_rank] > 0)
	{

		FmlObjectHandle cType = Fieldml_CreateContinuousType( session, "test.scalar_real" );

		testOk = writeLowResCoordinates(session, cType, sourceD);

		testOk = testOk && writeLowResActivationTime(session, cType, potentialSourceD);

		testOk = testOk && writeLowResConnectivity(session, emsembleType, connectivityD);
	}

	if( testOk )
	{
		printf( "lowResHdf5Write - ok\n" );
	}
	else
	{
		printf( "lowResHdf5Write - failed\n" );
	}

	delete[] arraySizeForEachNode;

	delete[] numberOfRowsForEachNode;

	return 0;
}

int writeLowResFieldMLSolution()
{
	bool testOk = true;

	FmlSessionHandle session = Fieldml_Create( "test", "test" );

   int importHandle = Fieldml_AddImportSource( session, "http://www.fieldml.org/resources/xml/0.5/FieldML_Library_0.5.xml", "library" );
   FmlObjectHandle chart3dArgumentHandle = Fieldml_AddImport( session, importHandle, "chart.3d.argument", "chart.3d.argument" );
   FmlObjectHandle realType = Fieldml_AddImport( session, importHandle, "real.type", "real.1d" );
   FmlObjectHandle shapeType = Fieldml_AddImport( session, importHandle, "shape.unit.cube", "shape.unit.cube" );
   FmlObjectHandle trilinearPointsArgumentHandle = Fieldml_AddImport( session, importHandle,
   	"trilinearLagrange.points.argument", "parameters.3d.unit.trilinearLagrange.component.argument");
   FmlObjectHandle trilinearLagrangeParametersHandle = Fieldml_AddImport( session, importHandle,
   	"trilinearLagrange.parameters", "parameters.3d.unit.trilinearLagrange");
   FmlObjectHandle trilinearLagrangeParameteArgumentrsHandle = Fieldml_AddImport( session, importHandle,
   	"trilinearLagrange.parameters.argument", "parameters.3d.unit.trilinearLagrange.argument");
   FmlObjectHandle trilinearInterpolatorHandle = Fieldml_AddImport( session, importHandle,
      "trilinearLagrange.interpolator", "interpolator.3d.unit.trilinearLagrange");
   FmlObjectHandle coordinatesRC3DComponenentHandle = Fieldml_AddImport( session, importHandle,
      "coordinates.rc.3d.component.argument", "coordinates.rc.3d.component.argument");
   FmlObjectHandle coordinatesRC3DCHandle = Fieldml_AddImport( session, importHandle,
         "coordinates.rc.3d", "coordinates.rc.3d");

   FmlObjectHandle myEnsmebleHandle = Fieldml_CreateEnsembleType( session, "cube.nodes" );
   Fieldml_SetEnsembleMembersRange( session, myEnsmebleHandle, 1, lowResArraySize, 1 );

   FmlObjectHandle nodesArgumentHandle = Fieldml_CreateArgumentEvaluator( session, "cube.nodes.argument", myEnsmebleHandle );

   FmlObjectHandle cubeMeshHandle =Fieldml_CreateMeshType( session, "cube.mesh" );
   FmlObjectHandle meshEnsembleHandle = Fieldml_CreateMeshElementsType( session, cubeMeshHandle, "elements" );
   Fieldml_SetEnsembleMembersRange( session, meshEnsembleHandle, 1, lowResElementSize, 1 );
   FmlObjectHandle meshChartHandle = Fieldml_CreateMeshChartType( session, cubeMeshHandle, "chart" );
   //FmlObjectHandle chartContinuousHandle = Fieldml_CreateContinuousType( FmlSessionHandle handle, const char * name );
   Fieldml_CreateContinuousTypeComponents( session, meshChartHandle, "cube.mesh.chart.component", 3 );
   Fieldml_SetMeshShapes( session, cubeMeshHandle, shapeType );

   Fieldml_CreateArgumentEvaluator( session, "cube.mesh.argument", cubeMeshHandle );

   FmlObjectHandle cubeDOFsNodeArgumentHandle = Fieldml_CreateArgumentEvaluator( session, "cube.dofs.node.argument", realType );
   Fieldml_AddArgument( session, cubeDOFsNodeArgumentHandle, nodesArgumentHandle );

	FmlObjectHandle connectivityResource = Fieldml_CreateHrefDataResource( session,
		"cube.component1.trilinearLagrange.connectivity.resource", "PHDF5", "./output/lowResConnectivity.h5" );
	FmlObjectHandle sourceConnectivityD = Fieldml_CreateArrayDataSource( session,
		"cube.component1.trilinearLagrange.connectivity.resource.data", connectivityResource, "1", 2 );
	int rawSizes[2];
	rawSizes[0]= lowResElementSize;
	rawSizes[1]= pow(2, sizeY);
	Fieldml_SetArrayDataSourceRawSizes( session, sourceConnectivityD, rawSizes );
	Fieldml_SetArrayDataSourceSizes( session, sourceConnectivityD, rawSizes );

	FmlObjectHandle connectivityParameterHandle = Fieldml_CreateParameterEvaluator( session,
		"cube.component1.trilinearLagrange.connectivity", myEnsmebleHandle );
	Fieldml_SetParameterDataDescription( session, connectivityParameterHandle, FML_DATA_DESCRIPTION_DENSE_ARRAY );
	Fieldml_SetDataSource( session, connectivityParameterHandle, sourceConnectivityD );
	Fieldml_AddDenseIndexEvaluator( session, connectivityParameterHandle,
		Fieldml_GetObjectByName( session, "cube.mesh.argument.elements"), FML_INVALID_HANDLE );
	Fieldml_AddDenseIndexEvaluator( session, connectivityParameterHandle, trilinearPointsArgumentHandle, FML_INVALID_HANDLE );

	FmlObjectHandle cubeTrilinearLagrangeParameters = Fieldml_CreateAggregateEvaluator( session,
		"cube.component1.trilinearLagrange.parameters", trilinearLagrangeParametersHandle );
	Fieldml_SetIndexEvaluator( session, cubeTrilinearLagrangeParameters, 1, trilinearPointsArgumentHandle );
	Fieldml_SetDefaultEvaluator( session, cubeTrilinearLagrangeParameters, cubeDOFsNodeArgumentHandle );
	Fieldml_SetBind(session, cubeTrilinearLagrangeParameters, nodesArgumentHandle, connectivityParameterHandle);

	FmlObjectHandle cubeReferenceHandle = Fieldml_CreateReferenceEvaluator( session,
		"cube.component1.trilinearLagrange", trilinearInterpolatorHandle );
	Fieldml_SetBind(session, cubeReferenceHandle, chart3dArgumentHandle,
		Fieldml_GetObjectByName( session, "cube.mesh.argument.chart"));
	Fieldml_SetBind(session, cubeReferenceHandle, trilinearLagrangeParameteArgumentrsHandle,
		cubeTrilinearLagrangeParameters);

	FmlObjectHandle cubeTemplatehandle  = Fieldml_CreatePiecewiseEvaluator( session, "cube.component1.template", realType );
	Fieldml_SetIndexEvaluator( session, cubeTemplatehandle, 1, Fieldml_GetObjectByName( session, "cube.mesh.argument.elements") );
	Fieldml_SetDefaultEvaluator( session, cubeTemplatehandle, cubeReferenceHandle );

	FmlObjectHandle nodeResource = Fieldml_CreateHrefDataResource( session,
		"cube.geometric.dofs.node.resource", "PHDF5", "./output/lowrescoordinates.h5" );
	FmlObjectHandle nodeSourceD = Fieldml_CreateArrayDataSource( session,
		"cube.geometric.dofs.node.data", nodeResource, "1", 2 );
	rawSizes[0]= lowResArraySize;
	rawSizes[1]= 3;
	Fieldml_SetArrayDataSourceRawSizes( session, nodeSourceD, rawSizes );
	Fieldml_SetArrayDataSourceSizes( session, nodeSourceD, rawSizes );

	FmlObjectHandle cubeNodeParameterHandle = Fieldml_CreateParameterEvaluator( session,
		"cube.geometric.dofs.node", realType );
	Fieldml_SetParameterDataDescription( session, cubeNodeParameterHandle, FML_DATA_DESCRIPTION_DENSE_ARRAY );
	Fieldml_SetDataSource( session, cubeNodeParameterHandle, nodeSourceD );
	Fieldml_AddDenseIndexEvaluator( session, cubeNodeParameterHandle,
		nodesArgumentHandle, FML_INVALID_HANDLE );
	Fieldml_AddDenseIndexEvaluator( session, cubeNodeParameterHandle, coordinatesRC3DComponenentHandle, FML_INVALID_HANDLE );

	FmlObjectHandle cubeGiometricParameters = Fieldml_CreateAggregateEvaluator( session,
		"cube.geometric.parameters", coordinatesRC3DCHandle );
	Fieldml_SetIndexEvaluator( session, cubeGiometricParameters, 1, coordinatesRC3DComponenentHandle );
	Fieldml_SetDefaultEvaluator( session, cubeGiometricParameters, cubeTemplatehandle );
	Fieldml_SetBind(session, cubeGiometricParameters, cubeDOFsNodeArgumentHandle, cubeNodeParameterHandle);

	FmlObjectHandle potentialResource = Fieldml_CreateHrefDataResource( session, "cube.geometric.dofs.activationtime.resource",
		"PHDF5", "./output/lowresActivationTime.h5" );
	FmlObjectHandle potentialSourceD = Fieldml_CreateArrayDataSource( session, "cube.geometric.dofs.activationtime.data",
		potentialResource, "1", 1 );
	Fieldml_SetArrayDataSourceRawSizes( session, potentialSourceD, &lowResArraySize );
	Fieldml_SetArrayDataSourceSizes( session, potentialSourceD, &lowResArraySize );

	FmlObjectHandle nodeActivationParameterHandle = Fieldml_CreateParameterEvaluator( session,
		"cube.activationTime.dofs", realType );
	Fieldml_SetParameterDataDescription( session, nodeActivationParameterHandle, FML_DATA_DESCRIPTION_DENSE_ARRAY );
	Fieldml_SetDataSource( session, nodeActivationParameterHandle, potentialSourceD );
	Fieldml_AddDenseIndexEvaluator( session, nodeActivationParameterHandle,
		nodesArgumentHandle, FML_INVALID_HANDLE );

	FmlObjectHandle activationReferenceHandle = Fieldml_CreateReferenceEvaluator( session,
		"cube.activationTime.source", cubeTemplatehandle );
	Fieldml_SetBind(session, activationReferenceHandle, Fieldml_GetObjectByName( session, "cube.dofs.node.argument"),
		nodeActivationParameterHandle);

	testOk = lowResHdf5Write(session, nodeSourceD, potentialSourceD, myEnsmebleHandle, sourceConnectivityD);

	if (mpi_rank == 0)
		Fieldml_WriteFile( session, "myTest.xml" );

	Fieldml_Destroy( session );

	return testOk;
}



double findPotential(double x, double y, double z, double time)
{
	double propagation_speed = maximumTime / ( numberOfTimes - 1 );
	double radius = (( size - 1) / 2 );
	double distanceX = x - radius;
	double distanceY = y - radius;
	double distanceZ = z - radius;
	double distance = sqrt(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ);
	double activation_level = cos ( (M_PI / (double)2.0 * ( distance - (propagation_speed * time)) / radius ) );
	if (0.0 > activation_level )
		activation_level = 0.0;
	return activation_level;
}

int writeHighResPotential(FmlSessionHandle session, FmlObjectHandle cType, FmlObjectHandle sourceD)
{
	bool testOk = true;
	int offsets[2];
	int sizes[1];

	sizes[0] = arraySize;
	sizes[1] = numberOfTimes;

	FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, cType, 0, sizes, 2 );

	sizes[0] = arraySizeForEachNode[mpi_rank];
	sizes[1] = 1;
	offsets[0] = 0;
	for (int i = 0; i < mpi_rank; i ++)
	{
		offsets[0] = offsets[0] + arraySizeForEachNode[i];
	}
	double *potentialArray = new double[arraySizeForEachNode[mpi_rank]];
	double increment = maximumTime / ( numberOfTimes - 1 );
	for( long int i = 0; i < numberOfTimes ; i++)
	{
		double time = increment * i;
		for (int j = 0; j < arraySizeForEachNode[mpi_rank]; j++)
		{
			int nodeID = offsets[0] + j;
			potentialArray[j] = findPotential(highResCoordinates[0][nodeID], highResCoordinates[1][nodeID], highResCoordinates[2][nodeID], time );
		}
		offsets[1] = i;
		Fieldml_WriteDoubleSlab( writer, offsets, sizes, potentialArray );
	}

	Fieldml_CloseWriter( writer );

	return testOk;
}

int writeHighResCoordinates(FmlSessionHandle session, FmlObjectHandle cType, FmlObjectHandle sourceD)
{
	bool testOk = true;
	int offsets[2];
	int sizes[2];

	sizes[0] = arraySize;
	sizes[1] = sizeY;

	FmlObjectHandle writer = Fieldml_OpenArrayWriter( session, sourceD, cType, 0, sizes, 2 );

	sizes[0] = arraySizeForEachNode[mpi_rank];
	sizes[1] = 1;
	offsets[0] = 0;
	for (int i = 0; i < mpi_rank; i ++)
	{
		offsets[0] = offsets[0] + arraySizeForEachNode[i];
	}
	for( long int i = 0; i < sizeY ; i++)
	{
		offsets[1] = i;
		Fieldml_WriteDoubleSlab( writer, offsets, sizes, &(highResCoordinates[i][offsets[0]]) );
	}

	Fieldml_CloseWriter( writer );

	return testOk;
}

void findHighResArraySizePerNode(int sizeOfArray, int numebrOfNodes)
{
	arraySizeForEachNode = new int[numebrOfNodes];
	int remainder = sizeOfArray % numebrOfNodes;
	int lowArraySize =  (int)(sizeOfArray / numebrOfNodes);
	for (int i = 0; i < numebrOfNodes; i++)
	{
		arraySizeForEachNode[i] = lowArraySize;
		if (remainder != 0)
		{
			arraySizeForEachNode[i]++;
			remainder--;
		}
	}
}

int highResHdf5Write()
{
	bool testOk = true;

	printf("high res HDF5 array write\n");

	findHighResArraySizePerNode(arraySize, mpi_size);

	printf("mpi_size %d, mpi_rank %d array size %d \n", mpi_size, mpi_rank, arraySizeForEachNode[mpi_rank]);

	FmlSessionHandle session = Fieldml_Create( "test", "test" );

	FmlObjectHandle cType = Fieldml_CreateContinuousType( session, "test.scalar_real" );

	FmlObjectHandle resource = Fieldml_CreateHrefDataResource( session, "coordinates.resource", "PHDF5", "./output/coordinates.h5" );
	FmlObjectHandle sourceD = Fieldml_CreateArrayDataSource( session, "coordinates.source_double", resource, "coordinates", 2 );

	FmlObjectHandle potentialResource = Fieldml_CreateHrefDataResource( session, "potential.resource", "PHDF5", "./output/potential.h5" );
	FmlObjectHandle potentialSourceD = Fieldml_CreateArrayDataSource( session, "potential.source_double", potentialResource, "potential", 2 );

	testOk = writeHighResCoordinates(session, cType, sourceD);

	testOk = testOk && writeHighResPotential(session, cType, potentialSourceD);

	Fieldml_Destroy( session );

	if( testOk )
	{
		printf( "highResHdf5Write - ok\n" );
	}
	else
	{
		printf( "highResHdf5Write - failed\n" );
	}

	delete[] arraySizeForEachNode;

	return 0;
}

void setUpDataInOut()
{
	highResCoordinates = new double*[sizeY];

	for( long int i = 0; i < sizeY ; i++)
	{
		highResCoordinates[i] = new double[arraySize];
	}

	int j = 0;
	for (long int z = 0; z < size ; z++)
	{
		double zValue = (size /(size - 1)) * z;
		for (long int y = 0; y < size ; y++)
		{
			double yValue = (size /(size - 1)) * y;
			for (long int x = 0; x < size ; x++)
			{
				highResCoordinates[0][j] = (size /(size - 1)) * x;
				highResCoordinates[1][j] = yValue;
				highResCoordinates[2][j] = zValue;
				j++;
			}
		}
	}
}

void setUplowResCoordinates()
{
	lowResCoordinates = new double*[sizeY];
	for( long int i = 0; i < sizeY ; i++)
	{
		lowResCoordinates[i] = new double[lowResArraySize];
	}

	int j = 0;
	int increment = (size - 1) / (lowResSize - 1);
	for (long int z = 0; z< lowResSize ; z++)
	{
		double zValue = increment * z;
		for (long int y = 0; y < lowResSize ; y++)
		{
			double yValue = increment * y;
			for (long int x = 0; x < lowResSize ; x++)
			{
				lowResCoordinates[0][j] = increment * x;
				lowResCoordinates[1][j] = yValue;
				lowResCoordinates[2][j] = zValue;
				j++;
			}
		}
	}
}


int main( int argc, char **argv )
{
	store_argc = argc;

	store_argv = argv;

	suseconds_t prev_tv_usec = -1;
	suseconds_t prev_tv_sec = -1;

	timeval time_v;

	gettimeofday(&time_v, 0);

	prev_tv_usec = time_v.tv_usec;
	prev_tv_sec = time_v.tv_sec;

	setUpDataInOut();

	setUplowResCoordinates();

	MPI_Init(&store_argc, &store_argv);

	MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);

	MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

	highResHdf5Write();

	if (mpi_size > 1)
		lowResNumberOfNodes = mpi_size - 1;

	writeLowResFieldMLSolution();

	MPI_Finalize();

	for ( long int i = 0; i < sizeY; i++ )
	{
		delete[] highResCoordinates[i];
	}

	delete[] highResCoordinates;

	for ( long int i = 0; i < sizeY; i++ )
	{
		delete[] lowResCoordinates[i];
	}

	delete[] lowResCoordinates;

	gettimeofday(&time_v, 0);

	suseconds_t elapsed_time_usec = time_v.tv_usec - prev_tv_usec;
	suseconds_t elapsed_time = time_v.tv_sec - prev_tv_sec;
	if (elapsed_time < 0) elapsed_time += 100000000;
	{
		cout << " procs " << mpi_rank << " elapsed system time = " << elapsed_time << ".";
		cout.fill('0');
		cout.width(6);
		cout << right << elapsed_time_usec << endl;
	}

	// testTextWrite();

	return 0;
}
