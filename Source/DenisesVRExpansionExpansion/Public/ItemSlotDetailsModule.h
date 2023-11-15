// Fill out your copyright notice in the Description page of Project Settings.
#if WITH_EDITOR
#pragma once

#include "CoreMinimal.h"
#include "ItemSlotDetails.h"
#include "PropertyEditorModule.h"

/**
 * 
 */
class ItemSlotDetailsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
#endif