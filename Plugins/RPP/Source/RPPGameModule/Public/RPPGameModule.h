// Copyright 2019 - 2021, Chenkai Zhou, Rhythm Platformer Plugin, All Rights Reserved.



#pragma once
 
#include "Modules/ModuleManager.h"
 
DECLARE_LOG_CATEGORY_EXTERN(RPPGameModule, All, All);
 
class FRPPGameModule : public IModuleInterface
{
	public:
 
	/* This will get called when the editor loads the module */
	virtual void StartupModule() override;
 
	/* This will get called when the editor unloads the module */
	virtual void ShutdownModule() override;
};