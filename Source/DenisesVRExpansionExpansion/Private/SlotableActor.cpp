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
	if (currentAvailableSlots.Num() > 1 && currentGripState == EItemGripState::gripped)
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
		this->DisableComponentsSimulatePhysics();
	}
	else
		currentGripState = EItemGripState::loose;

	resetGrippedParameters();
}

void ASlotableActor::manualFindAvailableSlotsCall()
{
	currentAvailableSlots.Empty();
	TArray<UPrimitiveComponent*> overlappingComponents;
	triggerComponent->GetOverlappingComponents(overlappingComponents);

	for (int i = 0; i < overlappingComponents.Num(); i++)
	{
		auto itemSlot = Cast<UItemSlot>(overlappingComponents[i]);
		if (itemSlot != nullptr)
		{
			if (!itemSlot->IsOccupied())
				currentAvailableSlots.Add(itemSlot);
			else
				subscribeToSlotAvailableEvent(itemSlot);
		}
	}
	refreshNearestSlot();
}
void ASlotableActor::refreshNearestSlot()
{
	UItemSlot* newNearest = findNearestSlot(currentAvailableSlots);
	if (newNearest != currentNearestSlot)
	{
		if (currentNearestSlot != nullptr)
		{
			currentNearestSlot->ActorOutOfRangeEvent(this);
			unsubscribeFromOccupiedEvent(currentNearestSlot);
			unsubscribeFromAvailableEvent(currentNearestSlot);
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
		if (itemSlot != nullptr)
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
		if (currentAvailableSlots.Contains(slotToRemove))
		{
			currentAvailableSlots.Remove(slotToRemove);
		}
		if (slotToRemove == currentNearestSlot)
			refreshNearestSlot();
	}
}

void ASlotableActor::addSlotToList(UItemSlot* slotToAdd)
{
	if (slotToAdd != nullptr)
	{
		if (!currentAvailableSlots.Contains(slotToAdd))
		{
			currentAvailableSlots.Add(slotToAdd);
		}
		refreshNearestSlot();
	}
}

void ASlotableActor::resetGrippedParameters()
{
	currentGrippingController = nullptr;
	currentAvailableSlots.Empty();
	currentNearestSlot = nullptr;
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
	AvailableEventHandle = slot->OnAvailableEvent.AddLambda([this](UItemSlot* pSlot)
		{
			this->addSlotToList(pSlot);
			unsubscribeFromAvailableEvent(pSlot);
		}
	);
}

void ASlotableActor::unsubscribeFromAvailableEvent(UItemSlot* slot)
{
	if (slot->OnAvailableEvent.Remove(AvailableEventHandle))
		AvailableEventHandle.Reset();
}