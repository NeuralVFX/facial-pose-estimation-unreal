// Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "cDataStorageWrapper.h"
#include "cDataStorageGameInstance.generated.h"

/** Struct to hold attribute data needed to initialize DLL */
struct AttributeData
{
	int outCameraWidth, outCameraHeight, detectRatio, camId;
	float fovZoom;
	bool draw, lockEyesNose;
};


/** Game Instance which is responsible for loading and calling DLL wrapper */
UCLASS()
class FACIALPOSEESTIMATION_API UcDataStorageGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:

	/** Storage for DLL object */
	UPROPERTY()
	class UcDataStorageWrapper* m_refDataStorageUtil;

	/**
	* Attempt to import DLL and all of its functions.
	* @return Whether the operation is succesfull.
	*/
	bool ImportDataStorageLibrary();

public:

	virtual void Init() override;

	/**
	* Call DLL Wrapper - Initiate OpenCV camera stream and Neural Networks.
	* @param outCameraWidth - Width which OpenCV used for camera stream.
	* @param outCameraHeight - Height which OpenCV used for camera stream.
	* @param detectRatio - ratio to scale image by for initial face detection
	* @param camId - Which camera id OpenCV should try to use.
	* @param inFovZoom - Zoom amount for pinhole camera, to match Unreal.
	* @param draw - Wheher or not to draw technical indicators over frame.
	* @param lockEyesNose - Whether to lock eye and nose points for PnP solve.
	*/
	void CustomStart(int& outCameraWidth, int& outCameraHeight, int detectRatio, int camId, float fovZoom, bool draw, bool lockEyesNose);

	/**
	* Call DLL Wrapper - Close OpenCV connection to camera.
	*/
	virtual void Shutdown() override;

	/**
	* Call DLL Wrapper - Get single frame from OpenCV camera stream, resize and reformat for Unreal.
	* @param image - Pointer to write OpenCV image to.
	* @param width - Resize width.
	* @param height - Resize height.
	*/
	void GetImage(unsigned char* image, int width, int height);

	/**
	* Call DLL Wrapper - Exectute whole facial pose estimation pipeline, and return result.
	* @param outTransform - Pointer where result transform is copied to.
	* @param outExpression - Pointer where result blendshapes are copied to.
	*/
	void GetTransform(TransformData& outTransform, float* outExpression);

};
