#include "NeighborsFinder.h"

#include "Simulator.h"

UNeighborsFinder::~UNeighborsFinder()
{
}

void UNeighborsFinder::Build(double supportRange)
{
	SupportRange = supportRange;
}

void UNeighborsFinder::FindNeighbors(const UParticleContext& particleContext, double particleDistance, FNeighborsSearchRelations searchRelations)
{
	throw("This is an abstract base class and should not be called!");
}

FNeighborhood UNeighborsFinder::NeighborsOfPosition(const Vector3D& position, const UParticleContext& particleContext) const
{
	throw("This is an abstract base class and should not be called!");
	return (FNeighborhood());
}

void UNeighborsFinder::AddStaticParticles(UStaticBorder * borders, double particleDistance)
{
	throw("This is an abstract base class and should not be called!");
}

void UNeighborsFinder::AddStaticParticles(TArray<UStaticBorder*>& borders, double particleDistance)
{
	throw("This is an abstract base class and should not be called!");
}

void UNeighborsFinder::AddStaticParticles(std::vector<UStaticBorder*>& borders, double particleDistance)
{
	throw("This is an abstract base class and should not be called!");
}

ENeighborhoodSearch UNeighborsFinder::GetNeighborsFinderType()
{
	return NeighborsFinderType;
}
