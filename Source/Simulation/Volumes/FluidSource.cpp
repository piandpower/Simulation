#include "FluidSource.h"

AFluidSource::AFluidSource() : LastSpawnTime(0.0f), NextSpawnTime(0.0f) {
	VolumeType = EVolumeType::FixedVelocity;
	Velocity = 1;
	FirstSpawn = true;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
	ArrowComponent->bHiddenInGame = false;
	ArrowComponent->SetRelativeLocation({ 0, 0, -100 });
	ArrowComponent->SetRelativeRotation({ 90, 0, 0 });

	SpawnPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpawnPlane"));
	SpawnPlane->SetupAttachment(RootComponent);
	SpawnPlane->SetRelativeTransform(FTransform(FRotator( 0, 0, 0 ), { 0, 0, -100 }, { 1, 1, 1 }));
	SpawnPlane->bHiddenInGame = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshSquare (TEXT("StaticMesh'/Game/Geometry/Shapes/Shape_Plane.Shape_Plane'"));
	if (meshSquare.Object) {
		SpawnSquare = meshSquare.Object;
		SpawnPlane->SetStaticMesh(SpawnSquare);
	}
	else {
		throw("No Shape Plane flound!");
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> meshCricle(TEXT("StaticMesh'/Game/Geometry/Shapes/Shape_Circle.Shape_Circle'"));
	if (meshCricle.Object) {
		SpawnCircle = meshCricle.Object;
	}
	else {
		throw("No Shape Circle flound!");
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> defaultMaterial(TEXT("Material'/Game/Geometry/Materials/Blue.Blue'"));
	if (defaultMaterial.Object) {
		SpawnPlane->SetMaterial(0, defaultMaterial.Object);
	}
	else {
		throw("No Shape Plane flound!");
	}


}

AFluidSource::~AFluidSource()
{
}

void AFluidSource::Build(UWorld * world, UParticleContext & particleContext)
{

	Fluid = UFluid::CreateFluidFromPositionsAndVelocities(TArray<FVector>(), TArray<FVector>(), RestDensity, Viscosity);
	particleContext.AddFluid(Fluid);

	switch (VolumeForm) {
	case EVolumeForm::Cuboid:
		SpawnBorderCuboid(world, particleContext);
		break;

	case EVolumeForm::Cylinder:
		SpawnBorderCylinder(world, particleContext);
		break;
	}
}


void AFluidSource::ApplyBeforeIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		// move particles with specified velocity
		for (UFluid * fluid : particleContext.GetFluids()) {
			ParallelFor(fluid->Particles->size(), [&](int32 i) {
				Particle& particle = fluid->Particles->at(i);

				if (IsParticleAffected(particle)) {
					particle.IsScripted = true;
					particle.Acceleration = Vector3D(0, 0, 0);
					particle.Velocity = GetActorUpVector() * Velocity;
					particle.Position += timestep * particle.Velocity;
				}
			});
		}
	}
}

void AFluidSource::ApplyAfterIntegration(UParticleContext & particleContext, double timestep, double simulationTime)
{
	if (IsEnabled) {
		// spawn new particles
		if (NextSpawnTime <= simulationTime) {

			LastSpawnTime = NextSpawnTime;
			NextSpawnTime = LastSpawnTime + particleContext.GetParticleDistance() / Velocity * 1.05;

			switch (VolumeForm) {
			case Cuboid:
				SpawnParticlesSquare(particleContext, simulationTime - LastSpawnTime);
				break;
			case Cylinder:
				SpawnParticlesCircle(particleContext, simulationTime - LastSpawnTime);
				break;
			}
		}
	}
}

double AFluidSource::MaxTimeStep(double timestepFactor, const UParticleContext & particleContext) const
{
	return particleContext.GetParticleDistance() * timestepFactor / Velocity;
}

void AFluidSource::SetSpawnRateByVelocity(float velocity)
{
	Velocity = velocity;
}

void AFluidSource::SetSpawnRateByParticlesPerSecond(float particlesPerSecond)
{
	// TODO: think, might be only approximately
}

void AFluidSource::SetSpawnRateByVolumePerSecond(float volume)
{
	float area;

	switch (VolumeForm) {
	case Cuboid:
		area = 200 * GetActorScale3D().X * 200 * GetActorScale().Y;
		break;

	case Cylinder:
		area = PI * 100 * GetActorScale3D().X * 100 * GetActorScale().Y;
		break;
	
	default:
		area = 0;
		break;
	}

	Velocity = volume / area;
}

void AFluidSource::SpawnParticlesSquare(UParticleContext & particleContext, double timeOffset)
{
	FVector worldSpaceVelocity = GetActorRotation().RotateVector({ 0, 0, Velocity });
	// since timesteps for new particles aren't hit exactly, but we want a continuous water stream
	FVector positionOffset = timeOffset * Velocity * GetActorUpVector() + particleContext.GetParticleDistance() * GetActorUpVector();
	FTransform actorTransform = GetActorTransform();


	// square reaches from (-100, -100, -100) to (100, 100, -100) in unreal scale!
	FVector currentPosition = FVector( -100, -100, -100 ) * GetActorScale3D() + FVector(0.5 * particleContext.GetParticleDistance(), 0.5 * particleContext.GetParticleDistance() * 10, 0);
	while (currentPosition.Y < 100 * GetActorScale3D().Y) {
		currentPosition.X = -100 * GetActorScale3D().X + 0.5 * particleContext.GetParticleDistance() * 10;
		while (currentPosition.X < 100 * GetActorScale3D().X) {
			
			Fluid->AddParticle(actorTransform.TransformPositionNoScale(currentPosition) * 0.1 + positionOffset, worldSpaceVelocity);

			currentPosition.X += particleContext.GetParticleDistance() * 10;
		}
		currentPosition.Y += particleContext.GetParticleDistance() * 10;
	}
}

void AFluidSource::SpawnParticlesCircle(UParticleContext & particleContext, double timeOffset)
{
	FVector worldSpaceVelocity = GetActorRotation().RotateVector({ 0, 0, Velocity });
	// since timesteps for new particles aren't hit exactly, but we want a continuous water stream
	FVector positionOffset = timeOffset * Velocity * GetActorUpVector() + GetActorUpVector() * particleContext.GetParticleDistance();
	FTransform actorTransform = GetActorTransform();


	// circle is centered at 0, 0, 0 with radius 100 in unreal scale!
	FVector currentPosition = FVector(-100, -100, -100) * GetActorScale3D() + FVector(0.5 * particleContext.GetParticleDistance(), 0.5 * particleContext.GetParticleDistance() * 10, 0);
	while (currentPosition.Y < 100 * GetActorScale3D().Y) {
		currentPosition.X = -100 * GetActorScale3D().X + 0.5 * particleContext.GetParticleDistance() * 10;
		while (currentPosition.X < 100 * GetActorScale3D().X) {
			
			// check if currentPosition is in the ellipse by using
			// x^2 / (100 * scaleX)^2 + y^2 / (100 * scaleY)^2 < 1  
			if (pow(currentPosition.X + positionOffset.X, 2) / pow(100 * GetActorScale3D().X, 2) + pow(currentPosition.Y + positionOffset.Y, 2) / pow(100 * GetActorScale3D().Y, 2) < 1) {
				Fluid->AddParticle(actorTransform.TransformPositionNoScale(currentPosition) * 0.1 + positionOffset, worldSpaceVelocity);
			}

			currentPosition.X += particleContext.GetParticleDistance() * 10;
		}
		currentPosition.Y += particleContext.GetParticleDistance() * 10;
	}
}

void AFluidSource::SpawnBorderCuboid(UWorld * world, UParticleContext & particleContext)
{
	FVector worldSpaceVelocity = GetActorRotation().RotateVector({ 0, 0, Velocity });
	FTransform actorTransform = GetActorTransform();
	TArray<FVector> positions;

	// square reaches from (-100, -100, -100) to (100, 100, -100) in unreal scale!
	// first build the plane
	FVector currentPosition = FVector(-100, -100, -100) * GetActorScale3D() + FVector(-particleContext.GetParticleDistance() * 10, -particleContext.GetParticleDistance() * 10, -10 * particleContext.GetParticleDistance());
	while (currentPosition.Y <= 100 * GetActorScale3D().Y +  particleContext.GetParticleDistance() * 10) {
		currentPosition.X = -100 * GetActorScale3D().X +-particleContext.GetParticleDistance() * 10;
		while (currentPosition.X <= 100 * GetActorScale3D().X + particleContext.GetParticleDistance() * 10) {

			positions.Add(actorTransform.TransformPositionNoScale(currentPosition));

			currentPosition.X += particleContext.GetParticleDistance() * 10;
		}
		currentPosition.Y += particleContext.GetParticleDistance() * 10;
	}

	// then build 5 rows of border particles
	for (int layer = 0; layer < 5; layer++) {
		currentPosition = FVector(-100, -100, -100) * GetActorScale3D() + FVector(0, -particleContext.GetParticleDistance() * 10, layer * particleContext.GetParticleDistance() * 10);
		while (currentPosition.X <= 100 * GetActorScale3D().X) {
			positions.Add(actorTransform.TransformPositionNoScale(currentPosition));
			currentPosition.X += particleContext.GetParticleDistance() * 10;
		}

		currentPosition = FVector(-100, 100, -100) * GetActorScale3D() + FVector(0, particleContext.GetParticleDistance() * 10, layer * particleContext.GetParticleDistance() * 10);
		while (currentPosition.X <= 100 * GetActorScale3D().X) {
			positions.Add(actorTransform.TransformPositionNoScale(currentPosition));
			currentPosition.X += particleContext.GetParticleDistance() * 10;
		}

		currentPosition = FVector(-100, -100, -100) * GetActorScale3D() + FVector(-particleContext.GetParticleDistance() * 10, 0, layer * particleContext.GetParticleDistance() * 10);
		while (currentPosition.Y <= 100 * GetActorScale3D().Y + particleContext.GetParticleDistance()) {
			positions.Add(actorTransform.TransformPositionNoScale(currentPosition));
			currentPosition.Y += particleContext.GetParticleDistance() * 10;
		}

		currentPosition = FVector(100, -100, -100) * GetActorScale3D() + FVector(particleContext.GetParticleDistance() * 10, 0, layer * particleContext.GetParticleDistance() * 10);
		while (currentPosition.Y <= 100 * GetActorScale3D().Y + particleContext.GetParticleDistance()) {
			positions.Add(actorTransform.TransformPositionNoScale(currentPosition));
			currentPosition.Y += particleContext.GetParticleDistance() * 10;
		}
	}
	
	particleContext.AddStaticBorder(UStaticBorder::CreateStaticBorderFromPositions(positions, FTransform(FRotator(0), FVector(0), FVector(0.1)), 1.3f, 1.0, 0.8));
}

void AFluidSource::SpawnBorderCylinder(UWorld * world, UParticleContext & particleContext)
{
	FVector worldSpaceVelocity = GetActorRotation().RotateVector({ 0, 0, Velocity });
	FTransform actorTransform = GetActorTransform();
	TArray<FVector> positions;


	// width radius
	float a = 100 * GetActorScale3D().X + particleContext.GetParticleDistance() * 10;
	// height radius
	float b = 100 * GetActorScale3D().Y + particleContext.GetParticleDistance() * 10;

	float step = particleContext.GetParticleDistance() * 10;

	// circle is centered at 0, 0, 0 with radius 100 in unreal scale!
	FVector currentPosition = FVector(-100, -100, -100 ) * GetActorScale3D() + FVector(-2 * step, -2 * step, -step);
	while (currentPosition.Y < 100 * GetActorScale3D().Y + 2 * step) {
		currentPosition.X = -100 * GetActorScale3D().X - 2 * step;
		while (currentPosition.X < 100 * GetActorScale3D().X + 2 * step) {

			// check if currentPosition is in the ellipse by using
			// x^2 / (100 * scaleX)^2 + y^2 / (100 * scaleY)^2 < 1  
			if (pow(currentPosition.X, 2) / pow(a, 2) + pow(currentPosition.Y, 2) / pow(b, 2) <= 1) {
				positions.Add(actorTransform.TransformPositionNoScale(currentPosition));
			}

			currentPosition.X += step;
		}
		currentPosition.Y += step;
	}


	// then build 5 rows of border particles
	for (int layer = -1; layer < 5; layer++) {

		// midpoint algorithm for ellipses 
		// region 1 where the line is more horizontal than vertical
		float currentX = 0.0f;
		float currentY = b;
		float decisionParameter = b * b * currentX * currentX + a * a * currentY * currentY - a * a * b * b;

		while (2 * b * b * currentX < 2 * a * a * currentY) {

			// add 4 symetric particles
			positions.Add(actorTransform.TransformPositionNoScale(FVector(currentX, currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(-currentX, currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(currentX, -currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(-currentX, -currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));

			currentX += step;
			currentY = sqrt((1 - pow(currentX / a, 2)) * b * b);
			decisionParameter = decisionParameter = b * b * currentX * currentX + a * a * currentY * currentY - a * a * b * b;
		}

		// region 2 where the line is more vertical than horizontal

		while (currentY >= 0) {

			// add 4 symetric particles
			positions.Add(actorTransform.TransformPositionNoScale(FVector(currentX, currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(-currentX, currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(currentX, -currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));
			positions.Add(actorTransform.TransformPositionNoScale(FVector(-currentX, -currentY, -100 * GetActorScale3D().Z + layer * particleContext.GetParticleDistance() * 10)));

			currentY -= step;
			currentX = sqrt((1 - pow(currentY / b, 2)) * a * a);


			decisionParameter = decisionParameter = b * b * currentX * currentX + a * a * currentY * currentY - a * a * b * b;
		}

 	}

	particleContext.AddStaticBorder(UStaticBorder::CreateStaticBorderFromPositions(positions, FTransform(FRotator(0), FVector(0), FVector(0.1)), 1.3f, 1.0, 0.8));

}

void AFluidSource::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	switch (VolumeForm) {
	case Cuboid:
		SpawnPlane->SetStaticMesh(SpawnSquare);
		break;
	case Cylinder:
		SpawnPlane->SetStaticMesh(SpawnCircle);
		break;
	}
}



