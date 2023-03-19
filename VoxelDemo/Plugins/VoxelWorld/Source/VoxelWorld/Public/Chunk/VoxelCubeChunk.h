// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelCubeChunk.generated.h"

class AVoxelGrid;

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Forward, Right, Back, Left, Up, Down
};

struct FVoxelCube : public TSharedFromThis<FVoxelCube>
{
	FVoxelCube()
	{
		Links.SetNumZeroed(6);
	}

	~FVoxelCube()
	{
		Links.Empty();
	};

	FVoxelCube(const FVoxelCube&) = delete;
	FVoxelCube& operator=(const FVoxelCube&) = delete;

public:
	static bool LinkCubes(TSharedPtr<FVoxelCube> FirstCube, TSharedPtr<FVoxelCube> SecondCube, EDirection LinkDirection);

private:
	TArray<TWeakPtr<FVoxelCube>> Links;
};


USTRUCT()
struct FVoxelChunkMeshData
{
	GENERATED_BODY();

public:
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<FVector2D> UV0;

	void Clear();
};

/*
struct FGreedyMeshTask : public FNonAbandonableTask
{
public:

	FGreedyMeshTask(const FIntVector InOffset, const TArray<TSharedPtr<FVoxelCube>>&Cubes)
		: Offset(InOffset)
		, CubeData(Cubes)
	{

	}

	void DoWork()
	{
		MergeMesh();
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGreedyMeshTask, STATGROUP_ThreadPoolAsyncTasks);
	}

	friend class FAsyncTask<FGreedyMeshTask>;
private:
	void MergeMesh();
	int GetCubeIndex(const FIntVector LocalCoord) const;

private:
	FIntVector Offset;

	const TArray<TSharedPtr<FVoxelCube>> CubeData;

	FVoxelChunkMeshData MeshData;
};*/

USTRUCT()
struct FVoxelCubeChunk
{
	GENERATED_BODY();

	FVoxelCubeChunk()
		:CoordOffset(FIntVector::ZeroValue)
	{
		Cubes.SetNumZeroed(32 * 32 * 32);
	}

	~FVoxelCubeChunk()
	{
		Cubes.Empty();
	}

	friend class AVoxelGrid;

public:
	void MergeMesh();
	int GetCubeIndex(const FIntVector LocalCoord) const;
	static void CreateQuad(const bool bObverse, const FIntVector AxisMask, const int Height, const int Width
		, const FIntVector V1, const FIntVector V2, const FIntVector V3, const FIntVector V4, FVoxelChunkMeshData& OutMeshData);
private:
	bool AddCubeAt(const FIntVector LocalCoord);
	void AddCubeMeshAt(const FIntVector LocalCoord);
private:

	FIntVector CoordOffset;
	TArray<TSharedPtr<FVoxelCube>> Cubes;
	TArray<FIntVector> DirtyCubes;
	FVoxelChunkMeshData MeshData;
	int CubeNum = 0;
};
