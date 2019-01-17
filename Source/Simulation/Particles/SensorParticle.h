#pragma once
#include "Particle.h"

class SensorParticle : public Particle {

public:
	SensorParticle();

	// Constructs a sensor particle
	SensorParticle(Vector3D position);

	double SensorDensity;

	std::vector<FluidNeighbor> FluidNeighbors;
	std::vector<StaticBorderNeighbor> StaticBorderNeighbors;
};



