#include "SurfaceTension.h"

USurfaceTension::~USurfaceTension()
{
}


USurfaceTension * USurfaceTension::CreateSurfaceTension(float surfaceTensionFactor)
{
	USurfaceTension * surfaceTension = NewObject<USurfaceTension>();
	surfaceTension->SurfaceTensionFactor = surfaceTensionFactor;

	// prevent garbage collection
	surfaceTension->AddToRoot();

	return surfaceTension;
}

void USurfaceTension::ApplyAcceleration(UParticleContext * particleContext)
{
	for (UFluid * fluid : particleContext->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);
			for (const Particle& ff : f.FluidNeighbors) {
				if ((ff.Position - f.Position).Size() > particleContext->GetParticleDistance()) {
					f.Acceleration += -SurfaceTensionFactor * ff.Mass * (f.Position - ff.Position).Normalized() * GetKernel()->ComputeKernel(f, ff);
				}
			}
		});
	}
}
