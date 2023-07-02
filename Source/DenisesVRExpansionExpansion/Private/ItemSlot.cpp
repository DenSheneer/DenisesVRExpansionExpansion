#include "ItemSlot.h"
#include "SlotableActor.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include <Editor.h>
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Editor/UnrealEd/Public/Editor.h"
#include "Net/UnrealNetwork.h"
#include "ItemSlotDetails.h"

UItemSlot::UItemSlot()
{
	this->SetCastShadow(false);
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;

	triggerVisuals.ID = "trigger";
	E_SetTriggerShape(ECollisionShape::Sphere);

	rootVisuals.ID = "root";
	E_ModifyRootComponent();
}

void UItemSlot::BeginPlay()
{
	Super::BeginPlay();
	setupMulti();
}

void UItemSlot::setupMulti_Implementation()
{
	setupVisualsComponent();

	if (GetOwner()->HasAuthority())
		setupTriggerComponent();

	this->SetVisibility(false);
}

bool UItemSlot::CheckForCompatibility(const ASlotableActor* actor)
{
	int index = acceptedActors.IndexOfByKey(actor->GetClass());
	if (index != INDEX_NONE)
		return true;
	else
		return false;
}



void UItemSlot::E_ToggleVisibility()
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
void UItemSlot::E_SetPreviewVisuals(const FSlotableActorVisuals visuals)
{
	currentlyDisplayedVisuals = visuals;

	if (visuals.ID.Equals(triggerVisuals.ID) || visuals.ID.Equals(rootVisuals.ID))
		SetMaterial(0, editorColliderMaterial);
	else
		SetMaterial(0, nullptr);

	SetStaticMesh(visuals.Mesh);
	SetWorldScale3D(visuals.Scale);
	SetRelativeLocation(visuals.RelativePosition);
	SetRelativeRotation(visuals.RelativeRotation);
}

void UItemSlot::E_SetVisibility(bool hidden) { this->SetVisibility(hidden); }

void UItemSlot::E_ResetActorMeshToRootTransform(TSubclassOf<class ASlotableActor> actorToReset_Key)
{
	if (actorVisuals_Map.Contains(actorToReset_Key))
	{
		if (currentlyDisplayedSlotableActor == actorToReset_Key)
		{
			SetRelativeLocationAndRotation(rootVisuals.RelativePosition, rootVisuals.RelativeRotation);

			FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

			if (GEditor)
			{
				GEditor->SelectNone(false, true, false);
				GEditor->SelectComponent(this, true, true, true);
				GEditor->RedrawAllViewports(true);
			}
		}

		auto visualsToReset = actorVisuals_Map[actorToReset_Key];
		visualsToReset.RelativePosition = rootVisuals.RelativePosition;
		visualsToReset.RelativeRotation = rootVisuals.RelativeRotation;
	}
}

void UItemSlot::E_ResetTriggerMeshToRootTransform()
{
	if (currentlyDisplayedVisuals.ID.Equals(triggerVisuals.ID))
	{
		SetRelativeLocationAndRotation(rootVisuals.RelativePosition, rootVisuals.RelativeRotation);

		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		if (GEditor)
		{
			GEditor->SelectNone(false, true, false);
			GEditor->SelectComponent(this, true, true, true);
			GEditor->RedrawAllViewports(true);
		}
	}
	triggerVisuals.RelativePosition = rootVisuals.RelativePosition;
	triggerVisuals.RelativeRotation = rootVisuals.RelativeRotation;
}

void UItemSlot::E_SetTriggerShape(const ECollisionShape::Type shapeType)
{
	switch (shapeType)
	{
	case ECollisionShape::Sphere:
		editorCollisionShape = 1;
		triggerVisuals.Mesh = sphereMesh;
		triggerVisuals.Scale = FVector(1.0f, 1.0f, 1.0f);
		break;
	case ECollisionShape::Box:
		editorCollisionShape = 2;
		triggerVisuals.Mesh = boxMesh;
		triggerVisuals.Scale = FVector(1.0f, 1.0f, 1.0f);
		break;
	}
	E_ModifyTriggerComponent();
}

void UItemSlot::ReserveForActor_Server_Implementation(ASlotableActor* actor, const EControllerHand handSide)
{
	if (currentState == EItemSlotState::available)
	{
		int index = acceptedActors.IndexOfByKey(actor->GetClass());
		if (index != INDEX_NONE)
		{
			SetClientVisualsOnReserve(actor, handSide);
		}
		reservedForActor = actor;
		currentState = EItemSlotState::reserved;
		OnOccupied.ExecuteIfBound(this);
	}
}
void UItemSlot::SetClientVisualsOnReserve_Implementation(ASlotableActor* actor, const EControllerHand handSide)
{
	if (actorVisuals_Map.Contains(actor->GetClass()))
	{
		currentlyDisplayedVisuals = *actorVisuals_Map.Find(actor->GetClass());
		SetVisuals(currentlyDisplayedVisuals, handSide);
	}
}

void UItemSlot::SetVisuals_Implementation(const FSlotableActorVisuals visualProperties, const EControllerHand handSide)
{
	if (visualsComponent)
	{
		FVector newPosition = GetAttachmentRoot()->GetComponentTransform().TransformPosition(visualProperties.RelativePosition);
		auto newRotation = GetAttachmentRoot()->GetComponentTransform().TransformRotation(FQuat(visualProperties.RelativeRotation));
		visualsComponent->SetWorldLocation(newPosition);
		visualsComponent->SetWorldRotation(newRotation);
		visualsComponent->SetWorldScale3D(visualProperties.Scale);
		visualsComponent->SetStaticMesh(visualProperties.Mesh);

		switch (handSide)
		{
		case EControllerHand::Left:
			visualsComponent->SetMaterial(0, leftHandMaterial);
			break;
		case EControllerHand::Right:
			visualsComponent->SetMaterial(0, rightHandMaterial);
			break;
		}

		visualsComponent->SetVisibility(true);
	}
}

void UItemSlot::ReceiveActorInstigator_Implementation(ASlotableActor* actor)
{
	if (actor == reservedForActor)
	{
		actor->DisableComponentsSimulatePhysics();
		actor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		auto colComp = actor->GetRootComponent();
		auto castToMesh = Cast<UStaticMeshComponent>(colComp);
		castToMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		actor->SetActorRelativeLocation(visualsComponent->GetRelativeLocation());
		actor->SetActorRelativeRotation(visualsComponent->GetRelativeRotation());

		reservedForActor = nullptr;
		currentState = EItemSlotState::occupied;

		SetVisualsOn_ActorReceive(actor);


	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor(150, 150, 150), TEXT("received enter request from an actor that is not the ReservedFor Actor."));
}
void UItemSlot::SetVisualsOn_ActorReceive_Implementation(ASlotableActor* actor)
{
	actor->DisableComponentsSimulatePhysics();
	actor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	auto colComp = actor->GetRootComponent();
	auto castToMesh = Cast<UStaticMeshComponent>(colComp);
	castToMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	actor->SetActorRelativeLocation(visualsComponent->GetRelativeLocation());
	actor->SetActorRelativeRotation(visualsComponent->GetRelativeRotation());

	reservedForActor = nullptr;
	currentState = EItemSlotState::occupied;
	//OnOccupiedEvent.Broadcast(this);

	ReceiveActor();
}
void UItemSlot::ReceiveActor_Implementation()
{
	visualsComponent->SetVisibility(false);
}

void UItemSlot::RemoveSlotableActor(ASlotableActor* actor)
{
	actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	currentState = EItemSlotState::available;
	OnAvailable.ExecuteIfBound(this);
	//OnAvailableEvent.Broadcast(this);
}

void UItemSlot::setupTriggerComponent_Implementation()
{
	AActor* Owner = GetOwner();
	USphereComponent* triggerAsSphere;
	UBoxComponent* triggerAsBox;

	switch (editorCollisionShape)
	{
	case 1:
		colliderComponent = Cast<UShapeComponent>(Owner->AddComponentByClass(
			USphereComponent::StaticClass(),
			true,
			FTransform::Identity,
			false));
		colliderComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
		triggerAsSphere = Cast<USphereComponent>(colliderComponent);
		triggerAsSphere->SetSphereRadius(50.0f);
		break;
	case 2:
		colliderComponent = NewObject<UBoxComponent>(GetOwner(), FName(GetName() + "_triggerComponent"));
		triggerAsBox = Cast<UBoxComponent>(colliderComponent);
		triggerAsBox->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
		break;
	default:
		break;
	}

	if (colliderComponent)
	{
		GetOwner()->AddInstanceComponent(colliderComponent);

		FVector newPosition = GetAttachmentRoot()->GetComponentTransform().TransformPosition(triggerVisuals.RelativePosition);
		auto newRotation = GetAttachmentRoot()->GetComponentTransform().TransformRotation(FQuat(triggerVisuals.RelativeRotation));
		colliderComponent->SetWorldLocation(newPosition);
		colliderComponent->SetWorldRotation(newRotation);

		colliderComponent->SetCollisionProfileName("Trigger", true);
		colliderComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);

		colliderComponent->bHiddenInGame = false;
		colliderComponent->SetUsingAbsoluteScale(true);
		colliderComponent->SetWorldScale3D(triggerVisuals.Scale);
		colliderComponent->SetVisibility(false);
	}
}

void UItemSlot::setupVisualsComponent_Implementation()
{
	AActor* Owner = GetOwner();
	visualsComponent = Cast<UStaticMeshComponent>(Owner->AddComponentByClass(
		UStaticMeshComponent::StaticClass(),
		true,
		FTransform::Identity,
		false));

	visualsComponent->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	GetOwner()->AddInstanceComponent(visualsComponent);

	visualsComponent->SetGenerateOverlapEvents(false);
	visualsComponent->SetSimulatePhysics(false);
	visualsComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	visualsComponent->SetUsingAbsoluteScale(true);
	visualsComponent->SetCastShadow(false);
	visualsComponent->SetVisibility(false);
}

void UItemSlot::ActorOutOfRangeEventInstigation_Implementation(ASlotableActor* actor)
{
	ActorOutOfRangeEventMulti(actor);
}
void UItemSlot::ActorOutOfRangeEventMulti_Implementation(ASlotableActor* actor)
{
	if (actor == reservedForActor)
	{
		currentState = EItemSlotState::available;
		reservedForActor = nullptr;
		ActorOutOfRangeEvent(actor);
		OnAvailable.ExecuteIfBound(this);

	}
}
void UItemSlot::ActorOutOfRangeEvent_Implementation(ASlotableActor* actor)
{
	visualsComponent->SetVisibility(false);
}

void UItemSlot::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UItemSlot::E_ModifyAcceptedActorMesh(TSubclassOf<class ASlotableActor> actorToModify_Key)
{
	SaveEdit();

	if (actorVisuals_Map.Contains(actorToModify_Key))
	{
		currentlyDisplayedSlotableActor = actorToModify_Key;
		E_SetPreviewVisuals(actorVisuals_Map[currentlyDisplayedSlotableActor]);

		if (IsSelectedInEditor())
		{
			GEditor->SelectNone(false, true, false);
			GEditor->SelectComponent(this, true, true, true);
			GEditor->RedrawAllViewports(true);
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Visuals not found in visuals array: '%s'"), *actorToModify_Key->GetName());
}

void UItemSlot::SaveEdit()
{
	if (currentlyDisplayedVisuals.ID.Equals(rootVisuals.ID))
		SaveRootTransform();
	else if (currentlyDisplayedVisuals.ID.Equals(triggerVisuals.ID))
		SaveTriggerTransform();
	else
		SaveMeshTransform();
}

void UItemSlot::SaveRootTransform()
{
	if (GEditor)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);
	}

	rootVisuals.RelativePosition = GetRelativeLocation();
	rootVisuals.RelativeRotation = GetRelativeRotation();
	rootVisuals.Scale = GetComponentScale();

	UE_LOG(LogTemp, Log, TEXT("Saved root."));
}

void UItemSlot::SaveMeshTransform()
{
	if (GEditor)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);
	}

	if (actorVisuals_Map.Contains(currentlyDisplayedSlotableActor))
	{
		actorVisuals_Map[currentlyDisplayedSlotableActor].RelativePosition = GetRelativeLocation();
		actorVisuals_Map[currentlyDisplayedSlotableActor].RelativeRotation = GetRelativeRotation();
		actorVisuals_Map[currentlyDisplayedSlotableActor].Scale = GetComponentScale();
		UE_LOG(LogTemp, Log, TEXT("Saved slot mesh: '%s'"), *currentlyDisplayedSlotableActor->GetName());
	}

	if (GEditor)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);
	}
}

void UItemSlot::SaveTriggerTransform()
{
	if (GEditor)
	{
		FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();
		GEditor->RedrawLevelEditingViewports(true);
	}

	triggerVisuals.RelativePosition = GetRelativeLocation();
	triggerVisuals.RelativeRotation = GetRelativeRotation();
	triggerVisuals.Scale = GetComponentScale();


	UE_LOG(LogTemp, Log, TEXT("Saved trigger mesh."));
}

/// <summary>
/// </summary>
void UItemSlot::E_ReloadVisuals()
{
	int acceptedActorsNr = acceptedActors.Num();
	actorVisuals_Map.Empty();
	currentlyDisplayedSlotableActor = nullptr;

	for (int i = 0; i < acceptedActorsNr; i++)
	{
		if (acceptedActors[i]->IsValidLowLevel())
		{
			addActorToVisualsMap(acceptedActors[i]);
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

	if (IsSelectedInEditor())
	{
		GEditor->SelectNone(false, true, false);
		GEditor->SelectComponent(this, true, true, true);
		GEditor->RedrawAllViewports(true);
	}
}

void UItemSlot::E_ModifyTriggerComponent()
{
	SaveEdit();

	currentlyDisplayedSlotableActor = nullptr;
	E_SetVisibility(true);
	E_SetPreviewVisuals(triggerVisuals);

	if (IsSelectedInEditor())
	{
		GEditor->SelectNone(false, true, false);
		GEditor->SelectComponent(this, true, true, true);
		GEditor->RedrawAllViewports(true);
	}
}

void UItemSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SaveEdit();
	if (currentlyDisplayedVisuals.ID.Equals(rootVisuals.ID))
		E_ModifyRootComponent();


	// Check if the modified property is the ItemSlots array
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UItemSlot, acceptedActors))
	{
		int32 ModifiedIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
		if (!acceptedActors.IsValidIndex(ModifiedIndex))
		{
			TArray<TSubclassOf<ASlotableActor>> keys;
			actorVisuals_Map.GetKeys(keys);

			for (int i = 0; i < keys.Num(); i++)
			{
				if (acceptedActors.Contains(keys[i])) { continue; }

				if (currentlyDisplayedSlotableActor == keys[i])
					E_ModifyRootComponent();

				if (actorVisuals_Map.Contains(keys[i]))
					actorVisuals_Map.Remove(keys[i]);
			}
		}
		else
		{
			TSubclassOf<ASlotableActor>& ModifiedActor = acceptedActors[ModifiedIndex];
			if (!ModifiedActor) { return; }

			if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove)
			{
				removeActorFromVisualsArray(ModifiedActor);
			}
			else if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
				addActorToVisualsMap(acceptedActors[ModifiedIndex]);

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
}

void UItemSlot::addActorToVisualsMap(TSubclassOf<class ASlotableActor> newActor)
{
	FSlotableActorVisuals gfx;
	auto obj = newActor.GetDefaultObject();
	gfx.ID = newActor->GetName();
	gfx.Mesh = obj->PreviewMesh;
	gfx.Scale = obj->MeshScale;
	gfx.RelativePosition = rootVisuals.RelativePosition;
	gfx.RelativeRotation = rootVisuals.RelativeRotation;

	if (!actorVisuals_Map.Contains(newActor))
		actorVisuals_Map.Add(newActor, gfx);
	else
		actorVisuals_Map[newActor] = gfx;
}

void UItemSlot::removeActorFromVisualsArray(TSubclassOf<class ASlotableActor> removeActor)
{
	if (actorVisuals_Map.Contains(removeActor))
	{
		if (currentlyDisplayedVisuals.ID.Equals(actorVisuals_Map[removeActor].ID))
			E_ModifyRootComponent();

		actorVisuals_Map.Remove(removeActor);
	}
}


