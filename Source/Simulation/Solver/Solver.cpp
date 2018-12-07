#include "Solver.h"
#include "Simulator.h"


USolver::~USolver() {

}

void USolver::Step()
{
	throw("This shouldn't be called. This is a abstract parent class.");
}

void USolver::Build(ASimulator * simulator, TArray<AScriptedVolume*> volumes)
{
	Simulator = simulator;
	Volumes = volumes;

	for (UAcceleration * acceleration : Accelerations) {
		acceleration->Build(this);
	}

	GetBoundaryPressure()->Build(simulator->GetDimensionality());
}

double USolver::ComputeAverageDensityError()
{
	double sum = 0.0;
	int totalParticles = 0;
	for (UFluid * fluid : GetParticleContext()->GetFluids()) {
		sum += ParallelSum<Particle, double>(*fluid->Particles, [](const Particle& particle) -> double { 
			return std::max(0.0, (particle.Density - particle.Fluid->GetRestDensity()));
		}) / fluid->GetRestDensity();

		totalParticles += fluid->Particles->size();
	}

	LastAverageDensityError = sum / totalParticles;

	return sum / totalParticles;
}

void USolver::ComputeSolverStatistics(bool computeSolverStats)
{
	throw("This shouldn't be called. This is a abstract parent class.");
}

ESolverMethod USolver::GetSolverType() const {
	return SolverType;
}

double USolver::GetCurrentTimestep() const
{
	return CurrentTimestep;
}

FComputationTimesPerStep USolver::GetComputationTimes() const
{
	return ComputationTimes;
}

float USolver::GetLastAverageDensityError() const
{
	return LastAverageDensityError;
}

int USolver::GetLastIterationCount() const
{
	return LastIterationCount;
}

void USolver::FindNeighbors()
{
	FDateTime startTime = FDateTime::UtcNow();
	GetNeighborsFinder()->FindNeighbors(*GetParticleContext(), Simulator->GetParticleContext()->GetParticleDistance(), GetBoundaryPressure()->GetRequiredNeighborhoods());
	ComputationTimes.NeighborhoodSearchTime = (FDateTime::UtcNow() - startTime).GetTotalSeconds();
}

void USolver::InitializePeriodicCondition()
{
	// In the special case of a periodic setting new particles are generated at necessary positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->MoveOutOfBoundsParticles();
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticles();
	}
}

void USolver::ClearAcceleration()
{
	for (UFluid* fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.Acceleration = Vector3D::Zero;
		});
	}

	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAcceleration();
	}
}

void USolver::ComputeDensitiesExplicit() {
	for (UFluid* fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			f.Density = 0;
			double fluidDensitySum = 0;
			double staticDensitySum = 0;

			for (const Particle& ff : f.FluidNeighbors) {
				fluidDensitySum += ff.Mass  * GetKernel()->ComputeKernel(f, ff);
			}

			for (const Particle& fb : f.StaticBorderNeighbors) {
				staticDensitySum += fb.Border->BorderDensityFactor * fb.Mass * GetKernel()->ComputeKernel(f, fb);
			}

			// sum the contributions of neighbors
			f.Density = fluidDensitySum + staticDensitySum;
		});
	}

	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleDensity();
	}
}

void USolver::IntegrateEulerCromer()
{
	for (UFluid* fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);

			if (!particle.IsScripted) {
				particle.Velocity += particle.Acceleration * CurrentTimestep;
				particle.Position += particle.Velocity * CurrentTimestep;
			}

		});
	}

	// In the special case of a periodic setting update positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAttributes(true);
	}
}

void USolver::ApplyScriptedVolumesBeforeIntegration()
{
	// clean the IsScripted Flags
	for (UFluid* fluid : GetParticleContext()->GetFluids()) {
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& particle = fluid->Particles->at(i);
			particle.IsScripted = false;
		});
	}


	// Apply the effects of the volumes
	FDateTime ScriptedVolumeStartTime = FDateTime::UtcNow();
	for (AScriptedVolume * volume : Volumes) {
		volume->ApplyBeforeIntegration(*GetParticleContext(), GetCurrentTimestep(), GetSimulator()->GetSimulatedTime());
	}
	ComputationTimes.ScriptedTime = (FDateTime::UtcNow() - ScriptedVolumeStartTime).GetTotalSeconds();

	// In the special case of a periodic setting new particles are generated at necessary positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAttributes(true);
	}
}

void USolver::ApplyScriptedVolumesAfterIntegration()
{
	// Apply the effects of the volumes
	FDateTime ScriptedVolumeStartTime = FDateTime::UtcNow();
	for (AScriptedVolume * volume : Volumes) {
		volume->ApplyAfterIntegration(*GetParticleContext(), GetCurrentTimestep(), GetSimulator()->GetSimulatedTime());
	}
	ComputationTimes.ScriptedTime += (FDateTime::UtcNow() - ScriptedVolumeStartTime).GetTotalSeconds();

	// In the special case of a periodic setting new particles are generated at necessary positions
	if (GetParticleContext()->GetPeriodicCondition() != nullptr) {
		GetParticleContext()->GetPeriodicCondition()->UpdateGhostParticleAttributes(true);
	}
}

ASimulator * USolver::GetSimulator() const
{
	return Simulator;
}

UParticleContext * USolver::GetParticleContext() const
{
	return GetSimulator()->GetParticleContext();
}

UKernel * USolver::GetKernel() const
{
	return Simulator->GetKernel();
}

const std::vector<UFluid*>& USolver::GetFluids() const
{
	return GetSimulator()->GetParticleContext()->GetFluids();
}

const std::vector<UStaticBorder*>& USolver::GetBorders() const
{
	return GetSimulator()->GetParticleContext()->GetStaticBorders();
}

UNeighborsFinder * USolver::GetNeighborsFinder() const
{
	return Simulator->GetNeighborsFinder();
}

UBoundaryPressure * USolver::GetBoundaryPressure() const
{
	return BoundaryPressureComputer;
}

TArray<UAcceleration*> USolver::GetAccelerations() const
{
	return Accelerations;
}

TArray<AScriptedVolume*> USolver::GetScriptedVolumes() const
{
	return Volumes;
}

