#include "SPHPressureGradient.h"
#include "Simulator.h"

Vector3D USPHPressureGradient::ComputePressureGradient(Particle& f, int particleIndex) const
{
	Vector3D pressureGradient = { 0.0, 0.0, 0.0 };
	for (const Particle& ff : f.FluidNeighbors) {
		pressureGradient += ff.GetVolume() * (ff.Pressure / pow(ff.Fluid->GetRestDensity(), 2) + f.Pressure / pow(f.Fluid->GetRestDensity(), 2)) * GetKernel().ComputeGradient(f, ff);
	}

	for (const Particle& fb : f.StaticBorderNeighbors) {
		pressureGradient += fb.Mass / f.Density * (GetBoundaryPressure().GetPressureValue(fb, f) / pow(f.Fluid->GetRestDensity(), 2) + f.Pressure / pow(f.Fluid->GetRestDensity(), 2)) * GetKernel().ComputeGradient(f, fb);
	}

	return pressureGradient;
}

USPHPressureGradient * USPHPressureGradient::CreateSPHPressureGradient()
{
	USPHPressureGradient * sphPressureGradient = NewObject<USPHPressureGradient>();

	// prevent garbage collection
	sphPressureGradient->AddToRoot();
	return sphPressureGradient;
}
