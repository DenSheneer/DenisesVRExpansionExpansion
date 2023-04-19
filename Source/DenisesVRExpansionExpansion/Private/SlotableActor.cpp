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
	if (currentlyAvailable_Slots.Num() > 1 && currentGripState == EItemGripState::gripped)
		refreshNearestSlot();
}

void ASlotableActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	if (currentGripState == EItemGripState::slotted)
	{
		current_ResidingSlot->RemoveSlotableActor(this);
		current_ResidingSlot = nullptr;
	}
	currentGripState = EItemGripState::gripped;
	currentGrippingController = GrippingController;
	manualFindAvailableSlotsCall();
}

void ASlotableActor::OnGripRelease_Implementation(UGripMotionControllerComponent* ReleasingController, const FBPActorGripInformation& GripInformation, bool bWasSocketed)
{
	UItemSlot* nearestSlot = currentNearestSlot;
	if (nearestSlot != nullptr)
	{
		unsubscribeFromOccupiedEvent(nearestSlot);
		if (nearestSlot->TryToReceiveActor(this))
		{
			//this->DisableComponentsSimulatePhysics();
			current_ResidingSlot = nearestSlot;
			currentGripState = EItemGripState::slotted;

			//this->SetActorLocationAndRotation(nearestSlot->GetComponentLocation(), nearestSlot->GetComponentRotation());
		}
		else
			currentGripState = EItemGripState::loose;
	}
	else
		currentGripState = EItemGripState::loose;

	reset_GrippingParameters();
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
			auto itemSlot = Cast<UItemSlot>(overlappingComponents[i]);
			handleSlotOverlap(itemSlot, true);
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
			currentNearestSlot->ActorOutOfRangeEvent(this);
			unsubscribeFromOccupiedEvent(currentNearestSlot);
		}

		if (newNearest != nullptr)
		{
			EControllerHand handSide;
			if (currentGrippingController->MotionSource == "left")
				handSide = EControllerHand::Left;
			else
				handSide = EControllerHand::Right;

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
void ASlotableActor::ComponentOverlapBegin(UActorComponent* otherComponent)
{
	auto cast = Cast<UStaticMeshComponent>(otherComponent);
	if (cast != nullptr)
		handleSlotOverlap(Cast<UItemSlot>(cast->GetAttachParent()));
}
void ASlotableActor::ComponentOverlapEnd(UActorComponent* otherComponent)
{
	if (currentGripState == EItemGripState::gripped)
	{
		auto cast = Cast<UStaticMeshComponent>(otherComponent);
		if (cast != nullptr)
		{
			auto itemSlot = Cast<UItemSlot>(cast->GetAttachParent());
			if (itemSlot != nullptr)
				removeSlotFromList(itemSlot);
		}
	}
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
		if (subscribedTo_Slots.Contains(slotToRemove))
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
	for (int i = 0; i < subscribedTo_Slots.Num(); i++)
		unsubscribeFromAvailableEvent(subscribedTo_Slots[i]);
}

void ASlotableActor::subscribeToSlotOccupiedEvent(UItemSlot* slot)
{
	OccupiedEventHandle = slot->OnOccupiedEvent.AddLambda([this](UItemSlot* pSlot)
		{

			this->removeSlotFromList(pSlot);
			subscribeToSlotAvailableEvent(pSlot);
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
	subscribedTo_Slots.Add(slot);
	AvailableEventHandle = slot->OnAvailableEvent.AddLambda([this](UItemSlot* pSlot)
		{
			this->addSlotToList(pSlot);
			unsubscribeFromAvailableEvent(pSlot);
			subscribedTo_Slots.Remove(pSlot);
		}
	);
}

void ASlotableActor::unsubscribeFromAvailableEvent(UItemSlot* slot)
{
	if (subscribedTo_Slots.Contains(slot))
	{
		if (slot->OnAvailableEvent.Remove(AvailableEventHandle))
		{
			AvailableEventHandle.Reset();
			subscribedTo_Slots.Remove(slot);
		}
	}
}
