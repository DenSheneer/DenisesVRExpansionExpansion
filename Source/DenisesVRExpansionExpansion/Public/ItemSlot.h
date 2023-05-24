// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "SlotableActorVisuals.h"
#include "ItemSlotState.h"
#include "DetailCategoryBuilder.h"
#include "ItemSlotTrigger.h"
#include "ItemSlot.generated.h"

class ASlotableActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class UItemSlot : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UItemSlot();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "1"))
		FSlotableActorVisuals triggerVisuals;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "2"))
		UMaterial* lefthandMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "3"))
		UMaterial* rightHandMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "5"))
		TMap<TSubclassOf<class ASlotableActor>, FSlotableActorVisuals> visualsArray;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Debug")
		TEnumAsByte<EItemSlotState> currentState = EItemSlotState::available;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Debug")
		AActor* reservedForActor;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Debug")
		FSlotableActorVisuals currentlyDisplayedVisuals;

	void SaveEdit();
	void SaveMeshTransform();
	void SaveTriggerTransform();


public:
	//	(E)ditor functions
	void E_TogglePreviewVisibility();
	void E_ReloadVisuals();
	void E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> visuals);
	void E_ModifyTriggerShape();
	void E_SetPreviewVisuals(FSlotableActorVisuals visualProperties);
	void E_SetVisibility(bool hidden);

	// (R)untime functions
	UFUNCTION(NetMulticast, Reliable) void R_SetPreviewVisuals(FSlotableActorVisuals visualProperties);
	void R_SetVisibility(bool hidden);

	bool CheckForCompatibility(ASlotableActor* actor);

	UFUNCTION(Server, Reliable) void ReserveForActor(ASlotableActor* actor, EControllerHand handSide);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "4"))
		TArray<TSubclassOf<class ASlotableActor>> acceptedActors;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif
	UFUNCTION(NetMulticast, Reliable)	void ReceiveActor(ASlotableActor* actor);

	void RemoveSlotableActor(ASlotableActor* actor);
	const EItemSlotState SlotState() { return currentState; }
	void ActorOutOfRangeEvent(ASlotableActor* actor);

	DECLARE_EVENT_OneParam(UItemSlot, FSlotOccupiedEvent, UItemSlot*)
		FSlotOccupiedEvent& OnOccupied(ASlotableActor*, UItemSlot*) { return OnOccupiedEvent; }

	DECLARE_EVENT_OneParam(UItemSlot, FSlotAvailableEvent, UItemSlot*)
		FSlotAvailableEvent& OnIsAvailable(UItemSlot*) { return OnAvailableEvent; }

	FSlotOccupiedEvent OnOccupiedEvent;
	FSlotAvailableEvent OnAvailableEvent;

private:
	UFUNCTION(Server, Reliable) void setVisualsOnReservation(ASlotableActor* actor, EControllerHand handSide);
	TSubclassOf<class ASlotableActor> currentlyDisplayedSlotableActor;

	UPROPERTY(Replicated)	UStaticMeshComponent* previewMesh;
	UPROPERTY(Replicated)	UItemSlotTrigger* trigger;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//UFUNCTION(Server, Reliable)
	void setupTriggerComponent();
	//UFUNCTION(Server, Reliable)
	void setupMeshShapeComponent();

	UFUNCTION(NetMulticast, Reliable)
		void server_Setup();
};
