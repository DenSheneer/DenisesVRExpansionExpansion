// Fill out your copyright notice in the Description page of Project Settings.

#include "SlotableActor.h"
#include "GripMotionControllerComponent.h"
#include "Net/UnrealNetwork.h"
#include "ItemGripState.h"

ASlotableActor::ASlotableActor(const FObjectInitializer& ObjectInitializer) : AGrippableActor(ObjectInitializer)
{
	bReplicates = true;
}

void ASlotableActor::BeginPlay()
{
	Super::BeginPlay();

	setupColliderRef();
	if (ColliderComponent)
	{
		ColliderComponent->OnComponentBeginOverlap.AddDynamic(this, &ASlotableActor::checkForSlotOnOverlapBegin);
		ColliderComponent->OnComponentEndOverlap.AddDynamic(this, &ASlotableActor::checkForSlotOnOverlapEnd);
	}
}
void ASlotableActor::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (currentlyAvailable_Slots.Num() > 1 && currentGripState == EItemGripState::gripped)
	{
		if (!HasAuthority()) { return; }
		refreshNearestSlot();
	}
}

void ASlotableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Super::EndPlay(EndPlayReason);
}

void ASlotableActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);

	if (!HasAuthority()) { return; }
	Server_Grip(GrippingController);
}

void ASlotableActor::Server_Grip_Implementation(UGripMotionControllerComponent* GrippingController)
{
	rootAsPrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent());
	rootAsPrimitiveComponent->SetSimulatePhysics(false);
	rootAsPrimitiveComponent->SetCollisionProfileName("Trigger");
	rootAsPrimitiveComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

	currentGrippingController = GrippingController;

	if (currentGrippingController->MotionSource == "left")
		handSide = EControllerHand::Left;
	else
		handSide = EControllerHand::Right;

	if (currentGripState == EItemGripState::slotted)
	{
		current_ResidingSlot->RemoveSlotableActor(this);
		current_ResidingSlot = nullptr;
	}

	auto controllerOwner = GrippingController->GetOwner();
	if (!controllerOwner) { return; }
	SetOwner(controllerOwner);

	currentGripState = EItemGripState::gripped;
	manualFindAvailableSlotsCall();
}

void ASlotableActor::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	if (HasAuthority())
		Server_GripRelease(ReleasingController);

	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
}


void ASlotableActor::Server_GripRelease_Implementation(UGripMotionControllerComponent* ReleasingController)
{
	if (currentNearestSlot != nullptr)
	{
		unsubscribeFromOccupiedEvent(currentNearestSlot);
		currentGripState = EItemGripState::slotted;

		if (HasAuthority())
			currentNearestSlot->ReceiveActorInstigator(this);

		current_ResidingSlot = currentNearestSlot;
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		currentGripState = EItemGripState::loose;
		rootAsPrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent());
		rootAsPrimitiveComponent->SetSimulatePhysics(true);
		rootAsPrimitiveComponent->SetCollisionProfileName("BlockAllDynamic");
		rootAsPrimitiveComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	}


	reset_GrippingParameters();
}

void ASlotableActor::manualFindAvailableSlotsCall()
{
	currentlyAvailable_Slots.Empty();
	TArray<UPrimitiveComponent*> overlappingComponents;
	ColliderComponent->GetOverlappingComponents(overlappingComponents);

	int nrOfOverlaps = overlappingComponents.Num();
	if (nrOfOverlaps > 0)
	{
		for (int i = 0; i < nrOfOverlaps; i++)
		{
			UItemSlot* slot = Cast<UItemSlot>(overlappingComponents[i]->GetAttachParent());
			if (slot)
			{
				if (currentGripState == EItemGripState::gripped)
				{
					if (slot->CheckForCompatibility(this))
						if (slot->SlotState() == EItemSlotState::available)
							addSlotToList(slot, true);
						else
							subscribeToSlotAvailableEvent(slot);
				}
			}
		}
		if (HasAuthority())
			refreshNearestSlot();
	}
}
void ASlotableActor::refreshNearestSlot_Implementation()
{
	UItemSlot* newNearest = findNearestSlot(currentlyAvailable_Slots);
	if (newNearest != currentNearestSlot)
	{
		if (currentNearestSlot != nullptr)
		{
			unsubscribeFromOccupiedEvent(currentNearestSlot);
			unsubscribeFromAvailableEvent(currentNearestSlot);

			currentNearestSlot->ActorOutOfRangeEventInstigation(this);
		}

		if (newNearest != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s: reserving"), *GetName());
			newNearest->ReserveForActor_Server(this, handSide);
		}
		currentNearestSlot = newNearest;
	}
}
UItemSlot* ASlotableActor::findNearestSlot(TArray<UItemSlot*> slotsToCheck)
{
	if (slotsToCheck.Num() > 0)
		if (slotsToCheck.Num() > 1)
		{
			float nearestDist = std::numeric_limits<float>::max();	// max float value
			int nearestSlotIndex = 0;
			for (int i = 0; i < slotsToCheck.Num(); i++)
			{
				auto thisSlotPtr = slotsToCheck[i];
				if (thisSlotPtr != nullptr)
				{
					float thisDistance = FVector::DistSquared(this->GetActorLocation(), thisSlotPtr->GetComponentLocation());
					if (thisDistance < nearestDist)
					{
						nearestDist = thisDistance;
						nearestSlotIndex = i;
					}
				}
			}
			return slotsToCheck[nearestSlotIndex];
		}
		else
		{
			return slotsToCheck[0];
		}
	return nullptr;
}

void ASlotableActor::setupColliderRef()
{
	ColliderComponent = FindComponentByClass<USphereComponent>();
}

void ASlotableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASlotableActor, currentGripState);
	DOREPLIFETIME(ASlotableActor, currentGrippingController);
	DOREPLIFETIME(ASlotableActor, handSide);
	DOREPLIFETIME(ASlotableActor, current_ResidingSlot);
	DOREPLIFETIME(ASlotableActor, currentNearestSlot);
}

void ASlotableActor::checkForSlotOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (currentGripState != EItemGripState::gripped) { return; }

	UItemSlot* overlappingSlot;
	overlappingSlot = Cast<UItemSlot>(OtherComp->GetAttachParent());

	if (overlappingSlot != nullptr)
	{
		if (overlappingSlot->CheckForCompatibility(this))
			if (overlappingSlot->SlotState() == EItemSlotState::available)
				addSlotToList(overlappingSlot, false);
			else
				subscribeToSlotAvailableEvent(overlappingSlot);
	}
}

void ASlotableActor::checkForSlotOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (currentGripState != EItemGripState::gripped) { return; }
	checkForSlotOnOverlapEndServer(OtherComp);
}

void ASlotableActor::checkForSlotOnOverlapEndServer_Implementation(UPrimitiveComponent* OtherComp)
{
	UItemSlot* overlappingSlot;
	overlappingSlot = Cast<UItemSlot>(OtherComp->GetAttachParent());

	if (overlappingSlot != nullptr)
	{
		if (overlappingSlot->CheckForCompatibility(this))
		{
			removeSlotFromList(overlappingSlot);
		}
	}
}

void ASlotableActor::removeSlotFromList(UItemSlot* slotToRemove)
{
	if (slotToRemove != nullptr)
	{
		if (currentlyAvailable_Slots.Contains(slotToRemove))
			currentlyAvailable_Slots.Remove(slotToRemove);

		if (becomeAvailableSlots.Contains(slotToRemove))
			unsubscribeFromAvailableEvent(slotToRemove);

		if (becomeOccupiedSlots.Contains(slotToRemove))
			unsubscribeFromOccupiedEvent(slotToRemove);

		if (!HasAuthority()) { return; }
		if (slotToRemove == currentNearestSlot)
			refreshNearestSlot();
	}
}

void ASlotableActor::addSlotToList(UItemSlot* slotToAdd, bool skipNearestRefresh)
{
	if (!slotToAdd) { return; }

	if (!currentlyAvailable_Slots.Contains(slotToAdd))
	{
		currentlyAvailable_Slots.Add(slotToAdd);
	}

	if (skipNearestRefresh) { return; }
	if (!HasAuthority()) { return; }
	refreshNearestSlot();
}

void ASlotableActor::reset_GrippingParameters()
{
	currentGrippingController = nullptr;
	currentlyAvailable_Slots.Empty();
	currentNearestSlot = nullptr;

	for (int32 Index = becomeAvailableSlots.Num() - 1; Index >= 0; --Index)
	{
		unsubscribeFromAvailableEvent(becomeAvailableSlots[Index]);
	}
	for (int32 Index = becomeOccupiedSlots.Num() - 1; Index >= 0; --Index)
	{
		unsubscribeFromOccupiedEvent(becomeOccupiedSlots[Index]);
	}
}

void ASlotableActor::subscribeToSlotOccupiedEvent(UItemSlot* slot)
{
	if (becomeOccupiedSlots.Contains(slot)) { return; }
	becomeOccupiedSlots.Add(slot);

	slot->OnOccupied.BindLambda([this](UItemSlot* pSlot)
		{
			this->removeSlotFromList(pSlot);
			unsubscribeFromOccupiedEvent(pSlot);
			subscribeToSlotAvailableEvent(pSlot);
		}
	);
}

void ASlotableActor::unsubscribeFromOccupiedEvent(UItemSlot* slot)
{
	if (!slot) { return; }
	if (!becomeOccupiedSlots.Contains(slot)) { return; }
	becomeOccupiedSlots.Remove(slot);

	slot->OnOccupied.Unbind();
}
void ASlotableActor::subscribeToSlotAvailableEvent(UItemSlot* slot)
{
	if (becomeAvailableSlots.Contains(slot)) { return; }
	becomeAvailableSlots.Add(slot);

	slot->OnAvailable.BindLambda([this](UItemSlot* pSlot)
		{
			addSlotToList(pSlot);
			unsubscribeFromAvailableEvent(pSlot);
		}
	);

	UE_LOG(LogTemp, Warning, TEXT("%s: Subbed"), *GetName());
}
void ASlotableActor::unsubscribeFromAvailableEvent(UItemSlot* slot)
{
	if (!slot) { return; }
	if (!becomeAvailableSlots.Contains(slot)) { return; }
	becomeAvailableSlots.Remove(slot);
	slot->OnAvailable.Unbind();
	UE_LOG(LogTemp, Warning, TEXT("%s: Unsubbed"), *GetName());
}
