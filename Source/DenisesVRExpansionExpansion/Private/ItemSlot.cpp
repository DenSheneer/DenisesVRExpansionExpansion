// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSlot.h"
#include "SlotableActor.h"

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
		gfx.PreviewMaterial = LoadObject<UMaterial>(NULL, TEXT("/Script/Engine.Material'/Game/VRE/Core/Character/BasicShapeMaterialTrans.BasicShapeMaterialTrans'"));

		visualsArray.Add(i, gfx);
	}

	if (visualsArray.Num() > 0)
	{
		SetVisibility(true);
		SetPreviewVisuals(visualsArray[0]);
	}
}

void UItemSlot::SavePreviewPosAndRot()
{
	visualsArray[currentVisualIndex].RelativePosition = GetRelativeLocation();
	visualsArray[currentVisualIndex].RelativeRotation = GetRelativeRotation();
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

void UItemSlot::ReceiveSlotableActor(ASlotableActor* actor)
{
	if (actor != nullptr)
	{
		actorsInRange = 0;
		SetVisibility(false);
		isOccupied = true;
		OnOccupiedEvent.Broadcast(this);
	}
}

void UItemSlot::RemoveSlotableActor(ASlotableActor* actor)
{
	isOccupied = false;
	OnAvailableEvent.Broadcast(this);

	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, TEXT("sent available broadcast"));

}

void UItemSlot::ActorInRangeEvent(ASlotableActor* actor)
{
	int index = acceptedActors.IndexOfByKey(actor->GetClass());
	if (index != INDEX_NONE)
	{
		if (visualsArray.Contains(index))
			SetPreviewVisuals(visualsArray[index]);

		actorsInRange++;
		SetVisibility(true);
	}
}

void UItemSlot::ActorOutOfRangeEvent(ASlotableActor* actor)
{
	actorsInRange--;
	if (actorsInRange < 1)
		SetVisibility(false);
}

// Called every frame
void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}