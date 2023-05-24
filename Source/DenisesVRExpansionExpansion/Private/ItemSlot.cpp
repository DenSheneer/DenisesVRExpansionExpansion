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
	//previewMesh->SetVisibility(false);
	//E_SetVisibility(false);
	//R_SetVisibility(false);
}

void UItemSlot::R_SetPreviewVisuals_Implementation(FSlotableActorVisuals visualProperties)
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
	}
}

void UItemSlot::R_SetVisibility(bool hidden) { previewMesh->SetVisibility(hidden); }

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
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("setVisualsOnReservation"));
	else
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("setVisualsOnReservation"));

	if (visualsArray.Contains(actor->GetClass()))
	{
		currentlyDisplayedVisuals = *visualsArray.Find(actor->GetClass());
		R_SetPreviewVisuals(currentlyDisplayedVisuals);

		switch (handSide)
		{
		case EControllerHand::Left:
			SetMaterial(0, lefthandMaterial);
			break;
		case EControllerHand::Right:
			SetMaterial(0, rightHandMaterial);
			break;
		default:
			break;
		}

	}

	previewMesh->SetVisibility(true);
	//R_SetVisibility(true);
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
		R_SetVisibility(false);
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
		R_SetVisibility(false);
		OnAvailableEvent.Broadcast(this);
	}
}

void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UItemSlot::E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> visualsInAcceptedActors)
{
	//UE_LOG(LogTemp, Warning, TEXT("visuals: '%s'"), *visuals->GetName());
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
	{
		UE_LOG(LogTemp, Warning, TEXT("Visuals not found in visuals array: '%s'"), *visualsInAcceptedActors->GetName());
	}
}

void UItemSlot::SaveEdit()
{
	//UE_LOG(LogTemp, Warning, TEXT("%s"), currentlyDisplayedVisuals);

	if (currentlyDisplayedVisuals.ID == triggerVisuals.ID)
	{
		SaveTriggerTransform();
	}
	else
		SaveMeshTransform();
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
			FSlotableActorVisuals gfx;
			auto obj = acceptedActors[i].GetDefaultObject();
			gfx.ID = acceptedActors[i]->GetName();
			gfx.Mesh = obj->PreviewMesh;
			gfx.Scale = obj->MeshScale;
			gfx.RelativePosition = GetRelativeLocation();
			gfx.RelativeRotation = FRotator::ZeroRotator;

			visualsArray.Add(acceptedActors[i], gfx);
		}
		else
			acceptedActors.RemoveAt(i);
	}
	UPackage* Package = this->GetOutermost();
	if (Package != nullptr)
	{
		Package->SetDirtyFlag(true);
	}

	E_ModifyTriggerShape();

	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);
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

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UItemSlot::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	SaveEdit();

	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// Check if the modified property is the ItemSlots array
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UItemSlot, acceptedActors))
	{
		// Check what specifically changed in the array
		int32 ModifiedIndices = PropertyChangedEvent.GetNumObjectsBeingEdited();
		// The array was modified, check which items were added, removed, or modified
		for (int32 i = 0; i < ModifiedIndices; i++)
		{
			if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
			{
				// An item was added to the array
				auto AddedObject = PropertyChangedEvent.GetObjectBeingEdited(i);
				if (AddedObject)
				{
					UE_LOG(LogTemp, Warning, TEXT("Property was added"));
				}
			}
			else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
			{
				// An item was removed from the array
				auto RemovedObject = PropertyChangedEvent.GetObjectBeingEdited(i);
				if (RemovedObject)
				{
					UE_LOG(LogTemp, Warning, TEXT("Property was removed"));
				}
			}
			else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
			{
				// An item in the array was modified
				auto ModifiedObject = PropertyChangedEvent.GetObjectBeingEdited(i);
				if (ModifiedObject)
				{
					UE_LOG(LogTemp, Warning, TEXT("Property was edited"));
				}
			}
		}
	}
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


