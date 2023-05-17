// Fill out your copyright notice in the Description page of Project Settings.

#include "SlotableActor.h"
#include "GripMotionControllerComponent.h"
#include "ItemGripState.h"

void ASlotableActor::BeginPlay()
{
	Super::BeginPlay();
	setupColliderRef();
}
void ASlotableActor::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (currentlyAvailable_Slots.Num() > 1 && currentGripState == EItemGripState::gripped)
		refreshNearestSlot();
}

void ASlotableActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	Super::OnGrip_Implementation(GrippingController, GripInformation);
	currentGrippingController = GrippingController;

	if (currentGrippingController->MotionSource == "left")
		handSide = EControllerHand::Left;
	else
		handSide = EControllerHand::Right;

	if (currentGripState == EItemGripState::slotted)
	{
		current_ResidingSlot->RemoveSlotableActor(this);
		current_ResidingSlot = nullptr;
		SetOwner(GrippingController->GetOwner());
	}
	currentGripState = EItemGripState::gripped;
	manualFindAvailableSlotsCall();
}

void ASlotableActor::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	if (currentNearestSlot != nullptr)
	{
		unsubscribeFromOccupiedEvent(currentNearestSlot);
		currentGripState = EItemGripState::slotted;

		if (currentNearestSlot->TryToReceiveActor(this))
		{
			current_ResidingSlot = currentNearestSlot;
		}
		else
			currentGripState = EItemGripState::loose;
	}
	else
		currentGripState = EItemGripState::loose;

	reset_GrippingParameters();
	Super::OnGripRelease_Implementation(ReleasingController, GripInformation, bWasSocketed);
}

void ASlotableActor::manualFindAvailableSlotsCall()
{
	currentlyAvailable_Slots.Empty();
	TArray<UPrimitiveComponent*> overlappingComponents;
	triggerComponent->GetOverlappingComponents(overlappingComponents);

	int nrOfOverlaps = overlappingComponents.Num();
	if (nrOfOverlaps > 0)
	{
		for (int i = 0; i < nrOfOverlaps; i++)
		{
			UItemSlotTrigger* castTo = Cast<UItemSlotTrigger>(overlappingComponents[i]);
			if (castTo) 
			{
				UItemSlot* slot = castTo->AttachedTo();
				if (slot)
				{
					handleSlotOverlap(slot, true);
				}
			}
		}
		refreshNearestSlot();
	}
}
void ASlotableActor::refreshNearestSlot()
{
	UItemSlot* newNearest = findNearestSlot(currentlyAvailable_Slots);
	if (newNearest != currentNearestSlot)
	{
		if (currentNearestSlot != nullptr)
		{
			unsubscribeFromOccupiedEvent(currentNearestSlot);
			unsubscribeFromAvailableEvent(currentNearestSlot);
			currentNearestSlot->ActorOutOfRangeEvent(this);
		}

		if (newNearest != nullptr)
		{
			//UE_LOG(LogTemp, Warning, TEXT("visuals: '%s'"), handSide);
			newNearest->TryToReserve(this, handSide);
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
void ASlotableActor::ComponentOverlapBegin(UActorComponent* other)
{
	UItemSlotTrigger* castTo = Cast<UItemSlotTrigger>(other);
	if (!castTo) { return; }

	UItemSlot* slot = castTo->AttachedTo();
	if (!slot) { return; }

	handleSlotOverlap(slot);
}

void ASlotableActor::ComponentOverlapEnd(UActorComponent* other)
{
	UItemSlotTrigger* castTo = Cast<UItemSlotTrigger>(other);

	if (!castTo) { return; }
	UItemSlot* slot = castTo->AttachedTo();

	if (!slot) { return; }
	removeSlotFromList(slot);
}

void ASlotableActor::setupColliderRef()
{
	triggerComponent = FindComponentByClass<USphereComponent>();
}

void ASlotableActor::handleSlotOverlap(UItemSlot* overlappingSlot, bool skipNearestRefreshFlag)
{
	if (overlappingSlot != nullptr)
	{
		if (currentGripState == EItemGripState::gripped)
		{
			if (overlappingSlot != nullptr && overlappingSlot->CheckForCompatibility(this))
				if (overlappingSlot->SlotState() == EItemSlotState::available)
					addSlotToList(overlappingSlot, skipNearestRefreshFlag);
				else
					subscribeToSlotAvailableEvent(overlappingSlot);
		}
	}
}

void ASlotableActor::removeSlotFromList(UItemSlot* slotToRemove)
{
	if (slotToRemove != nullptr)
	{
		if (currentlyAvailable_Slots.Contains(slotToRemove))
		{
			currentlyAvailable_Slots.Remove(slotToRemove);
		}
		if (AvailableEventHandles.Contains(slotToRemove))
		{
			unsubscribeFromAvailableEvent(slotToRemove);
		}
		if (slotToRemove == currentNearestSlot)
			refreshNearestSlot();
	}
}

void ASlotableActor::addSlotToList(UItemSlot* slotToAdd, bool skipNearestRefresh)
{
	if (slotToAdd != nullptr)
	{
		if (!currentlyAvailable_Slots.Contains(slotToAdd))
		{
			currentlyAvailable_Slots.Add(slotToAdd);
		}
		if (!skipNearestRefresh)
			refreshNearestSlot();
	}
}

void ASlotableActor::reset_GrippingParameters()
{
	currentGrippingController = nullptr;
	currentlyAvailable_Slots.Empty();
	currentNearestSlot = nullptr;

	TArray<UItemSlot*> keys;
	for (const auto& Pair : AvailableEventHandles)
	{
		keys.Add(Pair.Key);
	}
	for (int32 Index = keys.Num() - 1; Index >= 0; --Index)
	{
		unsubscribeFromAvailableEvent(keys[Index]);
	}
	AvailableEventHandles.Empty();
}

void ASlotableActor::subscribeToSlotOccupiedEvent(UItemSlot* slot)
{
	OccupiedEventHandle = slot->OnOccupiedEvent.AddLambda([this](UItemSlot* pSlot)
		{
			this->removeSlotFromList(pSlot);

			auto handle = AvailableEventHandles[pSlot];
			if (handle.IsValid())
				pSlot->OnAvailableEvent.Remove(handle);

			unsubscribeFromOccupiedEvent(pSlot);
		}
	);
}

void ASlotableActor::unsubscribeFromOccupiedEvent(UItemSlot* slot)
{
	if (slot != nullptr)
	{
		if (slot->OnOccupiedEvent.Remove(OccupiedEventHandle))
			OccupiedEventHandle.Reset();
	}
}
void ASlotableActor::subscribeToSlotAvailableEvent(UItemSlot* slot)
{
	AvailableEventHandles.Add(slot, slot->OnAvailableEvent.AddLambda([&](UItemSlot* pSlot)
		{
			if (!pSlot || !this) return;

			this->unsubscribeFromAvailableEvent(pSlot);
			this->addSlotToList(pSlot);
		}
	));

	UE_LOG(LogTemp, Warning, TEXT("%s Should be subbed"), *this->GetName());
}
void ASlotableActor::unsubscribeFromAvailableEvent(UItemSlot* slot)
{
	if (!slot) { return; }
	if (!AvailableEventHandles.Contains(slot)) { return; }

	auto handle = AvailableEventHandles[slot];
	if (!handle.IsValid()) { return; }

	UE_LOG(LogTemp, Warning, TEXT("%s Should be unsubbed"), *this->GetName());
	slot->OnAvailableEvent.Remove(handle);
	AvailableEventHandles.Remove(slot);
}
