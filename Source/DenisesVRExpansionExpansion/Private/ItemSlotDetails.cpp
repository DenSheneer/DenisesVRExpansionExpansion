// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemSlotDetails.h"
#include "IDetailGroup.h"
#include "SlotableActor.h"
#include "ItemSlot.h"

TSharedRef<IDetailCustomization> ItemSlotDetails::MakeInstance()
{
	return MakeShareable(new ItemSlotDetails);
}

void ItemSlotDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	IDetailCategoryBuilder& itemSlotcategory = DetailLayout.EditCategory("Item Slot editing");
	itemSlotcategory.SetSortOrder(0);

	IDetailGroup& AcceptedActorsGroup = itemSlotcategory.AddGroup(FName(TEXT("Editable meshes")), FText::FromString("Editable meshes"));

	uint32 AcceptedActorsNr;
	auto arrayRef = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UItemSlot, acceptedActors))->AsArray();

	TSharedRef<SWidget> editTriggerShapeButton = SNew(SButton)
		.Text(FText::FromString("edit"))
		.ContentPadding(10.0f)
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
				// Get a pointer to the actor component instance
				TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();

				for (int32 i = 0; i < SelectedObjects.Num(); ++i)
				{
					UItemSlot* ItemSlot = Cast<UItemSlot>(SelectedObjects[i]);
					if (ItemSlot)
					{
						ItemSlot->EditTriggerShape();
					}
				}

				return FReply::Handled();
			}));

	AcceptedActorsGroup.AddWidgetRow()
		.NameContent()
		[SNew(STextBlock).Text(FText::FromString("Trigger Shape"))]
	.ValueContent()
		[editTriggerShapeButton];

	arrayRef->GetNumElements(AcceptedActorsNr);

	for (uint32 i = 0; i < AcceptedActorsNr; i++)
	{
		FString ActorName = "NONE";
		TSharedPtr<IPropertyHandle> childHandle = arrayRef->GetElement(i);
		UObject* childObject = nullptr;
		childHandle->GetValue(childObject);
		UClass* elementClass = Cast<UClass>(childObject);

		if (childHandle.IsValid())
		{
			if (elementClass != nullptr)
			{
				auto convertedToSlotableActorClass = TSubclassOf<ASlotableActor>(elementClass);
				if (convertedToSlotableActorClass != nullptr)
				{
					ActorName = FString::FromInt(i + 1) + ". " + convertedToSlotableActorClass->GetName();

					// Get a pointer to the actor component instance
					TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();
					auto selectedItemSlot = Cast<UItemSlot>(SelectedObjects[0]);

					if (selectedItemSlot != nullptr)
					{
						TSharedRef<SButton> ActorButton = SNew(SButton)
							.Text(FText::FromString("edit"))
							.ContentPadding(10.0f)
							.OnClicked(FOnClicked::CreateLambda([selectedItemSlot, convertedToSlotableActorClass]() -> FReply
								{
									selectedItemSlot->SetVisualsToAcceptedActor(convertedToSlotableActorClass);

									return FReply::Handled();
								}));

						AcceptedActorsGroup.AddWidgetRow()
							.NameContent()
							[SNew(STextBlock).Text(FText::FromString(ActorName))]
						.ValueContent()
							[ActorButton];
					}
				}
			}
		}
	}

	//	--------------------

			// Add your custom function to the General category
	TSharedRef<SWidget> visibilityButton = SNew(SButton)
		.Text(FText::FromString("Toggle Visuals Visibility"))
		.ContentPadding(10.0f)
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
				// Get a pointer to the actor component instance
				TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();

				for (int32 i = 0; i < SelectedObjects.Num(); ++i)
				{
					UItemSlot* ItemSlot = Cast<UItemSlot>(SelectedObjects[i]);
					if (ItemSlot)
					{
						ItemSlot->ToggleVisibility();
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

				for (int32 i = 0; i < SelectedObjects.Num(); ++i)
				{
					UItemSlot* ItemSlot = Cast<UItemSlot>(SelectedObjects[i]);
					if (ItemSlot != nullptr)
					{
						ItemSlot->ReloadVisuals();
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
