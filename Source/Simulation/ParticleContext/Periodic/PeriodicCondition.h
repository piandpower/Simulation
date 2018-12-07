#pragma once

#include <vector>
#include "DataStructures/Vector3D.h"

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

#include "PeriodicCondition.generated.h"

class UParticleContext;
class Particle;
enum EDimensionality;

enum class EMirroringCause {
	Left,
	Right,
	Bottom,
	Top,
	Front,
	Back,
	BottomLeft,
	BottomRight,
	TopLeft,
	TopRight
};

UCLASS(BlueprintType)
// Periodic Condition implements a periodic simulation domain.
// This is done by copying all particles near the periodic limit to the other side. Copied particles are calles ghost particles.
// These particles are copies of the original particles. Throughout a simulation step the ghost particle attributes need to be refreshed multiple times.
class UPeriodicCondition : public UObject{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure)
	static UPeriodicCondition * CreatePeriodicCondition(FVector min = FVector(-10.f, -10.f, -10.f), FVector max = FVector(10.f, 10.f, 10.f));

	void Build(UParticleContext * particleContext, EDimensionality dimensionality);

	// Generated ghost particles required for the periodic condition, assings positions and copies all other attributes from the original particle
	// Do not call if current neighborhoods are still required, since GhostParticle vector is modified
	void UpdateGhostParticles();

	// Copies all attributes from the refrenced particles to the ghost particle except the position
	void UpdateGhostParticleAttributes(bool updatePosition = false);

	// Updates the density stored in ghost particles by their referenced particles density
	void UpdateGhostParticleDensity();

	// Updates the pressure stored in ghost particles by their referenced particles pressure
	void UpdateGhostParticlePressure();

	// Updates the acceleration stored in ghost particles by their referenced particles acceleration
	void UpdateGhostParticleAcceleration();

	void MoveOutOfBoundsParticles();

	const std::vector<Particle>& GetGhostParticles() const;
	const std::vector<Particle*>& GetReferencedParticles() const;
	const std::vector<int>& GetReferencedParticlesIndices() const;

protected:
	// Defines the bounds of the periodic space
	Vector3D BoxMin;
	Vector3D BoxMax;
	
	// this is important since it determines how far particles need to be copied
	double SupportRange;

	// DImensionality is important to know in which directions the fluid needs to be copied
	EDimensionality Dimensionality;

	// the particle context the periodic condition acts on
	UParticleContext * ParticleContext;

	// Particles that are generated for the periodicCondition
	std::vector<Particle> GhostParticles;

	// References to the particles that are the real counterpart to ghost particles
	std::vector<Particle*> ReferencedParticles;
	std::vector<int> ReferencedParticlesIndices;
	std::vector<EMirroringCause> MirroringCauses;

	static Particle MakeGhostParticle(const Vector3D& ghostPosition, const Particle& referenceParticle);
};