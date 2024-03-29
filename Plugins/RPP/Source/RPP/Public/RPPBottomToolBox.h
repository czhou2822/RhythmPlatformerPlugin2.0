// Copyright 2019 - 2021, Chenkai Zhou, Rhythm Platformer Plugin, All Rights Reserved.

#pragma once

//engine includes
#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "Widgets/SCompoundWidget.h"

//user includes
#include "RPPMain.h"



/**
 * 
 */
class RPP_API SRPPBottomToolBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRPPBottomToolBox)
	{}
	SLATE_ARGUMENT(int32, TestValue)
	SLATE_ARGUMENT(class SRPPMain*, RhythmPlatformingPluginMain)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */


private:

	//TAttribute< int32 > TestValue;

	int32 NewValue;

	SRPPMain* RPPMain;

	TSharedPtr<STextBlock> PlaybackRateText;

	TSharedPtr<SSlider> PlaybackSlider;

public:
	
	void Construct(const FArguments& InArgs);

	FReply TogglePlay();

	void HandleOnSliderChanged(float InFloat);

};
