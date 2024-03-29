// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Grippables/GrippableActor.h"
#include "ItemGripState.h"
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
	ASlotableActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Static values", meta = (DisplayName = "Preview Mesh", MakeStructureDefaultValue = ""))
		TObjectPtr<UStaticMesh> PreviewMesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Static values", meta = (DisplayName = "Scale", MakeStructureDefaultValue = "1.000000,1.000000,1.000000"))
		FVector MeshScale;

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere)							UPrimitiveComponent* rootAsPrimitiveComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")				USphereComponent* ColliderComponent;
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")	TEnumAsByte<EItemGripState> currentGripState = EItemGripState::loose;
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")	UGripMotionControllerComponent* currentGrippingController = nullptr;
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Grip info")	EControllerHand handSide = EControllerHand::AnyHand;

	UPROPERTY(Replicated) UItemSlot* current_ResidingSlot = nullptr;
	UPROPERTY(Replicated) UItemSlot* currentNearestSlot = nullptr;
	TArray<UItemSlot*> currentlyAvailable_Slots;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaSeconds) override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	UFUNCTION(NetMulticast, Reliable) void Server_Grip(UGripMotionControllerComponent* GrippingController);

	virtual void OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed = false) override;
	UFUNCTION(NetMulticast, Reliable) void Server_GripRelease(UGripMotionControllerComponent* ReleasingController);


private:
	void setupColliderRef();
	void manualFindAvailableSlotsCall();

	UFUNCTION(Server, Reliable) void refreshNearestSlot();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION() void checkForSlotOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void checkForSlotOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION(Server, Reliable) void checkForSlotOnOverlapEndServer(UPrimitiveComponent* OtherComp);

	void removeSlotFromList(UItemSlot* slotToRemove);
	void addSlotToList(UItemSlot* slotToAdd, bool skipNearestRefresh = false);
	void reset_GrippingParameters();
	UItemSlot* findNearestSlot(TArray<UItemSlot*> slotsToCheck);


	//	availability events
	//FDelegateHandle OccupiedEventHandle;
	//TMap<UItemSlot*, FDelegateHandle> AvailableEventHandles;

	TArray<UItemSlot*> becomeAvailableSlots;
	TArray<UItemSlot*> becomeOccupiedSlots;

	void subscribeToSlotOccupiedEvent(UItemSlot* slot);
	void unsubscribeFromOccupiedEvent(UItemSlot* slot);
	void subscribeToSlotAvailableEvent(UItemSlot* slot);
	void unsubscribeFromAvailableEvent(UItemSlot* slot);
};
