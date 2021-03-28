// Copyright 2019 - 2021, Chenkai Zhou, Rhythm Platformer Plugin, All Rights Reserved.



#include "RPPMain.h"

//engine include
#include "Kismet/GameplayStatics.h"
#include "EditorViewportClient.h"
#include "Components/AudioComponent.h"
#include "Editor.h"

//user include
#include "RPPGameModule/Public/RPPEventBase.h"
#include "RPPGameModule/Public/RPPPluginManager.h"
#include "RPP.h"
#include "RPPUtility.h"

#define LOCTEXT_NAMESPACE "FRPPModule"



BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION



void SRPPMain::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::SelfHitTestInvisible);

	URPPUtility::WidgetHeight = InArgs._RPPHeight;

	URPPUtility::WidgetWidth = InArgs._RPPWidth;

	RPPPluginManager = InArgs._ExternalRPPPluginManager;

	ChildSlot
		[
			//construct ButtonVBox
			SNew(SOverlay)
			+SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(FMargin(10.0,50.0,0.0,0.0))
			[
				SAssignNew(RPPButtonVBox, SRPPButtonVBox)
				.RhythmPlatformingPluginMain(this)
			]
			//construct MainCanvas
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(RPPMainCanvas, SRPPMainCanvas)
				.RhythmPlatformingPluginMain(this)
			]
		];



	Initilization();

}

void SRPPMain::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bIsValidated)
	{

		//process zoom
		{
			if (EditorViewportClient->GetOrthoZoom() != OrthoZoom)   //if zoom happened
			{
				OrthoZoom = EditorViewportClient->GetOrthoZoom();
				ProcessZoom();                                       //recalculate zoom and display
			}
		}

		//update look at
		{ 
			//if audio is paused, use viewport camera to update audio percentage
			if ((AudioComponent->bIsPaused) /*&& (AudioPercentage <= 1)*/)
			{
				AudioCursor = (EditorViewportClient->GetViewLocation().X - CameraStartingLocation.X) / RPPPluginManager->RunningSpeed;
				AudioPercentage = AudioCursor / AudioDuration;
			}

			//at last, pass updated AudioPercentage to update lookat
			SnaplineCursor = (AudioPercentage * URPPUtility::AudioDataDrawArray.Num());

			UpdateCamaraLookAt();
		}
	}

}

void SRPPMain::UpdateCamaraLookAt()
{
	//client could be null
	if (EditorViewportClient)
	{
		FVector newLookAt = EditorViewportClient->GetViewLocation();

		//calculate the offset from 0 to current location in UE unit
		//e.g. AudioCursor = 3, RunningSpeed = 600; delta = 1800
		float Delta = AudioCursor * RPPPluginManager->RunningSpeed;

		//resample audio draw array based on new audio location
		URPPUtility::RawAudioDrawArrayToAudioDrawArray(AudioCursor);

		//set lookat location
		EditorViewportClient->SetViewLocation(FVector(CameraStartingLocation.X + Delta, newLookAt.Y, newLookAt.Z));

	}
}

void SRPPMain::HandleOnAudioPlaybackPercentNative(const UAudioComponent* InAudioComponent, const USoundWave* PlayingSoundWave, const float PlaybackPercent)
{
	//ue4 api
	if (InAudioComponent->bIsPaused)    //while audio is paused, use cached data
	{
		LastPausePercentage = AudioPercentage;
	}
	else
	{
		AudioPercentage = PlaybackPercent;   //else use feed in data
		AudioCursor = AudioPercentage * AudioDuration;
	}

	//playback time in second
	float PlaybackTime = AudioPercentage * AudioDuration;
	//calculate offset based on running speed and playback time
	float PlayerLocation = PlaybackTime * (float)RPPPluginManager->RunningSpeed;
	//UE_LOG(LogRPP, Warning, TEXT("Percent: %s, PlaybackTime: %s"), *FString::SanitizeFloat(AudioPercentage), *FString::SanitizeFloat(PlaybackTime));
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
		//default zoom level
		OrthoZoom = EditorViewportClient->GetOrthoZoom();

		//default camera location
		CameraStartingLocation = EditorViewportClient->GetViewLocation();   //get default location of the camera, mainly for y and z as x will be override by tick shortly

		//FEditorDelegates::OnEditorCameraMoved.AddSP(this, &SRPPMain::OnEditorCameraMoved);

		UWorld* World = EditorViewportClient->GetWorld();
		if (World)
		{
			if (RPPPluginManager)
			{
				if (ValidatePluginManager(RPPPluginManager))  //if plugin is valid
				{
					SoundWave = (USoundWave*)RPPPluginManager->AudioTrack;  //get soundtrack
					if (SoundWave)
					{
						AudioDuration = SoundWave->Duration;
						AudioComponent = RPPPluginManager->PluginAudioPlayer;
						AudioComponent->SetSound(SoundWave);
						AudioComponent->SetPaused(true);
						AudioComponent->SetUISound(true);
						AudioComponent->OnAudioPlaybackPercentNative.AddSP(this, &SRPPMain::HandleOnAudioPlaybackPercentNative);
						URPPUtility::SetWorld(World);   //pass world to RPPUtility
						ProcessSoundWave();             //read-in, compress and cache audio data
					}
					ResetViewport();
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

			SceneView->DeprojectFVector2D(FVector2D(Width, Height / 2), WorldRPosTemp, WorldDirectTemp);

			WindowLength = (FMath::Abs(WorldRPosTemp.X - WorldLPosTemp.X)) / RPPPluginManager->RunningSpeed;

			URPPUtility::WindowLength = WindowLength;

			ZoomFactor = WindowLength / (float)SoundWave->Duration * ((float)URPPUtility::AudioDataRawArray.Num()) / NUMBER_OF_LINES_IN_WINDOW;

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

	URPPUtility::RawAudioDataArrayToRawAudioDrawArray(ZoomFactor);  //bucket size
	URPPUtility::RawAudioDrawArrayToAudioDrawArray(0);  //start, end

}

//void SRPPMain::OnEditorCameraMoved(const FVector& InFVector, const FRotator& InRotator, ELevelViewportType InViewportType, int32 InInt)
//{
//	UE_LOG(LogTemp, Warning, TEXT("Pos: %s, Dir: %i"), *InFVector.ToString(), InInt);
//
//}

//void SRPPMain::HandleOnNewActorsDropped(const TArray<UObject*>& InUObjects, const TArray<AActor*>& InAActors)
//{
//	UE_LOG(LogTemp, Warning, TEXT("OnNewActorsDropped event"));
//
//	for (auto& Tmp : InAActors)
//	{
//		ARPPEventBase* NewEventBase = Cast<ARPPEventBase>(Tmp);
//		if (NewEventBase)
//		{
//			UE_LOG(LogTemp, Warning, TEXT("Catch an event actor"));
//		}
//	}
//}

/*
* ensure all vital parameter are set (soundtrack and player running speed)
* 
*/
bool SRPPMain::ValidatePluginManager(ARPPPluginManager* InRPPPluginManager)
{
	if (InRPPPluginManager)
	{
		//check sound track
		if (!InRPPPluginManager->AudioTrack)
		{
			UE_LOG(LogRPP, Error, TEXT("Audio Track NULL!"));

			// Put your "OnButtonClicked" stuff here
			FText DialogText = FText::Format(
				LOCTEXT("FRPPModule", "Audio Track is not set!"),
				FText::FromString(TEXT("SRPPMain::ValidatePluginManager")),
				FText::FromString(TEXT("RPPMain.cpp"))
			);
			FMessageDialog::Open(EAppMsgType::Ok, DialogText);

			return false;
		}
		//check running speed
		if (!InRPPPluginManager->RunningSpeed)
		{
			UE_LOG(LogRPP, Warning, TEXT("Running Speed can't be 0."));

			FText DialogText = FText::Format(
				LOCTEXT("FRPPModule", "Running Speed can't be 0!"),
				FText::FromString(TEXT("SRPPMain::ValidatePluginManager")),
				FText::FromString(TEXT("RPPMain.cpp"))
			);
			FMessageDialog::Open(EAppMsgType::Ok, DialogText);

			return false;
		}
		bIsValidated = true;
		return true;

	}
	return false;
}

//reset viewport to default positon (to the beginning of the sound track)
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

#undef LOCTEXT_NAMESPACE 


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
