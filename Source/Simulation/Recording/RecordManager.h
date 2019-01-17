// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <experimental/filesystem>


#include "CoreMinimal.h"
#include "RecordingCamera.h"
#include "Sensors/Sensor.h"
#include "UnrealComponents/ParticleCloudActor.h"

#include "RecordManager.generated.h"

class ASimulator;


USTRUCT(BlueprintType)
struct FReplayInformation {
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float ReplaySpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float ReplayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		int Frame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Struct")
		float CurrentFramesPerSecond;

	FReplayInformation() :
		ReplaySpeed(1.f),
		ReplayTime(0.f),
		Frame(0),
		CurrentFramesPerSecond(60.f)
	{
	}

	FReplayInformation(float replaySpeed, float replayTime, int frame, float currentFramesPerSecond) :
		ReplaySpeed(replaySpeed),
		ReplayTime(replayTime),
		Frame(frame),
		CurrentFramesPerSecond(currentFramesPerSecond)
	{
	}
};


UCLASS(BlueprintType)
class SIMULATION_API URecordManager : public UObject {
	
	GENERATED_BODY()

public:

	~URecordManager();

	UFUNCTION(BlueprintPure, Category = "Recording")
	static URecordManager * CreateRecordManager(TArray<ARecordingCamera*> cameras,
		TArray<ASensor*> sensors,
		EColorVisualisation fluidColorMode = EColorVisualisation::Normal,
		EColorVisualisation staticBorderColorMode = EColorVisualisation::Normal,
		float frameRate = 60.0,
		int saveEachNthIteration = 0,
		bool takeScreenshots = false);

	void Build(UWorld * world, ASimulator * simulator);

	// returns true if recordings are done in this frame
	bool UpdateTimeAndRecordings(double deltaTime, int iteration, double simulatedTime);

	void SetRecordedParticlePosition(double deltaTime = 0);

	void WriteSimulationStateToFile(int iteration);
		
	void CamerasCapture(int frame, FString simulationName);

	UFUNCTION(BlueprintCallable, Category = "Recording")
	void TakeScreenshot(int currentFrame, FString simulationName);

	UFUNCTION(BlueprintCallable, Category = "Recording")
	void WriteSolverStatisticsToFile();

	UFUNCTION(BlueprintCallable, Category = "Recording")
	void WriteSensorDataToFile();

	ASimulator * GetSimulator() const;

	UFUNCTION(BlueprintPure)
	bool GetTakeScreenshots() const;

	UFUNCTION(BlueprintCallable)
	void SetTakeScreenshots(bool takeScreenshots);

	UFUNCTION(BlueprintPure)
		bool GetRecordParticles() const;

	UFUNCTION(BlueprintCallable)
		void SetRecordParticles(bool recordParticles);

	UFUNCTION(BlueprintPure)
		bool GetSaveSimulationState() const;

	UFUNCTION(BlueprintCallable)
		void SetSaveSimulationState(bool saveSimulationState);

	UFUNCTION(BlueprintPure)
		float GetFrameRate() const;

	UFUNCTION(BlueprintCallable)
		void SetFrameRate(float frameRate);

	UFUNCTION(BlueprintCallable)
		void RewindReplay();

	UFUNCTION(BlueprintCallable)
		void ChangeReplaySpeed(float replaySpeed);

	UFUNCTION(BlueprintCallable)
		void IncreaseReplaySpeed();

	UFUNCTION(BlueprintCallable)
		void DecreaseReplaySpeed();

	UFUNCTION(BlueprintPure)
		float GetReplaySpeed() const;

	UFUNCTION(BlueprintPure)
	bool ReplayIsAtEnd() const;

	UFUNCTION(BlueprintPure)
		FReplayInformation ReplayInformation() const;


	double GetLastRecordedTime() const;
	double GetNextRecordTime() const;

	AParticleCloudActor * GetParticleCloudActor();

	// Flag that determines whether sensors calculate and record the properties. 
	// Can be turned off for small performance gain.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool SensorsActive = true;


protected:

	double FrameRate = 60.0;
	int SaveEachNthIteration = 0;
	bool SaveSimulationState = false;
	bool TakeScreenshots = false;
	bool RecordParticles = true;

	int RecordedFrames = 0;
	double LastRecordedTime = 0;
	double NextRecordTime = 0;

	int CurrentFrame = 0;
	bool ReplayEnd = false;
	double ReplaySpeed = 1.0;
	double ReplayTime = 0.0;

	ASimulator * Simulator;

	TArray<ARecordingCamera*> Cameras;

	TArray<ASensor*> Sensors;

	AParticleCloudActor * ParticleVisualizer;

	std::vector<std::vector<std::vector<Vector3D>>> OldPositions;
	std::vector<std::vector<std::vector<Vector3D>>> OldVelocities;
	std::vector<std::vector<std::vector<double>>> OldDensities;

	TArray<double> OldAverageDensityErrors;

	EColorVisualisation FluidColorMode;
	EColorVisualisation StaticBorderColorMode;
};
