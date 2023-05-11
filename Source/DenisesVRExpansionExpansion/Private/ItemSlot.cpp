#include "ItemSlot.h"
#include "SlotableActor.h"
#include "Components/SphereComponent.h"
#include <Editor.h>
#include "Kismet/KismetMathLibrary.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "ItemSlotDetails.h"

UItemSlot::UItemSlot()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemSlot::BeginPlay()
{
	Super::BeginPlay();
	SetVisibility(false);

	SetPreviewVisuals(triggerMesh);
	trigger = NewObject<UStaticMeshComponent>(GetOwner(), FName(GetName() + "_triggerRoot"));
	trigger->SetStaticMesh(triggerMesh.Mesh);
	trigger->SetMaterial(0, triggerMesh.PreviewMaterial);
	trigger->SetWorldScale3D(triggerMesh.Scale);
	trigger->SetUsingAbsoluteScale(true);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
	AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld;
	trigger->SetRelativeLocationAndRotation(triggerMesh.RelativePosition, triggerMesh.RelativeRotation);

	trigger->AttachToComponent(this, AttachmentRules);
	trigger->RegisterComponent();
	GetOwner()->AddInstanceComponent(trigger);
	trigger->SetVisibility(true);

	this->SetUsingAbsoluteScale(true);
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

void UItemSlot::TogglePreviewVisibility()
{
	if (IsVisible())
		SetVisibility(false);
	else
		SetVisibility(true);

}

/// <summary>
/// Used to set this components visuals properties (mesh, material, world scale and relative transformation) according to the given FSlotableActor struct.
/// </summary>
/// <param name="visualProperties"></param>
void UItemSlot::SetPreviewVisuals(FSlotableActorVisuals visualProperties)
{
	if (trigger != nullptr)
	{
		trigger->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	SetStaticMesh(visualProperties.Mesh);
	SetWorldScale3D(visualProperties.Scale);
	SetRelativeLocation(visualProperties.RelativePosition);
	SetRelativeRotation(visualProperties.RelativeRotation);
	SetMaterial(0, visualProperties.PreviewMaterial);

	if (trigger != nullptr)
	{
		trigger->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	}
}

bool UItemSlot::TryToReceiveActor(ASlotableActor* actor)
{
	if (actor == reservedForActor)
	{
		this->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		actor->SetOwner(GetOwner());
		actor->DisableComponentsSimulatePhysics();
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		AttachmentRules.ScaleRule = EAttachmentRule::KeepRelative;
		actor->AttachToActor(this->GetOwner(), AttachmentRules);
		actor->SetActorRelativeLocation(this->GetRelativeLocation());
		actor->SetActorRelativeRotation(this->GetRelativeRotation());
		SetVisibility(false);
		reservedForActor = nullptr;
		currentState = EItemSlotState::occupied;
		OnOccupiedEvent.Broadcast(this);
		return true;
	}
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("received enter request from an actor that is not the ReservedFor Actor."));

	return false;
}

void UItemSlot::RemoveSlotableActor(ASlotableActor* actor)
{
	currentState = EItemSlotState::available;
	OnAvailableEvent.Broadcast(this);
}

void UItemSlot::reserveSlotForActor(ASlotableActor* actor, EControllerHand handSide)
{
	if (visualsArray.Contains(actor->GetClass()))
	{
		FSlotableActorVisuals visuals = *visualsArray.Find(actor->GetClass());
		SetPreviewVisuals(visuals);

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
	reservedForActor = actor;
	currentState = EItemSlotState::reserved;
	SetVisibility(true);
}

void UItemSlot::ActorOutOfRangeEvent(ASlotableActor* actor)
{
	currentState = EItemSlotState::available;
	reservedForActor = nullptr;
	SetVisibility(false);

	OnAvailableEvent.Broadcast(this);
}

bool UItemSlot::TryToReserve(ASlotableActor* actor, EControllerHand handSide)
{
	if (currentState == EItemSlotState::available)
	{
		int index = acceptedActors.IndexOfByKey(actor->GetClass());


		if (index != INDEX_NONE)
		{
			reserveSlotForActor(actor, handSide);
			return true;
		}
	}
	return false;
}

void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UItemSlot::SetVisualsToAcceptedActor(TSubclassOf<class ASlotableActor> visualsInAcceptedActors)
{
	//UE_LOG(LogTemp, Warning, TEXT("visuals: '%s'"), *visuals->GetName());
	//SaveMeshTransform();
	triggerEditing = false;

	if (visualsArray.Contains(visualsInAcceptedActors))
	{
		currentlyDisplayedVisuals = visualsInAcceptedActors;
		SetPreviewVisuals(visualsArray[currentlyDisplayedVisuals]);

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

void UItemSlot::SaveMeshTransform()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);

	if (triggerEditing)
	{
		triggerMesh.RelativePosition = GetRelativeLocation();
		triggerMesh.RelativeRotation = GetRelativeRotation();
		triggerMesh.Scale = GetRelativeScale3D();

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("Saved trigger mesh."));

		UE_LOG(LogTemp, Log, TEXT("Saved trigger mesh."));
	}
	else if (visualsArray.Contains(currentlyDisplayedVisuals))
	{
		visualsArray[currentlyDisplayedVisuals].RelativePosition = GetRelativeLocation();
		visualsArray[currentlyDisplayedVisuals].RelativeRotation = GetRelativeRotation();
		visualsArray[currentlyDisplayedVisuals].Scale = GetRelativeScale3D();
		UE_LOG(LogTemp, Log, TEXT("Saved slot mesh: '%s'"), *currentlyDisplayedVisuals->GetName());
	}

	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);
}

/// <summary>
/// Fetch the allowed actors' preview visual properties (mesh, material and world scale) and set this slot's properties to match those.
/// WARNING: Resets previously saved positions and rotations.
/// </summary>
void UItemSlot::ReloadVisuals()
{
	int acceptedActorsNr = acceptedActors.Num();
	visualsArray.Empty();
	currentlyDisplayedVisuals = nullptr;

	for (int i = 0; i < acceptedActorsNr; i++)
	{
		if (acceptedActors[i]->IsValidLowLevel())
		{
			FSlotableActorVisuals gfx;
			auto obj = acceptedActors[i].GetDefaultObject();
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

	EditTriggerShape();

	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
	GEditor->RedrawLevelEditingViewports(true);
	GEditor->RedrawAllViewports(true);
}

void UItemSlot::EditTriggerShape()
{
	//SaveMeshTransform();
	triggerEditing = true;

	currentlyDisplayedVisuals = nullptr;
	SetVisibility(true);
	SetPreviewVisuals(triggerMesh);

	GEditor->SelectNone(false, true, false);
	GEditor->SelectComponent(this, true, true, true);
	GEditor->RedrawAllViewports(true);
}

void UItemSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SaveMeshTransform();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UItemSlot::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
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
		SaveMeshTransform();

	Super::PostEditComponentMove(bFinished);
}


