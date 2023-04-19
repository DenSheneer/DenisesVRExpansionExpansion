// Copyright Epic Games, Inc. All Rights Reserved.

#include "DenisesVRExpansionExpansion.h"
#include "ItemSlotDetailsModule.h"

#define LOCTEXT_NAMESPACE "FDenisesVRExpansionExpansionModule"

void FDenisesVRExpansionExpansionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	    // Register our custom detail customization class
    FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout("ItemSlot", FOnGetDetailCustomizationInstance::CreateStatic(&ItemSlotDetails::MakeInstance));

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, (TEXT("this code ran")));

    UE_LOG(LogTemp, Warning, TEXT("this code ran"));
}

void FDenisesVRExpansionExpansionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	    // Unregister our custom detail customization class
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomClassLayout("ItemSlot");
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDenisesVRExpansionExpansionModule, DenisesVRExpansionExpansion)