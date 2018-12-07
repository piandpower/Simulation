// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"


#include "Runtime/Engine/Public/HighResScreenshot.h"

#include "RecordingCamera.generated.h"

UCLASS(BlueprintType)
class SIMULATION_API ARecordingCamera : public ASceneCapture2D
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARecordingCamera();

	virtual void BeginPlay() override;

	// Screen resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	int Width = 1920;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recording")
	int Height = 1080;


	UFUNCTION(BlueprintCallable, Category = "Recording")
	void CaptureView(bool saveToFile, FString fileName);
	
protected:

	UTextureRenderTarget2D * SceneAsTexture;

	static UTextureRenderTarget2D* CreateTextureRenderTarget2D(int32 Width = 256, int32 Height = 256, FLinearColor ClearColor = FLinearColor::White, float Gamma = 1);
	void SaveRenderTargetToDisk(UTextureRenderTarget2D* InRenderTarget, FString Filename);

	void OnConstruction(const FTransform& Transform) override;

};
