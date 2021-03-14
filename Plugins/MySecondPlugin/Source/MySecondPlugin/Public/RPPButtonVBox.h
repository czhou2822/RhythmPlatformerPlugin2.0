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
class MYSECONDPLUGIN_API SRPPButtonVBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRPPButtonVBox)
	{}
	SLATE_ARGUMENT(class SRPPMain*, RhythmPlatformingPluginMain)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */


private:
	class SRPPMain* RPPMain;

private:

	FReply BackToTop();

	FReply ReloadWave();

	FReply RefreshRunningSpeed();

	FReply ChangeBeatStartingTime();

public:
	void Construct(const FArguments& InArgs);




};
