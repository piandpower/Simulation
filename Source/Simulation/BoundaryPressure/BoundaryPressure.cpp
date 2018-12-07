#include "BoundaryPressure.h"

void UBoundaryPressure::ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel) 
{
};

double UBoundaryPressure::GetPressureValue(const Particle & staticBoundaryParticle, const Particle & neighboringFluidParticle) const
{
	throw("This should never be called since it should always be overwritten by a child class!");
}

void UBoundaryPressure::Build(const EDimensionality & dimensionality)
{
	Dimensionality = dimensionality;
}

FNeighborsSearchRelations UBoundaryPressure::GetRequiredNeighborhoods() const
{
	return FNeighborsSearchRelations(true, true,
									 false, false);
}
