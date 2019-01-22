#include "ParticleContext.h"

#include "Simulator.h"
#include "ParticleContext/Periodic/PeriodicCondition.h"


UParticleContext * UParticleContext::CreateParticleContext(double particleDistance, std::vector<UFluid*> fluids, std::vector<UStaticBorder*> staticBorders, FVisualisationInformation visualisationInformation, UPeriodicCondition * periodicCondition)
{
	UParticleContext * particleContext = NewObject<UParticleContext>();

	if (particleDistance <= 0) {
		throw("Particle distance must be greater than 0");
	}
	particleContext->VisualisationInformation = visualisationInformation;
	particleContext->ParticleDistance = particleDistance;
	particleContext->Fluids = fluids;
	particleContext->StaticBorders = staticBorders;
	particleContext->PeriodicCondition = periodicCondition;

	// prevent garbage collection
	particleContext->AddToRoot();

	return particleContext;
}

UParticleContext * UParticleContext::CreateParticleContext(float particleDistance, TArray<UFluid*> fluids, TArray<UStaticBorder*> staticBorders, FVisualisationInformation visualisationInformation, UPeriodicCondition * periodicCondition) {
	
	UParticleContext * particleContext = NewObject<UParticleContext>();
	
	particleContext->ParticleDistance = particleDistance;
	particleContext->VisualisationInformation = visualisationInformation;
	particleContext->PeriodicCondition = periodicCondition;
	for (UFluid * fluid : fluids) {
		particleContext->Fluids.push_back(fluid);
	}

	for (UStaticBorder * staticBorder : staticBorders) {
		particleContext->StaticBorders.push_back(staticBorder);
	}

	// prevent garbage collection
	particleContext->AddToRoot();

	return particleContext;
}

void UParticleContext::Build(UWorld * world, ASimulator * simulator, EDimensionality dimensionality)
{
	Simulator = simulator;

	for (int i = 0; i < Fluids.size(); i++) {
		Fluids[i]->Build(this, dimensionality);
		Fluids[i]->Index = i;
	}

	for (int i = 0; i < StaticBorders.size(); i++) {
		StaticBorders[i]->Build(this, simulator);
		StaticBorders[i]->Index = i;
	}

	ParticleVisualiser = world->SpawnActor<AParticleCloudActor>(FVector(0), FRotator(0));
	ParticleVisualiser->Build(this, VisualisationInformation);
	
	if (PeriodicCondition != nullptr) {
		PeriodicCondition->Build(this, dimensionality);
	}
}

void UParticleContext::UpdateVisual()
{
	ParticleVisualiser->UpdateParticleContextVisualisation(VisualisationInformation);
}

const std::vector<UFluid*>& UParticleContext::GetFluids() const
{
	return Fluids;
}

const std::vector<UStaticBorder*>& UParticleContext::GetStaticBorders() const
{
	return StaticBorders;
}

TArray<UFluid*> UParticleContext::GetFluidsBlueprint() const
{
	TArray<UFluid*> fluids;

	fluids.Append(&Fluids[0], Fluids.size());

	return fluids;
}

TArray<UStaticBorder*> UParticleContext::GetStaticBordersBlueprint() const
{
	TArray<UStaticBorder*> borders;

	for (UStaticBorder * border : StaticBorders) {
		borders.Add(border);
	}
	return borders;
}

void UParticleContext::AddFluid(UFluid * fluid)
{
	Fluids.push_back(fluid);
	fluid->Build(this, GetSimulator()->GetDimensionality());
	fluid->Index = Fluids.size() - 1;
	UpdateVisual();
}

void UParticleContext::AddStaticBorder(UStaticBorder * staticBorder)
{
	StaticBorders.push_back(staticBorder);
	staticBorder->Build(this, GetSimulator());
	staticBorder->Index = StaticBorders.size() - 1;
	UpdateVisual();
}

void UParticleContext::RemoveFluid(UFluid * fluid)
{
	Fluids.erase(std::remove_if(Fluids.begin(), Fluids.end(), [&](UFluid* candidate) { return candidate == fluid; }), Fluids.end());

	// refresh the fluid indices
	for (int i = 0; i < Fluids.size(); i++) {
		Fluids[i]->Index = i;
	}
	UpdateVisual();
}

void UParticleContext::RemoveFluid(int index)
{
	Fluids.erase(Fluids.begin() + index);

	// refresh the fluid indices
	for (int i = 0; i < Fluids.size(); i++) {
		Fluids[i]->Index = i;
	}
	UpdateVisual();
}

void UParticleContext::RemoveStaticBorder(UStaticBorder * staticBorder)
{
	StaticBorders.erase(std::remove_if(StaticBorders.begin(), StaticBorders.end(), [&](UStaticBorder* candidate) { return candidate == staticBorder; }), StaticBorders.end());

	// refresh the border indices
	for (int i = 0; i < StaticBorders.size(); i++) {
		StaticBorders[i]->Index = i;
	}
	UpdateVisual();
}

void UParticleContext::RemoveStaticBorder(int index)
{
	StaticBorders.erase(StaticBorders.begin() + index);

	// refresh the border indices
	for (int i = 0; i < StaticBorders.size(); i++) {
		StaticBorders[i]->Index = i;
	}
	UpdateVisual();
}

double UParticleContext::GetParticleDistance() const
{
	return ParticleDistance;
}

float UParticleContext::GetParticleDistanceBlueprint() const
{
	return ParticleDistance;
}


ASimulator * UParticleContext::GetSimulator() const
{
	return Simulator;
}

AParticleCloudActor * UParticleContext::GetParticleVisualiser() const
{
	return ParticleVisualiser;
}

UPeriodicCondition * UParticleContext::GetPeriodicCondition() const
{
	return PeriodicCondition;
}

void UParticleContext::SetVisualisationInformation(FVisualisationInformation visualisationInformation)
{
	VisualisationInformation = visualisationInformation;
}

FVisualisationInformation UParticleContext::GetVisualisationInformation() const
{
	return VisualisationInformation;
}
