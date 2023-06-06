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

	// Add your custom function to the General category
	TSharedRef<SWidget> editRootButton = SNew(SButton)
		.Text(FText::FromString("edit root"))
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
						ItemSlot->E_ModifyRootComponent();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			editRootButton
		];

	IDetailGroup& AcceptedActorsGroup = itemSlotcategory.AddGroup(FName(TEXT("Editable meshes")), FText::FromString("Editable meshes"));

	uint32 AcceptedActorsNr;
	auto arrayRef = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UItemSlot, acceptedActors))->AsArray();

	arrayRef->GetNumElements(AcceptedActorsNr);

	if (AcceptedActorsNr == 0)
	{
		AcceptedActorsGroup.AddWidgetRow()
			.WholeRowContent()
			[
				SNew(STextBlock)
				.TextStyle(FCoreStyle::Get(), "EmbossedText")
			.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 9))
			.Text(FText::FromString("No accepted actors"))
			.ColorAndOpacity(FSlateColor(FLinearColor::Red))
			];
	}

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
									selectedItemSlot->E_ModifyAcceptedActorMesh(convertedToSlotableActorClass);

									return FReply::Handled();
								}));
						TSharedRef<SButton> ResetPositionButton = SNew(SButton)
							.Text(FText::FromString("reset to root"))
							.ContentPadding(10.0f)
							.OnClicked(FOnClicked::CreateLambda([selectedItemSlot, convertedToSlotableActorClass]() -> FReply
								{
									selectedItemSlot->E_ResetActorMeshToRootTransform(convertedToSlotableActorClass);

									return FReply::Handled();
								}));

						AcceptedActorsGroup.AddWidgetRow()
							.NameContent()
							[SNew(STextBlock).Text(FText::FromString(ActorName))]
						.ValueContent()
							[SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.Padding(2.0f)
							[
								ActorButton
							]
						+ SHorizontalBox::Slot()
							.Padding(2.0f)
							.AutoWidth()
							[
								ResetPositionButton
							]
							];
					}
				}
			}
		}
	}

	//	--------------------

	TSharedRef<SWidget> TitleWidget = SNew(STextBlock)
		.Text(FText::FromString("Collider shapes"))
		.Margin(5.0f)
		.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 9));

	itemSlotcategory.AddCustomRow(FText::FromString("ColliderSelectTitle"))
		.WholeRowContent()
		[
			TitleWidget
		];

	//	--------------------

	TSharedRef<SWidget> EditTriggerButton = SNew(SButton)
		.Text(FText::FromString("Edit trigger shape"))
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
						ItemSlot->E_ModifyTriggerShape();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			EditTriggerButton
		];
	//--------------------

	TSharedRef<SWidget> ResetTriggerToRootButton = SNew(SButton)
		.Text(FText::FromString("Reset trigger position"))
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
						ItemSlot->E_ResetTriggerMeshToRootTransform();
					}
				}

				return FReply::Handled();
			}));

	itemSlotcategory.AddCustomRow(FText::FromString("Item Slot editing"))
		.WholeRowContent()
		[
			ResetTriggerToRootButton
		];



	//---------------------

	TSharedRef<SHorizontalBox> ColliderButtonRow = SNew(SHorizontalBox);


	TSharedPtr<SButton> SphereButton;
	ColliderButtonRow->AddSlot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Left)
		.Padding(5.0f)
		[
			SAssignNew(SphereButton, SButton)
			.Text(FText::FromString("Use sphere collision"))
		.ContentPadding(10.0f)
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
				TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();

				for (int32 i = 0; i < SelectedObjects.Num(); ++i)
				{
					UItemSlot* ItemSlot = Cast<UItemSlot>(SelectedObjects[i]);
					if (ItemSlot != nullptr)
					{
						ItemSlot->E_SetTriggerShape(ECollisionShape::Sphere);
					}
				}
				return FReply::Handled();
			}))
		];

	TSharedPtr<SButton> BoxButton;
	ColliderButtonRow->AddSlot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Center)
		.Padding(5.0f)
		[
			SAssignNew(BoxButton, SButton)
			.Text(FText::FromString("Use box collision"))
		.ContentPadding(10.0f)
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
				TArray<TWeakObjectPtr<UObject>> SelectedObjects = DetailLayout.GetDetailsView()->GetSelectedObjects();

				for (int32 i = 0; i < SelectedObjects.Num(); ++i)
				{
					UItemSlot* ItemSlot = Cast<UItemSlot>(SelectedObjects[i]);
					if (ItemSlot != nullptr)
					{
						ItemSlot->E_SetTriggerShape(ECollisionShape::Box);
					}
				}
				return FReply::Handled();
			}))
		];

	itemSlotcategory.AddCustomRow(FText::FromString("Collider selection"))
		.WholeRowContent()
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
		.Value(1)
		[
			SphereButton.ToSharedRef()
		]
	+ SSplitter::Slot()
		.Value(1)
		[
			BoxButton.ToSharedRef()
		]
		];

	// Adding a spacer
	itemSlotcategory.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		[
			SNew(SBox)
			.HeightOverride(10.0f)
		];

	//	--------------------

	TSharedRef<SWidget> visibilityButton = SNew(SButton)
		.Text(FText::FromString("toggle visuals visibility"))
		.ContentPadding(10.0f)
		.OnClicked(FOnClicked::CreateLambda([&]() -> FReply
			{
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

	TSharedRef<SWidget> reloadButton = SNew(SButton)
		.Text(FText::FromString("(re)load visuals"))
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
						ItemSlot->E_ReloadVisuals();
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
