#include "HashNeighborsFinder.h"

UHashNeighborsFinder::~UHashNeighborsFinder()
{
}

UHashNeighborsFinder * UHashNeighborsFinder::CreateHashNeighborsFinder()
{
	UHashNeighborsFinder * hashNeighborsFinder = NewObject<UHashNeighborsFinder>();

	hashNeighborsFinder->NeighborsFinderType = ENeighborhoodSearch::SpatialHash;

	hashNeighborsFinder->AddToRoot();

	return hashNeighborsFinder;
}

void UHashNeighborsFinder::AddStaticParticles(TArray<UStaticBorder*>& borders, double particleDistance)
{
	for (UStaticBorder * border : borders) {
		AddStaticParticles(border, particleDistance);
	}
}

void UHashNeighborsFinder::AddStaticParticles(std::vector<UStaticBorder*>& borders, double particleDistance) {
	for (UStaticBorder * border : borders) {
		AddStaticParticles(border, particleDistance);
	}
}

void UHashNeighborsFinder::AddStaticParticles(UStaticBorder * border, double particleDistance) {
	for (int i = 0; i < border->Particles->size(); i++) {
		Particle& b = border->Particles->at(i);

		int x = GetHash(b, SupportRange * particleDistance);

		if (StaticHashtable.count(GetHash(b, SupportRange * particleDistance)) == 1) {
			// if there is already a list at this box, just add the neighbor entry
			StaticHashtable.at(GetHash(b, SupportRange * particleDistance)).emplace_back(i, b);
		}
		else {

			// Create new list at Hash and add the neighbor entry
			StaticHashtable.insert(std::pair<int, std::vector<StaticBorderNeighbor>>(GetHash(b, SupportRange * particleDistance), std::vector<StaticBorderNeighbor>()));
			StaticHashtable.at(GetHash(b, SupportRange * particleDistance)).emplace_back(i, b);
		}
	}
}

void UHashNeighborsFinder::FindNeighbors(const UParticleContext& particleContext, double particleDistance, FNeighborsSearchRelations searchRelations)
{
	if (searchRelations.FluidNeedsNeighbors()) {
		FillHashtableDynamic(particleContext, particleDistance);
		RegisterNeighborsFluids(particleContext.GetFluids(), particleDistance, searchRelations);
	}
	if (searchRelations.StaticBorderNeedsNeighbors()) {
		RegisterNeighborsStaticBorders(particleContext.GetStaticBorders(), particleDistance, searchRelations);
	}

	FindPeriodicNeighbors(particleContext, searchRelations);
}


void UHashNeighborsFinder::FindBorderNeighbors(UStaticBorder * border, double particleDistance)
{
	// Fill the particles in the hashtable
	AddStaticParticles(border, particleDistance);

	// Register the neighbors
	ParallelFor(border->Particles->size(), [&](int32 i) {
		Particle& b = border->Particles->at(i);

		b.StaticBorderNeighbors.clear();

		int xGrid = floor(b.Position.X / (SupportRange * particleDistance));
		int yGrid = floor(b.Position.Y / (SupportRange * particleDistance));
		int zGrid = floor(b.Position.Z / (SupportRange * particleDistance));

		for (double xOffset = -1; xOffset <= 1; xOffset++) {
			for (double yOffset = -1; yOffset <= 1; yOffset++) {
				for (double zOffset = -1; zOffset <= 1; zOffset++) {

					// get hashindex of current cell
					int hash = GetHash(xGrid + xOffset, yGrid + yOffset, zGrid + zOffset);

					// only search through static particles like borders
					if (StaticHashtable.find(hash) != StaticHashtable.end()) {
						std::vector<StaticBorderNeighbor>& neighbors = StaticHashtable.at(hash);
						for (StaticBorderNeighbor& bb : neighbors) {
							// check if particle is near enough
							if ((b.Position - bb.GetParticle()->Position).Size() < (SupportRange * particleDistance)) {
								b.StaticBorderNeighbors.push_back(bb);
							}
						}
					}
				}
			}
		}
	});
}

void UHashNeighborsFinder::FillHashtableDynamic(const UParticleContext& particleContext, double particleDistance)
{
	DynamicHashtable.clear();
	int totalFluidParticles = 0;
	for (UFluid* fluid : particleContext.GetFluids()) {
		totalFluidParticles += fluid->Particles->size();
	}

	DynamicHashtable.reserve(totalFluidParticles);
	
	for (UFluid* fluid : particleContext.GetFluids()) {
		for (int i = 0; i < fluid->Particles->size(); i++) {
			Particle& f = fluid->Particles->at(i);


			if (DynamicHashtable.count(GetHash(f, (SupportRange * particleDistance))) == 1) {
				// if there is already a list at this box, just add the particle
				DynamicHashtable.at(GetHash(f, (SupportRange * particleDistance))).emplace_back(i, f);
			}
			else {
				// Create new list at Hash and add the particle
				DynamicHashtable.insert(std::pair<int, std::vector<FluidNeighbor>>(GetHash(f, (SupportRange * particleDistance)), std::vector<FluidNeighbor>()));
				DynamicHashtable.at(GetHash(f, (SupportRange * particleDistance))).emplace_back(i, f);
			}
		}
	}
}

void UHashNeighborsFinder::RegisterNeighborsFluids(const std::vector<UFluid*>& fluids, double particleDistance, FNeighborsSearchRelations searchRelations)
{
	ParallelFor(fluids.size(), [&](int fluidIndex) {
		UFluid * fluid = fluids[fluidIndex];
		ParallelFor(fluid->Particles->size(), [&](int32 i) {
			Particle& f = fluid->Particles->at(i);

			if (searchRelations.FluidNeighborsOfFluidRequired)
				f.FluidNeighbors.clear();
			if (searchRelations.StaticBorderNeighborsOfFluidRequired)
				f.StaticBorderNeighbors.clear();


			int xGrid = floor(f.Position.X / (SupportRange * particleDistance));
			int yGrid = floor(f.Position.Y / (SupportRange * particleDistance));;
			int zGrid = floor(f.Position.Z / (SupportRange * particleDistance));;

			for (int xOffset = -1; xOffset <= 1; xOffset++) {
				for (int yOffset = -1; yOffset <= 1; yOffset++) {
					for (int zOffset = -1; zOffset <= 1; zOffset++) {

						// get hashindex of current cell
						int hash = GetHash(xGrid + xOffset, yGrid + yOffset, zGrid + zOffset);

						if (searchRelations.FluidNeighborsOfFluidRequired) {
							// search through dynamic particles like fluid and rigid bodies
							if (DynamicHashtable.find(hash) != DynamicHashtable.end()) {
								std::vector<FluidNeighbor>& neighbors = DynamicHashtable.at(hash);
								for (const FluidNeighbor& neighbor : neighbors) {
									// check if particle is near enough
									if ((f.Position - neighbor.GetParticle()->Position).Size() < (SupportRange * particleDistance)) {
										f.FluidNeighbors.push_back(neighbor);
									}
								}
							}

						}
						
						if (searchRelations.StaticBorderNeighborsOfFluidRequired) {
							// Search through static particles like borders
							if (StaticHashtable.find(hash) != StaticHashtable.end()) {
								std::vector<StaticBorderNeighbor>& neighbors = StaticHashtable.at(hash);
								for (const StaticBorderNeighbor& neighbor : neighbors) {
									// check if particle is near enough
									if ((f.Position - neighbor.GetParticle()->Position).Size() < (SupportRange * particleDistance)) {
										f.StaticBorderNeighbors.push_back(neighbor);
									}
								}
							}
						}
					}
				}
			}
		});
	});
}

void UHashNeighborsFinder::RegisterNeighborsStaticBorders(const std::vector<UStaticBorder*>& borders, double particleDistance, FNeighborsSearchRelations searchRelations)
{
	for (UStaticBorder * border : borders) {
		ParallelFor(border->Particles->size(), [&](int32 i) {
			Particle& b = border->Particles->at(i);

			if (searchRelations.FluidNeighborsOfStaticBorderRequired)
				b.FluidNeighbors.clear();
			if (searchRelations.StaticBorderNeighborsOfStaticBorderRequired)
				b.StaticBorderNeighbors.clear();


			int xGrid = floor(b.Position.X / (SupportRange * particleDistance));
			int yGrid = floor(b.Position.Y / (SupportRange * particleDistance));;
			int zGrid = floor(b.Position.Z / (SupportRange * particleDistance));;

			for (int xOffset = -1; xOffset <= 1; xOffset++) {
				for (int yOffset = -1; yOffset <= 1; yOffset++) {
					for (int zOffset = -1; zOffset <= 1; zOffset++) {

						// get hashindex of current cell
						int hash = GetHash(xGrid + xOffset, yGrid + yOffset, zGrid + zOffset);

						if (searchRelations.FluidNeighborsOfStaticBorderRequired) {
							// search through dynamic particles like fluid and rigid bodies
							if (DynamicHashtable.find(hash) != DynamicHashtable.end()) {
								std::vector<FluidNeighbor>& neighbors = DynamicHashtable.at(hash);
								for (const FluidNeighbor& bf : neighbors) {
									// check if particle is near enough
									if ((b.Position - bf.GetParticle()->Position).Size() < (SupportRange * particleDistance)) {
										b.FluidNeighbors.push_back(bf);
									}
								}
							}
						}

						if (searchRelations.StaticBorderNeighborsOfStaticBorderRequired) {
							// Search through static particles like borders
							if (StaticHashtable.find(hash) != StaticHashtable.end()) {
								std::vector<StaticBorderNeighbor>& neighbors = StaticHashtable.at(hash);
								for (const StaticBorderNeighbor& bb : neighbors) {
									// check if particle is near enough
									if ((b.Position - bb.GetParticle()->Position).Size() < (SupportRange * particleDistance)) {
										b.StaticBorderNeighbors.push_back(bb);
									}
								}
							}
						}
					}
				}
			}
		});
	}
}

void UHashNeighborsFinder::FindPeriodicNeighbors(const UParticleContext& particleContext, FNeighborsSearchRelations searchRelations)
{
	if (particleContext.GetPeriodicCondition() != nullptr && searchRelations.FluidNeighborsRequired()) {
		UPeriodicCondition * periodic = particleContext.GetPeriodicCondition();
		for (int j = 0; j < periodic->GetGhostParticles().size(); j++) {
			Particle& ghost = const_cast<Particle&>(periodic->GetGhostParticles()[j]);

			int xGrid = floor(ghost.Position.X / (SupportRange * particleContext.GetParticleDistance()));
			int yGrid = floor(ghost.Position.Y / (SupportRange * particleContext.GetParticleDistance()));;
			int zGrid = floor(ghost.Position.Z / (SupportRange * particleContext.GetParticleDistance()));;

			for (int xOffset = -1; xOffset <= 1; xOffset++) {
				for (int yOffset = -1; yOffset <= 1; yOffset++) {
					for (int zOffset = -1; zOffset <= 1; zOffset++) {

						// get hashindex of current cell
						int hash = GetHash(xGrid + xOffset, yGrid + yOffset, zGrid + zOffset);

						if (searchRelations.FluidNeighborsOfFluidRequired) {
							// search through dynamic particles like fluid and rigid bodies
							if (DynamicHashtable.find(hash) != DynamicHashtable.end()) {
								std::vector<FluidNeighbor>& neighbors = DynamicHashtable.at(hash);
								for (const FluidNeighbor& ff : neighbors) {
									// check if particle is near enough and push ghost particle inside if close enough
									if ((ghost.Position - ff.GetParticle()->Position).Size() < (SupportRange * particleContext.GetParticleDistance())) {
										ff.GetParticle()->FluidNeighbors.push_back(FluidNeighbor(periodic->GetReferencedParticlesIndices()[j], ghost));
									}
								}
							}

						}
						if (searchRelations.FluidNeighborsOfStaticBorderRequired) {
							// search through dynamic particles like fluid and rigid bodies
							if (StaticHashtable.find(hash) != StaticHashtable.end()) {
								std::vector<StaticBorderNeighbor>& neighbors = StaticHashtable.at(hash);
								for (const StaticBorderNeighbor& fb : neighbors) {
									// check if particle is near enough
									if ((ghost.Position - fb.GetParticle()->Position).Size() < (SupportRange *  particleContext.GetParticleDistance())) {
										fb.GetParticle()->FluidNeighbors.push_back(FluidNeighbor(periodic->GetReferencedParticlesIndices()[j], ghost));
									}
								}
							}
						}
					}
				}
			}
	
		}
	}
}

int UHashNeighborsFinder::GetHash(const Vector3D& vector, double supportDistance) {
	return 611657 * (int)floor(vector.X / supportDistance) + 1109 * (int)floor(vector.Y / supportDistance) + (int)floor(vector.Z / supportDistance);
}

int UHashNeighborsFinder::GetHash(const Particle& particle, double supportDistance)
{
	return GetHash(particle.Position, supportDistance);
}

int UHashNeighborsFinder::GetHash(int x, int y, int z)
{
	return 611657 * x + 1109 * y + z;
}