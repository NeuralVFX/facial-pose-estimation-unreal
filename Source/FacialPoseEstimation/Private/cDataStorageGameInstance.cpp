// Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#include "cDataStorageGameInstance.h"


void UcDataStorageGameInstance::Init()
{
	// Init DLL
	Super::Init();
	if (ImportDataStorageLibrary())
	{
		UE_LOG(LogTemp, Log, TEXT("OpenCV DLL Loaded"));

	}
}


bool UcDataStorageGameInstance::ImportDataStorageLibrary()
{
	// Import the DLL
	m_refDataStorageUtil = NewObject<UcDataStorageWrapper>(this);
	if (m_refDataStorageUtil == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Create the Data Storage Object"));
		return false;
	}

	if (!m_refDataStorageUtil->ImportDLL("facial-pose-estimation-unreal/Binaries/Win64",
		"facial-pose-estimation-libtorch.dll"))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Import DLL"));
		return false;
	}
	// Import all methods of DLL
	if (!m_refDataStorageUtil->ImportMethods())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to Import DLL Methods"));
		return false;
	}
	return true;
}


void UcDataStorageGameInstance::Shutdown()
{
	int Result = m_refDataStorageUtil->CallCloseCV();
	Super::Shutdown();
	UE_LOG(LogTemp, Log, TEXT("Release Camera"))
}


void UcDataStorageGameInstance::CustomStart(int&outCameraWidth, int&outCameraHeight,
	int detectRatio, int camId, float fovZoom, bool draw, bool lockEyesNose)
{
	int Result = m_refDataStorageUtil->CallInitCV(outCameraWidth,
		outCameraHeight,
		detectRatio,
		camId,
		fovZoom,
		draw,
		lockEyesNose);

	UE_LOG(LogTemp, Log, TEXT("Opened Camera"));
}


void UcDataStorageGameInstance::GetImage(unsigned char* image, int width, int height)
{
	int Result = m_refDataStorageUtil->CallGetImageCV(image, width, height);
}



void UcDataStorageGameInstance::GetTransform(TransformData& outFaces, float* outExpression)
{
	int Result = m_refDataStorageUtil->CallDetect(outFaces, outExpression);
}

