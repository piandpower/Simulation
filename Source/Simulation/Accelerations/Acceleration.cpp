#include "Acceleration.h"

#include "Solver/Solver.h"

void UAcceleration::ApplyAcceleration(UParticleContext * particleContext)
{
	throw("This is an abstract base class. This should never be called!");
}

void UAcceleration::Build(USolver * solver)
{
	Solver = solver;
}

UKernel * UAcceleration::GetKernel()
{
	return Solver->GetKernel();
}
