// Copyright Chenkai Zhou. All Rights Reserved.


#include "RPPButtonVBox.h"
#include "EditorViewportClient.h"


//user includes
#include "RPPUtility.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION


void SRPPButtonVBox::Construct(const FArguments& InArgs)
{
	RPPMain = InArgs._RhythmPlatformingPluginMain;
	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()   //hidden button
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SButton)
				.OnClicked(this,&SRPPButtonVBox::BackToTop)
				.Content()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("BackToBeginning")))
				]
			]

			+ SVerticalBox::Slot()   //Reload
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SButton)
				.OnClicked(this, &SRPPButtonVBox::ReloadWave)
				.Content()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Reset")))
				]
			]

			+ SVerticalBox::Slot()   //RefreshRunningSpeed button
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SButton)
				.OnClicked(this, &SRPPButtonVBox::RefreshRunningSpeed)
				.Content()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("RefreshRunningSpeed")))
				]
			]

			+ SVerticalBox::Slot()   //BeatStartingPoint button
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Left)
			[
				SNew(SButton)
				.OnClicked(this, &SRPPButtonVBox::ChangeBeatStartingTime)
				.Content()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("BeatStartingPoint")))
				]
			]
		];

}

FReply SRPPButtonVBox::BackToTop()
{
	if (RPPMain)
	{
		RPPMain->ResetViewport();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SRPPButtonVBox::ReloadWave()
{
	if (RPPMain)
	{
		RPPMain->Initilization();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SRPPButtonVBox::RefreshRunningSpeed()
{
	if (RPPMain)
	{
		URPPUtility::RefreshRunSpeed();
		return FReply::Handled();
	}
	return FReply::Unhandled();

}

FReply SRPPButtonVBox::ChangeBeatStartingTime()
{
	if (RPPMain)
	{
		RPPMain->SetCurrentAsBeatStartingTime();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}



END_SLATE_FUNCTION_BUILD_OPTIMIZATION

