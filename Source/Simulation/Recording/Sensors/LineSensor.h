// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sensor.h"
#include "Particles/SensorParticle.h"
#include "LineSensor.generated.h"

UCLASS(BlueprintType)
class SIMULATION_API ALineSensor : public ASensor
{
	GENERATED_BODY()
	
public:	

	ALineSensor();

	UFUNCTION(BlueprintCallable)
	static ALineSensor * SpawnSensorLine(UWorld * world,
		FMeasuredAttributes attributesToMeasure,
		FVector start,
		FVector end,
		float sensorParticleDistance = 1.f);

	void MeasureProperty(const UKernel& kernel) override;

	void FindNeighbors(const UNeighborsFinder& neighborsFinder) override;

	void WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory) override;

protected:

	Vector3D Start;
	Vector3D End;
	double SensorParticleDistance;

	std::vector<SensorParticle> LineSensorParticles;

	std::vector<std::vector<double>> VelocityMeasurePoints;
	std::vector<std::vector<Vector3D>> VelocityDirectionMeasurePoints;
	std::vector<std::vector<double>> VelocityDivergenceMeasurePoints;
	std::vector<std::vector<double>> VelocityXMeasurePoints;
	std::vector<std::vector<double>> DensityMeasurePoints;
	std::vector<std::vector<double>> PressureMeasurePoints;
	std::vector<std::vector<double>> CurlMeasurePoints;


	void OnConstruction(const FTransform& Transform) override;
};
