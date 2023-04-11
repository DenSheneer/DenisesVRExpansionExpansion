// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SlotableActorVisuals.h"
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
	bool isOccupied = false;
	int actorsInRange = 0;

	virtual void BeginPlay() override;

	//	editor functions
	UFUNCTION(CallInEditor,						Category = "Preview Visuals", meta=(DisplayPriority = "0"))
		void CycleThroughPreviews();
	UFUNCTION(CallInEditor,						Category = "Preview Visuals", meta=(DisplayPriority = "1"))
		void TogglePreviewVisibility();
	UFUNCTION(CallInEditor,						Category = "Preview Visuals", meta=(DisplayPriority = "2"))
		void ReloadVisuals();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview Visuals", meta=(DisplayPriority = "3"))
		TMap<int, FSlotableActorVisuals> visualsArray;

	void SavePreviewPosAndRot();
	void SetPreviewVisuals(FSlotableActorVisuals visualProperties);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accepted actors")
		TArray<TSubclassOf<class ASlotableActor>> acceptedActors;


public:
	bool checkCompatibility(ASlotableActor* actor);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation) override;
#endif

	virtual void ReceiveSlotableActor(ASlotableActor* actor);
	virtual void RemoveSlotableActor(ASlotableActor* actor);
	bool IsOccupied() { return isOccupied; }

	virtual void ActorInRangeEvent(ASlotableActor* actor);
	virtual void ActorOutOfRangeEvent(ASlotableActor* actor);

	DECLARE_EVENT_OneParam(UItemSlot, FSlotOccupiedEvent, UItemSlot*)
		FSlotOccupiedEvent& OnOccupied(UItemSlot*) { return OnOccupiedEvent; }

	DECLARE_EVENT_OneParam(UItemSlot, FSlotAvailableEvent, UItemSlot*)
		FSlotAvailableEvent& OnIsAvailable(UItemSlot*) { return OnAvailableEvent; }

	FSlotOccupiedEvent OnOccupiedEvent;
	FSlotAvailableEvent OnAvailableEvent;

private:
	int currentVisualIndex = 0;
	USphereComponent* triggerShape;
};
