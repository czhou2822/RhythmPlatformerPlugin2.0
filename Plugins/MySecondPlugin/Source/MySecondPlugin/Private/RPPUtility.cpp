// Copyright Chenkai Zhou. All Rights Reserved.

#include "RPPUtility.h"

#include "EditorViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"



#include "RPPGameModule/Public/RPPPluginManager.h"
#include "RPPGameModule/Public/RPPEventBase.h"


TArray<float> URPPUtility::DataRawArray;    //Raw data from wave file   
TArray<float> URPPUtility::DataDrawArray;   //RawDrawArray processed from RawDataArray, contain (RawDataArray.size()/bucketsize) elements
TArray<FVector2D> URPPUtility::DrawArray;	//DrawArray

TArray<float> URPPUtility::BeatRawArray{ 0.f, 1.f, 2.f, 3.f, 4.f,5.f, 6.f, 7.f, 8.f, 9.f, 10.f };    //Raw beat info; elements in second. for example, if BPM = 60, it shoud be [0,1,2,3,4,5...]
TArray<FVector2D> URPPUtility::BeatDrawArray;	//BeatArray

FEditorViewportClient* URPPUtility::EditorViewportClient = nullptr;

int32 URPPUtility::WidgetHeight = 0;

int32 URPPUtility::WidgetWidth = 0;

float URPPUtility::WindowLength = 0.0f;

float URPPUtility::AudioDuration = 0.0f;

ARPPPluginManager* URPPUtility::RPPPluginManager = nullptr;

UWorld* URPPUtility::World = nullptr;


URPPUtility::URPPUtility()
{

	if (GetWorld())
	{

	}

}

void URPPUtility::SetEditorViewportClient(FEditorViewportClient* InEditorViewportClient)
{
	if (InEditorViewportClient)
	{
		EditorViewportClient = InEditorViewportClient;
	}
}
/**
 Extract data from USoundWave and put them into RawDataArray
 @param Soundwave, the Soundwave pointer
 @return TArray containing raw data from the USoundWave object.
*/
TArray<float> URPPUtility::WaveToRawDataArray(USoundWave* SoundWave)
{
	TArray<float> Output{};
	const int32 NumChannels = SoundWave->NumChannels;

	if ((NumChannels == 0) || (SoundWave->RawData.GetBulkDataSize() == 0))
	{
		return Output;
	}

	uint8* RawWaveData = (uint8*)SoundWave->RawData.Lock(LOCK_READ_ONLY);
	int32 RawDataSize = SoundWave->RawData.GetBulkDataSize();
	FWaveModInfo WaveInfo;

	if (WaveInfo.ReadWaveHeader(RawWaveData, RawDataSize, 0))
	{
		uint32 SampleCount = WaveInfo.SampleDataSize / (2 * NumChannels);

		const int16* SamplePtr = reinterpret_cast<const int16*>(WaveInfo.SampleDataStart);

		for (uint32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
		{
			int64 SampleSum = 0;
			for (int32 ChannelIndex = 0; ChannelIndex < NumChannels; ++ChannelIndex)
			{
				SampleSum += (*SamplePtr);
				SamplePtr++;
			}
			SampleSum = SampleSum / (float)NumChannels;
			Output.Add(SampleSum);
		}
	}

	SoundWave->RawData.Unlock();

	return Output;
}

void URPPUtility::SetDataRawArray(USoundWave* SoundWave)
{
	if (SoundWave)
	{
		DataRawArray = WaveToRawDataArray(SoundWave);
	}
}

/*
 Extract raw data from RawDataArray and put them into RawDrawArray
 @param InputArray, the RawDataArray from USoundWave
 @param BucketSize, we only push the absolute max value in the bucket into the DrawArray.
 Therefore, by default, the size of RawDrawArray is (InputArray.size()/200)
*/
void URPPUtility::RawDataArrayToRawDrawArray(int BucketSize)
{
	DataDrawArray.Empty();

	//float YOffset = Padding + BorderHeight / 2.f;

	if (WidgetHeight == 0 || WidgetWidth == 0)
	{
		return;
	}

	//if (DrawArray.Num() != 0)
	//{
	//	return;
	//}

	float YOffset = WidgetHeight/2;

	int InputArrayIndex = 0;
	float AbsMax = 0;
	int NumberOfBuckets = (DataRawArray.Num() - (DataRawArray.Num() % BucketSize)) / BucketSize;
	if (DataRawArray.Num())
	{
		AbsMax = DataRawArray[0];
		for (int i = 0; i < DataRawArray.Num(); i++)
		{
			if (FMath::Abs(DataRawArray[i]) > AbsMax)
			{
				AbsMax = DataRawArray[i];
			}
		}
	}
	else
	{
		return;
	}


	float YScale = WidgetHeight / (2 * AbsMax);

	for (int i = 0; i < NumberOfBuckets; i++)
	{
		float Temp = 0;

		for (int j = 0; j < BucketSize; j++)      //absmax with bucket
		{
			if (FMath::Abs(DataRawArray[InputArrayIndex]) > Temp)
			{
				Temp = DataRawArray[InputArrayIndex];
			}
			InputArrayIndex++;
		}
		Temp = YOffset - (Temp * YScale);
		DataDrawArray.Add(Temp);
	}


}


void URPPUtility::RawDrawArrayToDrawArray(float CurrentCursor)
{

	/*
	* get upper bound and lower bound, draw betweeb
	* 
	* need to solve edge case
	*/

	if (CurrentCursor >= AudioDuration || CurrentCursor < -(WindowLength * 0.5)) return;


	float XCord = 0.f;

	float UnitPerSecond = WidgetWidth / WindowLength; //how many units(pixels) repersents a second.

	float Left = 0.0f;
	//left condition
	if (CurrentCursor - WindowLength * 0.5 >= 0)
	{
		Left = CurrentCursor - WindowLength * 0.5;
	}
	else
	{
		XCord = FMath::Abs(CurrentCursor - WindowLength * 0.5) * UnitPerSecond;
	}

	float Right = CurrentCursor + WindowLength * 0.5;


	int LeftIndex = Left / AudioDuration * DataDrawArray.Num();
	int RightIndex = Right / AudioDuration * DataDrawArray.Num();

	LeftIndex = FMath::Clamp<int>(LeftIndex, 0, DataDrawArray.Num()-1);
	RightIndex = FMath::Clamp<int>(RightIndex, 0, DataDrawArray.Num()-1);


	float LineStepSize = (WidgetWidth - XCord) / (RightIndex - LeftIndex); //


	DrawArray.Empty();


	while (LeftIndex < RightIndex && LeftIndex<DataDrawArray.Num())
	{
		DrawArray.Add(FVector2D(XCord, DataDrawArray[LeftIndex]));
		XCord += (float)LineStepSize;
		LeftIndex++;
	}

	return;
}

/*
calculatess beat array. e.g. if bpm = 60. BeatRawArray = {0,1,2,3 ...}
*/
void URPPUtility::CalculateRawBeatArray(const float& InBPM, const float& InAudioDuration, const float& InBeatStartingTime)
{
	URPPUtility::BeatRawArray.Empty();

	if (InBPM)
	{
		URPPUtility::BeatRawArray.Add(0.f);

		URPPUtility::AudioDuration = InAudioDuration;

		float NextBeatTime = 0.f + InBeatStartingTime;

		float BeatInveral = 60.f / InBPM;

		while (NextBeatTime <= InAudioDuration)
		{
			URPPUtility::BeatRawArray.Add(NextBeatTime);
			NextBeatTime += BeatInveral;
		}
	}

	return;
}

float URPPUtility::WorldSpaceToAudioCursor(FVector InLocation, UWorld* InWorld)
{

	if (URPPUtility::RPPPluginManager)
	{
		float Delta = InLocation.X - URPPUtility::RPPPluginManager->GetActorLocation().X;

		float AudioCursor = Delta / URPPUtility::RPPPluginManager->RunningSpeed;

		UE_LOG(LogTemp, Warning, TEXT("Delta: %s, AudioCursor: %s"), *FString::SanitizeFloat(Delta, 2), *FString::SanitizeFloat(AudioCursor, 2));

		
		return AudioCursor;
	}
	return 0.0f;

}

void URPPUtility::RefreshRunSpeed()
{
	if (URPPUtility::World && URPPUtility::RPPPluginManager)
	{
		TSubclassOf<ARPPEventBase> classToFind;
		classToFind = ARPPEventBase::StaticClass();
		TArray<AActor*> FoundMarkers;
		UGameplayStatics::GetAllActorsOfClass(World, classToFind, FoundMarkers);

		FVector SpawnLocation = FVector(0, 0, 0);
		for (AActor* SingleActor : FoundMarkers)
		{
			ARPPEventBase* EventTemp = Cast<ARPPEventBase>(SingleActor);

			if (EventTemp)
			{
				SpawnLocation.X = EventTemp->AudioLocation * URPPUtility::RPPPluginManager->RunningSpeed;
				SpawnLocation.Z = SingleActor->GetActorLocation().Z;

				SingleActor->SetActorLocation(SpawnLocation);
			}
				
		}

	}
}

void URPPUtility::ClearEverything()
{
	URPPUtility::DataRawArray.Empty();    
	URPPUtility::DataDrawArray.Empty();   
	URPPUtility::DrawArray.Empty();	

	URPPUtility::BeatRawArray.Empty();    
	URPPUtility::BeatDrawArray.Empty();	


	URPPUtility::EditorViewportClient = nullptr;

	URPPUtility::RPPPluginManager = nullptr;


}

void URPPUtility::TestFunction()
{
	UE_LOG(LogTemp, Warning, TEXT("rpputil testfunction called"));
}

void URPPUtility::SetWorld(UWorld* InWorld)
{
	if (InWorld)
	{
		URPPUtility::World = InWorld;
	}
}

void URPPUtility::SetRPPPluginManager(ARPPPluginManager* InRPPPluginManager)
{
	if (InRPPPluginManager)
	{
		URPPUtility::RPPPluginManager = InRPPPluginManager;

	}
}




