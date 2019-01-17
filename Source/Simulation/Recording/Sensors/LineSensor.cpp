// Fill out your copyright notice in the Description page of Project Settings.

#include "LineSensor.h"

ALineSensor::ALineSensor() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Point"));
}

ALineSensor * ALineSensor::SpawnSensorLine(UWorld * world, 
	FMeasuredAttributes attributesToMeasure,
	FVector start,
	FVector end, 
	float sensorParticleDistance)
{
	ALineSensor * lineSensor = world->SpawnActor<ALineSensor>({ 0.f, 0.f, 0.f }, FRotator(0));
	lineSensor->MeasuredAttributes = attributesToMeasure;
	lineSensor->Start = start;
	lineSensor->End = end;
	lineSensor->SensorParticleDistance = sensorParticleDistance;

	// Spawn sensor particles
	Vector3D direction = static_cast<Vector3D>((end - start).GetSafeNormal() * sensorParticleDistance);
	int sensorParticleCount = std::ceil((end - start).Size() / sensorParticleDistance);

	for (int i = 0; i < sensorParticleCount; i++) {
		lineSensor->LineSensorParticles.emplace_back(static_cast<Vector3D>(lineSensor->Start + i * direction));
	}
	return lineSensor;
}

void ALineSensor::MeasureProperty(const UKernel& kernel) {
	// many approximations including boundary needs the density at sensor position to approximate boundary volume
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		double density = 0.0;
		for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
			density += ff.GetParticle()->Mass * kernel.ComputeValue(sensorParticle, *ff.GetParticle());
		}
		for (const StaticBorderNeighbor& fb : sensorParticle.StaticBorderNeighbors) {
			density += fb.GetParticle()->Mass * fb.GetParticle()->Border->BorderDensityFactor * kernel.ComputeValue(sensorParticle, *fb.GetParticle());
		}
		sensorParticle.SensorDensity = density;
	}
	
	if (MeasuredAttributes.MeasureCurl) {
		CurlMeasurePoints.push_back(std::vector<double>());
		for (const SensorParticle& sensorParticle : LineSensorParticles) {
			double curl = 0.0;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				curl += ff.GetParticle()->Mass / ff.GetParticle()->Density * Vector3D::CrossProduct(ff.GetParticle()->Velocity, kernel.ComputeGradient(sensorParticle, *ff.GetParticle())).Y;
			}
			CurlMeasurePoints.back().emplace_back(curl);
		}
	}
	if (MeasuredAttributes.MeasureDensity) {
		DensityMeasurePoints.push_back(std::vector<double>());
		for (const SensorParticle& sensorParticle : LineSensorParticles) {
			DensityMeasurePoints.back().emplace_back(sensorParticle.SensorDensity);
		}
	}
	if (MeasuredAttributes.MeasurePressure) {
		PressureMeasurePoints.push_back(std::vector<double>());
		for (const SensorParticle& sensorParticle : LineSensorParticles) {
			double pressure = 0.0;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				pressure += ff.GetParticle()->GetVolume() * ff.GetParticle()->Pressure * kernel.ComputeValue(sensorParticle, *ff.GetParticle());
			}
			PressureMeasurePoints.back().emplace_back(pressure);
		}
	}
	if (MeasuredAttributes.MeasureVelocity) {
		VelocityMeasurePoints.push_back(std::vector<double>());
		for (const SensorParticle& sensorParticle : LineSensorParticles) {
			double velocity = 0.0;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				velocity += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity.Length() * kernel.ComputeValue(sensorParticle, *ff.GetParticle());
			}
			for (const StaticBorderNeighbor& fb : sensorParticle.StaticBorderNeighbors) {
				velocity += fb.GetParticle()->Mass / sensorParticle.SensorDensity * fb.GetParticle()->Velocity.Length() * kernel.ComputeValue(sensorParticle, *fb.GetParticle());
			}
			VelocityMeasurePoints.back().emplace_back(velocity);
		}
	}
	if (MeasuredAttributes.MeasureVelocityDirection) {
		VelocityDirectionMeasurePoints.push_back(std::vector<Vector3D>());
		for (const SensorParticle& sensorParticle : LineSensorParticles) {
			Vector3D velocityDirection = Vector3D::Zero;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				velocityDirection += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity * kernel.ComputeValue(sensorParticle, *ff.GetParticle());
			}
			VelocityDirectionMeasurePoints.back().emplace_back(velocityDirection);
		}
	}
	if (MeasuredAttributes.MeasureVelocityDivergence) {
		VelocityDivergenceMeasurePoints.push_back(std::vector<double>());
		for (SensorParticle& sensorParticle : LineSensorParticles) {
			double velocityDivergence = 0.0;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				velocityDivergence += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity * kernel.ComputeGradient(sensorParticle, *ff.GetParticle());
			}
			for (const StaticBorderNeighbor& fb : sensorParticle.StaticBorderNeighbors) {
				velocityDivergence += fb.GetParticle()->Mass / sensorParticle.Density * fb.GetParticle()->Velocity * kernel.ComputeGradient(sensorParticle, *fb.GetParticle());
			}
			VelocityDivergenceMeasurePoints.back().emplace_back(velocityDivergence);
		}
	}
	if (MeasuredAttributes.MeasureVelocityX) {
		VelocityXMeasurePoints.push_back(std::vector<double>());
		for (SensorParticle& sensorParticle : LineSensorParticles) {
			double velocityX = 0.0;
			for (const FluidNeighbor& ff : sensorParticle.FluidNeighbors) {
				velocityX += ff.GetParticle()->GetVolume() * ff.GetParticle()->Velocity.X * kernel.ComputeValue(sensorParticle, *ff.GetParticle());
			}
			VelocityXMeasurePoints.back().emplace_back(velocityX);
		}
	}
}

void ALineSensor::FindNeighbors(const UNeighborsFinder& neighborsFinder)
{
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		FNeighborhood neighborhood = neighborsFinder.NeighborsOfPosition(sensorParticle.Position, *ParticleContext);
		sensorParticle.FluidNeighbors = neighborhood.FluidNeighbors;
		sensorParticle.StaticBorderNeighbors = neighborhood.StaticBorderNeighbors;
	}
}

void ALineSensor::WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory)
{
	std::ofstream file;
	file.open(directory.generic_string() + "/" + std::to_string(sensorIndex) + ".sensor");

	file << "PointSensor" << std::endl << std::endl;

	file << "Curl" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;

	for (int i = 0; i < CurlMeasurePoints.size(); i++) {
		for (double curlValue : CurlMeasurePoints[i]) {
			file << curlValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Density" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < DensityMeasurePoints.size(); i++) {
		for (double densityValue : DensityMeasurePoints[i]) {
			file << densityValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Pressure" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < PressureMeasurePoints.size(); i++) {
		for (double pressureValue : PressureMeasurePoints[i]) {
			file << pressureValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < VelocityMeasurePoints.size(); i++) {
		for (double velocityValue : VelocityMeasurePoints[i]) {
			file << velocityValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity Direction" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < VelocityDirectionMeasurePoints.size(); i++) {
		for (Vector3D velocityDirection : VelocityDirectionMeasurePoints[i]) {
			file << velocityDirection << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity Divergence" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < VelocityDivergenceMeasurePoints.size(); i++) {
		for (double velocityDivergenceValue : VelocityDivergenceMeasurePoints[i]) {
			file << velocityDivergenceValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;

	file << "Velocity X Component" << std::endl;
	for (SensorParticle& sensorParticle : LineSensorParticles) {
		file << sensorParticle.Position << "\t";
	}
	file << std::endl;
	for (int i = 0; i < VelocityXMeasurePoints.size(); i++) {
		for (double velocityXValue : VelocityXMeasurePoints[i]) {
			file << velocityXValue << "\t";
		}
		file << std::endl;
	}
	file << std::endl << std::endl;
	file.close();
}

void ALineSensor::OnConstruction(const FTransform & Transform)
{
	LineSensorParticles.clear();

	// update all line sensor particles
	Vector3D direction = static_cast<Vector3D>((End - Start).Normalize() * SensorParticleDistance);
	int sensorParticleCount = std::ceil((End - Start).Size() / SensorParticleDistance);

	for (int i = 0; i < sensorParticleCount; i++) {
		LineSensorParticles.emplace_back(Start + i * direction);
	}
}
