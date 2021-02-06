// Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ArFaceRig.generated.h"


/**
* Pawn class which runs AR facial pose estimation on live video stream.
* Contains a camera, face model, and an image plane.
* The camera position and fov, also the image plane placement are automated.
* The face positions and expression updates each tick.
*/
UCLASS()
class FACIALPOSEESTIMATION_API AArFaceRig : public APawn
{
	GENERATED_BODY()

public:

	AArFaceRig();

	/** Face mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ARFace | Geo")
	class USkeletalMeshComponent* FaceMesh;

	/** Face blendshapes */
	TArray<FString> BlendShapeArray;

	/** Custom camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ARFace | Camera" )
	class UCameraComponent* FaceCamera;

	/** Mesh to render camera stream texture on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | Geo")
	class UStaticMeshComponent* PlaneMesh;

	/** Material to override camera stream texture on */
	UPROPERTY(EditDefaultsOnly, Category = Materials)
	class UMaterialInstance* MasterMaterialRef;

	/** Matrix transforms */
	FMatrix Mat;
	FMatrix MatB;

	/** Prev frame transforms for blending */
	FVector PrevPosition;
	FRotator PrevRotation;

	/** Prev frame blendshapes for blending */
	float BlendValues[51];
	float PrevBlendValues[51];

	/** Face scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | Geo")
	float FaceScale;

	/** Face motion blending parameters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | Motion")
	float Momentum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | Motion")
	float Blend;

	/** Resolution attained by OpenCV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	int outCameraWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	int outCameraHeight;

	/** Downscale ratio for face detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	int detectRatio;
	
	/** Which camera to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	int camId;

	/** Zoom level to use for camera/solve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	float fovZoom;

	/** Whether to draw techincal indicators on background image */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	bool draw;

	/** Whether to lock eyes and nose during PnP solve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ArFace | OpenCV")
	bool lockEyesNose;


protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Set blendshapes on face mesh - called once each tick.
	 * @param Blendshapes - Array of 51 blend values.
	 */
	void SetBlendShapes(float* Blendshapes);

	/**
	 * Set transforms of face mesh - called once each tick.
	 * @param Up - Up vector.
	 * @param Forward - Forward vector.
	 * @param Translation - Translatin vector.
	 */
	void SetTransforms(FVector Up, FVector Forward, FVector Translation);

	/**
	 * Set texture on background plane - called once each tick.
	 * @param Image - TArray to hold pixel values
	 */
	void SetBackground(TArray<unsigned char, TFixedAllocator<512 * 512 * 4>> Image);

	/**
	 * Executes whole pose estimation pipeline - called once each tick.
	 */
	void RunDLL();
};
