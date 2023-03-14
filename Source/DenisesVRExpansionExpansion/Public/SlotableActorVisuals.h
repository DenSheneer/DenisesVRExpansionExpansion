// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlotableActorVisuals.generated.h"

USTRUCT(BlueprintType)
struct FSlotableActorVisuals
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Mesh", MakeStructureDefaultValue = "None"))
		TObjectPtr<UStaticMesh> Mesh;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "PreviewMaterial", MakeStructureDefaultValue = "None"))
		TObjectPtr<UMaterial> PreviewMaterial;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "Scale", MakeStructureDefaultValue = "1.000000,1.000000,1.000000"))
		FVector Scale;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "RelativePosition", MakeStructureDefaultValue = "0.000000,0.000000,0.000000"))
		FVector RelativePosition;
	
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "RelativeRotation", MakeStructureDefaultValue = "0.000000,0.000000,0.000000"))
		FRotator RelativeRotation;
};

