#include "PressureGradient.h"
#include "Simulator.h"

Vector3D UPressureGradient::ComputePressureGradient(Particle& f, int particleIndex) const
{
	throw("This should never be called since it should always be overwritten by a child class!");
	return Vector3D(0.0, 0.0, 0.0);
}

void UPressureGradient::PrecomputeAllGeometryData(const UParticleContext& particleContext)
{
}

void UPressureGradient::Build(const USolver * solver, const EDimensionality & dimensionality)
{
	Solver = solver;
	Dimensionality = dimensionality;
}

const UKernel & UPressureGradient::GetKernel() const
{
	return *Solver->GetKernel();
}

const UBoundaryPressure & UPressureGradient::GetBoundaryPressure() const
{
	return *Solver->GetBoundaryPressure();
}
