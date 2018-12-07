#include "MLSExtrapolation.h"
#include "Simulator.h"

UMLSExtrapolation * UMLSExtrapolation::CreateBoundaryMLSExtrapolationPressureComputation(FEpsilons nanoEpsilon) {
	UMLSExtrapolation * mlsPressureExtrapolation = NewObject<UMLSExtrapolation>();
	mlsPressureExtrapolation->Epsilon1D = static_cast<double>(nanoEpsilon.OneDimension) * 0.000000001;
	mlsPressureExtrapolation->Epsilon2D = static_cast<double>(nanoEpsilon.TwoDimensions) * 0.000000001;
	mlsPressureExtrapolation->Epsilon3D = static_cast<double>(nanoEpsilon.ThreeDimensions) * 0.000000001;

	// prevent garbage collection
	mlsPressureExtrapolation->AddToRoot();

	return mlsPressureExtrapolation;
}


UMLSExtrapolation::~UMLSExtrapolation()
{
}

void UMLSExtrapolation::ComputeAllPressureValues(UParticleContext * particleContext, UKernel * kernel)
{
	switch (Dimensionality) {
	case EDimensionality::One:
		for (UStaticBorder * border : particleContext->GetStaticBorders()) {
			ParallelFor(border->Particles->size(), [&](int32 i) {
				Particle& b = border->Particles->at(i);


				Vector3D sumWeightedPosition = { 0.0, 0.0, 0.0 };
				double sumWeights = 0.0;

				for (const Particle& bf : b.FluidNeighbors) {
					sumWeightedPosition += bf.Position * bf.GetVolume() * kernel->ComputeKernel(b, bf);
					sumWeights += bf.GetVolume() * kernel->ComputeKernel(b, bf);
				}

				if (sumWeights <= DBL_EPSILON) {
					b.Pressure = 0.0;
					return;
				}

				Vector3D weightedPosition = sumWeightedPosition / sumWeights;

				double alpha = 0.0;
				Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
				double matrix = 0.0;

				for (const Particle& bf : b.FluidNeighbors) {
					Vector3D r = bf.Position - weightedPosition;
					alpha += (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf)) / sumWeights;
					sourceTerm += r * (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf));
					matrix += r.X * r.X * bf.GetVolume() * kernel->ComputeKernel(b, bf);

				}

				// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
				if (abs(matrix) > Epsilon1D) {
					Vector3D planePitch = Vector3D(sourceTerm.X / matrix, 0.0, 0.0);

					b.Pressure = alpha + (b.Position - weightedPosition) * planePitch;
				}
				else {
					b.Pressure = alpha;
				}
			});
		}
		break;

	case EDimensionality::Two:
		for (UStaticBorder * border : particleContext->GetStaticBorders()) {
			ParallelFor(border->Particles->size(), [&](int32 i) {
				Particle& b = border->Particles->at(i);


				Vector3D sumWeightedPosition = { 0.0, 0.0, 0.0 };
				double sumWeights = 0.0;

				for (const Particle& bf : b.FluidNeighbors) {
					sumWeightedPosition += bf.Position * bf.GetVolume() * kernel->ComputeKernel(b, bf);
					sumWeights += bf.GetVolume() * kernel->ComputeKernel(b, bf);
				}

				if (sumWeights <= DBL_EPSILON) {
					b.Pressure = 0.0;
					return;
				}

				Vector3D weightedPosition = sumWeightedPosition / sumWeights;

				double alpha = 0.0;
				Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
				double xx = 0.0;
				double xz = 0.0;
				double zz = 0.0;

				for (const Particle& bf : b.FluidNeighbors) {
					Vector3D r = bf.Position - weightedPosition;
					alpha += (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf)) / sumWeights;
					sourceTerm += r * (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf));
					xx += r.X * r.X * bf.GetVolume() * kernel->ComputeKernel(b, bf);
					xz += r.X * r.Z * bf.GetVolume() * kernel->ComputeKernel(b, bf);
					zz += r.Z * r.Z * bf.GetVolume() * kernel->ComputeKernel(b, bf);
				}

				double determinant = xx * zz - xz * xz;
				double inverse_xx, inverse_xz, inverse_zz;
				// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
				if (abs(determinant) > Epsilon2D) {

					inverse_xx = zz / determinant;
					inverse_xz = -xz / determinant;
					inverse_zz = xx / determinant;

					Vector3D planePitch = Vector3D(inverse_xx * sourceTerm.X + inverse_xz * sourceTerm.Z, 0.0, inverse_xz * sourceTerm.X + inverse_zz * sourceTerm.Z);

					b.Pressure = std::max(0.0, alpha + (b.Position - weightedPosition) * planePitch);
				}
				else {
					b.Pressure = std::max(0.0, alpha);
				}
			});
		}
		break;

	case EDimensionality::Three:
		for (UStaticBorder * border : particleContext->GetStaticBorders()) {
			ParallelFor(border->Particles->size(), [&](int32 i) {
				Particle& b = border->Particles->at(i);


				Vector3D sumWeightedPositions = { 0.0, 0.0, 0.0 };
				double sumWeighted = 0.0;

				for (const Particle& bf : b.FluidNeighbors) {
					sumWeightedPositions += bf.Position * bf.GetVolume() * kernel->ComputeKernel(b, bf);
					sumWeighted += bf.GetVolume() * kernel->ComputeKernel(b, bf);
				}

				if (sumWeighted <= 0.0) {
					b.Pressure = 0.0;
					return;
				}

				Vector3D weightedPosition = sumWeightedPositions / sumWeighted;

				double alpha = 0.0;
				Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
				Matrix3D pitchMatrix = Matrix3D::Zero;

				for (const Particle& bf : b.FluidNeighbors) {
					Vector3D r = bf.Position - weightedPosition;
					alpha += (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf)) / sumWeighted;
					sourceTerm += r * (bf.Pressure * bf.GetVolume() * kernel->ComputeKernel(b, bf));
					pitchMatrix += Matrix3D::OuterProduct(r, r) * bf.GetVolume() * kernel->ComputeKernel(b, bf);
				}

				// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
				if (std::abs(pitchMatrix.GetDeterminant()) > Epsilon3D) {

					Vector3D planePitch = pitchMatrix.GetInverse() * sourceTerm;

					b.Pressure = std::max(0.0, alpha + (b.Position - weightedPosition) * planePitch);
				}
				else {
					b.Pressure = std::max(0.0, alpha);
				}
			});
		}

		break;
	}
	
}

double UMLSExtrapolation::GetPressureValue(const Particle & staticBoundaryParticle, const Particle & neighboringFluidParticle) const
{
	return staticBoundaryParticle.Pressure;
}

FNeighborsSearchRelations UMLSExtrapolation::GetRequiredNeighborhoods() const
{
	return FNeighborsSearchRelations(true, true,
									 true, false);
}
