#pragma once

#include <vector>
#include "DataStructures/Vector3D.h"

enum ParticleType {
	FluidParticle,
	BorderParticle
};


class UStaticBorder;
class UFluid;
class Particle;


struct FluidNeighbor {
	FluidNeighbor(int index, Particle& particleReference);
	operator int();
	operator Particle&();

	int GetIndex() const;
	Particle * GetParticle() const;
	UFluid* GetFluid() const;
protected:
	int Index;
	Particle * ParticleReference;
};

struct StaticBorderNeighbor {
	StaticBorderNeighbor(int index, Particle& particleReference);
	operator int();
	operator Particle&();

	int GetIndex() const;
	Particle * GetParticle() const;
	UStaticBorder* StaticBorder() const;

protected:
	int Index;
	Particle * ParticleReference;
};

class Particle {

public:
	// Constructs a fluid particle
	Particle(Vector3D position, Vector3D velocity, double mass, UFluid * fluid = nullptr);

	// Contains all relevant information for generating ghost particles
	Particle(Vector3D position, Vector3D velocity, Vector3D acceleration, double mass, UFluid * fluid = nullptr, double pressure = 0.0, double density = 0.0, bool isScripted = false);
	
	// Constructs a static-border particle
	Particle(Vector3D position, double mass, UStaticBorder * border = nullptr, Vector3D ghostVelocity = Vector3D(0, 0, 0));
	Particle();

	double Mass;

	Vector3D Position;

	Vector3D Velocity;

	Vector3D Acceleration;

	// Volume is calculated from mass and density
	double GetVolume() const;

	double Pressure;

	double Density;

	// Flags if particles is scripted by volumes
	bool IsScripted = false;


	std::vector<FluidNeighbor> FluidNeighbors;
	std::vector<StaticBorderNeighbor> StaticBorderNeighbors;

	UFluid * Fluid;
	UStaticBorder * Border;

	ParticleType GetType() const;

private:
	ParticleType Type;
};



