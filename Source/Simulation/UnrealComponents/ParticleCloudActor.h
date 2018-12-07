// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <chrono>
#include <thread>
#include <algorithm>

#include "CoreMinimal.h"
#include "PointCloud.h"
#include "PointCloudShared.h"
#include "PointCloudActor.h"

#include "DataStructures/Utility.h"

#include "SceneComponents/Particle.h"

#include "ParticleCloudActor.generated.h"

class UParticleContext;


UENUM(BlueprintType)
enum EColorVisualisation {
	None,
	Normal,
	Density,
	Velocity,
	VelocityDirection,
	Pressure,
	Curl
};


USTRUCT(BlueprintType)
struct FVisualisationInformation {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
		TEnumAsByte<EColorVisualisation> ColorCode = EColorVisualisation::Normal;

	UPROPERTY(BlueprintReadWrite)
		bool AutoLimits = true;

	UPROPERTY(BlueprintReadWrite)
		float Min = 0.0;

	UPROPERTY(BlueprintReadWrite)
		float Max = 1.0;

	UPROPERTY(BlueprintReadWrite)
		bool ShowFluids = true;

	UPROPERTY(BlueprintReadWrite)
		bool ShowStaticBorders = true;

	UPROPERTY(BlueprintReadWrite)
		bool ShowPeriodicGhostBorders = true;
};


UCLASS()
class SIMULATION_API AParticleCloudActor : public APointCloudActor
{
	GENERATED_BODY()
public:
	AParticleCloudActor();

	void Build(UParticleContext * particleContext, FVisualisationInformation visualisationInformation);

	void UpdatePointCloud();

	template <typename ParticleType>
	void VisualiseParticles(const std::vector<ParticleType>& particles, double size, EColorVisualisation colorMethod, double restDensity = 0);
	
	template <typename ParticleType>
	void VisualiseParticles(const std::vector<std::vector<ParticleType>*>& particles, double size, EColorVisualisation colorMethod, double restDensity = 0);

	void VisualisePositions(const std::vector<Vector3D>& positions, const std::vector<Vector3D>& velocities, double size, EColorVisualisation colorMethod);

	void VisualisePositions(const TArray<FVector>& positions, const std::vector<Vector3D>& velocities, double size, EColorVisualisation colorMethod);

	void VisualisePositions(const std::vector<std::vector<Vector3D>>& positions, const std::vector<std::vector<Vector3D>>& velocities, double size, EColorVisualisation colorMethod);

	void SetVisibilityOfCloud(bool newVisibility);

	void UpdateParticleContextVisualisation(FVisualisationInformation visualisationInformations);

protected:

	TArray<FPointCloudPoint> Points;

	UPointCloud * PointCloud;

	// The particle Context this point cloud should visualize
	UParticleContext * ParticleContext;

};
