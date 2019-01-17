#include "Particle.h"

Particle::Particle(Vector3D position) : 
	Position(position)
{
}

Particle::Particle(Vector3D position, Vector3D velocity, double mass, UFluid * fluid) :
	Type(ParticleType::FluidParticle),
	Position(position),
	Velocity(velocity),
	Acceleration(Vector3D(0.0)),
	Pressure(0.0),
	Density(0.0),
	Mass(mass),
	Fluid(fluid)
{
}

Particle::Particle(Vector3D position, Vector3D velocity, Vector3D acceleration, double mass, UFluid * fluid, double pressure, double density, bool isScripted) :
	Type(ParticleType::FluidParticle),
	Position(position),
	Velocity(velocity),
	Acceleration(acceleration),
	Pressure(pressure),
	Density(density),
	Mass(mass),
	Fluid(fluid),
	IsScripted(isScripted)
{
}

Particle::Particle(Vector3D position, double mass, UStaticBorder * border, Vector3D ghostVelocity) :
	Type(ParticleType::FluidParticle),
	Position(position),
	Velocity(ghostVelocity),
	Acceleration(Vector3D(0.0)),
	Pressure(0.0),
	Border(border)
{
}

Particle::Particle() : 
	Position(Vector3D(0.0)),
	Velocity(Vector3D(0.0)),
	Acceleration(Vector3D(0.0)),
	Mass(0.0)
{
}

double Particle::GetVolume() const
{
	return Mass / Density;
}

ParticleType Particle::GetType() const
{
	return Type;
}


StaticBorderNeighbor::StaticBorderNeighbor(int index, Particle& particleReference) :
	Index(index),
	ParticleReference(&particleReference)
{
}

StaticBorderNeighbor::operator int()
{
	return GetIndex();
}

StaticBorderNeighbor::operator Particle&()
{
	return *GetParticle();
}

int StaticBorderNeighbor::GetIndex() const
{
	return Index;
}

Particle* StaticBorderNeighbor::GetParticle() const
{
	return ParticleReference;
}

UStaticBorder* StaticBorderNeighbor::StaticBorder() const
{
	return GetParticle()->Border;
}

FluidNeighbor::FluidNeighbor(int index, Particle& particleReference) :
	Index(index),
	ParticleReference(&particleReference)
{
}

FluidNeighbor::operator int()
{
	return GetIndex();
}

FluidNeighbor::operator Particle&()
{
	return *GetParticle();
}

int FluidNeighbor::GetIndex() const
{
	return Index;
}

Particle* FluidNeighbor::GetParticle() const
{
	return ParticleReference;
}

UFluid* FluidNeighbor::GetFluid() const
{
	return GetParticle()->Fluid;
}
