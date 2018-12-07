#include "Viscosity.h"

UViscosity::~UViscosity()
{
}


UViscosity * UViscosity::CreateViscosity()
{
	UViscosity * viscosity = NewObject<UViscosity>();

	// prevent garbage collection
	viscosity->AddToRoot();

	return viscosity;
}

void UViscosity::ApplyAcceleration(UParticleContext * particleContext)
{
	for (UFluid * fluid : particleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			Vector3D sum = { 0, 0, 0 };

			for (const Particle& ff : f.FluidNeighbors) {

				Vector3D vij = f.Velocity - ff.Velocity;
				Vector3D xij = f.Position - ff.Position;
				sum += ff.Fluid->GetViscosity() * ff.Mass / ff.Fluid->GetRestDensity() * vij * (xij * GetKernel()->ComputeKernelDerivative(f, ff)) / ((xij* xij) + 0.01 * pow(GetKernel()->GetParticleSpacing(), 2));

			}

			for (const Particle& fb : f.StaticBorderNeighbors) {

				// Mimic ghost velocities
				Vector3D vij = f.Velocity - fb.Velocity;
				Vector3D xij = f.Position - fb.Position;
				sum += fb.Border->BorderViscosity * fb.Mass / f.Fluid->GetRestDensity() * vij * (xij * GetKernel()->ComputeKernelDerivative(f, fb)) / ((xij* xij) + 0.01 * pow(GetKernel()->GetParticleSpacing(), 2));
			}

			f.Acceleration += 2 * sum;

		});
	}
}
