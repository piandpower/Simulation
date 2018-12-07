// Fill out your copyright notice in the Description page of Project Settings.

#include "Fluid.h"
#include "Simulator.h"
#include "UnrealComponents/FluidSpawner.h"


// Sets default values
UFluid::UFluid() : 
	CurrentAverageDensityError(0.0f),
	CurrentKineticEnergy(0.0f),
	CurrentPotentialEnergy(0.0f)
{
}

UFluid::~UFluid()
{
}

void UFluid::InitializeBoxFluid(float particleDistance, FTransform transform, FVector initialVelocity, float fluidDensity, float viscosity, float massFactor, EDimensionality dimensionality, bool displaceParticles) {

	FVector size = transform.GetScale3D();
	FVector position = transform.GetLocation();
	FQuat rotation = transform.GetRotation();

	Particles = std::make_unique<std::vector<Particle>>();

	RestDensity = fluidDensity;
	Viscosity = viscosity;
	MassFactor = massFactor;

	bool displace = false;

	// build the box  of particles
	for (double z = -size.Z / 2 + std::fmod(size.Z, particleDistance) / 2; z <= size.Z / 2; z += particleDistance) {
		for (double y = -size.Y / 2 + std::fmod(size.Y, particleDistance) / 2; y <= size.Y / 2; y += particleDistance) {
			for (double x = -size.X / 2 + std::fmod(size.X, particleDistance) / 2; x <= size.X / 2; x += particleDistance) {

				Vector3D pos = { x, y, z };

				if (displaceParticles) {
					if (displace) {
						if (dimensionality == EDimensionality::Three) {
							pos += { particleDistance / 4, particleDistance / 4, 0.f};
						}
						if (dimensionality == EDimensionality::Two) {
							pos += { particleDistance / 4, 0.f, 0.f};
						}
					}
					else {
						if (dimensionality == EDimensionality::Three) {
							pos -= { particleDistance / 4, particleDistance / 4, 0.f};
						}
						if (dimensionality == EDimensionality::Two) {
							pos -= { particleDistance / 4, 0.f, 0.f};
						}
					}

					//pos.Z *= 1.05;
				}

				pos = rotation.RotateVector(pos);
				pos += (Vector3D)position;

				Particles->emplace_back(pos, (Vector3D)initialVelocity, 0, this);
			}
		}
		displace = !displace;
	}
	ComputeMasses(dimensionality, particleDistance, fluidDensity);
}

void UFluid::InitializeFluidFromPositionsAndVelocities(TArray<Vector3D>& positions, TArray<Vector3D>& velocities, float fluidDensity, float viscosity, EDimensionality dimensionality) {

	if (positions.Num() != velocities.Num()) {
		throw("There have to be the same amount of velocities and positions!");
	}
	
	Particles = std::make_unique<std::vector<Particle>>();
	Particles->reserve(positions.Num());

	RestDensity = fluidDensity;
	Viscosity = viscosity;

	for (int i = 0; i < positions.Num(); i++) {
		Particles->emplace_back(positions[i], velocities[i], 0, this);
	}

	// fill in masses
	ComputeMasses(dimensionality, GetParticleContext()->GetParticleDistance(), fluidDensity);

}

void UFluid::InitializeFluidFromPositionsVelocitiesAndMasses(TArray<Vector3D>& positions, TArray<Vector3D>& velocities, TArray<double> masses, float fluidDensity, float viscosity)
{
	if (positions.Num() != velocities.Num()) {
		throw("There have to be the same amount of velocities and positions!");
	}

	Particles = std::make_unique<std::vector<Particle>>();

	Particles->reserve(positions.Num());

	RestDensity = fluidDensity;
	Viscosity = viscosity;

	for (int i = 0; i < positions.Num(); i++) {
		Particles->emplace_back(positions[i], velocities[i], masses[i], this);
	}
}

UFluid * UFluid::CreateFluidFromSpawner(AFluidSpawner * fluidSpawner)
{
	UFluid * fluid = NewObject<UFluid>();

	fluid->FluidSpawner = fluidSpawner;
	fluid->SpawnSource = SpawnedFrom::Box;
	fluid->AddToRoot();

	return fluid;
}

UFluid * UFluid::CreateFluidFromPositionsAndVelocities(TArray<FVector> positions, TArray<FVector> velocities, float restDensity, float viscosity, float massFactor)
{
	UFluid * fluid = NewObject<UFluid>();

	fluid->SpawnSource = SpawnedFrom::PositionsAndVelocities;
	fluid->AddToRoot();

	fluid->RestDensity = restDensity;
	fluid->MassFactor = massFactor;
	fluid->Viscosity = viscosity;

	return fluid;
}

void UFluid::Build(UParticleContext * particleContext, EDimensionality dimensionality)
{
	ParticleContext = particleContext;

	switch (SpawnSource) {
	case SpawnedFrom::File:
		ReadFluidFromFileBlueprint(FluidFile, this);
		break;
	case SpawnedFrom::Box:
	{
		// Rescaling since unreal units are  different
		FTransform transform = FTransform(FluidSpawner->GetTransform().GetRotation(),
			FluidSpawner->GetTransform().GetLocation() / 10,
			FluidSpawner->GetTransform().GetScale3D() * 10
		);

		InitializeBoxFluid(GetParticleContext()->GetParticleDistance(),
			transform,
			FluidSpawner->InitialVelocity,
			FluidSpawner->RestDensity,
			FluidSpawner->Viscosity,
			FluidSpawner->MassFactor,
			dimensionality,
			FluidSpawner->DisplaceFluidParticles);

		FluidSpawner->Destroy();
		FluidSpawner = nullptr;
		break;
	}
	case SpawnedFrom::PositionsAndVelocities:

		InitializeFluidFromPositionsAndVelocities(SpawnPositions, SpawnVelocities, RestDensity, GetViscosity(), dimensionality);
		break;
	}
}

UFluid * UFluid::CreateFluidFromFile(FString fluidFile)
{
	UFluid * fluid = NewObject<UFluid>();

	fluid->FluidFile = fluidFile;
	fluid->SpawnSource = SpawnedFrom::File;
	fluid->AddToRoot();
	return fluid;
}

UParticleContext * UFluid::GetParticleContext() const
{
	return ParticleContext;
}

UFluid::operator int()
{
	return Index;
}

double UFluid::GetRestVolume() const
{
	return pow(GetParticleContext()->GetParticleDistance(), 3);
}

double UFluid::GetRestDensity() const
{
	return RestDensity;
}

double UFluid::GetMassFactor() const
{
	return MassFactor;
}

double UFluid::GetViscosity() const
{
	return Viscosity;
}

void UFluid::CalculateEnergies(bool calculateEnergies)
{
	if (calculateEnergies) {
		CalculatePotentialEnergy();
		CalculateKineticEnergy();
	}
}

double UFluid::CalculatePotentialEnergy()
{
	CurrentPotentialEnergy = ParallelSum<Particle, double>(*Particles, [](const Particle& particle) { return particle.Mass * 9.81 * particle.Position.Z; });
	
	return CurrentPotentialEnergy;
}

double UFluid::CalculateKineticEnergy()
{
	CurrentKineticEnergy = ParallelSum<Particle, double>(*Particles, [](const Particle& particle) { return 0.5 * particle.Mass * pow(particle.Velocity.Size(), 2); });

	return CurrentKineticEnergy;
}

double UFluid::GetPotentialEnergy() const
{
	return CurrentPotentialEnergy;
}

double UFluid::GetKineticEnergy() const
{
	return CurrentKineticEnergy;
}

double UFluid::GetTotalEnergy() const
{
	return CurrentKineticEnergy + CurrentPotentialEnergy;
}

float UFluid::GetPotentialEnergyBlueprint() const
{
	return GetPotentialEnergy();
}

float UFluid::GetKineticEnergyBlueprint() const
{
	return GetKineticEnergy();
}

float UFluid::GetTotalEnergyBlueprint() const
{
	return GetTotalEnergy();
}

TArray<FVector> UFluid::GetAllParticlePositions()
{
	TArray<FVector> positions;

	for (Particle particle : *Particles) {
		positions.Add(particle.Position);
	}
	return positions;
}

void UFluid::AddParticle(const Particle& particle)
{
	Particles->push_back(particle);
}

void UFluid::AddParticle(FVector position, FVector velocity)
{

	double mass;

	switch (GetParticleContext()->GetSimulator()->GetDimensionality()) {
	case One:
		mass = GetParticleContext()->GetParticleDistance() * RestDensity;
		break;
	case Two:
		mass = pow(GetParticleContext()->GetParticleDistance(), 2) * RestDensity;
		break;
	case Three:
		mass = pow(GetParticleContext()->GetParticleDistance(), 3) * RestDensity;
		break;
	}

	Particles->emplace_back((Vector3D)position, (Vector3D)velocity, mass, this);
}

void UFluid::AddParticles(const std::vector<FVector>& positions, const std::vector<FVector>& velocities) {
	if (positions.size() != velocities.size()) {
		throw("Not as many positions as velocities given");
	}

	double mass;

	switch (GetParticleContext()->GetSimulator()->GetDimensionality()) {
	case One:
		mass = GetParticleContext()->GetParticleDistance() * RestDensity;
		break;
	case Two:
		mass = pow(GetParticleContext()->GetParticleDistance(), 2) * RestDensity;
		break;
	case Three:
		mass = pow(GetParticleContext()->GetParticleDistance(), 3) * RestDensity;
		break;
	}
	
	Particles->reserve(Particles->size() + positions.size());

	for (int i = 0; i < positions.size(); i++) {
		Particles->emplace_back((Vector3D)positions[i], (Vector3D)velocities[i], mass, this);
	}

}

bool UFluid::RemoveParticle(int index) {

	if (index >= Particles->size()) {
		return false;
	}

	Particles->erase(Particles->begin() + index);

	return true;
}

bool UFluid::RemoveParticle(Particle & particle) {

	size_t oldSize = Particles->size();
	std::remove_if(Particles->begin(), Particles->end(),
		[&](const Particle & particleCandidate) {
		return &particleCandidate == &particle;
	});

	bool deletedParticle = Particles->size() != oldSize;

	return deletedParticle;
}

int UFluid::RemoveParticles(std::function<bool(const Particle&)> criteria) {
	size_t oldSize = Particles->size();
	Particles->erase(std::remove_if(Particles->begin(), Particles->end(), criteria), Particles->end());

	int deletedParticles = oldSize - Particles->size();

	return deletedParticles;
}

int UFluid::GetNumParticles() const
{
	return Particles->size();
}

void UFluid::ComputeMasses(EDimensionality dimensionality, double particleDistance, double fluidDensity)
{
	switch (dimensionality) {
	case One:
		ParallelFor(Particles->size(), [&](int32 i) {
			Particle& particle = Particles->at(i);
			particle.Mass = MassFactor * particleDistance * fluidDensity;
		});
		break;
	case Two:
		ParallelFor(Particles->size(), [&](int32 i) {
			Particle& particle = Particles->at(i);
			particle.Mass = MassFactor * pow(particleDistance, 2) * fluidDensity;
		});
		break;
	case Three:
		ParallelFor(Particles->size(), [&](int32 i) {
			Particle& particle = Particles->at(i);
			particle.Mass = MassFactor * pow(particleDistance, 3) * fluidDensity;
		});
		break;
	}
}

UFluid * UFluid::ReadFluidFromFileBlueprint(FString file, UFluid * fluid) {
	return ReadFluidFromFile(TCHAR_TO_UTF8(*file), fluid);
}

UFluid * UFluid::ReadFluidFromFile(std::string file, UFluid * fluid) {

	std::ifstream fluidFile;
	fluidFile.open(file);

	if (!fluidFile.is_open()) {
		return nullptr;
	}


	std::string line;
	// test if first line is "Fluid" header
	if (std::getline(fluidFile, line))
	{
		if (line == "Fluid") {
			// all good, continue
		}
		else {
			// first line isn't fluid
			return nullptr;
		}
	}
	else {
		// Couldn't read first line
		return nullptr;
	}

	double restDensity = 0;
	// retrieve RestDenstiy
	if (std::getline(fluidFile, line))
	{
		if (line.find("Rest Density:\t") != std::string::npos) {
			// all good, continue reading density
			try {
				restDensity = std::stod(line.substr(14, line.length() - 14));
			}
			catch (std::invalid_argument) {
				// No density could be read
				return nullptr;
			}
		}
		else {
			// couldn't find "Rest Density" Keyword
			return nullptr;
		}
	}
	else {
		// couldn't find "Rest Density" Keyword
		return nullptr;
	}

	double viscosity = 0;
	// retrieve Viscosity
	if (std::getline(fluidFile, line))
	{
		if (line.find("Viscosity:\t") != std::string::npos) {
			// all good, continue reading viscosity
			try {
				viscosity = std::stod(line.substr(11, line.length() - 11));
			}
			catch (std::invalid_argument) {
				// No viscosity could be read
				return nullptr;
			}
		}
		else {
			// couldn't find "Vicosity" Keyword
			return nullptr;
		}
	}
	else {
		// couldn't read next line
		return nullptr;
	}


	double particleDistance = 0;
	// retrieve Particle distance
	if (std::getline(fluidFile, line))
	{
		if (line.find("Particle Distance:\t") != std::string::npos) {
			// all good, continue reading particle distance
			try {
				particleDistance = std::stod(line.substr(19, line.length() - 19));
			}
			catch (std::invalid_argument) {
				// No particle distance could be read
				return nullptr;
			}
		}
		else {
			// couldn't find "Particle Distance" Keyword
			return nullptr;
		}
	}
	else {
		// couldn't read next line
		return nullptr;
	}


	// check Particle keyword
	if (std::getline(fluidFile, line))
	{
		if (line != "Particles") {
			// couldn't find "Particles" Keyword
			return nullptr;
		}
	}
	else {
		// couldn't read next line
		return nullptr;
	}

	TArray<Vector3D> positions =  TArray<Vector3D>();
	TArray<Vector3D> velocities = TArray<Vector3D>();
	TArray<double> masses = TArray<double>();

	// read all particles
	while (std::getline(fluidFile, line)) {

		if (line == "") {
			break;
		}

		Vector3D position;
		Vector3D velocity;
		double mass;

		int startindex = 6;
		int endindex = line.find("\t", startindex);
		try {
			mass = std::stod(line.substr(startindex, endindex - startindex));
		}
		catch (std::invalid_argument) {
			// No particle mass could be read
			return nullptr;
		}

		
		startindex = line.find("Position: ");
		endindex = line.find("\t", startindex);

		try {
			position = Vector3D::FromString(line.substr(startindex + 10, endindex - startindex - 10));
		}
		catch (std::invalid_argument) {
			// No position could be read
			return nullptr;
		}

		startindex = line.find("Velocity: ");
		endindex = line.find("\n", startindex);

		try {
			velocity = Vector3D::FromString(line.substr(startindex + 10, endindex - startindex - 10));
		}
		catch (std::invalid_argument) {
			// No position could be read
			return nullptr;
		}

		positions.Add(position);
		velocities.Add(velocity);
		masses.Add(mass);

	}
	fluid->InitializeFluidFromPositionsVelocitiesAndMasses(positions, velocities, masses, restDensity, viscosity);

	return fluid;
}

void UFluid::WriteFluidToFile(const std::string file, int iterationCount)
{
	// Write fluid
	std::ofstream fluidFile;
	fluidFile.open(file, std::ios_base::app);

	// test if we can write
	if (!fluidFile.is_open()) {
		throw("Could't write in file: " + file);
	}

	fluidFile << "Fluid" << std::endl;
	fluidFile << "Rest Density:\t" << RestDensity << std::endl;
	fluidFile << "Viscosity:\t" << Viscosity << std::endl;
	fluidFile << "Particle Distance:\t" << GetParticleContext()->GetParticleDistance() << std::endl;
	fluidFile << "Particles" << std::endl;
	for (Particle & particle : *Particles) {
		fluidFile << "Mass: " << particle.Mass << "\t" << "Position: " << particle.Position.ToString() << "\t" << "Velocity: " << particle.Velocity.ToString() << std::endl;
	}
	fluidFile << std::endl;
	fluidFile.close();
}
