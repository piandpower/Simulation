#include "NaiveNeighborsFinder.h"

UNaiveNeighborsFinder::UNaiveNeighborsFinder()
{
	NeighborsFinderType = ENeighborhoodSearch::Naive;
}

UNaiveNeighborsFinder::~UNaiveNeighborsFinder()
{
}

UNaiveNeighborsFinder * UNaiveNeighborsFinder::CreateNaiveNeighborsFinder()
{
	UNaiveNeighborsFinder * naiveNeighborsFinder = NewObject<UNaiveNeighborsFinder>();

	naiveNeighborsFinder->NeighborsFinderType = ENeighborhoodSearch::Naive;

	naiveNeighborsFinder->AddToRoot();

	return naiveNeighborsFinder;
}

void UNaiveNeighborsFinder::FindNeighbors(const UParticleContext& particleContext, double particleDistance, FNeighborsSearchRelations searchRelations)
{
	if (searchRelations.FluidNeedsNeighbors()) {
		// Register the neighbors of fluid particles
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& f = fluid->Particles->at(i);

				if (searchRelations.FluidNeighborsOfFluidRequired) {
					f.FluidNeighbors.clear();
					for (UFluid * neighborFluid : particleContext.GetFluids()) {
						for (int j = 0; j < neighborFluid->Particles->size(); j++) {
							Particle& ff = neighborFluid->Particles->at(j);
							// if distance between particles is smaller as supportrange * h, then add the particle to neighboring particles 
							if ((f.Position - ff.Position).Size() < (SupportRange * particleDistance)) {
								f.FluidNeighbors.emplace_back(j, ff);
							}
						}
					}

					if (particleContext.GetPeriodicCondition() != nullptr) {
						UPeriodicCondition * periodic = particleContext.GetPeriodicCondition();
						for (int j = 0; j < periodic->GetGhostParticles().size(); j++) {
							Particle& ghost = const_cast<Particle&>(periodic->GetGhostParticles()[j]);

							if ((ghost.Position - f.Position).Size() < SupportRange * particleDistance) {
								f.FluidNeighbors.emplace_back(periodic->GetReferencedParticlesIndices()[j], ghost);
							}
						}
					}
				}

				if (searchRelations.StaticBorderNeighborsOfFluidRequired) {
					f.StaticBorderNeighbors.clear();
					for (UStaticBorder * border : particleContext.GetStaticBorders()) {
						for (int j = 0; j < border->Particles->size(); j++) {
							Particle& fb = border->Particles->at(j);
							// if distance between particles is smaller as 2 * h, then add the particle to neighboring particles 
							if ((f.Position - fb.Position).Size() < (SupportRange * particleDistance)) {
								f.StaticBorderNeighbors.emplace_back(j, fb);
							}
						}
					}
				}
			});
		}
	}
	
	if (searchRelations.StaticBorderNeedsNeighbors()) {
		for (UStaticBorder * originBorder : particleContext.GetStaticBorders()) {
			ParallelFor(originBorder->Particles->size(), [&](int32 i) {
				Particle& b = originBorder->Particles->at(i);

				if (searchRelations.FluidNeighborsOfStaticBorderRequired) {
					b.FluidNeighbors.clear();
					for (UFluid * neighborFluid : particleContext.GetFluids()) {
						for (int j = 0; j < neighborFluid->Particles->size(); j++) {
							Particle& bf = neighborFluid->Particles->at(j);
							// if distance between particles is smaller as 2 * h, then add the particle to neighboring particles 
							if ((b.Position - bf.Position).Size() < (SupportRange * particleDistance)) {
								b.FluidNeighbors.emplace_back(j, bf);
							}
						}
					}

					if (particleContext.GetPeriodicCondition() != nullptr) {
						UPeriodicCondition * periodic = particleContext.GetPeriodicCondition();
						for (int j = 0; j < periodic->GetGhostParticles().size(); j++) {
							Particle& ghost = const_cast<Particle&>(periodic->GetGhostParticles()[j]);

							if ((ghost.Position - b.Position).Size() < SupportRange * particleDistance) {
								b.FluidNeighbors.emplace_back(periodic->GetReferencedParticlesIndices()[j], ghost);
							}
						}
					}
				}

				if (searchRelations.StaticBorderNeighborsOfStaticBorderRequired) {
					b.StaticBorderNeighbors.clear();
					for (UStaticBorder * neighborBorder : particleContext.GetStaticBorders()) {
						for (int j = 0; j < neighborBorder->Particles->size(); j++) {
							Particle& bb = neighborBorder->Particles->at(j);
							// if distance between particles is smaller as 2 * h, then add the particle to neighboring particles 
							if ((b.Position - bb.Position).Size() < (SupportRange * particleDistance)) {
								b.StaticBorderNeighbors.emplace_back(j, bb);
							}
						}
					}
				}
			});
		}
	}
}


void UNaiveNeighborsFinder::AddStaticParticles(UStaticBorder * borders, double particleDistance)
{
}

UNaiveNeighborsFinder * UNaiveNeighborsFinder::CreateNaiveNeighborhoodSearch()
{
	UNaiveNeighborsFinder * naiveNeighborsFinder = NewObject<UNaiveNeighborsFinder>();

	naiveNeighborsFinder->NeighborsFinderType = ENeighborhoodSearch::Naive;

	// prevent garbage collection
	naiveNeighborsFinder->AddToRoot();

	return naiveNeighborsFinder;
}
