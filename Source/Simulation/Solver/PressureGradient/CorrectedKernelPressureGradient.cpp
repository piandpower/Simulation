#include "CorrectedKernelPressureGradient.h"
#include "Simulator.h"
#include "CorrectedKernelPressureGradient.h"

void UCorrectedKernelPressureGradient::PrecomputeAllGeometryData(const UParticleContext& particleContext)
{
	InvertedCorrectionMatrices.clear();
	InvertedCorrectionMatrices.reserve(particleContext.GetFluids().size());
	for (UFluid * fluid : particleContext.GetFluids()) {
		InvertedCorrectionMatrices.push_back(std::vector<Matrix3D>());
		InvertedCorrectionMatrices.back().reserve(fluid->Particles->size());

		for (int i = 0; i < fluid->Particles->size(); i++) {
			Particle& f = fluid->Particles->at(i);

			Matrix3D correctionMatrix = Matrix3D::Zero;

			for (Particle& ff : f.FluidNeighbors) {
				correctionMatrix += Matrix3D::OuterProduct(ff.GetVolume() * GetKernel().ComputeGradient(f, ff), ff.Position - f.Position);
			}
			for (Particle& fb : f.StaticBorderNeighbors) {
				correctionMatrix += Matrix3D::OuterProduct(fb.Mass / f.Density * GetKernel().ComputeGradient(f, fb), fb.Position - f.Position);
			}
			
			// Invert correction matrix if possible. If no possile we use the identity matrix
			switch (Dimensionality) {
			case EDimensionality::One:
				if (1 / correctionMatrix.S11 > Epsilon1D) {
					correctionMatrix.Inverse();
				}
				else {
					correctionMatrix = Matrix3D::Identity;
				}
				break;
			case EDimensionality::Two: {
				double determinant = correctionMatrix.S11 * correctionMatrix.S33 - correctionMatrix.S13 * correctionMatrix.S31;
				if (std::abs(determinant) > Epsilon2D) {
					double inverse_xx, inverse_xz, inverse_zz;
					// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions

					inverse_xx = correctionMatrix.S33 / determinant;
					inverse_xz = -correctionMatrix.S13 / determinant;
					inverse_zz = correctionMatrix.S11 / determinant;

					correctionMatrix = Matrix3D(inverse_xx, 0.0, inverse_xz,
												0.0, 0.0, 0.0,
												inverse_xz, 0.0, inverse_zz);
				}
				else {
					correctionMatrix = Matrix3D::Identity;
				}
				break;
			}
			case EDimensionality::Three:
				if (correctionMatrix.GetDeterminant() > Epsilon3D) {
					correctionMatrix.Inverse();
				}
				else {
					correctionMatrix = Matrix3D::Identity;
				}
				break;
			}
			InvertedCorrectionMatrices.back().emplace_back(correctionMatrix);
		}
	}
}

Vector3D UCorrectedKernelPressureGradient::ComputePressureGradient(Particle& f, int particleIndex) const {
	Vector3D pressureGradient = Vector3D::Zero;

	for (const FluidNeighbor& fluidNeighbor : f.FluidNeighbors) {
		const Particle& ff = *fluidNeighbor.GetParticle();
		pressureGradient += ff.GetVolume() * (ff.Pressure / pow(ff.Fluid->GetRestDensity(), 2) * InvertedCorrectionMatrices[*f.Fluid][particleIndex] + f.Pressure / pow(f.Fluid->GetRestDensity(), 2) * InvertedCorrectionMatrices[*ff.Fluid][fluidNeighbor.GetIndex()]) * GetKernel().ComputeGradient(f, ff);
	}

	for (const Particle& fb : f.StaticBorderNeighbors) {
		pressureGradient += fb.Mass / f.Density * (GetBoundaryPressure().GetPressureValue(fb, f) / pow(f.Fluid->GetRestDensity(), 2) + f.Pressure / pow(f.Fluid->GetRestDensity(), 2)) * InvertedCorrectionMatrices[*f.Fluid][particleIndex] * GetKernel().ComputeGradient(f, fb);
	}

	return pressureGradient;
}


UCorrectedKernelPressureGradient * UCorrectedKernelPressureGradient::CreateCorrectedKernelPressureGradient(FEpsilons nanoEpsilons)
{
	UCorrectedKernelPressureGradient * correctedKernelPressureGradient = NewObject<UCorrectedKernelPressureGradient>();

	correctedKernelPressureGradient->Epsilon1D = static_cast<double>(nanoEpsilons.OneDimension) * 0.000000001;
	correctedKernelPressureGradient->Epsilon2D = static_cast<double>(nanoEpsilons.TwoDimensions) * 0.000000001;
	correctedKernelPressureGradient->Epsilon3D = static_cast<double>(nanoEpsilons.ThreeDimensions) * 0.000000001;

	// prevent garbage collection
	correctedKernelPressureGradient->AddToRoot();
	return correctedKernelPressureGradient;
}
