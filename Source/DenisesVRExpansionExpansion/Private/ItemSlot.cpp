// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSlot.h"
#include "SlotableActor.h"
#include "Components/SphereComponent.h"
#include <Editor.h>
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UItemSlot::UItemSlot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UItemSlot::BeginPlay()
{
	Super::BeginPlay();
	SetVisibility(false);

	USphereComponent* trigger = NewObject<USphereComponent>(GetOwner(), FName(GetName() + "_trigger"));
	trigger->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	trigger->RegisterComponent();
	GetOwner()->AddInstanceComponent(trigger);
	trigger->SetHiddenInGame(false);
	trigger->SetVisibility(true);
	trigger->SetSphereRadius(10.0f);
}

/// <summary>
/// Fetch the allowed actors' preview visual properties (mesh, material and world scale) and set this slot's properties to match those.
/// WARNING: Resets previously saved positions and rotations.
/// </summary>
void UItemSlot::ReloadVisuals()
{
	int acceptedActorsNr = acceptedActors.Num();
	visualsArray.Empty();
	currentVisualIndex = 0;

	for (int i = 0; i < acceptedActorsNr; i++)
	{
		FSlotableActorVisuals gfx;
		auto obj = acceptedActors[i].GetDefaultObject();
		gfx.Mesh = obj->PreviewMesh;
		gfx.Scale = obj->MeshScale;
		gfx.RelativePosition = GetRelativeLocation();
		gfx.RelativeRotation = GetRelativeRotation();

		visualsArray.Add(i, gfx);
	}

	if (visualsArray.Num() > 0)
	{
		SetVisibility(true);
		SetPreviewVisuals(visualsArray[0]);
	}
}

void UItemSlot::EditTriggerShape()
{
	currentVisualIndex = -1;
	SetVisibility(true);
	SetPreviewVisuals(triggerMesh);
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

void UItemSlot::SavePreviewPosAndRot()
{
	if (currentVisualIndex == -1)
	{
		triggerMesh.RelativePosition = GetRelativeLocation();
		triggerMesh.RelativeRotation = GetRelativeRotation();
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor(150, 150, 150), TEXT("saved trigger mesh"));
	}
	else if (visualsArray.Contains(currentVisualIndex))
	{
		visualsArray[currentVisualIndex].RelativePosition = GetRelativeLocation();
		visualsArray[currentVisualIndex].RelativeRotation = GetRelativeRotation();
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor(150, 150, 150), TEXT("saved preview mesh."));
	}

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

void UItemSlot::CycleThroughPreviews()
{
	int maxIndex = visualsArray.Num();
	if (maxIndex > 0)
	{
		if (currentVisualIndex + 1 < maxIndex)
			currentVisualIndex++;
		else
			currentVisualIndex = 0;

		SetPreviewVisuals(visualsArray[currentVisualIndex]);
	}
}

bool UItemSlot::TryToReceiveActor(ASlotableActor* actor)
{
	if (actor == reservedForActor)
	{
		actor->DisableComponentsSimulatePhysics();
		actor->AttachToActor(this->GetOwner(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
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
	int index = acceptedActors.IndexOfByKey(actor->GetClass());
	if (index != INDEX_NONE)
	{
		if (visualsArray.Contains(index))
		{
			SetPreviewVisuals(visualsArray[index]);

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

// Called every frame
void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
void UItemSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SavePreviewPosAndRot();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UItemSlot::PostEditComponentMove(bool bFinished)
{
	if (bFinished)
		SavePreviewPosAndRot();

	Super::PostEditComponentMove(bFinished);
}


