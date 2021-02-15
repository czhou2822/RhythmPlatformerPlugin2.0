// Copyright Chenkai Zhou. All Rights Reserved.



#pragma once

//engine include
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Sound/SoundWave.h"

//user include
#include "RPPUtility.generated.h"


UCLASS()
class MYSECONDPLUGIN_API URPPUtility : public UObject
{
	GENERATED_BODY()

private:

	static const int32 NUMBER_OF_LINES_IN_WINDOW = 20000;

public:
	//waveform-related draw arrays
	//wav data were loaded from file to DataRawArray
	//program then process DataRawArray to DataDrawArray (such as average peak value), served as a base to DrawArray
	static TArray<float> DataRawArray;    //Raw data from wave file   

	static TArray<float> DataDrawArray;   //RawDrawArray processed from RawDataArray, contain (RawDataArray.size()/bucketsize) elements

	static TArray<FVector2D> DrawArray;	//DrawArray

	//beat grid-related arrays
	static TArray<float> BeatRawArray;    //Raw beat info; elements in second. for example, if BPM = 60, it shoud be [0,1,2,3,4,5...]
	
	static TArray<FVector2D> BeatDrawArray;	//BeatArray

	static class ARPPPluginManager* RPPPluginManager;

	static int32 WidgetWidth;

	static int32 WidgetHeight;

	static float WindowLength; //window length in second

	static float AudioDuration; //window length in second

	static UWorld* World;

	static FEditorViewportClient* EditorViewportClient;




private:
	static TArray<float> WaveToRawDataArray(USoundWave* SoundWave);


public:
	URPPUtility();

	static void SetEditorViewportClient(FEditorViewportClient* InEditorViewportClient);

	static void SetDataRawArray(USoundWave* SoundWave);

	FORCEINLINE const TArray<FVector2D>& GetDrawArray() { return DrawArray; }

	static void RawDataArrayToRawDrawArray(int BucketSize);

	static void RawDrawArrayToDrawArray(float CurrentCursor);


	/*
	generates elements for RawBeatArray
	e.g., if bpm = 60, RawBeatArray = {0,1,2,3}
	*/
	static void CalculateRawBeatArray(const float& InBPM, const float& InAudioDuation, const float& InBeatStartingTime = 0.0f);

	static void RefreshRunSpeed();

	static void ClearEverything();

	static void TestFunction();

	static void SetWorld(UWorld* InWorld);

	static float WorldSpaceToAudioCursor(FVector InLocation, UWorld* InWorld);

	static void SetRPPPluginManager(ARPPPluginManager* InRPPPluginManager);



};
