// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "RecordManager.h"
#include "Simulator.h"


URecordManager::~URecordManager()
{
}

URecordManager * URecordManager::CreateRecordManager(TArray<ARecordingCamera*> cameras, EColorVisualisation fluidColorMode, EColorVisualisation staticBorderColorMode, float frameRate, int saveEachNthIteration, bool takeScreenshots)
{
	URecordManager * cameraManager = NewObject<URecordManager>();
	cameraManager->FrameRate = frameRate;
	cameraManager->NextRecordTime = 1 / cameraManager->FrameRate;
	cameraManager->Cameras = cameras;
	cameraManager->SaveEachNthIteration = saveEachNthIteration;
	cameraManager->TakeScreenshots = takeScreenshots;
	cameraManager->FluidColorMode = fluidColorMode;
	cameraManager->StaticBorderColorMode = staticBorderColorMode;

	// prevent garbage collection
	cameraManager->AddToRoot();

	return cameraManager;
}

void URecordManager::Build(UWorld * world, ASimulator * simulator)
{
	Simulator = simulator;

	ParticleVisualizer = world->SpawnActor<AParticleCloudActor>(FVector(0), FRotator(0));	
}

void URecordManager::CamerasCapture(int frame, FString simulationName)
{
	// Create recording folder
	std::experimental::filesystem::path simulationPath = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + "Simulation Recordings/" + simulationName));
	std::experimental::filesystem::create_directory(simulationPath);
	


	for (int i = 0; i < Cameras.Num(); i++) {

		// Create camera folder
		std::experimental::filesystem::path cameraPath = simulationPath.string() + "/Camera " + std::to_string(i);
		std::experimental::filesystem::create_directory(cameraPath);
		
		FString imagePath = UTF8_TO_TCHAR((cameraPath.string() + "/" + std::to_string(frame)).c_str());
		Cameras[i]->CaptureView(true, imagePath);
	}

	TakeScreenshot(frame, simulationName);
}

void URecordManager::WriteSolverStatisticsToFile()
{

		std::ofstream file;
		file.open(TCHAR_TO_UTF8(*(FPaths::ProjectDir() + "Solver Informations/" + GetSimulator()->GetSimulationName() + ".solverinfo")));

		file << "Elapsed Time Simulating\t" << GetSimulator()->GetElapsedTimeWhileSimulating() << std::endl;
		file << "Total Simulated Time\t" << GetSimulator()->GetSimulatedTime() << std::endl;
		file << "Computation Time per Simulated Second\t" << GetSimulator()->GetElapsedTimeWhileSimulating() / GetSimulator()->GetSimulatedTime() << std::endl;
		file << "Timestep Factor\t" << GetSimulator()->GetCFLNumber() << std::endl;
		file << "Max Timestep\t" << GetSimulator()->GetMaxTimestep() << std::endl;

		switch (GetSimulator()->GetSolver()->GetSolverType()) {
		case SESPH:
			file << "Solver\t" << "SESPH" << std::endl;
			break;
		case IISPH:
			file << "Solver\t" << "IISPH" << std::endl;
			break;
		}
		file << std::endl;
		file << "Realtime\tSimulated Time\tComputation Time per Step\tTimestep\tIteration Count\tAverage Density Error\tKineticEnergy" << std::endl;

		double elapsedTime = 0;
		double simulatedTime = 0;
		for (int i = 0; i < GetSimulator()->GetSolver()->OldAverageDensityErrors.size(); i++) {
			elapsedTime += GetSimulator()->GetSolver()->OldComputationTimesPerStep[i];
			simulatedTime += GetSimulator()->GetSolver()->OldTimesteps[i];

			file << elapsedTime << "\t";
			file << simulatedTime << "\t";
			file << GetSimulator()->GetSolver()->OldComputationTimesPerStep[i] << "\t";
			file << GetSimulator()->GetSolver()->OldTimesteps[i] << "\t";
			file << GetSimulator()->GetSolver()->OldIterationCounts[i] << "\t";
			file << GetSimulator()->GetSolver()->OldAverageDensityErrors[i] << "\t";
			file << GetSimulator()->GetSolver()->OldKineticEnergies[i] << "\t";
			file << std::endl;
		}
		file.close();

}

ASimulator * URecordManager::GetSimulator() const
{
	return Simulator;
}

bool URecordManager::GetTakeScreenshots() const
{
	return TakeScreenshots;
}

void URecordManager::SetTakeScreenshots(bool takeScreenshots)
{
	TakeScreenshots = takeScreenshots;
}

bool URecordManager::GetRecordParticles() const
{
	return RecordParticles;
}

void URecordManager::SetRecordParticles(bool recordParticles)
{
	RecordParticles = recordParticles;
}

float URecordManager::GetFrameRate() const
{
	return FrameRate;
}

void URecordManager::SetFrameRate(float frameRate)
{
	if (frameRate <= 0) {
		throw("Invalid recording FrameRate set");
	}
	FrameRate = frameRate;
}

void URecordManager::RewindReplay()
{
	if (GetSimulator()->GetSimulationStatus() != Running) {
		ReplayTime = 0;
		ReplayEnd = false;
		SetRecordedParticlePosition();
	}

}

void URecordManager::ChangeReplaySpeed(float replaySpeed) {
	ReplaySpeed = replaySpeed;
}

void URecordManager::IncreaseReplaySpeed() {
	ReplaySpeed *= 1.3;
}

void URecordManager::DecreaseReplaySpeed() {
	ReplaySpeed /= 1.3;
}

float URecordManager::GetReplaySpeed() const
{
	return ReplaySpeed;
}

bool URecordManager::ReplayIsAtEnd() const
{
	return ReplayEnd;
}

double URecordManager::GetLastRecordedTime() const
{
	return LastRecordedTime;
}

double URecordManager::GetNextRecordTime() const
{
	return NextRecordTime;
}

AParticleCloudActor * URecordManager::GetParticleCloudActor()
{
	return ParticleVisualizer;
}

bool URecordManager::GetSaveSimulationState() const
{
	return SaveSimulationState;
}

void URecordManager::SetSaveSimulationState(bool saveSimulationState)
{
	SaveSimulationState = saveSimulationState;
}

FReplayInformation URecordManager::ReplayInformation() const
{
	return FReplayInformation(ReplaySpeed, ReplayTime, CurrentFrame, FrameRate * ReplaySpeed);
}

void URecordManager::TakeScreenshot(int currentFrame, FString simulationName) {

	std::experimental::filesystem::path path = TCHAR_TO_UTF8(*(FPaths::ProjectDir() + "Simulation Recordings/" + simulationName));

	std::experimental::filesystem::create_directory(path);
	

	FString fileName = FPaths::ProjectDir() + "Simulation Recordings/" + simulationName + "/Viewport/" + FString::FromInt(currentFrame) + ".png";
	FScreenshotRequest::RequestScreenshot(fileName, false, false);
}

bool URecordManager::UpdateTimeAndRecordings(double deltaTime, int iteration, double simulatedTime)
{

	// Should Simulation State be saved to file in this frame?
	if (SaveSimulationState && SaveEachNthIteration != 0 && iteration % SaveEachNthIteration == 0) {
		WriteSimulationStateToFile(iteration);
	}


	// Should Recorder record frame?
	if (NextRecordTime - simulatedTime < 0.00001) {

		RecordedFrames++;
		LastRecordedTime = NextRecordTime;
		NextRecordTime += 1 / GetFrameRate();

		if (RecordParticles) {

			std::vector<std::vector<Vector3D>> positions;
			std::vector<std::vector<Vector3D>> velocities;
			std::vector<std::vector<double>> densities;

			positions.reserve(GetSimulator()->GetParticleContext()->GetFluids().size());
			velocities.reserve(GetSimulator()->GetParticleContext()->GetFluids().size());
			densities.reserve(GetSimulator()->GetParticleContext()->GetFluids().size());


			for (int fluidIndex = 0; fluidIndex < GetSimulator()->GetParticleContext()->GetFluids().size(); fluidIndex++) {
				UFluid* fluid = GetSimulator()->GetParticleContext()->GetFluids()[fluidIndex];
				positions.push_back(std::vector<Vector3D>());
				velocities.push_back(std::vector<Vector3D>());
				densities.push_back(std::vector<double>());

				positions[fluidIndex].reserve(fluid->Particles->size());
				velocities[fluidIndex].reserve(fluid->Particles->size());
				densities[fluidIndex].reserve(fluid->Particles->size());




				for (Particle & particle : *fluid->Particles) {
					positions[fluidIndex].push_back(particle.Position);
					velocities[fluidIndex].push_back(particle.Velocity);
					densities[fluidIndex].push_back(particle.Density);
				}
			}

			OldPositions.push_back(positions);
			OldVelocities.push_back(velocities);
			OldDensities.push_back(densities);
		}
		if (TakeScreenshots) {
			CamerasCapture(RecordedFrames, GetSimulator()->GetSimulationName());
		}

		ReplayEnd = false;
		return true;
	}
	return false;

}

void URecordManager::SetRecordedParticlePosition(double deltaTime)
{
	ReplayTime += deltaTime * ReplaySpeed;

	int frame = (int)(ReplayTime * FrameRate);

	if (frame != CurrentFrame) {
		CurrentFrame = frame;

		// End of replay
		if (OldPositions.size() <= frame) {
			GetSimulator()->PauseSimulation();
			ReplayEnd = true;
			return;
		}
		ParticleVisualizer->VisualisePositions(OldPositions[frame], OldVelocities[frame], GetSimulator()->GetParticleContext()->GetParticleDistance(), FluidColorMode);
	}
}

void URecordManager::WriteSimulationStateToFile(int iteration)
{
	std::experimental::filesystem::path path = TCHAR_TO_UTF8(*(GetSimulator()->GetSimulationPath() + "Simulation Saves/" + GetSimulator()->GetSimulationName()));

	std::experimental::filesystem::create_directory(path);

	// Write fluid
	std::remove((path.string() + "/" + std::to_string(iteration) + ".fluid").c_str());
	for (int fluidIndex = 0; fluidIndex < GetSimulator()->GetParticleContext()->GetFluids().size(); fluidIndex++) {
		GetSimulator()->GetParticleContext()->GetFluids()[fluidIndex]->WriteFluidToFile(path.string() + "/" + std::to_string(iteration) + "." + std::to_string(fluidIndex) + ".fluid", iteration);
	}
}