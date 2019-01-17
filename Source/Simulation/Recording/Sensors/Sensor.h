// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <experimental/filesystem>
#include <fstream>

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "DataStructures/Vector3D.h"
#include "GameFramework/Actor.h"
#include "ParticleContext/ParticleContext.h"
#include "NeighborsFinders/NeighborsFinder.h"
#include "Kernels/Kernel.h"

#include "Sensor.generated.h"

USTRUCT(BlueprintType)
struct FMeasuredAttributes {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureVelocity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureVelocityDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureVelocityDivergence = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureVelocityX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureDensity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasurePressure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool MeasureCurl = false;
};


UCLASS(BlueprintType)
class SIMULATION_API ASensor : public AActor
{
	GENERATED_BODY()
	
public:	

	// Sensors calculate the property at the given points
	virtual void MeasureProperty(const UKernel& kernel);

	// Flushes all the recorded sensor data to a file
	virtual void WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory);

	// Called when the sensors need to find all surrounding neighbors
	virtual void FindNeighbors(const UNeighborsFinder& neighborsFinder);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FMeasuredAttributes MeasuredAttributes;

	void Build(UParticleContext * particleContext);

protected:
	UParticleContext * ParticleContext;

};
