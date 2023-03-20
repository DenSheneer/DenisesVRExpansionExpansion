// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "GestureComponent.generated.h"

/**
 *
 */
UCLASS()
class DENISESVREXPANSIONEXPANSION_API UGestureComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pose")
		UAnimSequence* gesturePose;
};
