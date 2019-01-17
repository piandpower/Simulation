#include "MLSPressureGradient.h"
#include "Simulator.h"

Vector3D UMLSPressureGradient::ComputePressureGradient(Particle& f, int particleIndex) const
{
	switch(Dimensionality) {

	case EDimensionality::One:
	{
		Vector3D sumWeightedPosition = { 0.0, 0.0, 0.0 };
		double sumWeights = 0.0;

		for (const Particle& ff : f.FluidNeighbors) {
			sumWeightedPosition += ff.Position * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
			sumWeights += ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}
		for (const Particle& fb : f.StaticBorderNeighbors) {
			sumWeightedPosition += fb.Position * fb.GetVolume() * GetKernel().ComputeValue(f, fb);
			sumWeights += fb.GetVolume() * GetKernel().ComputeValue(f, fb);
		}

		if (sumWeights <= DBL_EPSILON) {
			return Vector3D(0.0, 0.0, 0.0);
		}

		Vector3D weightedPosition = sumWeightedPosition / sumWeights;

		double alpha = 0.0;
		Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
		double matrix = 0.0;

		for (const Particle& ff : f.FluidNeighbors) {
			Vector3D r = ff.Position - weightedPosition;
			sourceTerm += r * (ff.Pressure * ff.GetVolume() * GetKernel().ComputeValue(f, ff));
			matrix += r.X * r.X * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}
		for (const Particle& fb : f.StaticBorderNeighbors) {
			Vector3D r = fb.Position - weightedPosition;
			sourceTerm += r * (fb.Pressure * fb.GetVolume() * GetKernel().ComputeValue(f, fb));
			matrix += r.X * r.X * fb.GetVolume() * GetKernel().ComputeValue(f, fb);
		}

		// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
		if (abs(matrix) > Epsilon1D) {
			Vector3D planePitch = Vector3D(sourceTerm.X / matrix, 0.0, 0.0);
			return planePitch;
		}
		else {
			return Vector3D(0.0, 0.0, 0.0);
		}
	}
	break;

	case EDimensionality::Two:
	{
		Vector3D sumWeightedPosition = { 0.0, 0.0, 0.0 };
		double sumWeights = 0.0;

		for (const Particle& ff : f.FluidNeighbors) {
			sumWeightedPosition += ff.Position * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
			sumWeights += ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}
		for (const Particle& fb : f.StaticBorderNeighbors) {
			sumWeightedPosition += fb.Position * fb.GetVolume() * GetKernel().ComputeValue(f, fb);
			sumWeights += fb.Mass * f.Density * GetKernel().ComputeValue(f, fb);
		}

		if (sumWeights <= DBL_EPSILON) {
			return Vector3D(0.0, 0.0, 0.0);
		}

		Vector3D weightedPosition = sumWeightedPosition / sumWeights;

		double alpha = 0.0;
		Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
		double xx = 0.0;
		double xz = 0.0;
		double zz = 0.0;

		for (const Particle& ff : f.FluidNeighbors) {
			Vector3D r = ff.Position - weightedPosition;
			alpha += (ff.Pressure * ff.GetVolume() * GetKernel().ComputeValue(f, ff)) / sumWeights;
			sourceTerm += r * (ff.Pressure * ff.GetVolume() * GetKernel().ComputeValue(f, ff));
			xx += r.X * r.X * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
			xz += r.X * r.Z * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
			zz += r.Z * r.Z * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}
		for (const Particle& fb : f.StaticBorderNeighbors) {
			Vector3D r = fb.Position - weightedPosition;
			alpha += (fb.Pressure * fb.GetVolume() * GetKernel().ComputeValue(f, fb)) / sumWeights;
			sourceTerm += r * (fb.Pressure * fb.GetVolume() * GetKernel().ComputeValue(f, fb));
			xx += r.X * r.X * fb.Mass * f.Density * GetKernel().ComputeValue(f, fb);
			xz += r.X * r.Z * fb.Mass * f.Density * GetKernel().ComputeValue(f, fb);
			zz += r.Z * r.Z * fb.Mass * f.Density * GetKernel().ComputeValue(f, fb);
		}

		double determinant = xx * zz - xz * xz;
		double inverse_xx, inverse_xz, inverse_zz;
		// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
		if (abs(determinant) > Epsilon2D) {

			inverse_xx = zz / determinant;
			inverse_xz = -xz / determinant;
			inverse_zz = xx / determinant;

			Vector3D planePitch = Vector3D(inverse_xx * sourceTerm.X + inverse_xz * sourceTerm.Z, 0.0, inverse_xz * sourceTerm.X + inverse_zz * sourceTerm.Z);

			return planePitch;
		}
		else {
			return Vector3D::Zero;
		}
	}
	break;

	case EDimensionality::Three:
	{
		Vector3D sumWeightedPosition = { 0.0, 0.0, 0.0 };
		double sumWeights = 0.0;

		for (const Particle& ff : f.FluidNeighbors) {
			sumWeightedPosition += ff.Position * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
			sumWeights += ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}
		for (const Particle& fb : f.StaticBorderNeighbors) {
			sumWeightedPosition += fb.Position * fb.GetVolume() * GetKernel().ComputeValue(f, fb);
			sumWeights += fb.GetVolume() * GetKernel().ComputeValue(f, fb);
		}

		if (sumWeights <= DBL_EPSILON) {
			return Vector3D::Zero;
		}

		Vector3D weightedPosition = sumWeightedPosition / sumWeights;

		double alpha = 0.0;
		Vector3D sourceTerm = { 0.0, 0.0, 0.0 };
		Matrix3D pitchMatrix = Matrix3D::Zero;

		for (const Particle& ff : f.FluidNeighbors) {
			Vector3D r = ff.Position - weightedPosition;
			alpha += (ff.Pressure * ff.GetVolume() * GetKernel().ComputeValue(f, ff)) / sumWeights;
			sourceTerm += r * (ff.Pressure * ff.GetVolume() * GetKernel().ComputeValue(f, ff));
			pitchMatrix += Matrix3D::OuterProduct(r, r) * ff.GetVolume() * GetKernel().ComputeValue(f, ff);
		}

		for (const Particle& fb : f.StaticBorderNeighbors) {
			Vector3D r = fb.Position - weightedPosition;
			alpha += (fb.Pressure * fb.GetVolume() * GetKernel().ComputeValue(f, fb)) / sumWeights;
			sourceTerm += r * (fb.Pressure * fb.GetVolume() * GetKernel().ComputeValue(f, fb));
			pitchMatrix += Matrix3D::OuterProduct(r, r) * fb.GetVolume() * GetKernel().ComputeValue(f, fb);
		}

		// If the pitch Matrix is not invertible it means that neighboring particles are ordered in one or two dimensions
		if (std::abs(pitchMatrix.GetDeterminant()) > Epsilon3D) {

			Vector3D planePitch = pitchMatrix.GetInverse() * sourceTerm;

			return planePitch;
		}
		else {
			return Vector3D::Zero;
		}
	}
	break;
	}
	return Vector3D::Zero;
}

UMLSPressureGradient * UMLSPressureGradient::CreateMLSPressureGradient(FEpsilons nanoEpsilons)
{
	UMLSPressureGradient * mlsPressureGradient = NewObject<UMLSPressureGradient>();

	mlsPressureGradient->Epsilon1D = static_cast<double>(nanoEpsilons.OneDimension) * 0.000000001;
	mlsPressureGradient->Epsilon2D = static_cast<double>(nanoEpsilons.TwoDimensions) * 0.000000001;
	mlsPressureGradient->Epsilon3D = static_cast<double>(nanoEpsilons.ThreeDimensions) * 0.000000001;

	// prevent garbage collection
	mlsPressureGradient->AddToRoot();
	return mlsPressureGradient;
}
