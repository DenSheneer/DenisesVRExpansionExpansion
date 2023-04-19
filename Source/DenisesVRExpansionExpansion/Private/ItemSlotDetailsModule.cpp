// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemSlotDetailsModule.h"

void ItemSlotDetailsModule::StartupModule()
{
    // Register our custom detail customization class
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout("ItemSlot", FOnGetDetailCustomizationInstance::CreateStatic(&ItemSlotDetails::MakeInstance));

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, (TEXT("this code ran")));

    UE_LOG(LogTemp, Warning, TEXT("this code ran"));
}

void ItemSlotDetailsModule::ShutdownModule()
{
    // Unregister our custom detail customization class
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout("ItemSlot");
    }
}
