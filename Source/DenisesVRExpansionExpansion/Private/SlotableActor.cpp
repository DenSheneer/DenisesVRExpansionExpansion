// Fill out your copyright notice in the Description page of Project Settings.

#include "SlotableActor.h"
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
		currentSlot->RemoveSlotableActor(this);
		currentSlot = nullptr;
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
		nearestSlot->ReceiveSlotableActor(this);

		currentSlot = nearestSlot;
		currentGripState = EItemGripState::slotted;

		this->SetActorLocationAndRotation(nearestSlot->GetComponentLocation(), nearestSlot->GetComponentRotation());
		this->DisableComponentsSimulatePhysics();
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

	for (int i = 0; i < overlappingComponents.Num(); i++)
	{
		auto itemSlot = Cast<UItemSlot>(overlappingComponents[i]);
		if (itemSlot != nullptr && itemSlot->checkCompatibility(this))
		{
			if (!itemSlot->IsOccupied())
				currentlyAvailable_Slots.Add(itemSlot);
			else
			{
				subscribeToSlotAvailableEvent(itemSlot);
			}
		}
	}
	refreshNearestSlot();
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
			newNearest->ActorInRangeEvent(this);
			subscribeToSlotOccupiedEvent(newNearest);
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
	if (currentGripState == EItemGripState::gripped)
	{
		auto itemSlot = Cast<UItemSlot>(otherComponent);
		if (itemSlot != nullptr && itemSlot->checkCompatibility(this))
			if (!itemSlot->IsOccupied())
				addSlotToList(itemSlot);
			else
				subscribeToSlotAvailableEvent(itemSlot);
	}
}
void ASlotableActor::ComponentOverlapEnd(UActorComponent* otherComponent)
{
	if (currentGripState == EItemGripState::gripped)
	{
		auto itemSlot = Cast<UItemSlot>(otherComponent);
		removeSlotFromList(itemSlot);
	}
}

void ASlotableActor::setupColliderRef()
{
	triggerComponent = FindComponentByClass<USphereComponent>();
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

void ASlotableActor::addSlotToList(UItemSlot* slotToAdd)
{
	if (slotToAdd != nullptr)
	{
		if (!currentlyAvailable_Slots.Contains(slotToAdd))
		{
			currentlyAvailable_Slots.Add(slotToAdd);
		}
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
	if (slot->OnOccupiedEvent.Remove(OccupiedEventHandle))
		OccupiedEventHandle.Reset();
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
