// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "RPPStyle.h"

class FRPPCommands : public TCommands<FRPPCommands>
{
public:

	FRPPCommands()
		: TCommands<FRPPCommands>(TEXT("RPP"), NSLOCTEXT("Contexts", "RPP", "RPP Plugin"), NAME_None, FRPPStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
