#include "SensorParticle.h"

SensorParticle::SensorParticle() : 
	Particle (Vector3D::Zero)
{
}

SensorParticle::SensorParticle(Vector3D position) : 
	Particle(position)
{
}