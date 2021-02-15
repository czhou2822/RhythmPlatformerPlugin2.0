// Copyright Chenkai Zhou. All Rights Reserved.


#include "MySecondPluginCommands.h"

#define LOCTEXT_NAMESPACE "FMySecondPluginModule"

void FMySecondPluginCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "RPP", "Execute MySecondPlugin action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
