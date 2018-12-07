#include "PeriodicCondition.h"
#include "ParticleContext/ParticleContext.h"
#include "SceneComponents/Particle.h"
#include "Simulator.h"


UPeriodicCondition * UPeriodicCondition::CreatePeriodicCondition(FVector min, FVector max) {

	UPeriodicCondition * periodicCondition = NewObject<UPeriodicCondition>();
	periodicCondition->BoxMin = static_cast<Vector3D>(min);
	periodicCondition->BoxMax = static_cast<Vector3D>(max);

	// prevent garbage collection
	periodicCondition->AddToRoot();

	return periodicCondition;
}

void UPeriodicCondition::Build(UParticleContext * particleContext, EDimensionality dimensionality)
{
	SupportRange = particleContext->GetParticleDistance() * 2;
	ParticleContext = particleContext;
	Dimensionality = dimensionality;
}

void UPeriodicCondition::UpdateGhostParticles()
{
	GhostParticles.clear();
	ReferencedParticles.clear();
	ReferencedParticlesIndices.clear();
	MirroringCauses.clear();

	const std::vector<UFluid*>& fluids = ParticleContext->GetFluids();
	for (const UFluid * fluid : fluids) {
		for (int i = 0; i < fluid->Particles->size(); i++) {
			Particle& f = fluid->Particles->at(i);
			// check left side
			if (f.Position.X < BoxMin.X + SupportRange && f.Position.X > BoxMin.X) {
				// Particle needs to be mirrored in X direction
				Vector3D ghostPosition = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X) , f.Position.Y, f.Position.Z );
				GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
				ReferencedParticles.push_back(&f);
				ReferencedParticlesIndices.push_back(i);
				MirroringCauses.push_back(EMirroringCause::Left);
			}

			// check right side
			if (f.Position.X > BoxMax.X - SupportRange && f.Position.X < BoxMax.X) {
				// Particle needs to be mirrored in X direction
				Vector3D ghostPosition = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X) , f.Position.Y, f.Position.Z );
				GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
				ReferencedParticles.push_back(&f);
				ReferencedParticlesIndices.push_back(i);
				MirroringCauses.push_back(EMirroringCause::Right);
			}

			if (Dimensionality == EDimensionality::Three) {
				// check front side
				if (f.Position.Y < BoxMin.Y + SupportRange && f.Position.Y > BoxMin.Y) {
					// Particle needs to be mirrored in Y direction
					Vector3D ghostPosition = Vector3D(f.Position.X, f.Position.Y + (BoxMax.Y - BoxMin.Y), f.Position.Z);
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::Front);
				}

				// check back side
				if (f.Position.Y > BoxMax.Y - SupportRange && f.Position.Y < BoxMax.Y) {
					// Particle needs to be mirrored in Y direction
					Vector3D ghostPosition = Vector3D(f.Position.X, f.Position.Y - (BoxMax.Y - BoxMin.Y), f.Position.Z);
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::Back);
				}
			}
			
			if (Dimensionality == EDimensionality::Three || Dimensionality == EDimensionality::Two) {
				// check bottom side
				if (f.Position.Z < BoxMin.Z + SupportRange && f.Position.Z > BoxMin.Z) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X, f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::Bottom);
				}

				// check top side
				if (f.Position.Z > BoxMax.Z - SupportRange && f.Position.Z < BoxMax.Z) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X, f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::Top);
				}
			}

			if (Dimensionality == EDimensionality::Three) {
				// TODO
				// check all 8 corners
			}

			if (Dimensionality == EDimensionality::Two) {
				// check top right corner
				if (f.Position.Z > BoxMax.Z - SupportRange && f.Position.Z < BoxMax.Z &&
					f.Position.X > BoxMax.X - SupportRange && f.Position.X < BoxMax.X) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::TopRight);
				}

				// check top left corner
				if (f.Position.Z > BoxMax.Z - SupportRange && f.Position.Z < BoxMax.Z &&
					f.Position.X < BoxMin.X + SupportRange && f.Position.X > BoxMin.X) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::TopLeft);
				}

				// check bottom left corner
				if (f.Position.Z < BoxMin.Z + SupportRange && f.Position.Z > BoxMin.Z &&
					f.Position.X < BoxMin.X + SupportRange && f.Position.X > BoxMin.X) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::BottomLeft);
				}

				// check bottom right corner
				if (f.Position.Z < BoxMin.Z + SupportRange && f.Position.Z > BoxMin.Z &&
					f.Position.X > BoxMax.X - SupportRange && f.Position.X < BoxMax.X) {
					// Particle needs to be mirrored in Z direction
					Vector3D ghostPosition = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
					GhostParticles.push_back(MakeGhostParticle(ghostPosition, f));
					ReferencedParticles.push_back(&f);
					ReferencedParticlesIndices.push_back(i);
					MirroringCauses.push_back(EMirroringCause::BottomRight);
				}
			}

			if (Dimensionality == EDimensionality::One) {
				// check no corners
			}
		}
	}
}

void UPeriodicCondition::UpdateGhostParticleAttributes(bool updatePosition)
{
	for (int i = 0; i < GhostParticles.size(); i++) {
		GhostParticles[i] = MakeGhostParticle(GhostParticles[i].Position, *ReferencedParticles[i]);
	}

	if (updatePosition) {
		for (int i = 0; i < GhostParticles.size(); i++) {
			const Particle& f = *ReferencedParticles[i];

			switch (MirroringCauses[i]) {
			case EMirroringCause::Left:
				GhostParticles[i].Position = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z);
				break;
			case EMirroringCause::Right:
				GhostParticles[i].Position = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z);
				break;
			case EMirroringCause::Front:
				GhostParticles[i].Position = Vector3D(f.Position.X, f.Position.Y + (BoxMax.Y - BoxMin.Y), f.Position.Z);
				break;
			case EMirroringCause::Back:
				GhostParticles[i].Position = Vector3D(f.Position.X, f.Position.Y - (BoxMax.Y - BoxMin.Y), f.Position.Z);
				break;
			case EMirroringCause::Bottom:
				GhostParticles[i].Position = Vector3D(f.Position.X, f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
				break;	
			case EMirroringCause::Top:
				GhostParticles[i].Position = Vector3D(f.Position.X, f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
				break;
			case EMirroringCause::BottomLeft:
				GhostParticles[i].Position = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
				break;
			case EMirroringCause::BottomRight:
				GhostParticles[i].Position = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
				break;
			case EMirroringCause::TopLeft:
				GhostParticles[i].Position = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
				break;
			case EMirroringCause::TopRight:
				GhostParticles[i].Position = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));
				break;
			}
			 
		}
	}

}

void UPeriodicCondition::UpdateGhostParticleDensity()
{
	for (int i = 0; i < GhostParticles.size(); i++) {
		GhostParticles[i].Density = ReferencedParticles[i]->Density;
	}
}

void UPeriodicCondition::UpdateGhostParticlePressure()
{
	for (int i = 0; i < GhostParticles.size(); i++) {
		GhostParticles[i].Pressure = ReferencedParticles[i]->Pressure;
	}
}

void UPeriodicCondition::UpdateGhostParticleAcceleration()
{
	for (int i = 0; i < GhostParticles.size(); i++) {
		GhostParticles[i].Acceleration = ReferencedParticles[i]->Acceleration;
	}
}

void UPeriodicCondition::MoveOutOfBoundsParticles()
{
	for (const UFluid * fluid : ParticleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			// check left side
			if (f.Position.X < BoxMin.X) {
				// Particle needs to be mirrored in X direction
				f.Position = Vector3D(f.Position.X + (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z);
			}

			// check right side
			if (f.Position.X > BoxMax.X) {
				// Particle needs to be mirrored in X direction
				f.Position = Vector3D(f.Position.X - (BoxMax.X - BoxMin.X), f.Position.Y, f.Position.Z);
			}

			if (Dimensionality == EDimensionality::Three) {
				// check front side
				if (f.Position.Y < BoxMin.Y) {
					// Particle needs to be mirrored in Y direction
					f.Position = Vector3D(f.Position.X, f.Position.Y + (BoxMax.Y - BoxMin.Y), f.Position.Z);
				}

				// check back side
				if (f.Position.Y > BoxMax.Y) {
					// Particle needs to be mirrored in Y direction
					f.Position = Vector3D(f.Position.X, f.Position.Y - (BoxMax.Y - BoxMin.Y), f.Position.Z);
				}
			}

			if (Dimensionality == EDimensionality::Three || Dimensionality == EDimensionality::Two) {
				// check bottom side
				if (f.Position.Z < BoxMin.Z) {
					// Particle needs to be mirrored in Z direction
					f.Position = Vector3D(f.Position.X, f.Position.Y, f.Position.Z + (BoxMax.Z - BoxMin.Z));
				}

				// check top side
				if (f.Position.Z > BoxMax.Z) {
					// Particle needs to be mirrored in Z direction
					f.Position = Vector3D(f.Position.X, f.Position.Y, f.Position.Z - (BoxMax.Z - BoxMin.Z));

				}
			}
		});
	}
}

const std::vector<Particle>& UPeriodicCondition::GetGhostParticles() const
{
	return GhostParticles;
}

const std::vector<Particle*>& UPeriodicCondition::GetReferencedParticles() const
{
	return ReferencedParticles;
}

const std::vector<int>& UPeriodicCondition::GetReferencedParticlesIndices() const
{
	return ReferencedParticlesIndices;
}

Particle UPeriodicCondition::MakeGhostParticle(const Vector3D & ghostPosition, const Particle & referenceParticle)
{
	return Particle(ghostPosition,
		referenceParticle.Velocity,
		referenceParticle.Acceleration,
		referenceParticle.Mass,
		referenceParticle.Fluid,
		referenceParticle.Pressure,
		referenceParticle.Density,
		referenceParticle.IsScripted);
}
