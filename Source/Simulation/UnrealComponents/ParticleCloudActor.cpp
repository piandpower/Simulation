// Fill out your copyright notice in the Description page of Project Settings.

#include "ParticleCloudActor.h"
#include "ParticleContext/ParticleContext.h"
#include "Simulator.h"

AParticleCloudActor::AParticleCloudActor() {
	PointCloud = NewObject<UPointCloud>();
	PointCloud->RenderMethod = EPointCloudRenderMethod::Sprite_Lit_RGB;

	PointCloud->SectionSize = FVector(100000, 100000, 100000);
	PointCloud->MinimumSectionPointCount = 0;
	PointCloud->bUseLowPrecision = false;
	PointCloud->DensityReductionDistance = 0;
	PointCloud->NoiseReductionDensity = 0;
	PointCloud->NoiseReductionDistance = 0;
	PointCloud->Offset = EPointCloudOffset::None;
	PointCloud->Translation = FVector(0);
	PointCloud->Scale = FVector(1);

	PointCloud->LODBias = 0;
	PointCloud->LODAggressiveness = 0;
	PointCloud->LODCount = 1;
	PointCloud->LODReduction = 0;
	PointCloud->Saturation = 1;
	PointCloud->Brightness = 1;

	PointCloud->Color = FLinearColor(1, 1, 1, 1);
	PointCloud->SpriteSize = FVector2D(10, 10);
	PointCloud->SpriteTexture = nullptr;
	PointCloud->ColorMode = EPointCloudColorMode::RGB;

	PointCloud->SetPointCloudData(Points);
	PointCloud->SpriteMask = EPointCloudSpriteMask::Circle;

	SetCastDynamicShadow(true);
	SetPointCloud(PointCloud);

	UpdatePointCloud();

}

void AParticleCloudActor::Build(UParticleContext * particleContext, FVisualisationInformation visualisationInformation)
{
	ParticleContext = particleContext;
	UpdateParticleContextVisualisation(visualisationInformation);
}

void AParticleCloudActor::UpdatePointCloud() {
	
	PointCloud->ApplyRenderingParameters();
	GetPointCloud()->SetPointCloudData(Points);
	GetPointCloud()->Rebuild(true);
}



template <typename ParticleType>
void AParticleCloudActor::VisualiseParticles(const std::vector<ParticleType>& particles, double size, EColorVisualisation colorMethod, double restDensity)
{
	Points.Empty();
	Points.Reserve(particles.size());
	PointCloud->SpriteSize = FVector2D(size * 10, size * 10);

	switch (colorMethod) {

	case EColorVisualisation::Normal:
		for (const ParticleType& particle : particles) {

			Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
		}
		break;
	case EColorVisualisation::Density:
	{
		double maxDensity = particles.size() > 0 ? ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Density; }) : 0.0;

		double range = maxDensity - restDensity;

		// if range is zero all particles have the same density and are all colored blue
		if (range == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
			}
		}
		// else the density of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = (std::max(particle.Density, restDensity) - restDensity) / range;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
			}
		}

		break;
	}
	case EColorVisualisation::Velocity:
	{
		double maxVelocity = sqrt(ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Velocity.LengthSquared(); }));

		// if maxVelocity is zero all particles stand still
		if (maxVelocity == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
			}
		}
		// else the velcity of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = particle.Velocity.Length() / maxVelocity;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, percentage, (1 - percentage), 0.9f * (1 - percentage));
			}
		}
		break;

	}
	case EColorVisualisation::Pressure:
	{
		double maxPressure = 0;
		if (particles.size() > 0)
			maxPressure = std::max(500.0, ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Pressure; }));

		// if maxVelocity is zero all particles stand still
		if (maxPressure == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 1.f, 1.f);
			}
		}
		// else the velcity of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = particle.Pressure / maxPressure;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage) , (1 - percentage));
			}
		}
		break;

	}
	}
	// use the new points and rebuild the particle cloud
	UpdatePointCloud();

}

template void AParticleCloudActor::VisualiseParticles(const std::vector<Particle>& particles, double size, EColorVisualisation colorMethod, double restDensity);

template <typename ParticleType>
void VisualizeParticles(const std::vector<std::vector<ParticleType>*>& particles, double size, EColorVisualisation colorMethod, double restDensity = 0) {
	Points.Empty();
	int numParticles = 0;
	for (std::vector<ParticleType>* particleVec : particles) {
		numParticles += particleVec->size();
	}

	Points.Reserve(numParticles);
	PointCloud->SpriteSize = FVector2D(size * 10, size * 10);

	for (std::vector<ParticleType>* particlesFromComp : particles) {
		for (const ParticleType& particle : particles) {
			switch
		}
	}
	switch (colorMethod) {
	case EColorVisualisation::Normal:
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
			}
		break;

	case EColorVisualisation::DefaultFluid:
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
			}

		break;
	case EColorVisualisation::Density:
	{
		double maxDensity = particles.size() > 0 ? ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Density; }) : 0.0;

		double range = maxDensity - restDensity;

		// if range is zero all particles have the same density and are all colored blue
		if (range == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
			}
		}
		// else the density of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = (std::max(particle.Density, restDensity) - restDensity) / range;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
			}
		}

		break;
	}
	case EColorVisualisation::Velocity:
	{
		double maxVelocity = sqrt(ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Velocity.LengthSquared(); }));

		// if maxVelocity is zero all particles stand still
		if (maxVelocity == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
			}
		}
		// else the velcity of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = particle.Velocity.Length() / maxVelocity;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, percentage, (1 - percentage), 0.9f * (1 - percentage));
			}
		}
		break;

	}
	case EColorVisualisation::Pressure:
	{
		double maxPressure = 0;
		if (particles.size() > 0)
			maxPressure = std::max(500.0, ParallelMax<ParticleType, double>(particles, [](const ParticleType& particle) { return particle.Pressure; }));

		// if maxVelocity is zero all particles stand still
		if (maxPressure == 0) {
			for (const ParticleType& particle : particles) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 1.f, 1.f);
			}
		}
		// else the velcity of each particle is used to determine color
		else {
			for (const ParticleType& particle : particles) {
				// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
				float percentage = particle.Pressure / maxPressure;
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
			}
		}
		break;

	}
	}
	// use the new points and rebuild the particle cloud
	UpdatePointCloud();

}


void AParticleCloudActor::VisualisePositions(const std::vector<Vector3D>& positions, const std::vector<Vector3D>& velocities, double size, EColorVisualisation colorMethod)
{
	Points.Empty();
	Points.Reserve(positions.size());
	PointCloud->SpriteSize = FVector2D(size * 10, size * 10);

	for (const Vector3D& position : positions) {

		FVector Uposition = FVector(-position.X, position.Y, position.Z) * 10;

		FPointCloudPoint point = FPointCloudPoint(Uposition, FVector(0, 1, 1));

		Points.Add(point);
	}

	UpdatePointCloud();
}

void AParticleCloudActor::VisualisePositions(const std::vector<std::vector<Vector3D>>& positions, const std::vector<std::vector<Vector3D>>& velocities, double size, EColorVisualisation colorMethod) {
	Points.Empty();
	Points.Reserve(positions.size());
	PointCloud->SpriteSize = FVector2D(size * 10, size * 10);

	for (int i = 0; i < positions.size(); i++) {
		for (const Vector3D& position : positions[i]) {

			FVector Uposition = FVector(-position.X, position.Y, position.Z) * 10;

			FPointCloudPoint point = FPointCloudPoint(Uposition, FVector(0, 1, 1));

			Points.Add(point);
		}
	}

	UpdatePointCloud();
}


void AParticleCloudActor::VisualisePositions(const TArray<FVector>& positions, const std::vector<Vector3D>& velocities, double size, EColorVisualisation colorMethod)
{
	Points.Empty();
	Points.Reserve(positions.Num());
	PointCloud->SpriteSize = FVector2D(size * 10, size * 10);

	for (const FVector& position : positions) {

		FVector Uposition = FVector(-position.X, position.Y, position.Z) * 10;

		FPointCloudPoint point = FPointCloudPoint(Uposition, FVector(0, 1, 1));

		Points.Add(point);
	}

	UpdatePointCloud();
}

void AParticleCloudActor::SetVisibilityOfCloud(bool newVisibility)
{
	SetActorHiddenInGame(!newVisibility);
	
	if (!newVisibility) {
		// delete particles if not needed
		TArray<FPointCloudPoint> EmptyPoints;
		GetPointCloud()->SetPointCloudData(EmptyPoints);
		GetPointCloud()->Rebuild(true);
	}
}

void AParticleCloudActor::UpdateParticleContextVisualisation(FVisualisationInformation visualisationInformation)
{
	Points.Empty();
	int numParticles = 0;
	if (visualisationInformation.ShowFluids) {
		for (UFluid* fluid : ParticleContext->GetFluids()) {
			numParticles += fluid->GetNumParticles();
		}
		if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
			numParticles += ParticleContext->GetPeriodicCondition()->GetGhostParticles().size();
		}
	}
	if (visualisationInformation.ShowStaticBorders) {
		for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
			numParticles += staticBorder->GetNumParticles();
		}
	}


	Points.Reserve(numParticles);
	PointCloud->SpriteSize = FVector2D(ParticleContext->GetParticleDistance() * 10, ParticleContext->GetParticleDistance() * 10);

	switch (visualisationInformation.ColorCode) {
	case EColorVisualisation::None:

		break;
	case EColorVisualisation::Normal:
		if (visualisationInformation.ShowFluids) {
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				for (const Particle& particle : *fluid->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
				}
			}

			if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
				for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
				}
			}

		}
		if (visualisationInformation.ShowStaticBorders) {
			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				for (const Particle& particle : *staticBorder->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
				}
			}
		}

		break;
	case EColorVisualisation::Density:
	{
		double maxDensity = 0.0;
		double minDensity = DBL_MAX;
		double range = 0.0;


		if (visualisationInformation.AutoLimits) {
			// compute max and min
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				if (fluid->GetNumParticles() > 0) {
					maxDensity = std::max(maxDensity, ParallelMax<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Density; }));
					minDensity = std::min(minDensity, ParallelMin<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Density; }));
				}
			}
			range = maxDensity - minDensity;
		}
		else {
			// jut set max and min to the user specified values
			maxDensity = visualisationInformation.Max;
			minDensity = visualisationInformation.Min;
			range = maxDensity - minDensity;
		}

		if (visualisationInformation.ShowFluids) {

			// if range is zero all fluid particles have the same density and are all colored blue. StaticBorder is anyway colored yellow
			if (range == 0.0) {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
					}
				}
			}
			// else the density of each particle is used to determine color
			else {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						// on a scale from 0 to 1, how high is the density of this particle in relation to all other densities
						float percentage = (std::max(particle.Density, minDensity) - minDensity) / range;
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
					}
				}
			}

			if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
				for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
				}
			}
		}
		if (visualisationInformation.ShowStaticBorders) {
			// Static Borders are not color coded with density
			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				for (const Particle& particle : *staticBorder->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
				}
			}
		}

		break;
	}
	case EColorVisualisation::Velocity:
	{
		double maxVelocity = 0.0;
		double minVelocity = DBL_MAX;
		double range = 0.0;


		if (visualisationInformation.AutoLimits) {
			// compute max and min
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				if (fluid->GetNumParticles() > 0) {
					maxVelocity = std::max(maxVelocity, sqrt(ParallelMax<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Velocity.LengthSquared(); })));
					minVelocity = std::min(minVelocity, sqrt(ParallelMin<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Velocity.LengthSquared(); })));
				}
			}
			range = maxVelocity - minVelocity;
		}
		else {
			// jut set max and min to the user specified values
			maxVelocity = visualisationInformation.Max;
			minVelocity = visualisationInformation.Min;
			range = maxVelocity - minVelocity;
		}

		if (visualisationInformation.ShowFluids) {

			// if range is zero all fluid particles have the same velocity and are all colored blue. StaticBorder is anyway colored yellow
			if (range == 0.0) {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 0.f, 1.f, 0.9f);
					}
				}
			}
			// else the velocity of each particle is used to determine color
			else {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						// on a scale from 0 to 1, how high is the velocity of this particle in relation to all other densities
						float percentage = (std::max(particle.Velocity.Size(), minVelocity) - minVelocity) / range;
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
					}
				}
			}
			if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
				for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
				}
			}
		}

		if (visualisationInformation.ShowStaticBorders) {
			// velocity can't be used to color static Borders
			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				for (const Particle& particle : *staticBorder->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
				}
			}
		}
		break;

	}
	case EColorVisualisation::VelocityDirection:

		if (visualisationInformation.ShowFluids) {
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				for (const Particle& particle : *fluid->Particles) {
					FVector color = (static_cast<FVector>(particle.Velocity.Normalized()) + FVector(1.f, 1.f, 1.f)) * 0.5;
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, color.X, color.Y, color.Z);
				}
			}

			if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
				for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
				}
			}

		}
		if (visualisationInformation.ShowStaticBorders) {
			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				for (const Particle& particle : *staticBorder->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
				}
			}
		}

		break;

	case EColorVisualisation::Pressure:
	{
		double maxPressure = 0.0;
		double minPressure = DBL_MAX;
		double range = 0.0;


		if (visualisationInformation.AutoLimits) {
			// compute max and min
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				if (fluid->GetNumParticles() > 0) {
					maxPressure = std::max(maxPressure, ParallelMax<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Pressure; }));
					minPressure = std::min(minPressure, ParallelMin<Particle, double>(*fluid->Particles, [](const Particle& particle) { return particle.Pressure; }));
				}
			}

			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				if (staticBorder->GetNumParticles() > 0) {
					maxPressure = std::max(maxPressure, ParallelMax<Particle, double>(*staticBorder->Particles, [](const Particle& particle) { return particle.Pressure; }));
					minPressure = std::min(minPressure, ParallelMin<Particle, double>(*staticBorder->Particles, [](const Particle& particle) { return particle.Pressure; }));
				}
			}
			range = maxPressure - minPressure;
		}
		else {
			// jut set max and min to the user specified values
			maxPressure = visualisationInformation.Max;
			minPressure = visualisationInformation.Min;
			range = maxPressure - minPressure;
		}

		// if range is zero all particles have the same pressure and are all colored blue. StaticBorder is anyway colored yellow
		if (range == 0.0) {
			if (visualisationInformation.ShowFluids) {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 1.f, 1.f);
					}
				}
			}
			if (visualisationInformation.ShowStaticBorders) {
				for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
					for (const Particle& particle : *staticBorder->Particles) {
						// on a scale from 0 to 1, how high is the pressure of this particle in relation to all other pressures
						float percentage = (std::max(particle.Pressure, minPressure) - minPressure) / range;
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 1.f, 1.f);
					}
				}
			}
			if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
				for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
				}
			}
		}
		// else the pressure of each particle is used to determine color
		else {
			if (visualisationInformation.ShowFluids) {
				for (UFluid * fluid : ParticleContext->GetFluids()) {
					for (const Particle& particle : *fluid->Particles) {
						// on a scale from 0 to 1, how high is the pressure of this particle in relation to all other densities
						float percentage = (std::max(particle.Pressure, minPressure) - minPressure) / range;
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
					}
				}
			}
			if (visualisationInformation.ShowStaticBorders) {
				for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
					for (const Particle& particle : *staticBorder->Particles) {
						// on a scale from 0 to 1, how high is the pressure of this particle in relation to all other densities
						float percentage = (std::max(particle.Pressure, minPressure) - minPressure) / range;
						Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1, (1.0 - percentage), (1 - percentage));
					}
				}
			}
		}

		break;
	}

	case EColorVisualisation::Curl:
	{
		if (visualisationInformation.ShowFluids) {

			// calculate all curl values and store them
			UKernel * kernel = ParticleContext->GetSimulator()->GetKernel();
			std::vector<double> curlValues;
			for (UFluid * fluid : ParticleContext->GetFluids()) {
				for (const Particle& particle : *fluid->Particles) {
					double curl = 0.0;
					for (const FluidNeighbor& ff : particle.FluidNeighbors) {
						curl += ff.GetParticle()->Mass / ff.GetParticle()->Density * Vector3D::CrossProduct(ff.GetParticle()->Velocity, kernel->ComputeGradient(particle, *ff.GetParticle())).Y;
					}
					curlValues.push_back(curl);
				}
			}

			double maxCurl = 0.0;
			double minCurl = DBL_MAX;
			double range = 0.0;


			if (visualisationInformation.AutoLimits && curlValues.size() > 0) {
				// compute max and min
				maxCurl = ParallelMax<double>(curlValues);
				minCurl = ParallelMin<double>(curlValues);
				range = maxCurl - minCurl;
			}
			else {
				// jut set max and min to the user specified values
				maxCurl = visualisationInformation.Max;
				minCurl = visualisationInformation.Min;
				range = maxCurl - minCurl;
			}

			// if range is zero all fluid particles have the same curl and are all colored white.
			if (range == 0.0) {
				if (visualisationInformation.ShowFluids) {
					for (UFluid * fluid : ParticleContext->GetFluids()) {
						for (const Particle& particle : *fluid->Particles) {
							Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 1.f, 1.f);
						}
					}
				}
			}
			// else the curl of each fluid particle is used to determine color
			else {
				int i = 0;
				if (visualisationInformation.ShowFluids) {
					for (UFluid * fluid : ParticleContext->GetFluids()) {
						for (const Particle& particle : *fluid->Particles) {
							// on a scale from 0 to 2, how high is the pressure of this particle in relation to all other densities
							float percentage = (std::max(curlValues[i], minCurl) / range + 1);
							Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, std::min(percentage, 1.0f), std::min(2.f - percentage, 1.0f), std::min(2.f - percentage, 1.0f));
							i++;
						}
					}
				}
			}
		}

		// StaticBorder is anyway colored yellow
		if (visualisationInformation.ShowStaticBorders) {
			for (UStaticBorder * staticBorder : ParticleContext->GetStaticBorders()) {
				for (const Particle& particle : *staticBorder->Particles) {
					Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 0.f);
				}
			}
		}

		// Ghost particles are colored pink
		if (visualisationInformation.ShowPeriodicGhostBorders && ParticleContext->GetPeriodicCondition() != nullptr) {
			for (const Particle& particle : ParticleContext->GetPeriodicCondition()->GetGhostParticles()) {
				Points.Emplace(static_cast<float>(-particle.Position.X) * 10, static_cast<float>(particle.Position.Y) * 10, static_cast<float>(particle.Position.Z) * 10, 1.f, 0.4f, 1.f);
			}
		}

		break;
		}
	}
	// use the new points and rebuild the particle cloud
	UpdatePointCloud();
	
}
