// Copyright Epic Games, Inc. All Rights Reserved.

//engine includes
#include "RPP.h"
#include "RPPStyle.h"
#include "RPPCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IAssetViewport.h"
#include "Kismet/GameplayStatics.h"
#include "LevelEditor.h"

//user includes
#include "RPPGameModule/Public/RPPPluginManager.h"
#include "RPPMain.h"


static const FName RPPTabName("RPP");

DEFINE_LOG_CATEGORY(LogRPP);

#define LOCTEXT_NAMESPACE "FRPPModule"

void FRPPModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FRPPStyle::Initialize();
	FRPPStyle::ReloadTextures();

	FRPPCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRPPCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FRPPModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRPPModule::RegisterMenus));
}

void FRPPModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FRPPStyle::Shutdown();

	FRPPCommands::Unregister();
}

void FRPPModule::PluginButtonClicked()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");


	int32 Width = GEditor->GetActiveViewport()->GetSizeXY().X;

	int32 Height = GEditor->GetActiveViewport()->GetSizeXY().Y;

	UWorld* World = GEditor->GetActiveViewport()->GetClient()->GetWorld();
	if (World)
	{
		TArray<AActor*> foundManager;
		UGameplayStatics::GetAllActorsOfClass(World, ARPPPluginManager::StaticClass(), foundManager);

		if (foundManager.Num() == 1)
		{
			ARPPPluginManager* RPPPluginManager = Cast<ARPPPluginManager>(foundManager[0]);
			if (RPPMain == nullptr && RPPPluginManager)
			{
				RPPMain = SNew(SRPPMain)
					.ExternalRPPPluginManager(RPPPluginManager)
					.RPPWidth(Width * 0.8)
					.RPPHeight(Height * 0.4);


				LevelEditorModule.GetFirstActiveViewport()->AddOverlayWidget(RPPMain.ToSharedRef());
				//bSMyWidgetInitilized = true;
			}
			else
			{
				LevelEditorModule.GetFirstActiveViewport()->RemoveOverlayWidget(RPPMain.ToSharedRef());
				RPPMain.Reset();
			}
		}
		else
		{
			// Put your "OnButtonClicked" stuff here
			FText DialogText = FText::Format(
				LOCTEXT("FRPPModule", "Did not find plugin manager in scene!"),
				FText::FromString(TEXT("FRPPModule::PluginButtonClicked")),
				FText::FromString(TEXT("RPP.cpp"))
			);
			FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		}
	}
}

void FRPPModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRPPCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FRPPCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRPPModule, RPP)