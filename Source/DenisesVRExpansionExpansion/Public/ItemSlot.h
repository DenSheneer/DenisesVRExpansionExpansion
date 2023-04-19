// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SlotableActorVisuals.h"
#include "ItemSlotState.h"
#include "DetailCategoryBuilder.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		TEnumAsByte<EItemSlotState> currentState = EItemSlotState::available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		AActor* reservedForActor;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "1"))
		FSlotableActorVisuals triggerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "2"))
		UMaterial* lefthandMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "3"))
		UMaterial* rightHandMaterial;

	TMap<int, FSlotableActorVisuals> visualsArray;

	void SaveMeshTransform();
	void SetPreviewVisuals(FSlotableActorVisuals visualProperties);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accepted actors")
		TArray<TSubclassOf<class ASlotableActor>> acceptedActors;

public:
	//	editor functions
	void CycleThroughPreviews();
	void TogglePreviewVisibility();
	void ReloadVisuals();
	void EditTriggerShape();

	bool CheckForCompatibility(ASlotableActor* actor);
	bool TryToReserve(ASlotableActor* actor, EControllerHand handSide);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif

	bool TryToReceiveActor(ASlotableActor* actor);
	void RemoveSlotableActor(ASlotableActor* actor);
	const EItemSlotState SlotState() { return currentState; }

	void ActorOutOfRangeEvent(ASlotableActor* actor);

	DECLARE_EVENT_OneParam(UItemSlot, FSlotOccupiedEvent, UItemSlot*)
		FSlotOccupiedEvent& OnOccupied(UItemSlot*) { return OnOccupiedEvent; }

	DECLARE_EVENT_OneParam(UItemSlot, FSlotAvailableEvent, UItemSlot*)
		FSlotAvailableEvent& OnIsAvailable(UItemSlot*) { return OnAvailableEvent; }

	FSlotOccupiedEvent OnOccupiedEvent;
	FSlotAvailableEvent OnAvailableEvent;

private:
	virtual void reserveSlotForActor(ASlotableActor* actor, EControllerHand handSide);
	int currentVisualIndex = 0;
};
