// Copyright 2019 - 2021, Chenkai Zhou, Rhythm Platformer Plugin, All Rights Reserved.



#pragma once

//engine includes
#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "Widgets/SCompoundWidget.h"


//user includes
#include "RPPButtonVBox.h"
#include "RPPMainCanvas.h"


/**
 * 
 */
class RPP_API SRPPMain : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRPPMain)
	{}
	SLATE_ARGUMENT(float, RPPWidth)  //pass in widget width
		SLATE_ARGUMENT(float, RPPHeight) //pass in widget height
		SLATE_ARGUMENT(class ARPPPluginManager*, ExternalRPPPluginManager) //pass in plugin manager found in scene
		SLATE_END_ARGS()

		/** Constructs this widget with InArgs */


private:

	
	bool bIsValidated = false;  //whether plugin in running correctly, if not, Tick will be return

	float OrthoZoom = 0.f;

	class URPPUtility* RPPUtil;

	class UAudioComponent* AudioComponent;
 
	float AudioPercentage = 0.f;  //percentage of audio played.

	bool bIsPlaying = false;      //is track playing

	float LastPausePercentage = 0.f;  //played percentage since last paused

	int32 SnaplineCursor = 0; //index of the RawDrawArray / current location in the DrawArray

	const int32 NUMBER_OF_LINES_IN_WINDOW = 10000; //how many lines we used to construct waveform

	int32 ZoomFactor = 5;      //how many samples in each bucket, for example, if this is 32, RawDrawArray takes ONE peak value out of 32 in RawDataArray

	FVector CameraStartingLocation = FVector(0, 0, 0);

	USoundWave* SoundWave = nullptr;


public:
	TSharedPtr<class SRPPButtonVBox> RPPButtonVBox;

	TSharedPtr<class SRPPMainCanvas> RPPMainCanvas;

	class FEditorViewportClient* EditorViewportClient;

	class ARPPPluginManager* RPPPluginManager;

	float AudioCursor = 0.f;    //playtime of the track, in seconds

	float WindowLength = 0.f;	//how many time repersented in width 

	float AudioDuration = 0.f;  


private:
	//called every frame
	void UpdateCamaraLookAt();

	void HandleOnAudioPlaybackPercentNative(const class UAudioComponent* AudioComponent, const class USoundWave* PlayingSoundWave, const float PlaybackPercent);


public:

	void Construct(const FArguments& InArgs);

	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime); // called everyframe and used for our gamelogic

	//plugin init
	void Initilization();

	//reset viewport, going back to the beginning of the level
	void ResetViewport();

	void TogglePlay();

	void ChangePlaybackSpeed(float InFloat);

	void SetCurrentAsBeatStartingTime();

	//process soundwave in PluginObject->audiocomponent
	void ProcessSoundWave();

	//called when viewport zoomed
	int GetZoomFactor();

	void ProcessZoom();

	//void OnEditorCameraMoved(const FVector& InFVector, const FRotator& InRotator, ELevelViewportType InViewportType, int32 InInt);

	//void HandleOnNewActorsDropped(const TArray<UObject*>& InUObjects, const TArray<AActor*>& InAActors);

	bool ValidatePluginManager(ARPPPluginManager* RPPPluginManager);

};
