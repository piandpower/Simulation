// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Sensor.h"
#include "Particles/SensorParticle.h"
#include "PointSensor.generated.h"

UCLASS(BlueprintType)
class SIMULATION_API APointSensor : public ASensor
{
	GENERATED_BODY()
	
public:	

	APointSensor();

	UFUNCTION(BlueprintCallable)
	static APointSensor * SpawnSensorPoint(UWorld * world, FVector location, FMeasuredAttributes attributesToMeasure);

	void MeasureProperty(const UKernel& kernel) override;

	void FindNeighbors(const UNeighborsFinder& neighborsFinder) override;

	void WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory) override;

protected:

	SensorParticle PointSensorParticle;

	std::vector<double> VelocityMeasurePoints;
	std::vector<Vector3D> VelocityDirectionMeasurePoints;
	std::vector<double> VelocityDivergenceMeasurePoints;
	std::vector<double> VelocityXMeasurePoints;
	std::vector<double> DensityMeasurePoints;
	std::vector<double> PressureMeasurePoints;
	std::vector<double> CurlMeasurePoints;

	void OnConstruction(const FTransform& Transform) override;
};
