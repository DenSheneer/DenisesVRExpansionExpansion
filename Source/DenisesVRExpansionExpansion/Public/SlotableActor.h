// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Grippables/GrippableActor.h"
#include "ItemGripState.h"
#include "ItemSlotTrigger.h"
#include "GripMotionControllerComponent.h"
#include "ItemSlot.h"
#include "SlotableActor.generated.h"

/**
 *
 */

UCLASS()
class ASlotableActor : public AGrippableActor
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Static values", meta = (DisplayName = "Preview Mesh", MakeStructureDefaultValue = "C:/Program Files/Epic Games/UE_5.1/Engine/Content/BasicShapes/Cube.uasset"))
		TObjectPtr<UStaticMesh> PreviewMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Static values", meta = (DisplayName = "Scale", MakeStructureDefaultValue = "1.000000,1.000000,1.000000"))
		FVector MeshScale;

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")
		TEnumAsByte<EItemGripState> currentGripState = EItemGripState::loose;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")
		UGripMotionControllerComponent* currentGrippingController = nullptr;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")
	EControllerHand handSide = EControllerHand::AnyHand;

	UItemSlot* current_ResidingSlot = nullptr;
	UItemSlot* currentNearestSlot;
	TArray<UItemSlot*> currentlyAvailable_Slots;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;

	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;
	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;

	UFUNCTION(Blueprintcallable) void ComponentOverlapBegin(UActorComponent* other);
	UFUNCTION(Blueprintcallable) void ComponentOverlapEnd(UActorComponent* other);

private:
	void setupColliderRef();
	void manualFindAvailableSlotsCall();
	void refreshNearestSlot();
	void handleSlotOverlap(UItemSlot* overlappingSlot, bool skipNearestRefresh = false);
	void removeSlotFromList(UItemSlot* slotToRemove);
	void addSlotToList(UItemSlot* slotToAdd, bool skipNearestRefresh = false);
	void reset_GrippingParameters();
	UItemSlot* findNearestSlot(TArray<UItemSlot*> slotsToCheck);
	USphereComponent* triggerComponent;

	//	availability events
	FDelegateHandle OccupiedEventHandle;

	TMap<UItemSlot*, FDelegateHandle> AvailableEventHandles;
	void subscribeToSlotOccupiedEvent(UItemSlot* slot);
	void unsubscribeFromOccupiedEvent(UItemSlot* slot);
	void subscribeToSlotAvailableEvent(UItemSlot* slot);
	void unsubscribeFromAvailableEvent(UItemSlot* slot);
};
