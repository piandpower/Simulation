// Fill out your copyright notice in the Description page of Project Settings.

#include "RecordingCamera.h"


// Sets default values
ARecordingCamera::ARecordingCamera() : ASceneCapture2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	GetCaptureComponent2D()->bCaptureEveryFrame = false;
	GetCaptureComponent2D()->bCaptureOnMovement = false;
	GetCaptureComponent2D()->CaptureSource = SCS_FinalColorLDR;
}

void ARecordingCamera::BeginPlay()
{
	GetCaptureComponent2D()->bCaptureEveryFrame = false;
	GetCaptureComponent2D()->bCaptureOnMovement = false;
	GetCaptureComponent2D()->CaptureSource = SCS_FinalColorLDR;

	SceneAsTexture = CreateTextureRenderTarget2D(Width, Height);

	GetCaptureComponent2D()->TextureTarget = SceneAsTexture;
}

void ARecordingCamera::CaptureView(bool saveToFile, FString fileName)
{
	if (SceneAsTexture == nullptr) {
		return;
	}

	GetCaptureComponent2D()->CaptureScene();

	if (saveToFile) {
		
		SaveRenderTargetToDisk(SceneAsTexture, fileName);
	}


}

UTextureRenderTarget2D* ARecordingCamera::CreateTextureRenderTarget2D(int32 Width, int32 Height, FLinearColor ClearColor, float Gamma)
{
	UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>();
	if (!NewRenderTarget2D)
	{
		return nullptr;
	}
	NewRenderTarget2D->InitAutoFormat(Width, Height);
	NewRenderTarget2D->ClearColor = FLinearColor::White;
	NewRenderTarget2D->TargetGamma = Gamma;
	return NewRenderTarget2D;
}

void ARecordingCamera::SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString Filename)
{
	FTextureRenderTargetResource* RTResource = InRenderTarget->GameThread_GetRenderTargetResource();

	FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
	ReadPixelFlags.SetLinearToGamma(true);

	TArray<FColor> OutBMP;
	RTResource->ReadPixels(OutBMP, ReadPixelFlags);

	for (FColor& color : OutBMP)
	{
		color.A = 255;
	}


	FIntRect SourceRect;

	FIntPoint DestSize(InRenderTarget->GetSurfaceWidth(), InRenderTarget->GetSurfaceHeight());


	FString ResultPath;
	FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
	HighResScreenshotConfig.SaveImage(Filename, OutBMP, DestSize, &ResultPath);
}

void ARecordingCamera::OnConstruction(const FTransform & Transform)
{
	GetCaptureComponent2D()->bCaptureEveryFrame = false;
	GetCaptureComponent2D()->bCaptureOnMovement = false;
	GetCaptureComponent2D()->CaptureSource = SCS_FinalColorLDR;

	SceneAsTexture = CreateTextureRenderTarget2D(Width, Height);
	GetCaptureComponent2D()->TextureTarget = SceneAsTexture;
}
