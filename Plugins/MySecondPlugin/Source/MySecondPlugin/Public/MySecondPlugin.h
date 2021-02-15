// Copyright Chenkai Zhou. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "RPPMain.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

DECLARE_LOG_CATEGORY_EXTERN(LogRPP, Log, All);

class FMySecondPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<SRPPMain> RPPMain;

	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	bool bPluginIsActivated = false;
	bool bSMyWidgetInitilized = false;
};
