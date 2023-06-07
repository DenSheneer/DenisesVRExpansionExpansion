// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "SlotableActorVisuals.h"
#include "ItemSlotState.h"
#include "DetailCategoryBuilder.h"
#include "CollisionShape.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "4"))
		TMap<TSubclassOf<class ASlotableActor>, FSlotableActorVisuals> visualsArray;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)	UStaticMesh* boxMesh;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)	UStaticMesh* sphereMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)		FSlotableActorVisuals triggerVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	FSlotableActorVisuals rootVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	FSlotableActorVisuals currentlyDisplayedVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	TSubclassOf<class ASlotableActor> currentlyDisplayedSlotableActor;

	UPROPERTY(Replicated)	TEnumAsByte<EItemSlotState> currentState = EItemSlotState::available;
	UPROPERTY(Replicated)	AActor* reservedForActor;
	UPROPERTY(Replicated)	USphereComponent* transformRoot;
	UPROPERTY(Replicated)	UStaticMeshComponent* previewMesh;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)	UShapeComponent* trigger;


	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "2"))
		UMaterial* leftHandMaterial;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "3"))
		UMaterial* rightHandMaterial;

	void SaveEdit();
	void SaveRootTransform();
	void SaveMeshTransform();
	void SaveTriggerTransform();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//	(E)ditor functions
	void E_TogglePreviewVisibility();
	void E_ReloadVisuals();
	void E_ModifyRootComponent();
	void E_ModifyTriggerShape();
	void E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> visuals);
	void E_SetPreviewVisuals(const FSlotableActorVisuals visualProperties);
	void E_SetVisibility(bool hidden);
	void E_ResetActorMeshToRootTransform(TSubclassOf<class ASlotableActor> visuals);
	void E_ResetTriggerMeshToRootTransform();
	void E_SetTriggerShape(const ECollisionShape::Type shapeType);

	// (R)untime functions
	UFUNCTION(NetMulticast, Reliable) void R_SetPreviewVisuals(const FSlotableActorVisuals visualProperties, const EControllerHand handside);

	bool CheckForCompatibility(const ASlotableActor* actor);

	UFUNCTION(Server, Reliable) void ReserveForActor(ASlotableActor* actor, const EControllerHand handSide);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "1"))
		TArray<TSubclassOf<class ASlotableActor>> acceptedActors;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) uint8 editorCollisionShape = 1;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
#endif

private:
	UFUNCTION(Server, Reliable) void setVisualsOnReservation(const ASlotableActor* actor, const EControllerHand handSide);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void addActorToVisualArray(TSubclassOf<class ASlotableActor> newActor);
	void removeActorFromVisualsArray(TSubclassOf<class ASlotableActor> removeActor);
	void setupTriggerComponent();
	void setupMeshShapeComponent();


	UFUNCTION(NetMulticast, Reliable)
		void server_Setup();

};
