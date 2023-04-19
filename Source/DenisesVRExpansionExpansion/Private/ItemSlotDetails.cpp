// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSlotDetails.h"
#include "ItemSlot.h"

TSharedRef<IDetailCustomization> ItemSlotDetails::MakeInstance()
{
	return MakeShareable(new ItemSlotDetails);
}

void ItemSlotDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& itemSlotcategory = DetailLayout.EditCategory("Item Slot editing");
	itemSlotcategory.SetSortOrder(0);

	// Add your custom function to the General category
	TSharedRef<SWidget> cyclePreviewButton = SNew(SButton)
		.Text(FText::FromString("Cycle Through Previews"))
		.ContentPadding(10.0f)
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

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			cyclePreviewButton
		];

	//	--------------------

		// Add your custom function to the General category
	TSharedRef<SWidget> editTriggerShapeButton = SNew(SButton)
		.Text(FText::FromString("Edit Preview Mesh"))
		.ContentPadding(10.0f)
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
						ActorComponent->EditTriggerShape();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			editTriggerShapeButton
		];

	//	--------------------

			// Add your custom function to the General category
	TSharedRef<SWidget> visibilityButton = SNew(SButton)
		.Text(FText::FromString("Toggle Visuals Visibility"))
		.ContentPadding(10.0f)
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
						ActorComponent->TogglePreviewVisibility();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			visibilityButton
		];

	//	--------------------

	// Adding a spacer
	itemSlotcategory.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(SBox)
			.HeightOverride(10.0f)
		];

	//	--------------------

				// Add your custom function to the General category
	TSharedRef<SWidget> reloadButton = SNew(SButton)
		.Text(FText::FromString("Reload Visuals"))
		.ContentPadding(10.0f)
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
						ActorComponent->ReloadVisuals();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			reloadButton
		];
}
