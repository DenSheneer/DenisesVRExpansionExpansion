// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSlotDetails.h"
#include "ItemSlot.h"

TSharedRef<IDetailCustomization> ItemSlotDetails::MakeInstance()
{
	return MakeShareable(new ItemSlotDetails);
}

void ItemSlotDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, (TEXT("this code ran")));

	UE_LOG(LogTemp, Warning, TEXT("this code ran"));

	IDetailCategoryBuilder& GeneralCategory = DetailLayout.EditCategory("Preview Visuals");
	GeneralCategory.SetSortOrder(0);


	// Add your custom function to the General category
	TSharedRef<SWidget> FunctionWidget = SNew(SButton)
		.Text(FText::FromString("Cycle Through Previews"))
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
				// Get a pointer to the actor component instance
				TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();

				if (SelectedObjects.Num() > 0)
				{
					UItemSlot* ActorComponent = Cast<UItemSlot>(SelectedObjects[0].Get());

					// Call your function on the actor component
					if (ActorComponent != nullptr)
					{
						ActorComponent->CycleThroughPreviews();
					}
				}

				return FReply::Handled();
			}));

	GeneralCategory.AddCustomRow(FText::FromString("Cycle Through Previews")).WholeRowContent();
}
