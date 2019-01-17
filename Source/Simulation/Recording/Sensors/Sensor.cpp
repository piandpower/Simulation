// Fill out your copyright notice in the Description page of Project Settings.

#include "Sensor.h"

void ASensor::MeasureProperty(const UKernel& kernel)
{
	throw("This is an abstract base class and should never be called");
}

void ASensor::WriteSensorDataToFile(int sensorIndex, std::experimental::filesystem::path directory)
{
	throw("This is an abstract base class and should never be called");
}

void ASensor::FindNeighbors(const UNeighborsFinder& neighborsFinder)
{
	throw("This is an abstract base class and should never be called");
}

void ASensor::Build(UParticleContext * particleContext)
{
	ParticleContext = particleContext;
}