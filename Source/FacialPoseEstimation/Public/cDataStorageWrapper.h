// Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "cDataStorageWrapper.generated.h"


/**  Struct to pass transform datafrom DLL  */
struct TransformData
{
	TransformData(float tx, float ty, float tz, float rfx, float rfy,
		float rfz, float rux, float ruy, float ruz) :
		tX(tx), tY(ty), tZ(tz), rfX(rfx), rfY(rfy),
		rfZ(rfz), ruX(rux), ruY(ruy), ruZ(ruz) {}

	float tX, tY, tZ;
	float rfX, rfY, rfZ;
	float ruX, ruY, ruZ;
};


/** DLL functions */
typedef int(*__Init)(int& outCameraWidth, int& outCameraHeight, int detectRatio,
	int camId, float fovZoom, bool draw, bool lockEyesNose);
typedef void(*__Close)();
typedef int(*__GetImage)(unsigned char* data, int width, int height);
typedef void(*__Detect)(TransformData& outFaces, float* outExpression);


/** Wrapper for external DLL, executes pose estimation pipeline and passes the data back to Unreal */
UCLASS()
class FACIALPOSEESTIMATION_API UcDataStorageWrapper : public UObject
{
	GENERATED_BODY()
private:

	/** DLL Handle */
	void *v_dllHandle;

	/** DLL Functions */
	__Init m_funcInit;
	__Close m_funcClose;
	__GetImage m_funcGetRawImageBytes;
	__Detect m_funcDetect;

public:

	/**
	* Attempt to import DLL.
	* @param a_strFolderName -  Folder of DLL.
	* @param a_strDLLName - Name of DLL file.
	* @return Whether the operation is succesfull.
	*/
	bool ImportDLL(FString a_strFolderName, FString a_strDLLName);

	/**
	* Attempt to import all functions of the DLL.
	* @return Whether the operation is succesfull.
	*/
	bool ImportMethods();

	/**
	* Call DLL - Initiate OpenCV camera stream and Neural Networks.
	* @param outCameraWidth - Width which OpenCV used for camera stream.
	* @param outCameraHeight - Height which OpenCV used for camera stream.
    * @param detectRatio - ratio to scale image by for initial face detection
	* @param camId - Which camera id OpenCV should try to use.
	* @param inFovZoom - Zoom amount for pinhole camera, to match Unreal.
	* @param draw - Wheher or not to draw technical indicators over frame.
	* @param lockEyesNose - Whether to lock eye and nose points for PnP solve.
	* @return Whether operation is succesful.
	*/
	int CallInitCV(int& outCameraWidth, int& outCameraHeight, int detectRatio,
		int camId, float fovZoom, bool draw, bool lockEyesNose);

	/**
	* Call DLL - Close OpenCV connection to camera.
	*/
	int CallCloseCV();

	/**
	* Call DLL - Exectute whole facial pose estimation pipeline, and return result.
	* @param out_transform - Pointer where result transform is copied to.
	* @param out_expression - Pointer where result blendshapes are copied to.
	*/
	int CallDetect(TransformData& outTransform, float* outExpression);

	/**
	* Call DLL - Get single frame from OpenCV camera stream, resize and reformat for Unreal.
	* @param image - Pointer to write OpenCV image to.
	* @param width - Resize width.
	* @param height - Resize height.
	* @return Whether operation is succesful.
	*/
	int CallGetImageCV(unsigned char* image, int width, int height);
};






