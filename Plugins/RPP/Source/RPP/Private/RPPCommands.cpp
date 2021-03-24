// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPPCommands.h"

#define LOCTEXT_NAMESPACE "FRPPModule"

void FRPPCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "RPP", "Execute RPP action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
