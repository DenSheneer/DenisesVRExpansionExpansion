// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSlot.h"
#include "ItemSlotTrigger.h"

void UItemSlotTrigger::SetUp(UItemSlot* pAttachedTo, FSlotableActorVisuals visuals)
{
	attachedTo = pAttachedTo;

	SetStaticMesh(visuals.Mesh);
	SetMaterial(0, visuals.PreviewMaterial);
	SetWorldScale3D(visuals.Scale);

	AttachToComponent(pAttachedTo->GetAttachParent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetRelativeLocationAndRotation(visuals.RelativePosition, visuals.RelativeRotation);
	AttachToComponent(pAttachedTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	RegisterComponent();
	GetOwner()->AddInstanceComponent(this);
}

