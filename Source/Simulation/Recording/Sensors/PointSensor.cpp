// Fill out your copyright notice in the Description page of Project Settings.

#include "PointSensor.h"

APointSensor::APointSensor() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Point"));
}

APointSensor * APointSensor::SpawnSensorPoint(UWorld * world, FVector location, FMeasuredAttributes measuredAttributes)
{
	APointSensor * pointSensor = world->SpawnActor<APointSensor>(location, FRotator(0));
	pointSensor->SetActorLocation(location);
	pointSensor->MeasuredAttributes = measuredAttributes;
	return pointSensor;
}

void APointSensor::MeasureProperty(const UKernel& kernel) {

	// calculate density for further approximations
	double density = 0.0;
	for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
		density += ff.GetParticle()->Mass * kernel.ComputeValue(PointSensorParticle, *ff.GetParticle());
	}
	for (const StaticBorderNeighbor& fb : PointSensorParticle.StaticBorderNeighbors) {
		density += fb.GetParticle()->Mass * fb.GetParticle()->Border->BorderDensityFactor * kernel.ComputeValue(PointSensorParticle, *fb.GetParticle());
	}
	PointSensorParticle.SensorDensity = density;

	if (MeasuredAttributes.MeasureCurl) {
		double curl = 0.0;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			curl += ff.GetParticle()->Mass / ff.GetParticle()->Density * Vector3D::CrossProduct(ff.GetParticle()->Velocity, kernel.ComputeGradient(PointSensorParticle, *ff.GetParticle())).Y;
		}
		CurlMeasurePoints.emplace_back(curl);
	}
	if (MeasuredAttributes.MeasureDensity) {
		DensityMeasurePoints.emplace_back(PointSensorParticle.SensorDensity);
	}
	if (MeasuredAttributes.MeasurePressure) {
		double pressure = 0.0;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			pressure += ff.GetParticle()->GetVolume() * ff.GetParticle()->Pressure * kernel.ComputeValue(PointSensorParticle, *ff.GetParticle());
		}
		PressureMeasurePoints.emplace_back(pressure);
	}
	if (MeasuredAttributes.MeasureVelocity) {
		double velocity = 0.0;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			velocity += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity.Length() * kernel.ComputeValue(PointSensorParticle, *ff.GetParticle());
		}
		for (const StaticBorderNeighbor& fb : PointSensorParticle.StaticBorderNeighbors) {
			velocity += fb.GetParticle()->Mass / PointSensorParticle.SensorDensity * fb.GetParticle()->Velocity.Length() * kernel.ComputeValue(PointSensorParticle, *fb.GetParticle());
		}
		VelocityMeasurePoints.emplace_back(velocity);
	}
	if (MeasuredAttributes.MeasureVelocityDirection) {
		Vector3D velocityDirection = Vector3D::Zero;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			velocityDirection += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity * kernel.ComputeValue(PointSensorParticle, *ff.GetParticle());
		}
		VelocityDirectionMeasurePoints.emplace_back(velocityDirection);
	}
	if (MeasuredAttributes.MeasureVelocityDivergence) {
		double velocityDivergence = 0.0;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			velocityDivergence += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity * kernel.ComputeGradient(PointSensorParticle, *ff.GetParticle());
		}
		for (const StaticBorderNeighbor& fb : PointSensorParticle.StaticBorderNeighbors) {
			velocityDivergence += fb.GetParticle()->Mass / PointSensorParticle.Density * fb.GetParticle()->Velocity * kernel.ComputeGradient(PointSensorParticle, *fb.GetParticle());
		}
		VelocityDivergenceMeasurePoints.emplace_back(velocityDivergence);
	}
	if (MeasuredAttributes.MeasureVelocityX) {
		double velocityX = 0.0;
		for (const FluidNeighbor& ff : PointSensorParticle.FluidNeighbors) {
			velocityX += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity.X * kernel.ComputeValue(PointSensorParticle, *ff.GetParticle());
		}
		VelocityXMeasurePoints.emplace_back(velocityX);
	}
}

void APointSensor::FindNeighbors(const UNeighborsFinder& neighborsFinder)
{
	FNeighborhood neighborhood = neighborsFinder.NeighborsOfPosition(PointSensorParticle.Position, *ParticleContext);
	PointSensorParticle.FluidNeighbors = neighborhood.FluidNeighbors;
	PointSensorParticle.StaticBorderNeighbors = neighborhood.StaticBorderNeighbors;
}

void APointSensor::WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory)
{
	std::ofstream file;
	file.open(directory.generic_string() + "/" + std::to_string(sensorIndex) + ".sensor");

	file << "PointSensor" << std::endl << std::endl;

	file << "Curl" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double curlValue : CurlMeasurePoints) {
		file << curlValue << std::endl;
	}
	file << std::endl << std::endl;

	file << "Density" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double densityValue : DensityMeasurePoints) {
		file << densityValue << std::endl;
	}
	file << std::endl << std::endl;

	file << "Pressure" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double pressureValue : PressureMeasurePoints) {
		file << pressureValue << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double velocityValue : VelocityMeasurePoints) {
		file << velocityValue << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity Direction" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (Vector3D velocityDirection : VelocityDirectionMeasurePoints) {
		file << velocityDirection << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity Divergence" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double velocityDivergence : VelocityDivergenceMeasurePoints) {
		file << velocityDivergence << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity X Component" << std::endl;
	file << PointSensorParticle.Position << std::endl;
	for (double velocityX : VelocityXMeasurePoints) {
		file << velocityX << std::endl;
	}
	file << std::endl << std::endl;
}

void APointSensor::OnConstruction(const FTransform & Transform)
{
	// update the point position
	PointSensorParticle.Position = GetActorLocation();
}
