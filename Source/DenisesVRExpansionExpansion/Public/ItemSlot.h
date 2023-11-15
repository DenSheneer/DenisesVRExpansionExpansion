// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#if WITH_EDITOR
#include "DetailCategoryBuilder.h"
#endif

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "SlotableActorVisuals.h"
#include "ItemSlotState.h"
#include "CollisionShape.h"
#include "ItemSlot.generated.h"

class ASlotableActor;

DECLARE_DELEGATE_OneParam(FOnOccupiedDelegate, UItemSlot*);
DECLARE_DELEGATE_OneParam(FOnAvailableDelegate, UItemSlot*);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorReceivedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActorExitEvent);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class UItemSlot : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UItemSlot();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "4"))
	TMap<TSubclassOf<class ASlotableActor>, FSlotableActorVisuals> actorVisuals_Map;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)	UStaticMesh* boxMesh;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)	UStaticMesh* sphereMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)					uint8 editorCollisionShape = 1;	//sphere
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)		FSlotableActorVisuals triggerVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	FSlotableActorVisuals rootVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	FSlotableActorVisuals currentlyDisplayedVisuals;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)	TSubclassOf<class ASlotableActor> currentlyDisplayedSlotableActor;

	UPROPERTY(Replicated)										TEnumAsByte<EItemSlotState> currentState = EItemSlotState::available;
	UPROPERTY(Replicated)										AActor* reservedForActor;
	UPROPERTY(Replicated)										USphereComponent* transformRoot;
	UPROPERTY()	UStaticMeshComponent* visualsComponent;
	UPROPERTY()	UShapeComponent* colliderComponent;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "2"))
	UMaterial* leftHandMaterial;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "3"))
	UMaterial* rightHandMaterial;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "4"))
	UMaterial* editorColliderMaterial;

public:
	/**
	* Editor-time function.
	Toggles current visual's visibility.
	*/
	void E_ToggleVisibility();

	/**
	* Editor-time function.
	* Sets the currently edited visual's visibility.
	*/
	void E_SetVisibility(bool hidden);

	/**
	* Editor-time function.
	* Displays the provided FSlotableActorVisuals.
	* NOTE: This is the EDITOR version of this method. For runtime, use the RUNTIME version of this method.
	@param FSlotableActorVisuals visuals: Visuals to display. Sets mesh and transform.
	*/
	void E_SetPreviewVisuals(const FSlotableActorVisuals visuals);

#if WITH_EDITOR
	/**
	* Editor-time function.
	Iterates through acceptedActors array and loads default visuals from the acceptedActor's static class.
	Saves values to actorVisuals_Map.
	*/
	void E_ReloadVisuals();

	/**
	* Editor-time function.
	Switches current edit mode to target the Root Component.
	*/
	void E_ModifyRootComponent();

	/**
	* Editor-time function.
	* Switches current edit mode to target the Trigger Component.
	*/
	void E_ModifyTriggerComponent();

	/**
	* Editor-time function.
	* Switches current edit mode to target the provided ASlotableActor's visuals.
	@param TSubclassOf<class ASlotableActor> actorToModify_Key: Key to access a FSlotableActorVisuals value in actorVisuals_Map.
	*/
	void E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> actorToModify_Key);


	/**
	* Editor-time function.
	* Resets provided ASlotableActor's transform to the root's transform.
	@param TSubclassOf<class ASlotableActor> actorToReset_Key: Key to access a FSlotableActorVisuals value in actorVisuals_Map.
	*/
	void E_ResetActorMeshToRootTransform(TSubclassOf<class ASlotableActor> actorToReset_Key);

	/**
	* Editor-time function.
	* Resets Trigger Component's transform to the root's transform.
	@param TSubclassOf<class ASlotableActor> actorToReset_Key: Key to access a FSlotableActorVisuals value in actorVisuals_Map.
	*/
	void E_ResetTriggerMeshToRootTransform();

	/**
	* Editor-time function.
	* Sets the trigger shape according to the provided ECollisionShape.
	*/
	void E_SetTriggerShape(const ECollisionShape::Type shapeType);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;

	void SaveEdit();
	void SaveRootTransform();
	void SaveMeshTransform();
	void SaveTriggerTransform();
#endif

	UPROPERTY(BlueprintAssignable, Category = "ItemSlot")
	FOnActorReceivedEvent OnActorReceivedEvent;

	UPROPERTY(BlueprintAssignable, Category = "ItemSlot")
	FOnActorExitEvent OnActorExitEvent;

	UFUNCTION(Server, Reliable)			void ReserveForActor_Server(ASlotableActor* actor, const EControllerHand handSide);
	UFUNCTION(Server, Reliable)			void ReceiveActorInstigator(ASlotableActor* actor);

	bool CheckForCompatibility(const ASlotableActor* actor);
	void RemoveSlotableActor(ASlotableActor* actor);
	const EItemSlotState SlotState() { return currentState; }


	// Function that is called on the server when an actor exits this components's collision.
	UFUNCTION(Server, Reliable)			void ActorOutOfRangeEventInstigation(ASlotableActor* actor);

	//	Event an an ASlotableActor could subscribe to get notified when this slot became occupied.
	FOnOccupiedDelegate OnOccupied;

	//	Event an an ASlotableActor could subscribe to get notified when this slot became available again.
	FOnAvailableDelegate OnAvailable;


	//	Array that hold static subclasses of ASlotableActor that this slot should accept.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Slot editing", meta = (DisplayPriority = "1"))
	TArray<TSubclassOf<class ASlotableActor>> acceptedActors;

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//	Makes a new key entry in actorVisuals_Map
	void addActorToVisualsMap(TSubclassOf<class ASlotableActor> newActor);

	// Removes an entry from actorVisuals_Map.
	void removeActorFromVisualsArray(TSubclassOf<class ASlotableActor> removeActor);

	/**
	* Multicast method that sets up the required components.
	*/
	UFUNCTION(NetMulticast, Reliable)	void setupMulti();

	/**
	 * triggerComponent only exists on the server and so the setup should only run on it.
	 * Collision detection happens on the server only.
	 */
	UFUNCTION(Server, Reliable)
	void setupTriggerComponent();

	/**
	 * visualsComponent needs to be set up locally in order to work.
	 * It is not replicated.
	 */
	UFUNCTION(Client, Reliable)
	void setupVisualsComponent();

	// Multicast function that is called by the server to notify clients.
	UFUNCTION(NetMulticast, Reliable)	void ActorOutOfRangeEventMulti(ASlotableActor* actor);

	// Client function that is called by a multicast event to handle local components when an actor exits this components's collision
	UFUNCTION(Client, Reliable)			void ActorOutOfRangeEvent(ASlotableActor* actor);

	UFUNCTION(NetMulticast, Reliable)	void SetClientVisualsOnReserve(ASlotableActor* actor, const EControllerHand handSide);
	UFUNCTION(Client, Reliable)			void SetVisuals(const FSlotableActorVisuals visualProperties, const EControllerHand handSide);
	UFUNCTION(NetMulticast, Reliable)	void SetVisualsOn_ActorReceive(ASlotableActor* actor);
	UFUNCTION(Client, Reliable)			void ReceiveActor();
};
