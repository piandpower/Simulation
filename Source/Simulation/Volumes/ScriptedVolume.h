#pragma once

#include "CoreMinimal.h"
#include "ParticleContext/ParticleContext.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "ScriptedVolume.generated.h"

UENUM(BlueprintType)
enum EVolumeType {
	AddedAcceleration,
	FixedAcceleration,
	FixedVelocity,
	ShearWaveVelocity,
	Source,
	Kill
};

UENUM(BlueprintType)
enum EVolumeForm {
	Cuboid,
	Cylinder
};

UCLASS()
class AScriptedVolume : public AActor {
	GENERATED_BODY()
public:

	AScriptedVolume();

	virtual ~AScriptedVolume();

	virtual void Build(UWorld * world, UParticleContext & particleContext);

	// For modifications that influence fluid dynamics
	virtual void ApplyBeforeIntegration(UParticleContext& particleContext, double timestep, double simulationTime);

	// for adding / removing particles what can not happen during a simulation step
	virtual void ApplyAfterIntegration(UParticleContext& particleContext, double timestep, double simulationTime);

	// Volumes like sources and fixed velocity require a sufficiently small timestep
	virtual double MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const;

	UFUNCTION(BlueprintPure)
	EVolumeType GetVolumeType();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EVolumeForm> VolumeForm = EVolumeForm::Cuboid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ActInside = true;

	bool IsAffected(const Vector3D& position) const;
	bool IsParticleAffected(const Particle& particle) const;

	UFUNCTION(BlueprintCallable)
	void SetEnabled(bool enabled);

	UFUNCTION(BlueprintPure)
	bool GetEnabled() const;

protected:
	
	TEnumAsByte<EVolumeType> VolumeType;

	UStaticMeshComponent * MeshComponent;

	UStaticMesh * CuboidMesh;
	UStaticMesh * CylinderMesh;

	void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsEnabled = true;
};