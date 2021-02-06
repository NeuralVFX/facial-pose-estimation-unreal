// // Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#include "cDataStorageWrapper.h"
#include "Misc/Paths.h"


bool UcDataStorageWrapper::ImportDLL(FString FolderName, FString DLLName)
{
	// Init DLL from a Path
	FString FilePath = *FPaths::ProjectPluginsDir() + FolderName + "/" + DLLName;
	FString EnginePath = *FPaths::EngineDir();

	if (FPaths::FileExists(FilePath))
	{
		v_dllHandle = FPlatformProcess::GetDllHandle(*FilePath);
		if (v_dllHandle != NULL)
		{
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed DLL Import"));
		}
	}
	return false;
}


bool UcDataStorageWrapper::ImportMethods()
{
	// Loop Through and Import All Functions from DLL   --   Make Sure proc_name matches name of DLL method
	if (v_dllHandle != NULL)
	{
		FString ProcName = "Init";
		m_funcInit = (__Init)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_funcInit == NULL)
		{
			return false;
		}
		ProcName = "Close";
		m_funcClose = (__Close)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_funcClose == NULL)
		{
			return false;
		}
		ProcName = "GetRawImageBytes";
		m_funcGetRawImageBytes = (__GetImage)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_funcGetRawImageBytes == NULL)
		{
			return false;
		}
		ProcName = "Detect";
		m_funcDetect = (__Detect)FPlatformProcess::GetDllExport(v_dllHandle, *ProcName);
		if (m_funcDetect == NULL)
		{
			return false;
		}
	}
	return true;
}


int UcDataStorageWrapper::CallInitCV(int& outCameraWidth, int& outCameraHeight,
	int detectRatio, int camId, float fovZoom, bool draw, bool lockEyesNose)
{
	// Check if DLL function is loaded
	if (m_funcInit == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Function Not Loaded From DLL: InitNet"));
		return INT_MIN;
	}

	// Calls DLL function to activate camera and neural nets
	int init = m_funcInit(outCameraWidth,
		outCameraHeight,
		detectRatio,
		camId,
		fovZoom,
		draw,
		lockEyesNose);

	UE_LOG(LogTemp, Error, TEXT("OpenCV Connection Opened %d"), init);

	return 1;
}


int UcDataStorageWrapper::CallCloseCV()
{
	// Check if DLL function is loaded
	if (m_funcClose == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Function Not Loaded From DLL: CloseNet"));
		return INT_MIN;
	}

	// Calls DLL function to shut down camera
	m_funcClose();
	UE_LOG(LogTemp, Log, TEXT("OpenCV Connection Close"));

	return 1;
}


int UcDataStorageWrapper::CallGetImageCV(unsigned char* Image, int width, int height)
{
	// Check if DLL function is loaded
	if (m_funcGetRawImageBytes == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Function Not Loaded From DLL: Get Image "));
		return INT_MIN;
	}

	// Calls DLL function to get image from OpenCV stream
	int result = m_funcGetRawImageBytes(Image, width, height);

	return 1;
}


int UcDataStorageWrapper::CallDetect(TransformData& outTransform, float* outExpression)
{
	// Check if DLL function is loaded
	if (m_funcGetRawImageBytes == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Function Not Loaded From DLL: Get Transform "));
		return INT_MIN;
	}

	// Calls DLL function to exectute facial pose estimation for one frame
	m_funcDetect(outTransform, outExpression);

	return 1;
}
