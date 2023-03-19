// Copyright Epic Games, Inc. All Rights Reserved.

#include "VoxelDemoGameMode.h"
#include "VoxelDemoCharacter.h"
#include "UObject/ConstructorHelpers.h"

AVoxelDemoGameMode::AVoxelDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
