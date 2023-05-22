// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ShapeComponent.h"
#include "SlotableActorVisuals.h"
#include "ItemSlotTrigger.generated.h"

class UItemSlot;

UCLASS()
class DENISESVREXPANSIONEXPANSION_API UItemSlotTrigger : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	void SetUp(UItemSlot* pAttachedTo, FSlotableActorVisuals visuals);
	UItemSlot* AttachedTo() { return attachedTo; }

protected:



private:
	UItemSlot* attachedTo;
};
