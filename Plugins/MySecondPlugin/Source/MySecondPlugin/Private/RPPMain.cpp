// Fill out your copyright notice in the Description page of Project Settings.

#include "RPPMain.h"

//engine include
#include "Kismet/GameplayStatics.h"
#include "EditorViewportClient.h"
#include "Components/AudioComponent.h"
#include "Editor.h"

//user include
#include "RPPGameModule/Public/RPPEventBase.h"
#include "RPPGameModule/Public/RPPPluginManager.h"
#include "MySecondPlugin.h"
#include "RPPUtility.h"
#include "MySecondPluginManager.h"




BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION



void SRPPMain::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::SelfHitTestInvisible);

	URPPUtility::WidgetHeight = InArgs._RPPHeight;

	URPPUtility::WidgetWidth = InArgs._RPPWidth;

	ChildSlot
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(10.0,50.0,0.0,0.0))
			[
				SAssignNew(RPPButtonVBox, SRPPButtonVBox)
				.RhythmPlatformingPluginMain(this)
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(RPPMainCanvas, SRPPMainCanvas)
				.RhythmPlatformingPluginMain(this)
			]
		];

	//if (RPPMainCanvas)
	//{
	//	if (RPPMainCanvas->RPPWaveformCanvas)
	//	{
	//		FVector2D WidgetSize = RPPMainCanvas->RPPWaveformCanvas->GetTickSpaceGeometry().GetAbsoluteSize();
	//		UE_LOG(LogTemp, Warning, TEXT("Width: %i, Height: %i"), WidgetSize.X, WidgetSize.Y);

	//	}
	//}


	Initilization();

}

void SRPPMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bIsValidated)
	{

		//process zoom
		{
			if (EditorViewportClient->GetOrthoZoom() != OrthoZoom)   //zoom happened
			{
				OrthoZoom = EditorViewportClient->GetOrthoZoom();
				ProcessZoom();                                       //recalculate zoom and display
			}
		}

		//update look at
		{
			if ((AudioComponent->bIsPaused) /*&& (AudioPercentage <= 1)*/)
			{
				AudioCursor = (EditorViewportClient->GetViewLocation().X - CameraStartingLocation.X) / RPPPluginManager->RunningSpeed;
				AudioPercentage = AudioCursor / AudioDuration;
			}

			SnaplineCursor = (AudioPercentage * URPPUtility::DataDrawArray.Num());
			//UE_LOG(LogTemp, Warning, TEXT("ViewLocationX: %.02f"), EditorViewportClient->GetViewLocation().X);
			//UE_LOG(LogTemp, Warning, TEXT("AudioCursor: %.02f"), AudioCursor);
			//UE_LOG(LogTemp, Warning, TEXT("AudioPercentage: %.02f"), AudioPercentage);
			//UE_LOG(LogTemp, Warning, TEXT("SnaplineCursor: %i"), SnaplineCursor);
			UpdateCamaraLookAt();
		}
	}






}

void SRPPMain::UpdateCamaraLookAt()
{
	if (EditorViewportClient)
	{
		FVector newLookAt = EditorViewportClient->GetViewLocation();

		float Delta = AudioCursor * RPPPluginManager->RunningSpeed;


		/*change to start of window and end of window*/
		//URPPUtility::RawDrawArrayToDrawArray(SnaplineCursor, NUMBER_OF_LINES_IN_WINDOW + SnaplineCursor);

		URPPUtility::RawDrawArrayToDrawArray(AudioCursor);


		EditorViewportClient->SetViewLocation(FVector(CameraStartingLocation.X + Delta, newLookAt.Y, newLookAt.Z));

		//if (RPPMainCanvas)
		//{
		//	RPPMainCanvas->SetSnapLine();
		//}

	}
}

void SRPPMain::HandleOnAudioPlaybackPercentNative(const UAudioComponent* InAudioComponent, const USoundWave* PlayingSoundWave, const float PlaybackPercent)
{
	//this API returns the playback percent SINCE LAST PLAY/PAUSE
	//e.g. the playback percent resets to 0 whenever paused

	if (!InAudioComponent->bIsPaused)    //
	{
		AudioPercentage = PlaybackPercent + LastPausePercentage;
		AudioCursor = AudioPercentage * AudioDuration;
	}
	else
	{
		LastPausePercentage = AudioPercentage;
	}

	//float PlaybackTime = AudioPercentage * AudioDuration;
	//float PlayerLocation = PlaybackTime * (float)PlayerRunningSpeed;
	//UE_LOG(LogTemp, Warning, TEXT("Percent: %s, PlaybackTime: %s"), *FString::SanitizeFloat(AudioPercentage), *FString::SanitizeFloat(PlaybackTime));
}

void SRPPMain::ChangePlaybackSpeed(float InFloat)
{
	if (AudioComponent)
	{
		AudioComponent->SetPaused(true);
		AudioComponent->SetPitchMultiplier(InFloat);
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("AudioComponent not valid from SRPPMain::ChangePlaybackSpeed"));
	return;
}

void SRPPMain::SetCurrentAsBeatStartingTime()
{
	if (RPPPluginManager)
	{
		RPPPluginManager->BeatStartingTime = AudioCursor;
		URPPUtility::CalculateRawBeatArray(RPPPluginManager->BPM, AudioDuration, RPPPluginManager->BeatStartingTime);
	}
}

void SRPPMain::Initilization()
{

	//other init
	EditorViewportClient = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();

	if (EditorViewportClient)
	{

		OrthoZoom = EditorViewportClient->GetOrthoZoom();

		CameraStartingLocation = EditorViewportClient->GetViewLocation();   //get default location of the camera, mainly for y and z as x will be override by tick shortly

		FEditorDelegates::OnEditorCameraMoved.AddSP(this, &SRPPMain::OnEditorCameraMoved);
//		FEditorDelegates::OnNewActorsDropped.AddSP(this, &SRPPMain::HandleOnNewActorsDropped);

		UWorld* World = EditorViewportClient->GetWorld();
		if (World)
		{
			//TArray<AActor*> foundManager;
			//UGameplayStatics::GetAllActorsOfClass(World, AMySecondPluginManager::StaticClass(), foundManager);

			//if (foundManager.Num() == 1)
			//{
			//	PluginManagerObject = Cast<AMySecondPluginManager>(foundManager[0]);
			//	//plugin manager related settings 
			//	if (PluginManagerObject)
			//	{
			//		SoundWave = (USoundWave*)PluginManagerObject->PluginAudioPlayer->Sound;
			//		if (SoundWave)
			//		{
			//			AudioDuration = SoundWave->Duration;
			//			AudioComponent = PluginManagerObject->PluginAudioPlayer;
			//			AudioComponent->SetPaused(true);
			//			AudioComponent->OnAudioPlaybackPercentNative.AddSP(this, &SRPPMain::HandleOnAudioPlaybackPercentNative);
			//			ProcessSoundWave();
			//		}
			//		ResetViewport();
			//	}
			//}



			TArray<AActor*> foundManager;
			UGameplayStatics::GetAllActorsOfClass(World, ARPPPluginManager::StaticClass(), foundManager);

			if (foundManager.Num() == 1)
			{
				RPPPluginManager = Cast<ARPPPluginManager>(foundManager[0]);
				//plugin manager related settings 
				if (RPPPluginManager)
				{
					if (ValidatePluginManager(RPPPluginManager))
					{
						SoundWave = (USoundWave*)RPPPluginManager->AudioTrack;
						if (SoundWave)
						{
							AudioDuration = SoundWave->Duration;
							AudioComponent = RPPPluginManager->PluginAudioPlayer;
							AudioComponent->SetPaused(true);
							AudioComponent->OnAudioPlaybackPercentNative.AddSP(this, &SRPPMain::HandleOnAudioPlaybackPercentNative);
							URPPUtility::SetWorld(World);
							ProcessSoundWave();
						}
						ResetViewport();
					}
				}
			}
		}
	}
}

void SRPPMain::ProcessSoundWave()
{
	if (SoundWave)
	{
		
		URPPUtility::SetDataRawArray(SoundWave);
		URPPUtility::CalculateRawBeatArray(RPPPluginManager->BPM, SoundWave->Duration, RPPPluginManager->BeatStartingTime);
		URPPUtility::SetEditorViewportClient(EditorViewportClient);
//		URPPUtility::SetPluginManager(PluginManagerObject);

		URPPUtility::SetRPPPluginManager(RPPPluginManager);

		ProcessZoom();


	}
	else
	{
		URPPUtility::ClearEverything();
	}

}

int SRPPMain::GetZoomFactor()
{
	if (SoundWave)
	{
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			EditorViewportClient->Viewport,
			EditorViewportClient->GetWorld()->Scene,
			EditorViewportClient->EngineShowFlags)
			.SetRealtimeUpdate(true));

		FSceneView* SceneView = EditorViewportClient->CalcSceneView(&ViewFamily);

		int32 Width = URPPUtility::WidgetWidth;

		int32 Height = URPPUtility::WidgetHeight;
		
		if (Width && Height)
		{
			FVector WorldLPosTemp;
			FVector WorldDirectTemp;

			FVector WorldRPosTemp;

			SceneView->DeprojectFVector2D(FVector2D(0, Height / 2), WorldLPosTemp, WorldDirectTemp);
			//UE_LOG(LogTemp, Warning, TEXT("Pos: %s, Dir: %s"), *WorldLPosTemp.ToString(), *WorldDirectTemp.ToString());

			SceneView->DeprojectFVector2D(FVector2D(Width, Height / 2), WorldRPosTemp, WorldDirectTemp);
			//UE_LOG(LogTemp, Warning, TEXT("Pos: %s, Dir: %s"), *WorldRPosTemp.ToString(), *WorldDirectTemp.ToString());

			WindowLength = (FMath::Abs(WorldRPosTemp.X - WorldLPosTemp.X)) / RPPPluginManager->RunningSpeed;

			URPPUtility::WindowLength = WindowLength;

			ZoomFactor = WindowLength / (float)SoundWave->Duration * ((float)URPPUtility::DataRawArray.Num()) / NUMBER_OF_LINES_IN_WINDOW;

			if (ZoomFactor >= 1)
			{
				return ZoomFactor;
			}
		}
	}

	return 1;
}

void SRPPMain::ProcessZoom()
{
	ZoomFactor = GetZoomFactor();

	URPPUtility::RawDataArrayToRawDrawArray(ZoomFactor);  //bucket size
	//URPPUtility::RawDrawArrayToDrawArray(0, NUMBER_OF_LINES_IN_WINDOW);  //start, end
	URPPUtility::RawDrawArrayToDrawArray(0);  //start, end

}

void SRPPMain::OnEditorCameraMoved(const FVector& InFVector, const FRotator& InRotator, ELevelViewportType InViewportType, int32 InInt)
{
	UE_LOG(LogTemp, Warning, TEXT("Pos: %s, Dir: %i"), *InFVector.ToString(), InInt);

}

void SRPPMain::HandleOnNewActorsDropped(const TArray<UObject*>& InUObjects, const TArray<AActor*>& InAActors)
{
	UE_LOG(LogTemp, Warning, TEXT("OnNewActorsDropped event"));

	for (auto& Tmp : InAActors)
	{
		ARPPEventBase* NewEventBase = Cast<ARPPEventBase>(Tmp);
		if (NewEventBase)
		{
			UE_LOG(LogTemp, Warning, TEXT("Catch an event actor"));
		}
	}


}

void SRPPMain::HandleOnNewRPPEventPlaced(int32 InRPPID)
{
	UE_LOG(LogTemp, Warning, TEXT("OnNewRPPEventPlaced: %i"), InRPPID);

}

void SRPPMain::HandleOnNewRPPEventRemoved(int32 InRPPID)
{
	UE_LOG(LogTemp, Warning, TEXT("OnNewRPPEventRemoved: %i"), InRPPID);
}

bool SRPPMain::ValidatePluginManager(ARPPPluginManager* InRPPPluginManager)
{
	if (InRPPPluginManager)
	{
		if (!InRPPPluginManager->AudioTrack)
		{
			UE_LOG(LogRPP, Error, TEXT("Audio Track NULL!"))
			return false;
		}
		if (!InRPPPluginManager->RunningSpeed)
		{
			UE_LOG(LogRPP, Warning, TEXT("Running Speed can't be 0."))

			return false;
		}
		bIsValidated = true;
		return true;

	}
	return false;
}

void SRPPMain::ResetViewport()
{
	AudioCursor = 0.f;

	if (EditorViewportClient)
	{
		EditorViewportClient->SetViewportType(ELevelViewportType::LVT_OrthoXZ);
		EditorViewportClient->SetViewMode(EViewModeIndex::VMI_Lit);
		CameraStartingLocation = FVector(RPPPluginManager->GetActorLocation().X, CameraStartingLocation.Y, CameraStartingLocation.Z);
		EditorViewportClient->SetViewLocation(CameraStartingLocation);
	}
}

void SRPPMain::TogglePlay()
{
	//UE_LOG(LogTemp, Warning, TEXT("Play/Pause\n"));

	if (AudioComponent)
	{
		if (AudioComponent->bIsPaused)
		{
			AudioComponent->Play(AudioCursor);
			AudioComponent->SetPaused(!AudioComponent->bIsPaused);
		}
		else
		{
			AudioComponent->SetPaused(!AudioComponent->bIsPaused);
		}
	}
}




END_SLATE_FUNCTION_BUILD_OPTIMIZATION
