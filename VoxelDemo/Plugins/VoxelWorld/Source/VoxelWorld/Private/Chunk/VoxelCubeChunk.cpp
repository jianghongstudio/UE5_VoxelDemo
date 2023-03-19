// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk/VoxelCubeChunk.h"
//#pragma optimize("",off)

bool FVoxelCube::LinkCubes(TSharedPtr<FVoxelCube> FirstCube, TSharedPtr<FVoxelCube> SecondCube, EDirection LinkDirection)
{
	static const EDirection ReverseTable[] =
	{
		EDirection::Back,
		EDirection::Left,
		EDirection::Forward,
		EDirection::Right,
		EDirection::Down,
		EDirection::Up
	};

	const EDirection ReverseDirection = ReverseTable[(uint8)LinkDirection];

	if (FirstCube == SecondCube)
	{
		return false;
	}

	if (!FirstCube.IsValid() || !SecondCube.IsValid())
	{
		return false;
	}

	if (FirstCube->Links[(uint8)LinkDirection].IsValid())
	{
		return false;
	}

	if (SecondCube->Links[(uint8)ReverseDirection].IsValid())
	{
		return false;
	}

	FirstCube->Links[(uint8)LinkDirection] = SecondCube;
	SecondCube->Links[(uint8)ReverseDirection] = FirstCube;

	return true;
}

int FVoxelCubeChunk::GetCubeIndex(const FIntVector LocalCoord) const
{
	static FBox BoundBox = FBox(FVector::ZeroVector, FVector(31, 31, 31));
	if (BoundBox.IsInsideOrOn(FVector(LocalCoord)))
	{
		return LocalCoord.Z * 32 * 32 + LocalCoord.Y * 32 + LocalCoord.X;
	}
	
	return -1;
}

bool FVoxelCubeChunk::AddCubeAt(const FIntVector LocalCoord)
{
	int CubeIndex = GetCubeIndex(LocalCoord);
	if (CubeIndex < 0 || Cubes.Num() < CubeIndex + 1)
	{
		return false;
	}

	if (Cubes[CubeIndex].IsValid())
	{
		return false;
	}

	TSharedPtr<FVoxelCube> NewCube = nullptr;

	if (CubeNum > 0)
	{
		static const TArray<FIntVector> CheckList =
		{
			FIntVector(1,0,0),
			FIntVector(0,1,0),
			FIntVector(-1,0,0),
			FIntVector(0,-1,0),
			FIntVector(0,0,1),
			FIntVector(0,0,-1),
		};

		

		for (int i = 0; i < CheckList.Num(); ++ i)
		{
			const int CheckIndex = GetCubeIndex(LocalCoord + CheckList[i]);
			if (CheckIndex < 0 || Cubes.Num() < CheckIndex +1 || !Cubes[CheckIndex].IsValid())
			{
				continue;
			}
			if (!NewCube.IsValid())
			{
				NewCube = Cubes[CubeIndex] = MakeShared<FVoxelCube>();
			}
			FVoxelCube::LinkCubes(NewCube, Cubes[CheckIndex], (EDirection)i);
		}
	}
	else
	{
		NewCube = Cubes[CubeIndex] = MakeShared<FVoxelCube>();
	}
	
	DirtyCubes.Add(LocalCoord);
	AddCubeMeshAt(LocalCoord);
	CubeNum++;
	return true;
}

void FVoxelCubeChunk::AddCubeMeshAt(const FIntVector LocalCoord)
{
	for (int Axis = 0; Axis < 3; ++Axis)
	{
		FIntVector AxisMask = FIntVector::ZeroValue;
		AxisMask[Axis] = 1;
		const int Axis1 = (Axis + 1) % 3;
		const int Axis2 = (Axis + 2) % 3;

		FIntVector CurrentItr = LocalCoord + CoordOffset;
		FIntVector DeltaAxis1 = FIntVector::ZeroValue;
		FIntVector DeltaAxis2 = FIntVector::ZeroValue;

		DeltaAxis1[Axis1] = 1;
		DeltaAxis2[Axis2] = 1;

		CreateQuad(true, AxisMask, 1, 1, CurrentItr, CurrentItr + DeltaAxis1, CurrentItr + DeltaAxis2, CurrentItr + DeltaAxis1 + DeltaAxis2, MeshData);

		CurrentItr[Axis] += 1;
		CreateQuad(false, AxisMask, 1, 1, CurrentItr, CurrentItr + DeltaAxis1, CurrentItr + DeltaAxis2, CurrentItr + DeltaAxis1 + DeltaAxis2, MeshData);
	}
}

void FVoxelCubeChunk::CreateQuad(const bool bObverse, const FIntVector AxisMask, const int Height, const int Width, const FIntVector V1, const FIntVector V2, const FIntVector V3, const FIntVector V4, FVoxelChunkMeshData& OutMeshData)
{
	const int NormalFlag = (bObverse ? -1 : 1);
	const FVector Normal = FVector(AxisMask * NormalFlag);
	const FColor Color = FColor::Red;
	int VertexOffset = OutMeshData.Vertices.Num();
	OutMeshData.Vertices.Append({
		FVector(V1) * 100,
		FVector(V2) * 100,
		FVector(V3) * 100,
		FVector(V4) * 100,
		});

	OutMeshData.Triangles.Append({
		VertexOffset,
		VertexOffset + 2 + NormalFlag,
		VertexOffset + 2 - NormalFlag,
		VertexOffset + 3,
		VertexOffset + 1 - NormalFlag,
		VertexOffset + 1 + NormalFlag
		});

	OutMeshData.Normals.Append({
		Normal,
		Normal,
		Normal,
		Normal
		});

	OutMeshData.Colors.Append({
		Color,
		Color,
		Color,
		Color
		});

	if (Normal.X == 1 || Normal.X == -1)
	{
		OutMeshData.UV0.Append({
			FVector2D(Width, Height),
			FVector2D(0, Height),
			FVector2D(Width, 0),
			FVector2D(0, 0),
			});
	}
	else
	{
		OutMeshData.UV0.Append({
			FVector2D(Height, Width),
			FVector2D(Height, 0),
			FVector2D(0, Width),
			FVector2D(0, 0),
			});
	}
}

void FVoxelChunkMeshData::Clear()
{
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	Colors.Empty();
	UV0.Empty();
}

void FGreedyMeshTask::MergeMesh()
{
	MeshData.Clear();

	for (int Axis = 0; Axis < 3; ++Axis)
	{
		const int MainAxisLimit = 32;
		const int Axis1Limit = 32;
		const int Axis2Limit = 32;

		int Axis1 = (Axis + 1) % 3;
		int Axis2 = (Axis + 2) % 3;

		FIntVector DeltaAxis1 = FIntVector::ZeroValue;
		FIntVector DeltaAxis2 = FIntVector::ZeroValue;

		FIntVector ChunkItr = FIntVector::ZeroValue;

		FIntVector AxisMask = FIntVector::ZeroValue;
		AxisMask[Axis] = 1;

		TArray<int> QuadMask;
		QuadMask.SetNum(Axis1Limit * Axis2Limit);

		for (ChunkItr[Axis] = -1; ChunkItr[Axis] < MainAxisLimit;)
		{
			int N = 0;
			for (ChunkItr[Axis2] = 0; ChunkItr[Axis2] < Axis2Limit; ++ChunkItr[Axis2])
			{
				for (ChunkItr[Axis1] = 0; ChunkItr[Axis1] < Axis1Limit; ++ChunkItr[Axis1])
				{
					const int CurrentCubeIndex = GetCubeIndex(ChunkItr);
					const int CompareCubeIndex = GetCubeIndex(ChunkItr + AxisMask);

					TSharedPtr<FVoxelCube> CurrentCube = CurrentCubeIndex >= 0 ? CubeData[CurrentCubeIndex] : nullptr;
					TSharedPtr<FVoxelCube> CompareCube = CompareCubeIndex >= 0 ? CubeData[CompareCubeIndex] : nullptr;

					const bool CurrentCubeOpaque = CurrentCube.IsValid();
					const bool CompareCubeOpaque = CompareCube.IsValid();

					if (CurrentCubeOpaque == CompareCubeOpaque)
					{
						QuadMask[N++] = 0;
					}
					else if (CurrentCubeOpaque)
					{
						QuadMask[N++] = -1;
					}
					else
					{
						QuadMask[N++] = 1;
					}
				}
			}

			++ChunkItr[Axis];
			N = 0;

			for (int j = 0; j < Axis2Limit; ++j)
			{
				for (int i = 0; i < Axis1Limit;)
				{
					if (QuadMask[N] != 0)
					{
						const int CurrentMask = QuadMask[N];
						ChunkItr[Axis1] = i;
						ChunkItr[Axis2] = j;

						int Width;

						for (Width = 1; i + Width < Axis1Limit && QuadMask[N + Width] == CurrentMask; ++Width)
						{
						}

						int Height;
						bool Done = false;

						for (Height = 1; j + Height < Axis2Limit; ++Height)
						{
							for (int k = 0; k < Width; ++k)
							{
								if (QuadMask[N + k + Height * Axis1Limit] == CurrentMask)
								{
									continue;
								}
								Done = true;
								break;
							}
							if (Done)
							{
								break;
							}
						}
						DeltaAxis1[Axis1] = Width;
						DeltaAxis2[Axis2] = Height;

						const FIntVector CurrentCoord = ChunkItr + Offset;
						FVoxelCubeChunk::CreateQuad(CurrentMask > 0, AxisMask, Height, Width
							, CurrentCoord
							, CurrentCoord + DeltaAxis1
							, CurrentCoord + DeltaAxis2
							, CurrentCoord + DeltaAxis1 + DeltaAxis2
							, MeshData);
						DeltaAxis1 = FIntVector::ZeroValue;
						DeltaAxis2 = FIntVector::ZeroValue;

						for (int l = 0; l < Height; ++l)
						{
							for (int k = 0; k < Width; ++k)
							{
								QuadMask[N + k + l * Axis1Limit] = 0;
							}
						}
						i += Width;
						N += Width;
					}
					else
					{
						i++;
						N++;
					}
				}
			}
		}
	}
}

int FGreedyMeshTask::GetCubeIndex(const FIntVector LocalCoord) const
{
	static FBox BoundBox = FBox(FVector::ZeroVector, FVector(31, 31, 31));
	if (BoundBox.IsInsideOrOn(FVector(LocalCoord)))
	{
		return LocalCoord.Z * 32 * 32 + LocalCoord.Y * 32 + LocalCoord.X;
	}

	return -1;
}
