// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemGripState.generated.h"

UENUM(BlueprintType)
enum EItemGripState : int
{
	loose		UMETA(DisplayName = "loose"),
	gripped		UMETA(DisplayName = "gripped"),
	slotted		UMETA(DisplayName = "slotted")
};
