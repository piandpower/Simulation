#pragma once
#include "ScriptedVolume.h"
#include "Classes/Components/ArrowComponent.h"
#include "CoreMinimal.h"

#include "FluidSource.generated.h"

UCLASS()
class AFluidSource : public AScriptedVolume {
	GENERATED_BODY()
public:
	AFluidSource();

	~AFluidSource() override;

	void Build(UWorld * world, UParticleContext & particleContext) override;

	void ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;
	void ApplyAfterIntegration(UParticleContext & particleContext, double timestep, double simulationTime) override;

	double MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const override;

	UFUNCTION(BlueprintCallable)
	void SetSpawnRateByVelocity(float velocity);

	UFUNCTION(BlueprintCallable)
	void SetSpawnRateByParticlesPerSecond(float particlesPerSecond);

	UFUNCTION(BlueprintCallable)
	void SetSpawnRateByVolumePerSecond(float volume);

	void SpawnParticlesSquare(UParticleContext & particleContext, double timeOffset);
	void SpawnParticlesCircle(UParticleContext & particleContext, double timeOffset);

	void SpawnBorderCuboid(UWorld * world, UParticleContext & particleContext);
	void SpawnBorderCylinder(UWorld * world, UParticleContext & particleContext);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RestDensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Viscosity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool TwoDimensions = false;

protected:
	UArrowComponent * ArrowComponent;

	UStaticMeshComponent * SpawnPlane;
	UStaticMesh * SpawnSquare;
	UStaticMesh * SpawnCircle;

	void OnConstruction(const FTransform& Transform) override;

	double LastSpawnTime = 0;
	double NextSpawnTime = 0;

	// the first row shouldn't spawn since this causes pressure explosions if reloaded
	bool FirstSpawn = true;

	UFluid * Fluid;
};
