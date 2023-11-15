// Fill out your copyright notice in the Description page of Project Settings.
#if WITH_EDITOR
#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

/**
 * 
 */
class ItemSlotDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& detailLayout) override;
};
#endif