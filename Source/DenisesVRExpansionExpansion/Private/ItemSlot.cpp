#include "ItemSlot.h"
#include "SlotableActor.h"
#include "Components/SphereComponent.h"
#include <Editor.h>
#include "Kismet/KismetMathLibrary.h"
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
	UStaticMeshComponent* trigger = NewObject<UStaticMeshComponent>(GetOwner(), FName(GetName() + "_trigger"));
	trigger->SetStaticMesh(triggerMesh.Mesh);
	trigger->SetMaterial(0, triggerMesh.PreviewMaterial);
	trigger->SetWorldScale3D(triggerMesh.Scale);
	trigger->SetWorldLocation(triggerMesh.RelativePosition);
	trigger->SetRelativeRotation(triggerMesh.RelativeRotation);
	trigger->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	trigger->RegisterComponent();
	GetOwner()->AddInstanceComponent(trigger);
	trigger->SetVisibility(false);
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
	SetStaticMesh(visualProperties.Mesh);
	SetWorldScale3D(visualProperties.Scale);
	SetRelativeLocation(visualProperties.RelativePosition);
	SetRelativeRotation(visualProperties.RelativeRotation);
	SetMaterial(0, visualProperties.PreviewMaterial);
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

void UItemSlot::CycleThroughPreviews(TSubclassOf<class ASlotableActor> visuals)
{
	//UE_LOG(LogTemp, Warning, TEXT("visuals: '%s'"), *visuals->GetName());

	if (visualsArray.Contains(visuals))
	{
		SaveMeshTransform();
		currentlyDisplayedVisuals = visuals;
		SetPreviewVisuals(visualsArray[currentlyDisplayedVisuals]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Visuals not found in visuals array: '%s'"), *visuals->GetName());
	}
}

void UItemSlot::SaveMeshTransform()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor(150, 150, 150), FString::Printf(TEXT("saving: %s"), currentlyDisplayedVisuals.GetDefaultObject()->GetName()));

	if (currentlyDisplayedVisuals == nullptr)	// not fully safe: might modify trigger shape when currentlyDisplayedVisuals is actually invalid.
	{
		triggerMesh.RelativePosition = GetRelativeLocation();
		triggerMesh.RelativeRotation = GetRelativeRotation();
		triggerMesh.Scale = GetRelativeScale3D();
	}
	else if (visualsArray.Contains(currentlyDisplayedVisuals))
	{
		visualsArray[currentlyDisplayedVisuals].RelativePosition = GetRelativeLocation();
		visualsArray[currentlyDisplayedVisuals].RelativeRotation = GetRelativeRotation();
		visualsArray[currentlyDisplayedVisuals].Scale = GetRelativeScale3D();
	}
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

}

void UItemSlot::EditTriggerShape()
{
	SaveMeshTransform();

	currentlyDisplayedVisuals = nullptr;
	SetVisibility(true);
	SetPreviewVisuals(triggerMesh);
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


