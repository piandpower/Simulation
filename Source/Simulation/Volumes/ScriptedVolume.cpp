#include "ScriptedVolume.h"

AScriptedVolume::AScriptedVolume() {
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> cubeMesh(TEXT("StaticMesh'/Game/Geometry/Shapes/Shape_Cube.Shape_Cube'"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> cylinderMesh(TEXT("StaticMesh'/Game/Geometry/Shapes/Shape_Cylinder.Shape_Cylinder'"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultMaterial(TEXT("Material'/Game/Geometry/Materials/WireframeMaterial.WireframeMaterial'"));


	if (cubeMesh.Object) {
		CuboidMesh = cubeMesh.Object;
		MeshComponent->SetStaticMesh(cubeMesh.Object);
	}
	if (cylinderMesh.Object) {
		CylinderMesh = cylinderMesh.Object;
	}

	if (defaultMaterial.Object) {
		MeshComponent->SetMaterial(0, defaultMaterial.Object);
	}

	SetActorHiddenInGame(true);
	RootComponent = MeshComponent;
	MeshComponent->CastShadow = false;
}

AScriptedVolume::~AScriptedVolume()
{
}

void AScriptedVolume::Build(UWorld * world, UParticleContext & particleContext)
{
}

void AScriptedVolume::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
}

void AScriptedVolume::ApplyAfterIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
}

double AScriptedVolume::MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const
{
	// As a standard behaviour return no requirements to the timestep
	return DBL_MAX;
}

EVolumeType AScriptedVolume::GetVolumeType()
{
	return VolumeType;
}

bool AScriptedVolume::IsAffected(const Vector3D & position) const
{

	const FVector fPosition = position * 10;
	// Multiply by 10 since transforms and positions of Volumes are given in unreal units (10cm)
	const FVector temp = GetActorTransform().InverseTransformPosition(fPosition);
	
	bool inside = true;
	switch (VolumeForm) {
	case Cuboid:
		inside = temp.X < 100 && temp.X > -100 && temp.Y < 100 && temp.Y > -100 && temp.Z < 100 && temp.Z > -100;
		break;

	case Cylinder:
		inside = (temp.X * temp.X) + (temp.Y * temp.Y) < 10000 && temp.Z < 100 && temp.Z > -100;
		break;
	}

	if (ActInside) {
		return inside;
	}
	else {
		return !inside;
	}
}

bool AScriptedVolume::IsParticleAffected(const Particle & particle) const
{
	return IsAffected(particle.Position);
}

void AScriptedVolume::SetEnabled(bool enabled)
{
	IsEnabled = enabled;
}

bool AScriptedVolume::GetEnabled() const
{
	return IsEnabled;
}

void AScriptedVolume::OnConstruction(const FTransform& Transform) {
	
	switch (VolumeForm) {
	case Cuboid:
		MeshComponent->SetStaticMesh(CuboidMesh);
		break;
	case Cylinder:
		MeshComponent->SetStaticMesh(CylinderMesh);
		break;
	}
}