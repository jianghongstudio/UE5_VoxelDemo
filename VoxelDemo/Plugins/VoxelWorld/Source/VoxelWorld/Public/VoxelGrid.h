// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk/VoxelCubeChunk.h"
#include "VoxelGrid.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class VOXELWORLD_API AVoxelGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostLoad() override;

private:

	void Setup();

	void ApplyMesh();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void AddCubeAt(const FIntVector InCoord);

	UFUNCTION(BlueprintCallable)
	void MergeMesh();

	void RemoveCubeAt(const FIntVector InCoord);



private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMaterialInterface> Material;

private:

	TObjectPtr<UProceduralMeshComponent> Mesh;

	int VertexCount = 0;

	TArray<FVoxelCubeChunk> Chunks;

};
