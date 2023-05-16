// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ItemSlotState.generated.h"

UENUM(BlueprintType)
enum EItemSlotState : int
{
	available	UMETA(DisplayName = "available"),
	reserved	UMETA(DisplayName = "reserved"),
	occupied	UMETA(DisplayName = "occupied")
};