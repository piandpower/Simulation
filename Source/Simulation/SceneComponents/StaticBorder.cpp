// Fill out your copyright notice in the Description page of Project Settings.
#include "StaticBorder.h"
#include "Simulator.h"
#include "NeighborsFinders/HashNeighborsFinder.h"

namespace std {
	template <> struct hash<FVector>
	{
		size_t operator()(const FVector & x) const
		{
			return GetTypeHash(x);
		}
	};
}

// Sets default values
UStaticBorder::UStaticBorder()
{
}

UStaticBorder::~UStaticBorder()
{
}

void UStaticBorder::Build(UParticleContext * particleContext, ASimulator * simulator)
{
	ParticleContext = particleContext;
	Simulator = simulator;

	// build the static border according to span parameters that have been set
	switch (SpawnSource) {
	case ESpawnSource::Line:
		BuildStaticBorderFromLine();
		break;
	case ESpawnSource::Positions:
		BuildStaticBorderFromPositions();
		break;
	case ESpawnSource::Mesh:
		BuildStaticBorderFromMultipleMeshes();
		break;
	case ESpawnSource::MultipleMeshes:
		BuildStaticBorderFromMultipleMeshes();
		break;
	}

	GetSimulator()->GetNeighborsFinder()->AddStaticParticles(this, GetParticleContext()->GetParticleDistance());
	CalculateMasses();
}

UStaticBorder* UStaticBorder::CreateStaticBorderFromLine(FVector start, FVector end, float borderDensityFactor, float borderStiffness, float borderVolumeFactor, float borderViscosity, FVector ghostVelocity)
{
	UStaticBorder * staticBorder = NewObject<UStaticBorder>();
	// prevent garbage collection
	staticBorder->AddToRoot();

	staticBorder->Particles = std::unique_ptr<std::vector<Particle>>();

	staticBorder->BorderDensityFactor;
	staticBorder->BorderStiffness = borderStiffness;
	staticBorder->BorderVolumeFactor = borderVolumeFactor;
	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderViscosity = borderViscosity;

	// save for building later
	staticBorder->SpawnSource = ESpawnSource::Line;
	staticBorder->Start = start;
	staticBorder->End = end;
	staticBorder->GhostVelocity = ghostVelocity;

	return staticBorder;
}

UStaticBorder* UStaticBorder::CreateStaticBorderFromPositions(TArray<FVector> positions, FTransform transform, float borderDensityFactor, float borderStiffness, float borderVolumeFactor, float borderViscosity)
{
	UStaticBorder * staticBorder = NewObject<UStaticBorder>();
	// prevent garbage collection
	staticBorder->AddToRoot();

	staticBorder->Particles = std::unique_ptr<std::vector<Particle>>();

	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderStiffness = borderStiffness;
	staticBorder->BorderVolumeFactor = borderVolumeFactor;
	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderViscosity = borderViscosity;

	// save for building later
	staticBorder->SpawnSource = ESpawnSource::Positions;
	staticBorder->Positions = positions;
	staticBorder->Transforms.Add(transform);

	return staticBorder;
}

UStaticBorder* UStaticBorder::CreateStaticBorderFromMesh(UStaticMesh * mesh, FTransform transform, float borderDensityFactor, float borderStiffness, float borderVolumeFactor, float borderViscosity)
{
	UStaticBorder * staticBorder = NewObject<UStaticBorder>();
	// prevent garbage collection
	staticBorder->AddToRoot();

	staticBorder->Particles = std::unique_ptr<std::vector<Particle>>();

	staticBorder->BorderDensityFactor;
	staticBorder->BorderStiffness = borderStiffness;
	staticBorder->BorderVolumeFactor = borderVolumeFactor;
	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderViscosity = borderViscosity;


	// save for building later
	staticBorder->SpawnSource = ESpawnSource::MultipleMeshes;
	staticBorder->Meshes.Add(mesh);
	staticBorder->Transforms.Add(transform);
	
	return staticBorder;

}

UStaticBorder* UStaticBorder::CreateStaticBorderFromMultipleMeshes(TArray<UStaticMesh *> meshes, TArray<FTransform> transforms, float borderDensityFactor, float borderStiffness, float borderVolumeFactor, float borderViscosity)
{
	UStaticBorder * staticBorder = NewObject<UStaticBorder>();
	// prevent garbage collection
	staticBorder->AddToRoot();

	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderStiffness = borderStiffness;
	staticBorder->BorderVolumeFactor = borderVolumeFactor;
	staticBorder->BorderDensityFactor = borderDensityFactor;
	staticBorder->BorderViscosity = borderViscosity;


	// save for building later
	staticBorder->SpawnSource = ESpawnSource::MultipleMeshes;
	staticBorder->Meshes = meshes;
	staticBorder->Transforms = transforms;

	return staticBorder;
}

UStaticBorder* UStaticBorder::CreateStaticBorderFromStaticMeshActors(TArray<AStaticMeshActor*> meshActors, float borderDensityFactor, float borderStiffness, float borderVolumeFactor, float borderViscosity)
{
	TArray<UStaticMesh *> meshes;
	TArray<FTransform> transforms;

	for (AStaticMeshActor * meshActor : meshActors) {
		meshes.Add(meshActor->GetStaticMeshComponent()->GetStaticMesh());
		transforms.Add(meshActor->GetActorTransform());
	}

	return CreateStaticBorderFromMultipleMeshes(meshes, transforms, borderDensityFactor, borderStiffness, borderVolumeFactor, borderViscosity);
}

void UStaticBorder::BuildStaticBorderFromLine() {

	Particles = std::make_unique<std::vector<Particle>>();

	if (End == Start) {
		Particles->emplace_back((Vector3D)Start, 0, this, (Vector3D)GhostVelocity);
		return;
	}

	FVector direction = (End - Start);
	direction.Normalize();
	direction *= GetParticleContext()->GetParticleDistance();

	float distance = (End - Start).SizeSquared();
	// build the box around the position of he actor
	for (int i = 0; true; i++) {
		if ((i * direction).SizeSquared() > distance) {
			break;
		}
		Particles->emplace_back((Vector3D)(Start + i * direction), 0, this, (Vector3D)GhostVelocity);
	}
}

UStaticBorder::operator int()
{
	return Index;
}

void UStaticBorder::BuildStaticBorderFromPositions() {

	Particles = std::make_unique<std::vector<Particle>>();


	// transform all the positions according to transform
	ParallelFor(Positions.Num(), [&](int32 i) {
		Positions[i] = Transforms[0].TransformPosition(Positions[i]);
	});

	for(const FVector& position : Positions) {
		Particles->emplace_back((Vector3D)(position), 0, this);
	}

	Positions.Empty();
	Positions.Shrink();

	Transforms.Empty();
	Transforms.Shrink();
}

void UStaticBorder::BuildStaticBorderFromMultipleMeshes() {

	Particles = std::make_unique<std::vector<Particle>>();


	// Check if all meshes are readable by CPU
	for (UStaticMesh * mesh : Meshes) {
		if (!mesh->bAllowCPUAccess) {
			throw("Static Mesh needs to have CPU Access enabled");
		}
	}


	std::unordered_set<FVector> positions;
	for (int meshindex = 0; meshindex < Meshes.Num(); meshindex++) {
		UStaticMesh * mesh = Meshes[meshindex];

		FTransform transform;

		// if there arent enough tranforms give, take the first transform. 
		if (Transforms.Num() > meshindex) {
			transform = Transforms[meshindex];
		}
		else {
			transform = Transforms[0];
		}
		transform = FTransform(transform.GetRotation(), transform.GetTranslation() / 10, transform.GetScale3D() / 10);

		for (int section = 0; section < mesh->GetNumSections(0); section++) {

			TArray<FVector> vertices;
			TArray<int> triangles;
			TArray<FVector> normals;
			TArray<FVector2D> uVs;
			TArray<FProcMeshTangent> tangents;

			UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(mesh, 0, section, vertices, triangles, normals, uVs, tangents);

			// Transform the position
			for (FVector & vertex : vertices) {
				vertex = transform.TransformPosition(vertex);
			}

			for (int i = 0; i < triangles.Num(); i += 3) {
				AddPositionsFromTriangle(positions, vertices[triangles[i]], vertices[triangles[i + 1]], vertices[triangles[i + 2]], GetParticleContext()->GetParticleDistance());
			}
		}
	}

	RemoveCloseNeighbors(positions, GetParticleContext()->GetParticleDistance() / 2.1);

	for (FVector position : positions) {
		Particles->emplace_back((Vector3D)position, 0, this);
	}

	Meshes.Empty();
	Meshes.Shrink();

	Transforms.Empty();
	Transforms.Shrink();
}

void UStaticBorder::WriteStaticBorderToFile(std::string file)
{
	std::ofstream staticBorderFile;
	staticBorderFile.open(file, std::ios_base::app);

	// test if we can write
	if (!staticBorderFile.is_open()) {
		throw("Could't write in file: " + file);
	}

	staticBorderFile << "Static Border" << std::endl;
	staticBorderFile << "Stiffness:\t" << BorderStiffness << std::endl;
	staticBorderFile << "Viscosity:\t" << BorderViscosity << std::endl;
	staticBorderFile << "Particles" << std::endl;
	for (Particle & particle : *Particles) {
		staticBorderFile << "Mass: " << particle.Mass << "\t" << "Position: " << particle.Position.ToString() << std::endl;
	}
	staticBorderFile << std::endl << std::endl;

	staticBorderFile.close();
}

void UStaticBorder::CalculateMasses()
{
	// Compute all Neighbors using HashNeighborsFinder
	UHashNeighborsFinder * neighborFinder = UHashNeighborsFinder::CreateHashNeighborsFinder();
	neighborFinder->Build(GetSimulator()->GetKernel()->GetSupportRange());
	neighborFinder->FindBorderNeighbors(this, GetSimulator()->GetParticleContext()->GetParticleDistance());
	delete neighborFinder;

	// Compute Volumes of the border particles
	ParallelFor(Particles->size(), [&](int32 i) {
		Particle& b = Particles->at(i);

		double sum = 0;
		for (const Particle& bb : b.StaticBorderNeighbors) {
			sum += GetSimulator()->GetKernel()->ComputeKernel(b, bb);
		}

		b.Mass = BorderVolumeFactor / sum;

		b.StaticBorderNeighbors.clear();
	});
}

UParticleContext * UStaticBorder::GetParticleContext() const
{
	return ParticleContext;
}

int UStaticBorder::GetNumParticles() const
{
	return Particles->size();
}

ASimulator * UStaticBorder::GetSimulator() const
{
	return Simulator;
}

void UStaticBorder::AddPositionsFromTriangle(std::unordered_set<FVector> & positions, FVector a, FVector b, FVector c, float particleDistance, bool doubleThickness)
{
	FVector normal = FVector::CrossProduct(b - a, c - a);
	normal.Normalize();

	float aFactor = 1;
	float bFactor = 0;
	float cFactor = 0;

	float bStepSize = 1 / ((b - a).Size() / particleDistance);
	float cStepSize = 1 / ((c - a).Size() / particleDistance);

	while (cFactor <= 1.0) {
		while (bFactor + cFactor <= 1.0) {

			if (doubleThickness) {
				// Add two particles, so the wall is 2 particles thick
				positions.insert(aFactor * a + bFactor * b + cFactor * c + 0.5 * particleDistance * normal);
				positions.insert(aFactor * a + bFactor * b + cFactor * c - 0.5 * particleDistance * normal);
			}
			else {
				positions.insert(aFactor * a + bFactor * b + cFactor * c);
			}


			aFactor -= bStepSize;
			bFactor += bStepSize;
		}
		cFactor += cStepSize;
		aFactor = 1 - cFactor;
		bFactor = 0;
	}
}

void UStaticBorder::RemoveCloseNeighbors(std::unordered_set<FVector> & positions, double minDistance) {
	std::unordered_set<FVector> toDelete;

	std::unordered_map<int, std::unordered_set<FVector>> hashTable;
	hashTable.reserve(positions.size());

	// fill the hashtable
	for (FVector position : positions) {

		int hash = 611657 * (int)floor(position.X / minDistance) + 1109 * (int)floor(position.Y / minDistance) + (int)floor(position.Z / minDistance);

		if (hashTable.count(hash) == 1) {
			// if there is already a list at this box, just add the particle
			hashTable.at(hash).insert(position);
		}
		else {

			// Create new list at hash and add the position index
			hashTable.insert(std::pair<int, std::unordered_set<FVector>>(hash, std::unordered_set<FVector>()));
			hashTable.at(hash).insert(position);
		}
	}

	// Delete positions which are too close
	for (FVector position : positions) {

		if (toDelete.find(position) != toDelete.end()) {
			continue;
		}

		int xGrid = floor(position.X / minDistance);
		int yGrid = floor(position.Y / minDistance);;
		int zGrid = floor(position.Z / minDistance);;

		for (int xOffset = -1; xOffset <= 1; xOffset++) {
			for (int yOffset = -1; yOffset <= 1; yOffset++) {
				for (int zOffset = -1; zOffset <= 1; zOffset++) {

					// get hashindex of current cell
					int hash = 611657 * (xGrid + xOffset) + 1109 * (yGrid + yOffset) + (zGrid + zOffset);

					if (hashTable.find(hash) != hashTable.end()) {

						std::unordered_set<FVector> * neighbors = &hashTable.at(hash);

						for (FVector neighbor : *neighbors) {
							// check if position  and neighbor are too close also only delete particles which have bigger index, so that they don't delete each other
							if ((position - neighbor).Size() < minDistance && position != neighbor) {
								toDelete.insert(neighbor);
							}
						}
					}
				}
			}
		}
	}

	for (FVector position : toDelete) {
		positions.erase(position);
	}
}