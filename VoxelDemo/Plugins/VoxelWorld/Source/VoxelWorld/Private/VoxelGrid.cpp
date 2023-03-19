// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelGrid.h"
#include "ProceduralMeshComponent.h"

// Sets default values
AVoxelGrid::AVoxelGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>("GreedyCubes");
	Mesh->SetCastShadow(false);
	Mesh->SetMobility(EComponentMobility::Stationary);
	Mesh->bUseComplexAsSimpleCollision = false;
	Mesh->SetSimulatePhysics(false);
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void AVoxelGrid::BeginPlay()
{
	Super::BeginPlay();
	ApplyMesh();
}

void AVoxelGrid::PostLoad()
{
	Super::PostLoad();
	Setup();
}

void AVoxelGrid::Setup()
{
	AddCubeAt(FIntVector::ZeroValue);
}

void AVoxelGrid::ApplyMesh()
{
	int SectionNum = 0;
	for (const FVoxelCubeChunk& Chunk : Chunks)
	{
		Mesh->SetMaterial(SectionNum, Material.Get());
		Mesh->CreateMeshSection(
			SectionNum,
			Chunk.MeshData.Vertices,
			Chunk.MeshData.Triangles,
			Chunk.MeshData.Normals,
			Chunk.MeshData.UV0,
			Chunk.MeshData.Colors,
			TArray<FProcMeshTangent>(),
			true
		);
		Mesh->AddCollisionConvexMesh(Chunk.MeshData.Vertices);

		SectionNum++;
	}
	
}

// Called every frame
void AVoxelGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVoxelGrid::AddCubeAt(const FIntVector InCoord)
{
	const FIntVector ChunkOffset = (InCoord / 32) * 32;

	auto Chunk = Chunks.FindByPredicate([ChunkOffset](const FVoxelCubeChunk& InChunk) {
		return InChunk.CoordOffset == ChunkOffset;
		});
	bool Result = false;
	if (Chunk == nullptr)
	{
		FVoxelCubeChunk NewChunk;
		NewChunk.CoordOffset = ChunkOffset;
		Result = NewChunk.AddCubeAt(InCoord - ChunkOffset);
		Chunks.Add(NewChunk);
	}
	else
	{
		Result = Chunk->AddCubeAt(InCoord - ChunkOffset);
	}

	if (Result)
	{
		ApplyMesh();
	}
}

void AVoxelGrid::MergeMesh()
{
	int SectionNum = 0;
	for (FVoxelCubeChunk& Chunk : Chunks)
	{
		Chunk.MergeMesh();
		Mesh->SetMaterial(SectionNum, Material.Get());
		Mesh->CreateMeshSection(
			SectionNum,
			Chunk.MeshData.Vertices,
			Chunk.MeshData.Triangles,
			Chunk.MeshData.Normals,
			Chunk.MeshData.UV0,
			Chunk.MeshData.Colors,
			TArray<FProcMeshTangent>(),
			true
		);
		Mesh->AddCollisionConvexMesh(Chunk.MeshData.Vertices);

		SectionNum++;
	}
}

void AVoxelGrid::RemoveCubeAt(const FIntVector InCoord)
{

}


