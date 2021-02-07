// Copyright 2020 NeuralVFX, Inc. All Rights Reserved.

#include "ArFaceRig.h"
#include "Camera/CameraComponent.h"
#include "cDataStorageWrapper.h"
#include "cDataStorageGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h" 
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "Kismet/KismetMathLibrary.h"


AArFaceRig::AArFaceRig()
{
	PrimaryActorTick.bCanEverTick = true;

	// Our own camera
	FaceCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FaceCamera"));
	FaceCamera->bUsePawnControlRotation = false;
	SetRootComponent(FaceCamera);

	// Our face mesh
	FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));

	// Create and set plane mesh component
	FString PlaneStr = "StaticMesh'/Engine/BasicShapes/Plane.Plane'";
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>PlaneMeshAsset(*PlaneStr);
	PlaneMesh->SetStaticMesh(PlaneMeshAsset.Object);

	// Fetch viewer material from scene
	FString ViewMatStr = "Material'/FacialPoseEstimation/Assets/ViewerMaterial.ViewerMaterial'";
	static ConstructorHelpers::FObjectFinder<UMaterial>MaterialAsset(*ViewMatStr);
	PlaneMesh->SetMaterial(0, MaterialAsset.Object);

	// Set default matrices - used to convert OpenCV matrix to Unreal
	Mat = FMatrix::Identity;
	Mat.M[0][0] = 1, Mat.M[0][1] = 0, Mat.M[0][2] = 0;
	Mat.M[1][0] = 0, Mat.M[1][1] = 0, Mat.M[1][2] = 1;
	Mat.M[2][0] = 0, Mat.M[2][1] = -1, Mat.M[2][2] = 0;

	MatB = FMatrix::Identity;
	MatB.M[0][0] = 0, MatB.M[0][1] = 1, MatB.M[0][2] = 0;
	MatB.M[1][0] = 0, MatB.M[1][1] = 0, MatB.M[1][2] = 1;
	MatB.M[2][0] = -1, MatB.M[2][1] = 0, MatB.M[2][2] = 0;

	// Face scale
	FaceScale = 5.7;

	// Momentum smoothing parameters
	Momentum = 1.2;
	Blend = .5;

	// Transform blend values
	PrevRotation = FRotator(0, 0, 0);
	PrevPosition = FVector(0, 0, 0);

	// DLL properties
	OutCameraWidth = 1920;
	OutCameraHeight = 1080;
	DetectRatio = 1;
	CamId = 0;
	FovZoom = 1;
	Draw = false;
	LockEyesNose = true;
}


void AArFaceRig::BeginPlay()
{
	Super::BeginPlay();

	UcDataStorageGameInstance* GameInst = (UcDataStorageGameInstance*)GetGameInstance();

	GameInst->CustomStart(OutCameraWidth,
		OutCameraHeight,
		DetectRatio,
		CamId,
		FovZoom,
		Draw,
		LockEyesNose);

	// Set blend shape names
	USkeletalMesh* skelMesh = FaceMesh->SkeletalMesh;
	BlendShapeArray = skelMesh->K2_GetAllMorphTargetNames();

	// Setup material instance
	UMaterialInstance* Material = (UMaterialInstance *)PlaneMesh->GetMaterial(0);
	MasterMaterialRef = Material;

	// Set plane transform
	PlaneMesh->SetWorldLocationAndRotation(FVector((OutCameraWidth*FovZoom)*100, 0, 0),
		FQuat(FRotator(0, 90, 90)));

	PlaneMesh->SetWorldScale3D(FVector(OutCameraWidth, OutCameraHeight, 1));

	// Reset camera transform
	FaceCamera->SetWorldTransform(FTransform(FVector(0)));

	// Calculate FOV
	float Distance = FMath::Abs(OutCameraWidth*FovZoom);
	float Radians = 2.0f * FMath::Atan(OutCameraWidth * 0.5f / Distance);
	float Fov = FMath::RadiansToDegrees(Radians);

	// Set FOV
	FaceCamera->SetFieldOfView(Fov);
	FaceCamera->SetAspectRatio((float)OutCameraWidth / (float)OutCameraHeight);
	FaceCamera->SetConstraintAspectRatio(true);
}


void AArFaceRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunDLL();
}


void AArFaceRig::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AArFaceRig::SetBlendShapes(float* Blendshapes)
{
	// Loop through each blendshape and set value
	int count = 0;
	for (auto blendshape : BlendShapeArray)
	{
		FaceMesh->SetMorphTarget(FName(blendshape),
			Blendshapes[count]);

		count++;
	}
}


void AArFaceRig::SetTransforms(FVector Up, FVector Forward, FVector Translation)
{
	// Build matrix
	Up = Up.GetSafeNormal();
	Forward = Forward.GetSafeNormal();

	FMatrix BuiltMatrix = FRotationMatrix::MakeFromYZ(Up, Forward);
	BuiltMatrix = Mat * BuiltMatrix* MatB;

	// Fix flipped axes
	FTransform OutTransform(BuiltMatrix);
	FQuat Quat = OutTransform.GetRotation();
	FVector Mod = Quat.Euler();
	Mod.Z = -Mod.Z;
	FQuat NewRot = FQuat::MakeFromEuler(Mod);

	// Store transform
	FRotator CurrentRot = FaceMesh->GetComponentRotation();
	FVector CurrenTran = FaceMesh->GetComponentLocation();

	// Guess next transform
	FRotator RotGuess = FMath::Lerp(PrevRotation,
		CurrentRot,
		Momentum);

	FVector TranGuess = FMath::Lerp(PrevPosition,
		CurrenTran,
		Momentum);

	// Store previous frame
	PrevRotation = CurrentRot;
	PrevPosition = CurrenTran;

	// Blend with prediction
	RotGuess = FMath::Lerp(RotGuess, NewRot.Rotator(), Blend);

	TranGuess = FMath::Lerp(TranGuess, Translation, Blend);

	// Set transform
	FTransform FinalTransform(RotGuess,
		TranGuess,
		FVector(FaceScale));

	FaceMesh->SetWorldTransform(FinalTransform);
}


void AArFaceRig::SetBackground(TArray<unsigned char, TFixedAllocator<512 * 512 * 4>> Image)
{
	// Build material instance
	UMaterialInstanceDynamic* MatInst = UMaterialInstanceDynamic::Create(MasterMaterialRef, this);

	// Build texture
	UTexture2D* Texture = UTexture2D::CreateTransient(512, 512, PF_B8G8R8A8);
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Image.GetData(), 512 * 512 * 4);
	Mip.BulkData.Unlock();

	// Place texture into material instance parameter
	Texture->UpdateResource();
	MatInst->SetTextureParameterValue(FName("ViewInput"), (UTexture*)Texture);

	// Set material
	PlaneMesh->SetMaterial(0, MatInst);
}


void AArFaceRig::RunDLL()
{
	// Open GameInstance
	UcDataStorageGameInstance * GameInst = (UcDataStorageGameInstance*)GetGameInstance();

	// Prep image data  
	TArray<unsigned char, TFixedAllocator<512 * 512 * 4>> Image;
	Image.SetNumZeroed(512 * 512 * 4);

	// Prep transform data
	TransformData outFaces(0, 0, 0, 0, 0, 0, 0, 0, 0);
	float outExpression[51];

	// Get transform and image data
	GameInst->GetTransform(outFaces, outExpression);
	GameInst->GetImage(Image.GetData(), 512, 512);

	// Copy blenshapes
	for (int i = 0; i < 51; i++)
	{
		// Calc momentum
		float BlendVal = BlendValues[i] + ((BlendValues[i] - PrevBlendValues[i]) * Momentum);

		// Blend with prediction
		BlendVal = FMath::Lerp(BlendVal, outExpression[i], Blend);

		PrevBlendValues[i] = BlendValues[i];
		BlendValues[i] = BlendVal;
	}

	// Copy transforms
	FVector Translation(outFaces.tZ, outFaces.tX, -outFaces.tY);
	FVector Up(outFaces.ruX, outFaces.ruY, outFaces.ruZ);
	FVector Forward(outFaces.rfX, outFaces.rfY, outFaces.rfZ);

	// Set blendshapes and transform
	SetTransforms(Up, Forward, Translation);
	SetBlendShapes(BlendValues);
	SetBackground(Image);
}