#include "ItemSlot.h"
#include "SlotableActor.h"
#include "Components/SphereComponent.h"
#include <Editor.h>
#include "Kismet/KismetMathLibrary.h"
#include "ItemSlotTrigger.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "Net/UnrealNetwork.h"
#include "ItemSlotDetails.h"

UItemSlot::UItemSlot()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	triggerVisuals.ID = "trigger";

	rootVisuals.ID = "root";
	FSoftObjectPath AssetRef(TEXT("/Engine/Content/ArtTools/RenderToTexture/Meshes/S_1_Unit_Sphere.uasset"));
	rootVisuals.Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *AssetRef.ToString()));
}

void UItemSlot::BeginPlay()
{
	Super::BeginPlay();

	//if (this->GetOwner()->HasAuthority())
	server_Setup();
}

void UItemSlot::server_Setup_Implementation()
{
	E_SetPreviewVisuals(triggerVisuals);

	setupMeshShapeComponent();
	setupTriggerComponent();


	this->SetVisibility(false);
}

void UItemSlot::R_SetPreviewVisuals_Implementation(FSlotableActorVisuals visualProperties, EControllerHand handSide)
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, FString::Printf(TEXT("%s"), *visualProperties.RelativePosition.ToString()));

	if (previewMesh)
	{
		previewMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		previewMesh->AttachToComponent(this->GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		previewMesh->SetStaticMesh(visualProperties.Mesh);
		previewMesh->SetWorldScale3D(visualProperties.Scale);
		previewMesh->SetRelativeLocation(visualProperties.RelativePosition);
		previewMesh->SetRelativeRotation(visualProperties.RelativeRotation);
		previewMesh->SetMaterial(0, visualProperties.PreviewMaterial);
		previewMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		previewMesh->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

		switch (handSide)
		{
		case EControllerHand::Left:
			previewMesh->SetMaterial(0, lefthandMaterial);
			break;
		case EControllerHand::Right:
			previewMesh->SetMaterial(0, rightHandMaterial);
			break;
		case EControllerHand::AnyHand:
			return;
			break;
		}
	}
}

bool UItemSlot::CheckForCompatibility(ASlotableActor* actor)
{
	int index = acceptedActors.IndexOfByKey(actor->GetClass());
	if (index != INDEX_NONE)
	{
		return true;
	}
	return false;
}

void UItemSlot::E_TogglePreviewVisibility()
{
	if (IsVisible())
		E_SetVisibility(false);
	else
		E_SetVisibility(true);

}

/// <summary>
/// Used to set this components visuals properties (mesh, material, world scale and relative transformation) according to the given FSlotableActor struct.
/// </summary>
/// <param name="visualProperties"></param>
void UItemSlot::E_SetPreviewVisuals(FSlotableActorVisuals visualProperties)
{
	currentlyDisplayedVisuals = visualProperties;

	SetStaticMesh(visualProperties.Mesh);
	SetWorldScale3D(visualProperties.Scale);
	SetRelativeLocation(visualProperties.RelativePosition);
	SetRelativeRotation(visualProperties.RelativeRotation);
	SetMaterial(0, visualProperties.PreviewMaterial);
}

void UItemSlot::E_SetVisibility(bool hidden) { this->SetVisibility(hidden); }

void UItemSlot::E_ResetActorMeshToRootTransform(TSubclassOf<class ASlotableActor> visuals)
{
	if (visualsArray.Contains(visuals))
	{
		if (currentlyDisplayedSlotableActor == visuals)
		{
			SetRelativeLocationAndRotation(rootVisuals.RelativePosition, rootVisuals.RelativeRotation);

			FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
			GEditor->RedrawLevelEditingViewports(true);

			GEditor->SelectNone(false, true, false);
			GEditor->SelectComponent(this, true, true, true);
			GEditor->RedrawAllViewports(true);
		}

		auto visualsToReset = visualsArray[visuals];
		visualsToReset.RelativePosition = rootVisuals.RelativePosition;
		visualsToReset.RelativeRotation = rootVisuals.RelativeRotation;
	}
}

void UItemSlot::E_ResetTriggerMeshToRootTransform()
{
	if (currentlyDisplayedVisuals.ID == triggerVisuals.ID)
	{
		SetRelativeLocationAndRotation(rootVisuals.RelativePosition, rootVisuals.RelativeRotation);

		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);

		GEditor->SelectNone(false, true, false);
		GEditor->SelectComponent(this, true, true, true);
		GEditor->RedrawAllViewports(true);
	}
	triggerVisuals.RelativePosition = rootVisuals.RelativePosition;
	triggerVisuals.RelativeRotation = rootVisuals.RelativeRotation;
}

void UItemSlot::ReserveForActor_Implementation(ASlotableActor* actor, EControllerHand handSide)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("ReserveForActor"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("ReserveForActor"));

	if (currentState == EItemSlotState::available)
	{
		int index = acceptedActors.IndexOfByKey(actor->GetClass());
		if (index != INDEX_NONE)
		{
			setVisualsOnReservation(actor, handSide);
		}
		reservedForActor = actor;
		currentState = EItemSlotState::reserved;
	}
}
void UItemSlot::setVisualsOnReservation_Implementation(ASlotableActor* actor, EControllerHand handSide)
{
	if (visualsArray.Contains(actor->GetClass()))
	{
		currentlyDisplayedVisuals = *visualsArray.Find(actor->GetClass());
		R_SetPreviewVisuals(currentlyDisplayedVisuals, handSide);
	}
	previewMesh->SetVisibility(true);
}

void UItemSlot::ReceiveActor_Implementation(ASlotableActor* actor)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Receive"));

	if (actor == reservedForActor)
	{
		reservedForActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		reservedForActor->SetOwner(GetOwner());
		reservedForActor->DisableComponentsSimulatePhysics();
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		AttachmentRules.ScaleRule = EAttachmentRule::KeepRelative;
		reservedForActor->AttachToComponent(previewMesh, AttachmentRules);
		previewMesh->SetVisibility(false);
		reservedForActor = nullptr;
		currentState = EItemSlotState::occupied;
		OnOccupiedEvent.Broadcast(this);
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("received enter request from an actor that is not the ReservedFor Actor."));
}

void UItemSlot::RemoveSlotableActor(ASlotableActor* actor)
{
	actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	currentState = EItemSlotState::available;
	OnAvailableEvent.Broadcast(this);
}

void UItemSlot::setupTriggerComponent()
{
	trigger = NewObject<UItemSlotTrigger>(GetOwner(), FName(GetName() + "_triggerRoot"));
	trigger->SetIsReplicated(true);
	trigger->SetUp(this, triggerVisuals);
	trigger->SetVisibility(false);
}

void UItemSlot::setupMeshShapeComponent()
{
	previewMesh = NewObject<UStaticMeshComponent>(GetOwner(), FName(GetName() + "_previewMesh"));
	previewMesh->SetIsReplicated(true);
	previewMesh->SetUsingAbsoluteScale(true);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
	previewMesh->AttachToComponent(this, AttachmentRules);
	previewMesh->RegisterComponent();
	GetOwner()->AddInstanceComponent(previewMesh);

	previewMesh->SetVisibility(false);
}

void UItemSlot::ActorOutOfRangeEvent(ASlotableActor* actor)
{
	if (actor == reservedForActor)
	{
		currentState = EItemSlotState::available;
		reservedForActor = nullptr;
		previewMesh->SetVisibility(false);
		OnAvailableEvent.Broadcast(this);
	}
}

void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UItemSlot::E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> visualsInAcceptedActors)
{
	SaveEdit();

	if (visualsArray.Contains(visualsInAcceptedActors))
	{
		currentlyDisplayedSlotableActor = visualsInAcceptedActors;
		E_SetPreviewVisuals(visualsArray[currentlyDisplayedSlotableActor]);

		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);

		GEditor->SelectNone(false, true, false);
		GEditor->SelectComponent(this, true, true, true);
		GEditor->RedrawAllViewports(true);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Visuals not found in visuals array: '%s'"), *visualsInAcceptedActors->GetName());
}

void UItemSlot::SaveEdit()
{
	//UE_LOG(LogTemp, Warning, TEXT("%s"), currentlyDisplayedVisuals);
	if (currentlyDisplayedVisuals.ID == rootVisuals.ID)
		SaveRootTransform();
	else if (currentlyDisplayedVisuals.ID == triggerVisuals.ID)
		SaveTriggerTransform();
	else
		SaveMeshTransform();
}

void UItemSlot::SaveRootTransform()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);

	rootVisuals.RelativePosition = GetRelativeLocation();
	rootVisuals.RelativeRotation = GetRelativeRotation();
	rootVisuals.Scale = GetRelativeScale3D();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("Saved root."));

	UE_LOG(LogTemp, Log, TEXT("Saved root."));
}

void UItemSlot::SaveMeshTransform()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);
	if (visualsArray.Contains(currentlyDisplayedSlotableActor))
	{
		visualsArray[currentlyDisplayedSlotableActor].RelativePosition = GetRelativeLocation();
		visualsArray[currentlyDisplayedSlotableActor].RelativeRotation = GetRelativeRotation();
		visualsArray[currentlyDisplayedSlotableActor].Scale = GetRelativeScale3D();
		UE_LOG(LogTemp, Log, TEXT("Saved slot mesh: '%s'"), *currentlyDisplayedSlotableActor->GetName());
	}

	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);
}

void UItemSlot::SaveTriggerTransform()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);

	triggerVisuals.RelativePosition = GetRelativeLocation();
	triggerVisuals.RelativeRotation = GetRelativeRotation();
	triggerVisuals.Scale = GetRelativeScale3D();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("Saved trigger mesh."));

	UE_LOG(LogTemp, Log, TEXT("Saved trigger mesh."));
}

/// <summary>
/// Fetch the allowed actors' preview visual properties (mesh, material and world scale) and set this slot's properties to match those.
/// WARNING: Resets previously saved positions and rotations.
/// </summary>
void UItemSlot::E_ReloadVisuals()
{
	int acceptedActorsNr = acceptedActors.Num();
	visualsArray.Empty();
	currentlyDisplayedSlotableActor = nullptr;

	for (int i = 0; i < acceptedActorsNr; i++)
	{
		if (acceptedActors[i]->IsValidLowLevel())
		{
			addActorToVisualArray(acceptedActors[i]);
		}
		else
			acceptedActors.RemoveAt(i);
	}

	E_ModifyRootComponent();
}

void UItemSlot::E_ModifyRootComponent()
{
	SaveEdit();

	currentlyDisplayedSlotableActor = nullptr;
	E_SetVisibility(true);
	E_SetPreviewVisuals(rootVisuals);

	GEditor->SelectNone(false, true, false);
	GEditor->SelectComponent(this, true, true, true);
	GEditor->RedrawAllViewports(true);
}

void UItemSlot::E_ModifyTriggerShape()
{
	SaveEdit();

	currentlyDisplayedSlotableActor = nullptr;
	E_SetVisibility(true);
	E_SetPreviewVisuals(triggerVisuals);

	GEditor->SelectNone(false, true, false);
	GEditor->SelectComponent(this, true, true, true);
	GEditor->RedrawAllViewports(true);
}

void UItemSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SaveEdit();

	// Check if the modified property is the ItemSlots array
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UItemSlot, acceptedActors))
	{
		int32 ModifiedIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
		if (!acceptedActors.IsValidIndex(ModifiedIndex))
		{
			TArray<TSubclassOf<ASlotableActor>> keys;
			visualsArray.GetKeys(keys);

			for (int i = 0; i < keys.Num(); i++)
			{
				if (acceptedActors.Contains(keys[i])) { continue; }

				if (currentlyDisplayedSlotableActor == keys[i])
					E_ModifyRootComponent();

				if (visualsArray.Contains(keys[i]))
					visualsArray.Remove(keys[i]);
			}
		}
		else
		{
			TSubclassOf<ASlotableActor>& ModifiedActor = acceptedActors[ModifiedIndex];
			if (!ModifiedActor) { return; }

			if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
			{
				removeActorFromVisualsArray(ModifiedActor);
				UE_LOG(LogTemp, Warning, TEXT("array index removed"));
			}
			else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
				addActorToVisualArray(acceptedActors[ModifiedIndex]);

		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UItemSlot::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	SaveEdit();
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void UItemSlot::PostEditComponentMove(bool bFinished)
{
	if (bFinished)
		SaveEdit();

	Super::PostEditComponentMove(bFinished);
}

void UItemSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemSlot, triggerVisuals);
	DOREPLIFETIME(UItemSlot, currentState);
	DOREPLIFETIME(UItemSlot, reservedForActor);
	DOREPLIFETIME(UItemSlot, currentlyDisplayedVisuals);
	DOREPLIFETIME(UItemSlot, previewMesh);
	DOREPLIFETIME(UItemSlot, trigger);
}

void UItemSlot::addActorToVisualArray(TSubclassOf<class ASlotableActor> newActor)
{
	FSlotableActorVisuals gfx;
	auto obj = newActor.GetDefaultObject();
	gfx.ID = newActor->GetName();
	gfx.Mesh = obj->PreviewMesh;
	gfx.Scale = obj->MeshScale;
	gfx.RelativePosition = rootVisuals.RelativePosition;
	gfx.RelativeRotation = rootVisuals.RelativeRotation;

	if (!visualsArray.Contains(newActor))
	{
		visualsArray.Add(newActor, gfx);
	}
	else
	{
		visualsArray[newActor] = gfx;
	}

	E_ModifyAcceptedActorMesh(newActor);
}

void UItemSlot::removeActorFromVisualsArray(TSubclassOf<class ASlotableActor> removeActor)
{
	if (visualsArray.Contains(removeActor))
	{
		if (currentlyDisplayedVisuals.ID == visualsArray[removeActor].ID)
			E_ModifyRootComponent();

		visualsArray.Remove(removeActor);
	}
}


